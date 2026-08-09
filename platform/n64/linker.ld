ENTRY(_start)

SECTIONS {
    . = 0x80000400;

    .text : {
        . = ALIGN(8);
        *(.text)
    }

    .data : {
        . = ALIGN(8);
        *(.data)
    }

    .rodata : {
        . = ALIGN(8);
        *(.rodata)
    }

    Parrot_bss_start = .;
    .bss : {
        . = ALIGN(16);
        *(.bss)
    }
    Parrot_bss_end = .;

    _gp = . + 0x7FF0;

    .sdata : {
        . = ALIGN(8);
        *(.sdata*)
        . = ALIGN(8);
        *(.lit8)
        . = ALIGN(8);
        *(.lit4)
    }

    .sbss : {
        . = ALIGN(8);
        *(.sbss*)
        . = ALIGN(8);
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