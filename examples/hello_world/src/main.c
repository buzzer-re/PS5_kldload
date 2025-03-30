#include "../include/firmware/offsets.h"
// #include "../include/firmware/intrin.h"

#define MSR_LSTAR 0xC0000082
typedef unsigned long  uint64_t;
typedef unsigned int uint32_t;


inline uint64_t __readmsr(uint32_t msr) // wrapper into the rdmsr instruction
{
    uint32_t low, high;
    __asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));

    return (low | ((uint64_t) high << 32));
}

typedef struct __kproc_args
{
    uint64_t kdata_base;
} kproc_args;


void(*kprintf)(char* fmt, ...);
uint64_t kdata_address;


void init_kernel()
{
    kprintf =  (void(*)(char*, ...)) kdata_address + kprintf_offset;
}

int module_start(kproc_args* args)
{
    kdata_address = args->kdata_base;

    init_kernel();

    kprintf("Hello from kernel land, let's check the MSR_LSTAR value!\n");

    kprintf("MSR_LSTAR => %#02lx\n", __readmsr(MSR_LSTAR));
    // kprintf("CR0: %#02lx\n",  __readcr0());
    // char* kprintf_addr = (char*) kprintf;

    // kprintf("Let's read the kprintf: %x\n", kprintf_addr[0]); // crash the HV

    return 1;
}