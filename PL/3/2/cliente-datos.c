#include <stdio.h> // Para FILE*
#include <errno.h> // para perror()
#include <stdlib.h> // para exit()
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>

// rubennmg
int main() {
    int sock_datos;
    struct sockaddr_in d_serv;
    Persona alumno;
    FILE *fichero; // Fichero donde se escribirá
    XDR operacion;

    sock_datos = socket(PF_INET, SOCK_STREAM, 0);
    d_serv.sin_family = AF_INET;
    d_serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    d_serv.sin_port = htons(7890);

    fichero = fdopen(sock_datos, "r"); // Parámetros: descriptor y modo
    if (fichero == NULL) { // Comprobar errores
        perror("Al convertir el descriptor en *FILE");
        exit(1);
    }

    // Inicializar variable operacion para filtros subsiguientes
    xdrstdio_create(&operacion, fichero, XDR_DECODE);

    alumno.nombre = NULL; // Punteros inicializados siempre a NULL

    xdr_Persona(&operacion, (Persona *) &alumno);
    printf("El nombre del alumno es: %s\n", alumno.nombre);
    printf("La edad del alumno es: %d\n", alumno.edad);

    // Terminado, labores finales "domésticas"
    xdr_destroy(&operacion); // Destruir la variable operacion
    fclose(fichero); // Cerrar fichero
    return 0;

    // gcc lee-datos2.c -o lee-datos2 tipos_xdr.c
}
