.data
# OPTIMIZE AND REGALLOC TIME: 66.0us
.align 2
.text

_main:
la $sp, -8($sp)
sw $fp, 4($sp)
sw $ra, 0($sp)
la $fp, 0($sp)
la $sp, -96($sp)
li $t0, 100
sw $t0, -4($fp)
lw $t0, -4($fp)
sw $t0, -8($fp)
li $t0, 0
sw $t0, -4($fp)
lw $t0, -12($fp)
la $sp, -4($sp)
sw $t0, 0($sp)
jal _println
la $sp, 4($sp)
lw $t0, -4($fp)
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
