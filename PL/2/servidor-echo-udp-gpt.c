#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    int recibidos, sock_datos;
    struct sockaddr_in d_local, d_cliente;
    socklen_t ldir = sizeof(d_cliente);
    char *buffer;
    size_t bufsize = 1024;

    sock_datos = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_datos < 0) {
        perror("socket");
        exit(1);
    }
    memset(&d_local, 0, sizeof(d_local));
    d_local.sin_family = AF_INET;
    d_local.sin_addr.s_addr = htonl(INADDR_ANY);
    d_local.sin_port = htons(9998);
    if (bind(sock_datos, (struct sockaddr *)&d_local, sizeof(d_local)) < 0) {
        perror("bind");
        exit(1);
    }

    buffer = malloc(bufsize);
    if (buffer == NULL) {
        perror("malloc");
        exit(1);
    }

    while (1) {
        recibidos = recvfrom(sock_datos, buffer, bufsize, 0, (struct sockaddr *) &d_cliente, &ldir);
        if (recibidos < 0) {
            perror("recvfrom");
            continue;
        }
        printf("Cliente desde %s (%d)\n",
            inet_ntoa(d_cliente.sin_addr), ntohs(d_cliente.sin_port));
        if (sendto(sock_datos, buffer, recibidos, 0, (struct sockaddr *) &d_cliente, ldir) < 0) {
            perror("sendto");
        }
    }
}
