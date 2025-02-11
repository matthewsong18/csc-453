.align 2
.text

_main:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -88($sp)
li $t0, 4
li $t0, 5
add $t2, $t0, $t0
la $sp, -4($sp)
sw $t2, 0($sp)
jal _println
la $sp, 4($sp)
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

# hard coded println
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

# hard coded main call
.align 2
.text
main: j _main
