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
    FILE *personaDat; // Fichero donde se escribirá
    XDR operacionP;
    XDR operacionT;
    char c;
    FILE *textoDat = fopen("persona.dat", "w");
    personaDat = fopen("datos.xdr", "w"); // Abrir para "w"rite
    if (personaDat == NULL || textoDat == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }
    t = "Probando";
    xdrstdio_create(&operacionP, personaDat, XDR_ENCODE);
    xdrstdio_create(&operacionT, textoDat, XDR_ENCODE);

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

    return 0;
}
