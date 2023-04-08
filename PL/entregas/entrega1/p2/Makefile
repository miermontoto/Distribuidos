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


#**************************************************
# Generacion del cliente y del servidor
#**************************************************

$(CLIENTE): sislog_clnt.o sislog_xdr.o cliente.o util.o
	$(CC) $(CFLAGS) -o $(CLIENTE) sislog_clnt.o sislog_xdr.o cliente.o util.o

$(INICIALIZADOR): sislog_clnt.o sislog_xdr.o inicializador_sislog.o util.o
	$(CC) $(CFLAGS) -o $(INICIALIZADOR) sislog_clnt.o sislog_xdr.o inicializador_sislog.o util.o

$(SERVIDOR): sislog_svc.o sislog_xdr.o servicios.o util.o
	$(CC) $(CFLAGS) -o $(SERVIDOR) sislog_svc.o sislog_xdr.o servicios.o util.o $(LDFLAGS)

sislog_svc.o : sislog_svc.c sislog.h
	$(CC) $(CFLAGS) -c sislog_svc.c

servicios.o: servicios.c sislog.h util.h
	$(CC) $(CFLAGS) -c servicios.c

sislog_clnt.o : sislog_clnt.c sislog.h
	$(CC) $(CFLAGS) -c sislog_clnt.c

sislog_xdr.o : sislog_xdr.c sislog.h
	$(CC) $(CFLAGS) -c sislog_xdr.c

cliente.o : cliente.c sislog.h util.h
	$(CC) $(CFLAGS) -c cliente.c

inicializador_sislog.o : inicializador_sislog.c util.c util.h sislog.h
	$(CC) $(CFLAGS) -c inicializador_sislog.c

util.o : util.c util.h
	$(CC) $(CFLAGS) -c util.c

sislog.h: sislog.x
	$(RPCGEN) -C -h sislog.x &>/dev/null

sislog.x:
	rpcgen sislog.x


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
