.text
_main:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    lw $t0, -8($fp)
    lw $t1, -12($fp)
    beq $t0, $t1, _L0
    j _L1
_L0:
_L1:
    la $sp, 0($fp)
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    la $sp, 8($sp)
    jr $ra

main: j _main
