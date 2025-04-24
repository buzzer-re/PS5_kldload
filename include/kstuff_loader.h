#pragma once

typedef struct __r0gdb_functions
{
    int (*r0gdb_init_ptr)(void* ds, int a, int b, uintptr_t c, uintptr_t d);
    uint64_t (*r0gdb_kmalloc)(size_t sz);
    uint64_t (*r0gdb_kmem_alloc)(size_t sz);
    uint64_t (*r0gdb_kfncall)(uint64_t fn, ...);
    uint64_t (*r0gdb_kproc_create)(uint64_t kfn, uint64_t kthread_args, uint64_t kproc_name);
} __attribute__((__packed__)) r0gdb_functions;


int load_r0gdb(r0gdb_functions* r0_func);