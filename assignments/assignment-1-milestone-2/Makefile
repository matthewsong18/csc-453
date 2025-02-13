CC = gcc
CFLAGS = -Wall

all: compile

compile: driver.o parser_main.o token_service.o grammar_interface.o parser_service.o scanner.o complex.o keywords.o operators.o punctuation.o
	$(CC) $(CFLAGS) -o compile driver.o parser_main.o token_service.o grammar_interface.o parser_service.o scanner.o complex.o keywords.o operators.o punctuation.o

driver.o: driver.c token_service.h parser_service.h grammar_interface.h
	$(CC) $(CFLAGS) -c driver.c

parser_main.o: parser_main.c token_service.h parser_service.h
	$(CC) $(CFLAGS) -c parser_main.c

scanner.o: scanner.c scanner.h
	$(CC) $(CFLAGS) -c scanner.c

complex.o: complex.c
	$(CC) $(CFLAGS) -c complex.c

keywords.o: keywords.c
	$(CC) $(CFLAGS) -c keywords.c

operators.o: operators.c
	$(CC) $(CFLAGS) -c operators.c

punctuation.o: punctuation.c
	$(CC) $(CFLAGS) -c punctuation.c

token_service.o: token_service.c token_service.h scanner.h
	$(CC) $(CFLAGS) -c token_service.c

grammar_interface.o: grammar_interface.c grammar_interface.h token_service.h
	$(CC) $(CFLAGS) -c grammar_interface.c

parser_service.o: parser_service.c parser_service.h token_service.h grammar_interface.h
	$(CC) $(CFLAGS) -c parser_service.c

clean:
	rm -f *.o compile
