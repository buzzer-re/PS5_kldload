#pragma once

typedef unsigned long  uint64_t;
typedef unsigned int uint32_t;
#define X86_CR0_WP (1 << 16)
// MSRs


//
// This intrinsics code make use of the GAS syntax, which the whole FreeBSD uses, it will make our compilation less error-prone (trust me)
//

static inline __attribute__((always_inline)) uint64_t __readmsr(unsigned long __register) {
    unsigned long __edx;
    unsigned long __eax;
    __asm__ volatile("rdmsr"
            : "=d"(__edx), "=a"(__eax)
            : "c"(__register));
    return (((uint64_t)__edx) << 32) | (uint64_t)__eax;
}

static inline __attribute__((always_inline)) uint64_t __readcr0(void) {
    uint64_t cr0;
    __asm__ volatile("movq %%cr0, %0"
                    : "=r"(cr0)
                    :
                    : "memory");
  return cr0;
}

static inline __attribute__((always_inline)) uint64_t __readdr6(void) {
    uint64_t cr0;
    __asm__ volatile("movq %%dr6, %0"
                    : "=r"(cr0)
                    :
                    : "memory");
  return cr0;
}

static inline __attribute__((always_inline)) uint64_t __writedr6(uint64_t dr6) {
    uint64_t cr0;
    __asm__ volatile("movq %0, %%dr6"
                    : "=r"(cr0)
                    :
                    : "memory");
  return cr0;
}


static inline __attribute__((always_inline)) void __writecr0(uint64_t cr0) {
    __asm__ volatile("movq %0, %%cr0"
                    :
                    : "r"(cr0)
                    : "memory");
}

static inline __attribute__((always_inline)) void __sidt(uint64_t* idtr) {
    __asm__ volatile("sidt %0" :: "m"(*idtr));
}



static inline __attribute__((always_inline)) uint64_t read_rflags(void)
{
    uint64_t	rf;

    __asm__ volatile("pushfq; popq %0" : "=r" (rf));
    return (rf);
}

static inline __attribute__((always_inline)) void write_rflags(uint64_t rf)
{
    __asm__ volatile("pushq %0;  popfq" : : "r" (rf));
}

static inline __attribute__((always_inline)) void disable_intr(void)
{
    __asm__ volatile("cli" : : : "memory");
}

static inline __attribute__((always_inline)) uint64_t intr_disable(void)
{
    uint64_t rflags;

    rflags = read_rflags();
    disable_intr();
    return (rflags);
}

static inline __attribute__((always_inline)) void intr_restore(uint64_t rflags)
{
    write_rflags(rflags);
}