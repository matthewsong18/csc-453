.text
_main:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    li $t0, 0
    sw $t0, -8($fp)
_L0:
    li $t1, 3
    lw $t0, -8($fp)
    blt $t0, $t1, _L1
    j _L2
_L1:
    li $t2, 5
    la $sp, -4($sp)
    sw $t2, 0($sp)
    jal _println
    la $sp, 4($sp)
    li $t3, 5
    sw $t3, -8($fp)
    j _L0
_L2:
    la $sp, 0($fp)
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    la $sp, 8($sp)
    jr $ra
.align 2
.data
_nl: .asciiz "\n"
.align 2
.text
_println:
    li $v0, 1
    lw $a0, 0($sp)
    syscall
    li $v0, 4
    la $a0, _nl
    syscall
    jr $ra

main: j _main
