ENTRY(_start)

SECTIONS {
    . = 0x80000400;

    .text : {
        *(.text)
    }

    .data : {
        *(.data)
    }

    .rodata : {
        *(.rodata)
    }

    Parrot_bss_start = .;
    .bss : {
        *(.bss)
    }
    Parrot_bss_end = .;

    . = ALIGN(16);
    _gp = . + 0x7FF0;

    .sdata : {
        *(.sdata*)
        *(.lit8)
        *(.lit4)
    }

    .sbss : {
        *(.sbss*)
        *(.scommon)
    }

    .stack (NOLOAD) : {
        . = ALIGN(8);
        Parrot_stack_bottom = .;
        . += 8K;
        Parrot_stack_top = .;
    }

    . += 8;
    . = ALIGN(256);

    Parrot_heap_start_addr = .;
}