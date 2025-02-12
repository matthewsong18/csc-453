CC = gcc
CFLAGS = -Wall -I include

SRCS = complex.c keywords.c operators.c \
       punctuation.c scanner.c scanner-driver.c

COMPILE_SRC = complex.c keywords.c operators.c punctuation.c scanner.c driver.c

target:
	make compile

scanner: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o scanner

compile: $(COMPILE_SRC)
	$(CC) $(CFLAGS) $(COMPILE_SRC) -o compile

clean:
	rm -f scanner
	rm -f compile

.PHONY: clean
