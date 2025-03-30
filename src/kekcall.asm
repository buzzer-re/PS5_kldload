.intel_syntax noprefix
.global kmem_alloc
.global kekcall
.global kproc_create
.global kstuff_check

kekcall:
    mov rax, 0x100000027
    syscall
    ret

kmem_alloc:
    mov rax, 0x600000027
    syscall
    push rbx
    mov rbx, 0xffffff8000000000
    or rax, rbx
    pop rbx
    ret

kproc_create:
    mov rax, 0x700000027
    syscall
    ret


kstuff_check:
    mov rax, 0xffffffff00000027
    syscall
    ret