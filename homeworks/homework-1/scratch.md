# Scratch Paper

## Q5

t2 = 16

t3 = 4

t4 = 4

t5 = -12

## Q6

Find $t3

```
li $t1, 1
li $t2, 2
add $t3, $t1, $t2
bge $t2, $t3, _L1
li $t3, 10
_L1:
```

$t1 = 1

$t2 = 2

$t3 = 10

## Q7

What is the value stored in the memory location addressed by -12($fp)?

```
li $t1, 100
li $t2, 102
sw $t1, -12($fp)
add $t1, $t1, $t1
add $t2, $t1, $t2
lw $t1, -12($fp)
add $t2, $t1, $t2
sw $t2 -12($fp)
```

$t1 = 100

$t2 = 402

-12($fp) = 402

## Q8

Write a snippet of MIPS code that stores the value of the expression 10 + 4 into
register $t0. The MIPS code must evaluate the expression, do not simply store 14
into $t0.

You can use $t3 and $t4 to store intermediate values if needed. Do not modify
stack memory.

```
li $t3, 10
li $t4, 4
add $t0, $t3, $t4
```

## Q11

How many times does the following for loop MIPS code iterate (how many times the
line "addi $t1, $t1, 1" repeats)?

```
main:
    li $t1, 0
    li $t0, 5
loop:
    beq $t1, $t0, end
    # Body of the loop
    addi $t1, $t1, 1      < Focus line
    j loop
end:
```

$t0 = 5

$t1 = 5

5

## Q12

What is the value of $t0 after executing the "move $t0, $v0" line?

```
main:
    li $a0, 10
    li $a1, 20
    jal sum
    move $t0, $v0
    jr $ra

sum:
    add $v0, $a0, $a1
    jr $ra
```

$a0 = 10

$a1 = 20

$v0 = 30

$t0 = 30
