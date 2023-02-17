#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

void atrapa_zombie(int sig) {
    wait(0);
    return;
}


int main(int argc, char* argv[]) {
    int sock_pasivo, sock_datos;
    struct sockaddr_in d_local;
    int leidos;
    char buffer[1024];
    pid_t pid;

    signal(SIGCHLD, atrapa_zombie);
    sock_pasivo = socket(PF_INET, SOCK_STREAM, 0);
    d_local.sin_family = AF_INET;
    d_local.sin_addr.s_addr = htonl(INADDR_ANY);
    d_local.sin_port = htons(7890);
    bind(sock_pasivo, (struct sockaddr *)&d_local, sizeof(d_local));
    listen(sock_pasivo, SOMAXCONN);
    printf("Padre antes del while (%d)\n", getpid());
    while (1) { // Bucle infinito de atención a clientes
        printf("Padre antes del accept (%d)", getpid());
        sock_datos = accept(sock_pasivo, 0, 0);
        printf("Padre despues del accept (%d)", getpid());
        if ((pid = fork()) < 0) { // error de clonación, lo hacemos nosotros mismos
            printf("Error de clonación, continua el padre (%d)\n", getpid());
            while((leidos = read(sock_datos, buffer, sizeof(buffer))) > 0)
                write(sock_datos, buffer, leidos);
            close(sock_datos);
        } else if (pid == 0) { // retorno del hijo
            printf("Hijo antes del write (%d)\n", getpid());
            while((leidos = read(sock_datos, buffer, sizeof(buffer))) > 0)
                write(sock_datos, buffer, leidos);
            printf("Hijo despues del write (%d)\n", getpid());
            close(sock_datos);
            exit(0);
        } else { // retorno del padre
            printf("Padre antes del close (%d)\n", getpid());
            close(sock_datos);
        }
    }
}
