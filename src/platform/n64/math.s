.global _sqrt
_sqrt:
    sqrt.d $f0, $f12
    jr $ra

.global _sqrtf
_sqrtf:
    sqrt.s $f0, $f12
    jr $ra

.global _fabs
_fabs:
    abs.d $f0, $f12
    jr $ra

.global _fabsf
_fabsf:
    abs.s $f0, $f12
    jr $ra
