# PS5 kldload

This is a kernel module loader for the PlayStation 5 that aims to provide a deeper understanding of the system's internals and assist in my research into its internals.

The hypervisor remains active and untouched, but the kernel modules run normally and in harmony with it.

## How It Works

On PlayStation 4 systems, developers were able to patch the running kernel to force it to always allocate read-write-executable memory areas in the kmem_alloc function. On the PlayStation 5, things are a bit different, with a few caveats, such as the Hypervisor, that prevent .text read/write access due to `XOM` and the assumption that `GMET` is enabled.

GMET (Guest Mode Execute) is an AMD technology used by its hypervisor to specifically control which pages can execute at the guest level. When it is enabled and properly configured, the guest cannot request executable pages or execute code outside the allowed area. The PlayStation 5 lacks this configuration until firmware `6.50`. kldload works by requesting such pages, writing code into them using the kernel read/write primitives, and executing it as a real kthread, thus allowing real kernel-level code execution on the device.

To call functions and so on, one can use r0gdb or kstuff. However, neither patches the kernel. Instead, they use kernel read/write (RW) primitives to register themselves in the Interrupt Descriptor Table (IDT) to capture general protection faults (int13) and debug traps (int1). With the necessary gadgets, they can also read and write to the debug registers.

Using kstuff to request RWX kernel pages involves setting specific breakpoints in the kernel’s `kmem_alloc` function and then, prior to allocation, changing the default RW permissions to RWX. This is similar to how important patches such as the mprotect patch (which enables `PROT_EXEC` protection in userland) have been applied.

Using the `r0gdb` implementation is different: a tracer is set up when calling kmem_alloc, and it will trace for a specific [condition](https://github.com/buzzer-re/playstation_research_utils/blob/622b09b8433884d06514f8021dfc92ffac863389/ps5_kernel_research/kstuff-no-fpkg/prosper0gdb/r0gdb.c#L1103) that happens prior the page allocation, and then it will change it to `RWX` executable. 

`r0gdb` is the primary supported method of this project, as it is simpler and does not require forcing the system to trigger a `#GP` on every syscall attempt. The only caveat at the moment is that loaders such as `elfldr` may experience undefined behavior; this is currently on the debugging/fixing backlog. Even so, if kstuff is loaded into the system prior to kldload, it will check whether the necessary kekcalls are implemented. If they are, it will use the currently running kstuff module over the r0gdb. 

## Supported Firmware

Currently supported firmware:

- 3.0
- 3.10
- 3.20
- 3.21
- 4.03
- 4.51
- 5.0
- 5.02
- 5.10
- 5.50
- 6.0
- 6.02
- 6.50


# Building and Using

Make sure to have the latest version of the [SDK](https://github.com/ps5-payload-dev/sdk) installed.

Clone the repository recursively:

> git clone --recursive https://github.com/buzzer-re/PS5_kldload.git

Then, you can build and run the tool with:

> make clean && make && make test

It will listen on port 9022 and launch any kernel payloads in kernel mode.


## Example

Kernel payloads can be written similarly to how they were written for the PlayStation 4. In the future, an SDK might be developed to assist with this. Below is a simple [hello_world](https://github.com/buzzer-re/PS5_kldload/tree/main/examples/hello_world) example that reads the `MSR_LSTAR` value. Full source code can be found here.

```c
#include "../include/firmware/offsets.h"

#define MSR_LSTAR 0xC0000082
typedef unsigned long uint64_t;
typedef unsigned int uint32_t;

inline uint64_t __readmsr(uint32_t msr) // wrapper for the rdmsr instruction
{
    uint32_t low, high;
    __asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return (low | ((uint64_t) high << 32));
}

typedef struct __kproc_args
{
    uint64_t kdata_base;
} kproc_args;

void (*kprintf)(char* fmt, ...);
uint64_t kdata_address;

void init_kernel()
{
    kprintf = (void (*)(char*, ...)) (kdata_address + kprintf_offset);
}

int module_start(kproc_args* args)
{
    kdata_address = args->kdata_base;
    init_kernel();

    kprintf("Hello from kernel land! Let's check the MSR_LSTAR value!\n");
    kprintf("MSR_LSTAR => %#02lx\n", __readmsr(MSR_LSTAR));

    return 1;
}
```

Send it with `socat -t 99999999 - TCP:PS5_IP:9022 < hello_world.bin` then you can check the `klogs`:

![](screenshots/example.png)


## Future work

Here's a few things that I'm currently working and need assistance for anyone willing to help with:


- SDK and ELF loader for KLD
    - Similar on how FreeBSD already does, it's good to have a well defined format and loader similar to the current ones
- Offsets acquiring and function definitions
    - There is a lot of offsets already defined on the [offsets.c](https://github.com/buzzer-re/playstation_research_utils/blob/dc5a29fa289321cc983e8560a054f6e5207ec1af/ps5_kernel_research/kstuff-no-fpkg/prosper0gdb/offsets.c), some of them need a function definition to be used 
