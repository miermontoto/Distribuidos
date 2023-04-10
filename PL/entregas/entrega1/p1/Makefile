CC=gcc
CFLAGS=-I. -pthread -ggdb -Wall

all: sislog cliente

sislog: sislog.c
	$(CC) -o sislog sislog.c cola.c util.c $(CFLAGS)

cliente: cliente.c
	$(CC) -o cliente cliente.c util.c $(CFLAGS)

limpia:
	rm -f *.o

cleanall: limpia
	rm -f sislog cliente *.dat vgcore.*
