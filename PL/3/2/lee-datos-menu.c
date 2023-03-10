#include <stdio.h> // Para FILE*
#include <errno.h> // para perror()
#include <stdlib.h> // para exit()
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"

int main() {
    Resultado res;
    FILE *fichero;
    XDR operacion;

    fichero = fopen("menu.dat", "r");
    if (fichero == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }

    xdrstdio_create(&operacion, fichero, XDR_DECODE);
    res.Resultado_u.error = NULL;
    xdr_Resultado(&operacion, (Resultado *) &res);

    switch (res.caso) {
        case 1:     printf("El valor entero era: %d\n", res.Resultado_u.n);
                    break;
        case 2:     printf("El valor real era: %f\n", res.Resultado_u.x);
                    break;
        case 3:     printf("La cadena era: %s\n", res.Resultado_u.error);
                    break;
        default:    perror("Valor erróneo del selector");
    }

    xdr_destroy(&operacion);
    fclose(fichero);
    return 0;
}
