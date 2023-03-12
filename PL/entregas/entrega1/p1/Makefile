CC=gcc
CFLAGS=-I. -pthread

all: sislog cliente

sislog: sislog.c
	$(CC) -o sislog sislog.c cola.c util.c $(CFLAGS)

cliente: cliente.c
	$(CC) -o cliente cliente.c util.c $(CFLAGS)

limpia:
	-rm *.o

cleanall: limpia
	-rm sislog cliente *.dat
