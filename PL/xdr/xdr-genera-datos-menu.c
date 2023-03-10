// xdr-escribe-entero.c
#include <stdio.h> // Para FILE*
#include <errno.h> // para perror()
#include <stdlib.h> // para exit()
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"

// Compilar con: gcc -I/usr/include/tirpc -ltirpc xdr/xdr-genera-datos.c -o xdr/xdr-genera-datos

int main() {
    Persona p;
    Texto t;
    Resultado r;
    XDR operacion;
    char c;
    Resultado res;

    FILE *file = fopen("menu.dat", "w");
    if (file == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }

    xdrstdio_create(&operacion, file, XDR_ENCODE);

    printf("Menu:\n");
    printf("1. Entero\n");
    printf("2. Real\n");
    printf("3. Texto\n");
    printf("Elije una opción: ");
    scanf("%d", &res.caso);

    switch(res.caso) {
        case 1: printf("Introduce un valor entero: ");
                scanf("%d", &res.caso);
                break;
        case 2: printf("Introduce un valor real: ");
                scanf("%f", &res.caso);
                break;
        case 3: printf("Introduce un texto: ");
                res.Resultado_u.error = malloc(sizeof(char)*100);
                fgets(res.Resultado_u.error, 100, stdin);
                break;
    }

    xdr_Resultado(&operacion, &res);
    xdr_destroy(&operacion);
    fclose(file);

    return 0;
}
