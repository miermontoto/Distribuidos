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

    p.nombre = "María";
    p.edad = 22;

    xdr_Persona(&operacionP, (Persona *) &p);
    xdr_Texto(&operacionT, (Texto *) &t);

    xdr_destroy(&operacionP);
    xdr_destroy(&operacionT);

    fclose(personaDat);
    fclose(textoDat);
    return 0;
}
