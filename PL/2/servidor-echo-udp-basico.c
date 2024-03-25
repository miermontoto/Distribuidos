#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>


int main(int argc, char* argv[]) {
    int recibidos, sock_datos;
    struct sockaddr_in d_local, d_cliente;
    socklen_t ldir = sizeof(d_cliente);
    char buffer[1024];

    sock_datos = socket(PF_INET, SOCK_DGRAM, 0);
    d_local.sin_family = AF_INET;
    d_local.sin_addr.s_addr = htonl(INADDR_ANY);
    d_local.sin_port = htons(9998);
    bind(sock_datos, (struct sockaddr *)&d_local, sizeof(d_local));
    while (1) { // Bucle infinito de atención a clientes
        recibidos = recvfrom(sock_datos, buffer, sizeof(buffer), 0, (struct sockaddr *) &d_cliente, &ldir);
        printf("Cliente desde %s (%d)\n",
            inet_ntoa(d_cliente.sin_addr), ntohs(d_cliente.sin_port));
        sendto(sock_datos, buffer, recibidos, 0, (struct sockaddr *) &d_cliente, ldir);
    }
}
