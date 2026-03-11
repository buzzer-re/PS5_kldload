#pragma once

#include <stdint.h>
#include <sys/types.h>
#include "idt.h"

//
// kprim — Kernel Primitives
//
// Provides kernel function calls using only kernel R/W
// No r0gdb, no kstuff.
//

typedef struct __kprim_fw_entry
{
    uint32_t fw;
    int64_t idt;
    int64_t tss_array;
    int64_t kernel_pmap_store;
    int64_t doreti_iret;
    int64_t kmem_alloc;
    int64_t kproc_create;
    int64_t pop_all_iret;
} kprim_fw_entry;

static const kprim_fw_entry fw_table[] = {
    //
    // 3.xx
    //
    { .fw = 0x310,
      .idt = 0x642dc80,
      .tss_array = 0x6430820,
      .kernel_pmap_store = 0x31be218,
      .doreti_iret = -0x9aefbc,
      .kmem_alloc = -0xC2AA0,
      .kproc_create = -0x3568F0,
      .pop_all_iret = -0x9af01b,
    },
    { .fw = 0x320,
      .idt = 0x642dc80,
      .tss_array = 0x6430820,
      .kernel_pmap_store = 0x31be218,
      .doreti_iret = -0x9aec7c,
      .kmem_alloc = -0xC25E0,
      .kproc_create = -0x3565A0,
      .pop_all_iret = -0x9aecdb,
    },
    { .fw = 0x321,
      .idt = 0x642dc80,
      .tss_array = 0x6430820,
      .kernel_pmap_store = 0x31be218,
      .doreti_iret = -0x9aec7c,
      .kmem_alloc = -0xC25E0,
      .kproc_create = -0x3565A0,
      .pop_all_iret = -0x9aecdb,
    },

    //
    // 4.xx
    //
    { .fw = 0x403,
      .idt = 0x64cdc80,
      .tss_array = 0x64d0830,
      .kernel_pmap_store = 0x3257a78,
      .doreti_iret = -0x9cf84c,
      .kmem_alloc = -0xc1ed0,
      .kproc_create = -0x35ebf0,
      .pop_all_iret = -0x9cf8ab,
    },
    { .fw = 0x451,
      .idt = 0x64cdc80,
      .tss_array = 0x64d0830,
      .kernel_pmap_store = 0x3257a78,
      .doreti_iret = -0x9cf84c,
      .kmem_alloc = -0xC18E0,
      .kproc_create = -0x35E700,
      .pop_all_iret = -0x9cf8ab,
    },

    //
    // 5.xx
    //
    { .fw = 0x500,
      .idt = 0x660dca0,
      .tss_array = 0x6610850,
      .kernel_pmap_store = 0x3398a88,
      .doreti_iret = -0xa04f93,
      .kmem_alloc = -0xCD1E0,
      .kproc_create = -0x372460,
      .pop_all_iret = -0xa04ff2,
    },
    { .fw = 0x502,
      .idt = 0x660dca0,
      .tss_array = 0x6610850,
      .kernel_pmap_store = 0x3398a88,
      .doreti_iret = -0xa04f93,
      .kmem_alloc = -0xCD1E0,
      .kproc_create = -0x372460,
      .pop_all_iret = -0xa04ff2,
    },
    { .fw = 0x510,
      .idt = 0x660dca0,
      .tss_array = 0x6610850,
      .kernel_pmap_store = 0x3398a88,
      .doreti_iret = -0xa04f93,
      .kmem_alloc = -0xCCEB0,
      .kproc_create = -0x372290,
      .pop_all_iret = -0xa04ff2,
    },
    { .fw = 0x550,
      .idt = 0x660dca0,
      .tss_array = 0x6610850,
      .kernel_pmap_store = 0x3394a88,
      .doreti_iret = -0xa04fd3,
      .kmem_alloc = -0xCC0C0,
      .kproc_create = -0x3714A0,
      .pop_all_iret = -0xa05032,
    },

    //
    // 6.xx
    //
    { .fw = 0x600,
      .idt = 0x655dde0,
      .tss_array = 0x6560a00,
      .kernel_pmap_store = 0x32e4358,
      .doreti_iret = -0xa1b813,
      .kmem_alloc = -0xC2B50,
      .kproc_create = -0x372DB0,
      .pop_all_iret = -0xa1b872,
    },
    { .fw = 0x602,
      .idt = 0x655dde0,
      .tss_array = 0x6560a00,
      .kernel_pmap_store = 0x32e4358,
      .doreti_iret = -0xa1b813,
      .kmem_alloc = -0xC2B70,
      .kproc_create = -0x372DD0,
      .pop_all_iret = -0xa1b872,
    },
    { .fw = 0x650,
      .idt = 0x655dde0,
      .tss_array = 0x6560a00,
      .kernel_pmap_store = 0x32e4358,
      .doreti_iret = -0xa1b813,
      .kmem_alloc = -0xC2440,
      .kproc_create = -0x372B40,
      .pop_all_iret = -0xa1b872,
    },

    //
    // 7.xx
    //
    { .fw = 0x700,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AB0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0BA33,
      .kmem_alloc = -0xCF5F0,
      .kproc_create = -0x37B900,
      .pop_all_iret = -0xA0BA92,
    },
    { .fw = 0x701,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AB0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0BA33,
      .kmem_alloc = -0xCF5F0,
      .kproc_create = -0x37B900,
      .pop_all_iret = -0xA0BA92,
    },
    { .fw = 0x720,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AD0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0B7F3,
      .kmem_alloc = -0xCE5F0,
      .kproc_create = -0x37B600,
      .pop_all_iret = -0xA0B852,
    },
    { .fw = 0x740,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AD0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0B7F3,
      .kmem_alloc = -0xCE5F0,
      .kproc_create = -0x37B600,
      .pop_all_iret = -0xA0B852,
    },
    { .fw = 0x760,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AD0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0B7F3,
      .kmem_alloc = -0xCDFE0,
      .kproc_create = -0x37B4C0,
      .pop_all_iret = -0xA0B852,
    },
    { .fw = 0x761,
      .idt = 0x2E7FDF0,
      .tss_array = 0x2E82AD0,
      .kernel_pmap_store = 0x2E2C848,
      .doreti_iret = -0xA0B7F3,
      .kmem_alloc = -0xCDFE0,
      .kproc_create = -0x37B4C0,
      .pop_all_iret = -0xA0B852,
    },

    { 0 }  // sentinel
};

// IST2 instead of IST3 to avoid conflict with kstuff (uses IST3/IST4/IST7)
#define TSS_IST_OFFSET              0x2C
#define TSS_PER_CPU_SIZE            0x68
#define TSS_NUM_CPUS                16

#define KPRIM_IDT_VECTOR            0x48

#define PAGE_INTSTUB_OFF            0x300
#define PAGE_IST_TOP_OFF            0x800
#define PAGE_ENTRY_IRET_OFF         0x8C0
#define PAGE_TRAPFRAME_OFF          0xA00
#define PAGE_RETCHAIN_OFF           0xB20

typedef struct __kprim_ctx
{
    uint64_t dmap_base;
    uint64_t kernel_cr3;
    uint64_t kernel_pml4;
    uint64_t kdata_base;
    uint64_t page_dmap;
    uint64_t page_phys;
    uint64_t idt_base;
    uint64_t idt_entry_addr;
    idt_64   orig_gate;
    idt_64   new_gate;
    int      vector;
    uint64_t tss_base;
    uint64_t orig_ist[TSS_NUM_CPUS];
    void*    user_page;
    int      initialized;
    uint16_t kernel_cs;
    uint64_t gadget_swapgs_add_rsp_iret;
    uint64_t gadget_pop_all_iret;
    uint8_t  tf_skip;  // addq imm from POP_FRAME (bytes between regs and IRET frame)
    const kprim_fw_entry* fw;
} kprim_ctx;


int kprim_init(kprim_ctx* ctx, uint32_t fw_version);
uint64_t kprim_kcall(kprim_ctx* ctx, uint64_t target, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6);

uint64_t kprim_kmem_alloc(kprim_ctx* ctx, size_t size);
int kprim_kproc_create(kprim_ctx* ctx, uint64_t func, uint64_t arg, uint64_t name);
void kprim_cleanup(kprim_ctx* ctx);
