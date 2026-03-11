#include "../include/page.h"

#include <ps5/kernel.h>

#include <string.h>


int page_read_kernel_pmap(uint64_t kdata_base, int64_t off_kernel_pmap_store,
    uint64_t* dmap_base, uint64_t* kernel_cr3, uint64_t* kernel_pml4)
{
    flat_pmap pmap;
    uint64_t pmap_addr = kdata_base + off_kernel_pmap_store;

    kernel_copyout(pmap_addr, &pmap, sizeof(pmap));

    if (pmap.mtx_flags != 0x1430000 || pmap.pm_pml4 == 0) 
    {
        // scan nearby for pmap signature
        for (int delta = -0x100; delta <= 0x100; delta += 8) 
        {
            flat_pmap test;
            kernel_copyout(pmap_addr + delta, &test, sizeof(test));
            if (test.mtx_flags == 0x1430000 && test.mtx_lock == 0x4 && test.pm_pml4 != 0) 
            {
                memcpy(&pmap, &test, sizeof(pmap));
                break;
            }
        }
        if (pmap.pm_pml4 == 0)
            return -1;
    }

    *kernel_pml4 = pmap.pm_pml4;
    *kernel_cr3 = pmap.pm_cr3;
    *dmap_base = pmap.pm_pml4 - pmap.pm_cr3;
    return 0;
}

uint64_t page_vtophys(uint64_t dmap_base, uint64_t cr3, uint64_t va)
{
    uint64_t table_phys = cr3;

    for (int level = 0; level < 4; level++) 
    {
        int shift = 39 - (level * 9);
        uint64_t idx = (va >> shift) & 0x1FF;
        uint64_t entry;
        kernel_copyout(dmap_base + (table_phys & KPTE_ADDR_MASK) + idx * 8,
            &entry, sizeof(entry));

        if (!(entry & KPTE_PRESENT))
            return 0;

        if ((level == 1 || level == 2) && (entry & KPTE_PS)) 
        {
            uint64_t page_size = (level == 1) ? (1ULL << 30) : (1ULL << 21);
            return (entry & KPTE_ADDR_MASK) | (va & (page_size - 1));
        }

        if (level == 3)
            return (entry & KPTE_ADDR_MASK) | (va & 0xFFF);

        table_phys = entry & KPTE_ADDR_MASK;
    }
    return 0;
}

static int page_clear_nx(uint64_t dmap_base, uint64_t kernel_cr3, uint64_t va)
{
    uint64_t table_phys = kernel_cr3;

    for (int level = 0; level < 4; level++) 
    {
        int shift = 39 - (level * 9);
        uint64_t idx = (va >> shift) & 0x1FF;
        uint64_t pte_addr = dmap_base + (table_phys & KPTE_ADDR_MASK) + idx * 8;

        uint64_t entry;
        kernel_copyout(pte_addr, &entry, sizeof(entry));

        if (!(entry & KPTE_PRESENT))
            return -1;

        // large page (1GB at level 1, 2MB at level 2)
        if ((level == 1 || level == 2) && (entry & KPTE_PS)) 
        {
            if (entry & KPTE_NX) 
            {
                entry &= ~KPTE_NX;
                kernel_copyin(&entry, pte_addr, sizeof(entry));
            }
            return 0;
        }

        if (level == 3) {
            if (entry & KPTE_NX) 
            {
                entry &= ~KPTE_NX;
                kernel_copyin(&entry, pte_addr, sizeof(entry));
            }
            return 0;
        }

        table_phys = entry & KPTE_ADDR_MASK;
    }
    return -1;
}


int page_mark_exec(uint64_t dmap_base, uint64_t kernel_cr3,
    uint64_t kva, size_t size)
{
    uint64_t start = kva & ~0xFFFULL;
    uint64_t end = (kva + size + 0xFFF) & ~0xFFFULL;

    for (uint64_t page = start; page < end; page += 0x4000) 
    {
        if (page_clear_nx(dmap_base, kernel_cr3, page))
            return -1;
    }
    return 0;
}
