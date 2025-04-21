.data
.align 2
_x: .space 4
_z: .space 4
.text
_main:
    la $sp, -8($sp)
    sw $fp, 4($sp)
    sw $ra, 0($sp)
    la $fp, 0($sp)
    li $t0, 12345
    sw $t0, _x
    lw $t1, _x
    la $sp, -4($sp)
    sw $t1, 0($sp)
    jal _println
    la $sp, 4($sp)
    li $t2, 23456
    sw $t2, -8($fp)
    lw $t3, -8($fp)
    la $sp, -4($sp)
    sw $t3, 0($sp)
    jal _println
    la $sp, 4($sp)
    li $t4, 34567
    sw $t4, _z
    lw $t5, _z
    la $sp, -4($sp)
    sw $t5, 0($sp)
    jal _println
    la $sp, 4($sp)
    li $t6, 45678
    sw $t6, -12($fp)
    lw $t7, -12($fp)
    la $sp, -4($sp)
    sw $t7, 0($sp)
    jal _println
    la $sp, 4($sp)
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
