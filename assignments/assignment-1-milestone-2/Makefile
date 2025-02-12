CC = gcc
CFLAGS = -Wall

compile: parser_main.o token_service.o grammar_interface.o parser_service.o
	$(CC) $(CFLAGS) -o compile parser_main.o token_service.o grammar_interface.o parser_service.o

parser_main.o: parser_main.c driver.c token_service.h parser_service.h grammar_interface.h
	$(CC) $(CFLAGS) -c parser_main.c

token_service.o: token_service.c token_service.h
	$(CC) $(CFLAGS) -c token_service.c

grammar_interface.o: grammar_interface.c grammar_interface.h token_service.h
	$(CC) $(CFLAGS) -c grammar_interface.c

parser_service.o: parser_service.c parser_service.h token_service.h grammar_interface.h
	$(CC) $(CFLAGS) -c parser_service.c

clean:
	rm -f *.o compile
