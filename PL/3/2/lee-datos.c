#include <stdio.h> // Para FILE*
#include <errno.h> // para perror()
#include <stdlib.h> // para exit()
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"

int main() {
    Persona p;
    FILE *ficheroP; // Fichero donde se escribirá
    XDR operacion;
    FILE *ficheroT;

    ficheroP = fopen("persona.xdr", "r");
    if (ficheroP == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }

    ficheroT = fopen("texto.xdr", "r");
    if (ficheroT == NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }

    xdrstdio_create(&operacion, ficheroP, XDR_DECODE);
    p.nombre = NULL;
    xdr_Persona(&operacion, (Persona *) &p);

    printf("Nombre de la persona: %s\n", p.nombre);
    printf("Edad de la persona: %d\n", p.edad);

    xdr_destroy(&operacion);
    fclose(ficheroP);
    return 0;
}
