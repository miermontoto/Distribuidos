flags=-I/usr/include/tirpc -ltirpc

menu: xdr-genera-datos-menu.c
		gcc $(flags) xdr-genera-datos-menu.c tipos_xdr.c -o menu

lee: lee-datos.c
		gcc $(flags) lee-datos.c tipos_xdr.c -o lee

generar: xdr-genera-datos.c
		gcc $(flags) xdr-genera-datos.c tipos_xdr.c -o generar

clean:
		rm -f *.o *.xdr *.dat

lee-menu:
		gcc $(flags) lee-datos-menu.c tipos_xdr.c -o lee-menu

lista:
		gcc $(flags) lee-lista.c tipos_xdr.c -o lista

cliente: cliente-datos.c
		gcc $(flags) cliente-datos.c tipos_xdr.c -o cliente

servidor: servidor-datos.c
		gcc $(flags) servidor-datos.c tipos_xdr.c -o servidor
