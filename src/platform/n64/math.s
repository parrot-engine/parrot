.global sqrt
sqrt:
    sqrt.d $f0, $f12
    jr $ra

.global sqrtf
sqrtf:
    sqrt.s $f0, $f12
    jr $ra

.global fabs
fabs:
    abs.d $f0, $f12
    jr $ra

.global fabsf
fabsf:
    abs.s $f0, $f12
    jr $ra
