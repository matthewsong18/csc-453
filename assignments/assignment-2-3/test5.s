.text
_main:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    li $t0, 5
    sw $t0, -8($fp)
    li $t1, 5
    sw $t1, -12($fp)
    lw $t0, -8($fp)
    lw $t1, -12($fp)
    beq $t0, $t1, _L0
    j _L1
_L0:
    lw $t2, -8($fp)
    la $sp, -4($sp)
    sw $t2, 0($sp)
    jal _println
    la $sp, 4($sp)
_L1:
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
