CC = gcc
CFLAGS = -Wall -I include

SRCS = complex.c keywords.c operators.c \
       punctuation.c scanner.c scanner-driver.c

scanner: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o scanner

clean:
	rm -f scanner

.PHONY: clean
