// Fichero: convierte-a-anyos.c modified
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>


int main(int argc, char* argv[]) {
    int sock_dat, recibidos;
    struct sockaddr_in d_serv;
    socklen_t ldir = sizeof(d_serv);
    char buffer[50];
    unsigned long int numero;

    sock_dat = socket(PF_INET, SOCK_DGRAM, 0);
    d_serv.sin_family = AF_INET;
    d_serv.sin_addr.s_addr = inet_addr("129.6.15.28");
    d_serv.sin_port = htons(37);

    // Enviar datagrama vacio
    sendto(sock_dat, buffer, 0, 0, (struct sockaddr *) &d_serv, ldir);
    recibidos = recvfrom(sock_dat, &numero, sizeof(numero), 0, (struct sockaddr *) &d_serv, &ldir);

    printf("Valor recibido: %lu\n", numero); // lu -> long unsigned

    numero = ntohl(numero);
    printf("Valor recibido reordenado: %lu\n", numero);

    numero = numero - 2208988800; // Se restan los segundos entre 1900 y 1970
    /*
    double segundos_por_anyo = 365.25 * 24 * 60 * 60;
    double anyos = numero / segundos_por_anyo;
    printf("Años: %f\n", anyos);
    */
   printf("La fecha actual es %s\n", ctime((time_t *) &numero));
}
