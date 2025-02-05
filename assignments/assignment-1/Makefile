CC = gcc
CFLAGS = -Wall -I include

SRCS = src/tokens/complex.c src/tokens/keywords.c src/tokens/operators.c \
       src/tokens/punctuation.c src/scanner.c src/scanner-driver.c

scanner: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o scanner

clean:
	rm -f scanner

.PHONY: clean
