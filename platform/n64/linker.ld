ENTRY(_start)

SECTIONS {
	. = 0x00001000;

	. = ALIGN(4096);
    .text : {
        *(.text)
    }

	. = ALIGN(4096);
    .data : {
        *(.data)
    }

	. = ALIGN(4096);
    .rodata : {
        *(.rodata)
    }

	. = ALIGN(4096);
    .bss : {
        *(.bss)
    }

    _gp = . + 0x7FF0;

	. = ALIGN(4096);
    .sdata : {
        *(.sdata*)
        . = ALIGN(8);
        *(.lit8)
        . = ALIGN(8);
        *(.lit4)
    }

	. = ALIGN(4096);
    .sbss : {
        *(.sbss*)
        . = ALIGN(8);
        *(.scommon)
    }

    . = 0x80000400;

	. = ALIGN(4096);
	.os : {
		*(.os.boot)

        . = ALIGN(8);
		*(.os.text)
        . = ALIGN(8);
		*(.os.data)
        . = ALIGN(8);
		*(.os.rodata)
        . = ALIGN(8);
		*(.os.bss)
	}

	. = ALIGN(4096);
    .stack : {
        . += 4K;
        _stack_top = .;
    }

	. = ALIGN(4096);
	Parrot_n64_os_free_mem_begin = .;
}
