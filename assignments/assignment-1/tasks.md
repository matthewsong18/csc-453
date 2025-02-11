# CSC 352 Assignment 1

## Tasks

> Implement a function `get_token()`

- Read input from stdin
- Discards comments and whitespace (Still signals end of token).
  [Syntax for comments](https://d2l.arizona.edu/content/enforced/1567717-235-2251-1CSC453001/G0.html)
- Identifies and returns tokens based on the tokens and patterns from
  [Section 1.2](https://d2l.arizona.edu/content/enforced/1567717-235-2251-1CSC453001/G0.html)
- Create a Makefile that has the commands `make clean` and `make scanner`

## Features

### Get input

1. Read input from stdin
2. Return data

### Get tokens

1. Read tokens from data
2. Return tokens

## Notes

- Use scanner.h and scanner-driver.c
- scanner.h (a header file that defines the set of tokens together with their
  values)
- scanner-driver.c (a file containing the driver code that invokes your scanner
  and prints out the results)
- We are replacing scanner-driver.c in the future with a different driver.
