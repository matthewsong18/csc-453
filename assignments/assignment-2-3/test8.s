.text
_g:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    lw $t0, 8($fp)
    sw $t0, -16($fp)
    lw $t0, -16($fp)
    sw $t0, -20($fp)
    lw $t0, -16($fp)
    la $sp, -4($sp)
    sw $t0, 0($sp)
    jal _println
    la $sp, 4($sp)
    lw $t0, 12($fp)
    sw $t0, -16($fp)
    lw $t1, -16($fp)
    la $sp, -4($sp)
    sw $t1, 0($sp)
    jal _println
    la $sp, 4($sp)
    la $sp, 0($fp)
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    la $sp, 8($sp)
    jr $ra
_f:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    lw $t0, 8($fp)
    sw $t0, -16($fp)
    lw $t0, 12($fp)
    la $sp, -4($sp)
    sw $t0, 0($sp)
    lw $t0, -16($fp)
    la $sp, -4($sp)
    sw $t0, 0($sp)
    jal _g
    la $sp, 8($sp)
    lw $t0, 12($fp)
    sw $t0, -20($fp)
    lw $t0, 8($fp)
    la $sp, -4($sp)
    sw $t0, 0($sp)
    lw $t1, -20($fp)
    la $sp, -4($sp)
    sw $t1, 0($sp)
    jal _g
    la $sp, 8($sp)
    la $sp, 0($fp)
    lw $ra, 0($sp)
    lw $fp, 4($sp)
    la $sp, 8($sp)
    jr $ra
_main:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    li $t0, 12345
    sw $t0, -8($fp)
    li $t1, 23456
    sw $t1, -12($fp)
    lw $t2, -12($fp)
    la $sp, -4($sp)
    sw $t2, 0($sp)
    lw $t3, -8($fp)
    la $sp, -4($sp)
    sw $t3, 0($sp)
    jal _f
    la $sp, 8($sp)
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
