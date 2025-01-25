.data
.align 2
.text

_f:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -92($sp)
lw $t0, 8($fp)
lw $t1, 8($fp)
mul $t0, $t0, $t1
sw $t0, -4($fp)
lw $t0, -4($fp)
move $v0, $t0
la $sp, 0($fp)
lw $ra, 0($sp)
lw $fp, 4($sp)
la $sp, 8($sp)
jr $ra

.data
.align 2
.text

_main:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -144($sp)
li $t0, 100
sw $t0, -4($fp)
li $t0, 100
sw $t0, -8($fp)
li $t0, 50
sw $t0, -12($fp)
lw $t0, -8($fp)
li $t1, 50
add $t0, $t0, $t1
sw $t0, -16($fp)
lw $t0, -16($fp)
sw $t0, -20($fp)
li $t0, 100
sw $t0, -24($fp)
li $t0, 2
sw $t0, -28($fp)
li $t0, 2
lw $t1, -20($fp)
mul $t0, $t0, $t1
sw $t0, -32($fp)
lw $t0, -32($fp)
lw $t1, -20($fp)
mul $t0, $t0, $t1
sw $t0, -36($fp)
li $t0, 100
lw $t1, -36($fp)
sub $t0, $t0, $t1
sw $t0, -40($fp)
lw $t0, -40($fp)
lw $t1, -8($fp)
add $t0, $t0, $t1
sw $t0, -44($fp)
lw $t0, -44($fp)
sw $t0, -48($fp)
lw $t0, -8($fp)
lw $t1, -48($fp)
sub $t0, $t0, $t1
sw $t0, -52($fp)
lw $t0, -52($fp)
sw $t0, -8($fp)
lw $t0, -8($fp)
la $sp, -4($sp)
sw $t0, 0($sp)
jal _f
la $sp, 4($sp)
move $t0, $v0
sw $t0, -56($fp)
lw $t0, -56($fp)
sw $t0, -20($fp)
lw $t0, -20($fp)
la $sp, -4($sp)
sw $t0, 0($sp)
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

# dumping global symbol table into mips
.data

# hard coded main call
.align 2
.text
main: j _main
