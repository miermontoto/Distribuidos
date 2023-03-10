#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <rpc/types.h>
#include <rpc/rpc.h>
#include "tipos.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>


int main() {
    Persona p;
    FILE *fichero; // Fichero donde se escribirá
    XDR operacion;
    p.nombre = "María";
    p.edad = 22;

    int sock_pasivo, sock_datos;
    struct sockaddr_in d_local;
    sock_pasivo = socket(PF_INET, SOCK_STREAM, 0);
    d_local.sin_family = AF_INET;
    d_local.sin_addr.s_addr = htonl(INADDR_ANY);
    d_local.sin_port = htons(7890);
    bind(sock_pasivo, (struct sockaddr *)&d_local, sizeof(d_local));
    listen(sock_pasivo, SOMAXCONN);
    sock_datos = accept(sock_pasivo, NULL, NULL);

    fichero = fdopen(sock_datos, "w");
    if (fichero == NULL) {
        perror("Al hacer fdopen");
        exit(1);
    }
    xdrstdio_create(&operacion, fichero, XDR_ENCODE);
    xdr_Persona(&operacion, (Persona *) &p);

    xdr_destroy(&operacion);
    fclose(fichero);
    close(fichero);
    return 0;
}
