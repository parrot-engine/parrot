.text

fabsf:
    abs.s $f0, $f12
    jr $ra

fabs:
    abs.d $f0, $f12
    jr $ra

sqrtf:
    sqrt.s $f0, $f12
    jr $ra

sqrt:
    sqrt.d $f0, $f12
    jr $ra