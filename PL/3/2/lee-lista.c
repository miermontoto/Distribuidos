#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"

int main() {
    Lista l;
    FILE *fichero;
    XDR operacion;

    fichero = fopen("Lista.dat", "r");
    if (fichero == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }

    l.siguiente = NULL;
    xdrstdio_create(&operacion, fichero, XDR_DECODE);
    xdr_Lista(&operacion, (Lista *) &l);

    while (l.siguiente != NULL) {
        printf("%d", l.dato);
        l = *l.siguiente;
    }

    xdr_destroy(&operacion);
    fclose(fichero);
    return 0;
}
