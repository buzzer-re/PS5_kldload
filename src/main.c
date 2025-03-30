#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <elf.h>
#include <signal.h>

#include "../include/proc.h"
#include "../include/server.h"
#include "../include/notify.h"

#define DEBUG 1
#define PORT 9022
#define THREAD_NAME "kldload.elf"
#define KTHREAD_NAME "my_kthread\x00"
#define PAGE_SIZE 0x4000
#define ROUND_PG(x) (((x) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

typedef struct __kproc_args
{
    uint64_t kdata_base;
} kproc_args;


extern uint64_t kmem_alloc(size_t size);
extern int kproc_create(uint64_t addr, uint64_t args, uint64_t kproc_name);

void _kldload(int fd, void* data, ssize_t data_size)
{
    //
    // perform a kekcall to alloc the executable code area
    //
    
    uint64_t exec_code = kmem_alloc(data_size);
    uint64_t kproc_name = kmem_alloc(0x100); // leave the default prot
    uint64_t kthread_args = kmem_alloc(sizeof(kproc_args));

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
    //
    // Kernel write
    // 
    
    puts("Copying...");

    kernel_copyin(data, exec_code, data_size);
    kernel_copyin(KTHREAD_NAME, kproc_name, sizeof(KTHREAD_NAME));
    kernel_copyin(&args, kthread_args, sizeof(args));


    puts("Lauching kproc!");
    
    kproc_create(exec_code, kthread_args, kproc_name);
}


int main(int argc, char const *argv[])
{
    struct proc* existing_instance = find_proc_by_name(THREAD_NAME);
    
    if (existing_instance)
    {
        if (kill(existing_instance->pid, SIGKILL))
        {
            printf("Unable to kill %d\n", existing_instance->pid);
            return 1;
        }
    }

    syscall(SYS_thr_set_name, -1, THREAD_NAME);
    notify_send("Starting kldload on %d...", PORT);

    if (start_server(PORT, _kldload) <= 0)
    {
        notify_send("Unable to initialize kldload server on port %d! Aborting...", PORT);
        return 1;
    }

    return 0;
}