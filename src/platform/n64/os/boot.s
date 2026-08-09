.text

.global _start
_start:
    la $sp, Parrot_stack_top

    la $gp, _gp

    jal Parrot_os
    nop
