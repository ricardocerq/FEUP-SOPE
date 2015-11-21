CC=gcc
CFLAGS=-std=c99 -O0 -Wall -pedantic -lpthread
BIN=.

all: balcao ger_cl

balcao: balcao.o logging.o
	$(CC) $(CFLAGS) -o $(BIN)/balcao balcao.o logging.o

ger_cl: ger_cl.o logging.o str_concat.o
	$(CC) $(CFLAGS) -o $(BIN)/ger_cl ger_cl.o logging.o str_concat.o

balcao.o: balcao.c 
	$(CC) $(CFLAGS) -c balcao.c

logging.o: logging.c 
	$(CC) $(CFLAGS) -c logging.c

str_concat.o: str_concat.c 
	$(CC) $(CFLAGS) -c str_concat.c