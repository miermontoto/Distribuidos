#****************************************************
# Variables
#****************************************************
CC=gcc
RPCGEN=rpcgen
CFLAGS=-I. -Wall -ggdb -I/usr/include/tirpc -ltirpc
LDFLAGS=-pthread
CLIENTE=cliente
SERVIDOR=servidor
INICIALIZADOR=inicializador_sislog

todo : $(CLIENTE) $(SERVIDOR) $(INICIALIZADOR)
all : todo # alias

#**************************************************
# Generacion del cliente y del servidor
#**************************************************

$(CLIENTE): sislog_clnt.o sislog_xdr.o cliente.o util.o
	$(CC) $(CFLAGS) -o $(CLIENTE) sislog_clnt.o sislog_xdr.o cliente.o util.o $(LDFLAGS)

$(INICIALIZADOR): sislog_clnt.o sislog_xdr.o inicializador_sislog.o util.o
	$(CC) $(CFLAGS) -o $(INICIALIZADOR) sislog_clnt.o sislog_xdr.o inicializador_sislog.o util.o

$(SERVIDOR): sislog_svc.o sislog_xdr.o servicios.o util.o
	$(CC) $(CFLAGS) -o $(SERVIDOR) sislog_svc.o sislog_xdr.o servicios.o util.o

sislog_svc.o: sislog.h sislog_svc.c
	$(CC) $(CFLAGS) -c sislog_svc.c

servicios.o: servicios.c sislog.h util.h
	$(CC) $(CFLAGS) -c servicios.c

sislog_clnt.o: sislog.h sislog_clnt.c
	$(CC) $(CFLAGS) -c sislog_clnt.c

sislog_xdr.o: sislog.h sislog_xdr.c
	$(CC) $(CFLAGS) -c sislog_xdr.c

cliente.o: cliente.c sislog.h util.h
	$(CC) $(CFLAGS) -c cliente.c

inicializador_sislog.o: inicializador_sislog.c util.c util.h sislog.h
	$(CC) $(CFLAGS) -c inicializador_sislog.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

sislog.h: sislog.x
	$(RPCGEN) sislog.x &>/dev/null


#**************************************************
# Utilidades
#**************************************************

clean:
	rm -f *.o vgcore.*

cleanall: clean
	rm -f sislog_*
	rm -f sislog.h
	rm -f cliente servidor inicializador_sislog
	rm -f *.dat
