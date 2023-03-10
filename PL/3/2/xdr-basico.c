// xdr-escribe-entero.c
#include <stdio.h> // Para FILE*
#include <errno.h> // para perror()
#include <stdlib.h> // para exit()
#include <rpc/types.h>
#include <rpc/rpc.h>

// Compilar: gcc -I/usr/include/tirpc -ltirpc xdr/xdr-basico.c -o xdr/xdr-basico

int main() {
    int j = 2013; // Dato a escribir
    FILE *fichero; // Fichero donde se escribirá
    XDR operacion;
    int aux;
    char c;
    fichero=fopen("datos.xdr", "w"); // Abrir para "w"rite
    if (fichero==NULL) { // Comprobar errores
        perror("Al abrir fichero");
        exit(1);
    }
    // Inicializar variable operacion para filtros subsiguientes
    xdrstdio_create(&operacion, fichero, XDR_ENCODE);
    printf("Introduce un valor entero: ");
    scanf("%d", &j);
    getchar();
    printf("Escribe un caracter: ");
    c = (char) fgetc(stdin);
    // Escribir la variable j en el fichero, en representacion externa
    xdr_int(&operacion, &j); // Llamada al filtro. Codifica y guarda
    xdr_char(&operacion, &c);
    // Terminado, labores finales "domésticas"
    xdr_destroy(&operacion); // Destruir la variable operacion
    fclose(fichero); // Cerrar fichero
    return 0;
}
