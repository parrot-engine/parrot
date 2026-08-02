.text

.global _start
_start:
    la $sp, Parrot_stack_top

    la $gp, _gp

    la      $t0, Parrot_bss_start
    la      $t1, Parrot_bss_end
bss_clear:
    beq     $t0, $t1, bss_done
    sw      $zero, 0($t0)
    addiu   $t0, $t0, 4
    j       bss_clear
bss_done:

    jal Parrot_n64_main
    nop

    # Return from main hang
    j .
    nop
