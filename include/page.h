#pragma once

#include <stdint.h>
#include <sys/types.h>

// PTE flags
#define KPTE_PRESENT    (1ULL << 0)
#define KPTE_RW         (1ULL << 1)
#define KPTE_PS         (1ULL << 7)
#define KPTE_NX         (1ULL << 63)
#define KPTE_ADDR_MASK  0x000FFFFFFFFFF000ULL

// vmspace -> pmap offset (FreeBSD)
#define VMSPACE_PMAP_OFFSET     0x2E0

typedef struct __flat_pmap
{
    uint64_t mtx_name_ptr;
    uint64_t mtx_flags;
    uint64_t mtx_data;
    uint64_t mtx_lock;
    uint64_t pm_pml4;
    uint64_t pm_cr3;
} flat_pmap;

uint64_t page_vtophys(uint64_t dmap_base, uint64_t cr3, uint64_t va);
int page_mark_exec(uint64_t dmap_base, uint64_t kernel_cr3, uint64_t kva, size_t size);
