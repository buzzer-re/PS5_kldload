#include "../include/proc.h"
#include "../include/server.h"
#include "../include/notify.h"
#include "../include/kstuff_loader.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <elf.h>
#include <signal.h>

#define DEBUG 1
#define PORT 9022
#define THREAD_NAME "kldload.elf"
#define KTHREAD_NAME "my_kthread\x00"
#define PAGE_SIZE 0x4000
#define ROUND_PG(x) (((x) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

typedef struct __kproc_args
{
    uint64_t kdata_base;
    uint32_t fw_ver;
} kproc_args;


extern uint64_t kmem_alloc(size_t size);
extern int kproc_create(uint64_t addr, uint64_t args, uint64_t kproc_name);
extern int kstuff_check();


r0gdb_functions r0gdb;

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
    //
    // perform a kekcall to alloc the executable code area
    //
    
    uint64_t exec_code = r0gdb.r0gdb_kmem_alloc(data_size);
    uint64_t kproc_name = r0gdb.r0gdb_kmem_alloc(0x100); // leave the default prot
    uint64_t kthread_args = r0gdb.r0gdb_kmem_alloc(sizeof(kproc_args));
    #ifdef DEBUG

    printf("code size: %ld bytes\nExec code address: %#02lx\nkproc_name addr: %#02lx\n kthread_args: %#02lx\n", 
        data_size, 
        exec_code,
        kproc_name,
        kthread_args);
    #endif

    payload_args_t* payload_args = payload_get_args();
    kproc_args args;
    args.kdata_base = payload_args->kdata_base_addr;
    args.fw_ver = get_fw_version();

    //
    // Kernel write
    // 
    kernel_copyin(data, exec_code, data_size);
    kernel_copyin(KTHREAD_NAME, kproc_name, sizeof(KTHREAD_NAME));
    kernel_copyin(&args, kthread_args, sizeof(args));

    printf("Lauching kthread at %#02lx...\n", exec_code);

    r0gdb.r0gdb_kproc_create(exec_code, kthread_args, kproc_name);
}


int main(int argc, char const *argv[])
{
    puts("Starting kldload...");
    struct proc* existing_instance = find_proc_by_name(THREAD_NAME);
    payload_args_t* args = payload_get_args();

    if (existing_instance)
    {
        if (kill(existing_instance->pid, SIGKILL))
        {
            printf("Unable to kill %d\n", existing_instance->pid);
            return 1;
        }
    }

    load_r0gdb(&r0gdb);

    if (r0gdb.r0gdb_init_ptr(args->sys_dynlib_dlsym, (int) args->rwpair[0], (int) args->rwpair[1], 0, args->kdata_base_addr))
    {
        notify_send("Failed to start r0gdb, aborting kldload loading...");
        return 1;
    }

    if (start_server(PORT, _kldload) <= 0)
    {
        notify_send("Unable to initialize kldload server on port %d! Aborting...", PORT);
        return 1;
    }

    // while (1);
    return 0;
}