.bss
.align 4

MAX_RECURSIVE_INTERRUPTS = 3
INTERRUPT_STACK_SIZE = 0x200

interrupt_stack_bottom:
    .space MAX_RECURSIVE_INTERRUPTS * INTERRUPT_STACK_SIZE
interrupt_stack_top:
interrupt_count: .space 4

.text
.global Parrot_os_interrupt_handler_entry
.global Parrot_os_interrupt_handler_entry_size

interrupt_frame_table:
	.set i, 0
	.rept MAX_RECURSIVE_INTERRUPTS
		.word interrupt_stack_top - i * INTERRUPT_STACK_SIZE
		.set i, i + 1
	.endr
	
Parrot_os_interrupt_handler_entry:
	j Parrot_os_interrupt_handler_entry2
	nop

Parrot_os_interrupt_handler_entry_size: .long . - Parrot_os_interrupt_handler_entry

Parrot_os_interrupt_handler_entry2:
.set noat
	lui $k1, %hi(interrupt_count)
	lw $k1, %lo(interrupt_count)($k1)
	sll $k1, $k1, 2

	lui $k0, %hi(interrupt_frame_table)
	ori $k0, $k0, %lo(interrupt_frame_table)
	addu $k0, $k0, $k1
	lw $k0, 0($k0)

	addiu $k0, $k0, -0x90
	sw $sp, 0x00($k0)
	move $sp, $k0

	# Now that we've finished using $k0 and $k1, we have registers to work with
	lui $k1, %hi(interrupt_count)
	lw $k0, %lo(interrupt_count)($k1)
	addiu $k0, $k0, 1
	sw $k0, %lo(interrupt_count)($k1)
	sync

    sw $at, 0x04($sp)
.set at
    sw $v0, 0x08($sp)
    sw $v1, 0x0C($sp)
    sw $a0, 0x10($sp)
    sw $a1, 0x14($sp)
    sw $a2, 0x18($sp)
    sw $a3, 0x1C($sp)
    sw $t0, 0x20($sp)
    sw $t1, 0x24($sp)
    sw $t2, 0x28($sp)
    sw $t3, 0x2C($sp)
    sw $t4, 0x30($sp)
    sw $t5, 0x34($sp)
    sw $t6, 0x38($sp)
    sw $t7, 0x3C($sp)
    sw $s0, 0x40($sp)
    sw $s1, 0x44($sp)
    sw $s2, 0x48($sp)
    sw $s3, 0x4C($sp)
    sw $s4, 0x50($sp)
    sw $s5, 0x54($sp)
    sw $s6, 0x58($sp)
    sw $s7, 0x5C($sp)
    sw $t8, 0x60($sp)
    sw $t9, 0x64($sp)
    sw $gp, 0x68($sp)
    sw $fp, 0x6C($sp)
    sw $ra, 0x70($sp)

    mfhi $k0
    sw $k0, 0x74($sp)
    mflo $k0
    sw $k0, 0x78($sp)

    mfc0 $k0, $14 # epc
    sw $k0, 0x7C($sp)
    mfc0 $k0, $13 # cause
    sw $k0, 0x80($sp)

    mfc0 $k0, $12 # status
    li $k1, ~2 # EXL
    and $k0, $k0, $k1
    mtc0 $k0, $12

	move $a0, $sp
	jal Parrot_os_interrupt_handler
	nop

    lw $k0, 0x7C($sp) #epc
    mtc0 $k0, $14

	lw $k0, 0x78($sp)
	mtlo $k0
    lw $k0, 0x74($sp)
	mthi $k0

.set noat
    lw $ra, 0x70($sp)
    lw $fp, 0x6C($sp)
    lw $gp, 0x68($sp)
    lw $t9, 0x64($sp)
    lw $t8, 0x60($sp)
    lw $s7, 0x5C($sp)
    lw $s6, 0x58($sp)
    lw $s5, 0x54($sp)
    lw $s4, 0x50($sp)
    lw $s3, 0x4C($sp)
    lw $s2, 0x48($sp)
    lw $s1, 0x44($sp)
    lw $s0, 0x40($sp)
    lw $t7, 0x3C($sp)
    lw $t6, 0x38($sp)
    lw $t5, 0x34($sp)
    lw $t4, 0x30($sp)
    lw $t3, 0x2C($sp)
    lw $t2, 0x28($sp)
    lw $t1, 0x24($sp)
    lw $t0, 0x20($sp)
    lw $a3, 0x1C($sp)
    lw $a2, 0x18($sp)
    lw $a1, 0x14($sp)
    lw $a0, 0x10($sp)
    lw $v1, 0x0C($sp)
    lw $v0, 0x08($sp)
    lw $at, 0x04($sp)
	lw $sp, 0x00($sp)

	lui $k1, %hi(interrupt_count)
	ori $k1, $k1, %lo(interrupt_count)

	lw $k0, 0($k1)
	addiu $k0, $k0, -1
	sw $k0, 0($k1)
	sync

	ehb
	eret
.set at
