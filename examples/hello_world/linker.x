OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(module_start)

PHDRS
{
        code_seg PT_LOAD;
        rdata_seg PT_LOAD;
        data_seg PT_LOAD;
        bss_seg PT_LOAD;
}

SECTIONS
{
        .text : {
                *(.text.module_start)
                *(.text*)
        } : code_seg
        .rodata : {
                *(.rodata)
                *(.rodata*)
        } : rdata_seg
        .data : { *(.data) } : data_seg
        .bss  : { *(.bss) } : bss_seg
        /DISCARD/ : {
                *(.comment)
                *(.note.GNU-stack)
                *(.eh_frame)
                *(.interp)
        }
}