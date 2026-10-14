MAX_RECURSIVE_INTERRUPTS = 8
INTERRUPT_STACK_SIZE = 0x400

.section .os.boot
.global _start
_start:
	la $sp, _stack_top

	mfc0 $t0, $12

	li $t1, 0xFFFFFFFD
	and $t0, $t0, $t1
	li $t1, 0xFFFFFFFB
	and $t0, $t0, $t1

	ori $t0, $t0, 0x1

	lui $t1, 0x0000
	ori $t1, $t1, 0xFF00
	or $t0, $t0, $t1

	mtc0 $t0, $12

	jal Parrot_n64_os_main
	nop

	j .
	nop

.global _Parrot_n64_os_main_return
_Parrot_n64_os_main_return:
	move $a0, $v0
	j exit
	nop

.global _ParrotOSThreadState_new_impl_return
_ParrotOSThreadState_new_impl_return:
	li $v0, 2
	move $v1, $s0
	syscall

.global _ParrotOSThreadState_restore_impl_swap
# void _ParrotOSThreadState_restore_impl_swap(ParrotN64OSInterruptFrame *load, ParrotN64OSInterruptFrame *save)
_ParrotOSThreadState_restore_impl_swap:
	# Save
	li $v0, 1
	move $v1, $a1
	syscall
	sw $ra, 0x110($a1) # $a1->pc = $ra

	# Load
	li $v0, 2
	move $v1, $a0
	syscall

interrupt_frame_table:
	.set i, 0
	.rept MAX_RECURSIVE_INTERRUPTS
		.word interrupt_stack_top - i * INTERRUPT_STACK_SIZE
		.set i, i + 1
	.endr

.global Parrot_n64_os_interrupt_handler_entry
Parrot_n64_os_interrupt_handler_entry:
.set noat
	lui $k0, %hi(interrupt_handler_entry2)
	ori $k0, $k0, %lo(interrupt_handler_entry2)
	jr $k0
	nop
.set at

.global Parrot_n64_os_interrupt_handler_entry_size
Parrot_n64_os_interrupt_handler_entry_size: .long . - Parrot_n64_os_interrupt_handler_entry

interrupt_handler_entry2:
.set noat
	lui $k1, %hi(_Parrot_n64_os_interrupt_count)
	lw $k1, %lo(_Parrot_n64_os_interrupt_count)($k1)
	sll $k1, $k1, 2

	lui $k0, %hi(interrupt_frame_table)
	ori $k0, $k0, %lo(interrupt_frame_table)
	addu $k0, $k0, $k1
	lw $k0, 0($k0)

	addiu $k0, $k0, -0x120
	sd $sp, 0x0E0($k0)
	move $sp, $k0

	lui $k1, %hi(_Parrot_n64_os_interrupt_count)
	lw $k0, %lo(_Parrot_n64_os_interrupt_count)($k1)
	addiu $k0, $k0, 1
	sw $k0, %lo(_Parrot_n64_os_interrupt_count)($k1)

    sd $1, 0x000($sp)
    sd $2, 0x008($sp)
    sd $3, 0x010($sp)
    sd $4, 0x018($sp)
    sd $5, 0x020($sp)
    sd $6, 0x028($sp)
    sd $7, 0x030($sp)
    sd $8, 0x038($sp)
    sd $9, 0x040($sp)
    sd $10, 0x048($sp)
    sd $11, 0x050($sp)
    sd $12, 0x058($sp)
    sd $13, 0x060($sp)
    sd $14, 0x068($sp)
    sd $15, 0x070($sp)
    sd $16, 0x078($sp)
    sd $17, 0x080($sp)
    sd $18, 0x088($sp)
    sd $19, 0x090($sp)
    sd $20, 0x098($sp)
    sd $21, 0x0A0($sp)
    sd $22, 0x0A8($sp)
    sd $23, 0x0B0($sp)
    sd $24, 0x0B8($sp)
    sd $25, 0x0C0($sp)
    sd $26, 0x0C8($sp)
    sd $27, 0x0D0($sp)
    sd $28, 0x0D8($sp)
    sd $30, 0x0E8($sp)
    sd $31, 0x0F0($sp)
.set at

    mfhi $t0
    sd $t0, 0x0F8($sp)
    mflo $t0
    sd $t0, 0x100($sp)

    dmfc0 $t0, $8 # badvaddr
    sd $t0, 0x108($sp)
    mfc0 $t0, $14 # epc
    sw $t0, 0x110($sp)

    mfc0 $t0, $13 # cause
    sw $t0, 0x114($sp)
    mfc0 $t0, $12 # status
	sw $t0, 0x118($sp)
    li $t1, ~0xFF02 # EXL | IM7:IM0
    and $t0, $t0, $t1
    mtc0 $t0, $12
	ehb

	move $a0, $sp
	addiu $sp, $sp, -0x20

	la $t0, _Parrot_n64_os_interrupt_handler
	jalr $t0
	nop

	addiu $sp, $sp, 0x20

	lw $t0, 0x118($sp)
	li $t1, ~2
    and $t0, $t0, $t1
	mtc0 $t0, $12 # status
	ehb

    lw $t0, 0x110($sp) # epc
    mtc0 $t0, $14
	ehb

    ld $t0, 0x0F8($sp)
	mthi $t0
	ld $t0, 0x100($sp)
	mtlo $t0

.set noat
	ld $31, 0x0F0($sp)
	ld $30, 0x0E8($sp)
	ld $28, 0x0D8($sp)
	ld $27, 0x0D0($sp)
	ld $26, 0x0C8($sp)
	ld $25, 0x0C0($sp)
	ld $24, 0x0B8($sp)
	ld $23, 0x0B0($sp)
	ld $22, 0x0A8($sp)
	ld $21, 0x0A0($sp)
	ld $20, 0x098($sp)
	ld $19, 0x090($sp)
	ld $18, 0x088($sp)
	ld $17, 0x080($sp)
	ld $16, 0x078($sp)
	ld $15, 0x070($sp)
	ld $14, 0x068($sp)
	ld $13, 0x060($sp)
	ld $12, 0x058($sp)
	ld $11, 0x050($sp)
	ld $10, 0x048($sp)
	ld $9, 0x040($sp)
	ld $8, 0x038($sp)
	ld $7, 0x030($sp)
	ld $6, 0x028($sp)
	ld $5, 0x020($sp)
	ld $4, 0x018($sp)
	ld $3, 0x010($sp)
	ld $2, 0x008($sp)
    ld $1, 0x000($sp)

	ld $sp, 0x0E0($sp)

	lui $k1, %hi(_Parrot_n64_os_interrupt_count)
	lw $k0, %lo(_Parrot_n64_os_interrupt_count)($k1)
	addiu $k0, $k0, -1
	sw $k0, %lo(_Parrot_n64_os_interrupt_count)($k1)

	ehb
	eret
.set at

.section .os.bss
.align 8
interrupt_stack_bottom:
    .space MAX_RECURSIVE_INTERRUPTS * INTERRUPT_STACK_SIZE
interrupt_stack_top:
