#include "../include/proc.h"
#include "../include/server.h"
#include "../include/notify.h"
#include "../include/kprim.h"
#include "../include/page.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <signal.h>

#define DEBUG 0
#define PORT 9022
#define KTHREAD_NAME "my_kthread\x00"
#define PAGE_SIZE 0x4000
#define ROUND_PG(x) (((x) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

typedef struct __kproc_args
{
    uint64_t kdata_base;
    uint32_t fw_ver;
} kproc_args;

kprim_ctx g_kp;
int fw_version;

uint32_t get_fw_version()
{
    int mib[2] = {1, 46};
    unsigned long size = sizeof(mib);
    unsigned int version = 0;
    sysctl(mib, 2, &version, &size, 0, 0);
    return version >> 16;
}

void _kldload(int fd, void* data, ssize_t data_size)
{
    printf("kldload: received %zd bytes\n", data_size);

    //
    // Allocate kernel pages for code, name, and args
    //
    uint64_t code_size = ROUND_PG(data_size);
    uint64_t exec_code = kprim_kmem_alloc(&g_kp, code_size);
    uint64_t kproc_name = kprim_kmem_alloc(&g_kp, ROUND_PG(0x100));
    uint64_t kthread_args = kprim_kmem_alloc(&g_kp, ROUND_PG(sizeof(kproc_args)));

    if (!exec_code || !kproc_name || !kthread_args) {
        puts("kldload: kmem_alloc failed");
        return;
    }

// #ifdef DEBUG
    printf("kldload: code=%#lx (%zd bytes) name=%#lx args=%#lx\n",
        exec_code, data_size, kproc_name, kthread_args);
// #endif

    //
    // Mark code pages executable (clear NX in kernel PTEs)
    //
    if (page_mark_exec(g_kp.dmap_base, g_kp.kernel_cr3, exec_code, code_size)) {
        puts("kldload: mark_exec failed");
        return;
    }

    //
    // Prepare kproc args (passed to kmod entry)
    //
    payload_args_t* payload_args = payload_get_args();
    kproc_args args;
    args.kdata_base = payload_args->kdata_base_addr;
    args.fw_ver = fw_version;

    //
    // Write code, name, and args into kernel memory
    //
    puts("kldload: writing payload to kernel...");
    kernel_copyin(data, exec_code, data_size);
    kernel_copyin(KTHREAD_NAME, kproc_name, sizeof(KTHREAD_NAME));
    kernel_copyin(&args, kthread_args, sizeof(args));

#ifdef DEBUG
    // Readback verification
    uint8_t dump[16] = {0};
    kernel_copyout(exec_code, dump, sizeof(data));
    printf("kldload: first 16 bytes: ");
    for (int i = 0; i < 16; i++)
        printf("%02x ", dump[i]);
    puts("");
#endif

    //
    // Launch kernel thread
    //
    printf("kldload: launching kthread at %#lx...\n", exec_code);

    int ret = kprim_kproc_create(&g_kp, exec_code, kthread_args, kproc_name);
    printf("kldload: kproc_create returned %d\n", ret);

    if (!ret) 
    {
        puts("kldload: module launched successfully");
        notify_send("kldload: module loaded (%zd bytes)", data_size);
    } 
    else 
    {
        puts("kldload: kproc_create FAILED");
        notify_send("kldload: FAILED to load module");
    }
}


// int main_test(int argc, char const *argv[])
// {
//     fw_version = get_fw_version();
//     if (kprim_init(&g_kp, fw_version)) return 1;
//     uint8_t ret_code[] = { 0xC3 };
//     uint64_t code_size = ROUND_PG(sizeof(ret_code));
//     uint64_t exec_code = kprim_kmem_alloc(&g_kp, code_size);
//     if (!exec_code) return 1;
//     if (page_mark_exec(g_kp.dmap_base, g_kp.kernel_cr3, exec_code, code_size)) return 1;
//     kernel_copyin(ret_code, exec_code, sizeof(ret_code));
//     kprim_kcall(&g_kp, exec_code, 0, 0, 0, 0, 0, 0);
//     notify_send("works\n");
//     kprim_cleanup(&g_kp);
//     return 0;
// }

int main(int argc, char const *argv[])
{
    fw_version = get_fw_version();
    printf("Running on firmware %#x\n", fw_version);

    // kill previous instance if running
    struct proc* existing = find_proc_by_name("kldload.elf");
    if (existing && existing->pid != getpid())
    {
        printf("kldload: killing previous instance (pid %d)\n", existing->pid);
        kill(existing->pid, SIGKILL);
        free(existing);
    }

    // init kernel primitives
    if (kprim_init(&g_kp, fw_version))
    {
        puts("kldload: kprim_init failed");
        return 1;
    }
    
    printf("kldload ready\nStarting server on %d...\n", PORT);
    notify_send("kldload ready at %d\n", PORT);
    start_server(PORT, _kldload);
    kprim_cleanup(&g_kp);
    return 0;
}