.section .os.boot
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
