// Fichero: echo-tcp-udp-select.c
#include <stdio.h>       // printf()
#include <stdlib.h>      // exit()
#include <sys/socket.h>  // socket(), bind(), listen(), recv(), send(), etc
#include <arpa/inet.h>   // sockaddr_in
#include <errno.h>       // perror()
#include <sys/select.h>  // select() y fd_set
#include <unistd.h>      // close()

#define MAXCLIENTES 5

int CrearSocketTCP(int puerto)
{
    int s;
    struct sockaddr_in dir;
    int r;

    s = socket(PF_INET, SOCK_STREAM, 0);
    if (s==-1) {
        perror("En socket TCP");
        exit(1);
    }
    dir.sin_family = AF_INET;
    dir.sin_port   = htons(puerto);
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    r = bind(s, (struct sockaddr *) &dir, sizeof(dir));
    if (r==-1) {
        perror("En bind TCP");
        exit(1);
    }
    r = listen(s, SOMAXCONN);
    if (r==-1) {
        perror("En listen");
        exit(1);
    }
    return s;
}

int CrearSocketUDP(int puerto)
{
    int s;
    struct sockaddr_in dir;
    int r;

    s = socket(PF_INET, SOCK_DGRAM, 0);
    if (s == -1) {
        perror("En socket UDP");
        exit(1);
    }

    dir.sin_family = AF_INET;
    dir.sin_port   = htons(puerto);
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    r = bind(s, (struct sockaddr *) &dir, sizeof(dir));
    if (r == -1) {
        perror("En bind UDP");
        exit(1);
    }

    return s;
}

void dar_servicio_UDP(int s)
{
    // Lee un datagrama del socket s y lo reenvía a su origen
    struct sockaddr_in origen;
    socklen_t tamanio = sizeof(origen);
    char buffer[100];
    int leidos;

    leidos = recvfrom(s, buffer, 100, 0, (struct sockaddr *) &origen, &tamanio);
    sendto(s, buffer, leidos, 0, (struct sockaddr *) &origen, tamanio);
}


int dar_servicio_TCP(int s)
{
    // Lee datos del socket s y si lee distinto de cero, envia eco
    // Retorna el numero de datos leidos

   char buffer[100];
   int leidos;

   leidos = recv(s, buffer, 100, 0);
   if (leidos>0)
       send(s, buffer, leidos, 0);
   return leidos;
}

int max(int a, int b)
{
    // Devuelve el mayor entre a y b
    if (a>b) return a;
    else return b;
}

int buscar_maximo(int tcp, int *datos, int size)
{
    int maximo = tcp;
    int i;
    for(i = 0; i < size; i++) if(datos[i] > maximo) maximo = datos[i];
    return maximo;
}

void inicializar_array(int *arr, int size) {
    int i;
    for(i = 0; i < size; i++) arr[i] = 0;
}

int buscar_pos_libre(int *arr, int size) {
    int i;
    for(i = 0; i < size; i++) if(arr[i] == 0) return i;
    return -1;
}

int main(int argc, char * argv[])
{
    int puerto;
    int s_tcp;
    int s_datos[MAXCLIENTES];
    fd_set conjunto;
    int maximo;
    int numclientes = 0;

    if (argc<2) {
        printf("Uso: %s puerto\n", argv[0]);
        exit(0);
    }

    puerto = atoi(argv[1]);
    s_tcp = CrearSocketTCP(puerto);
    inicializar_array(s_datos, MAXCLIENTES);

    while (1) {  // Bucle infinito del servidor
        // Vaciar conjunto de descriptores a vigilar
        FD_ZERO(&conjunto);

        // Meter solo los que haya que vigilar
        // Si el número de clientes activos es menor que MAXCLIENTES,
        // se vigila el socket TCP para aceptar nuevos clientes.
        if (numclientes < MAXCLIENTES) FD_SET(s_tcp, &conjunto);
        int i;
        for(i = 0; i < MAXCLIENTES; i++) if(s_datos[i] != 0) FD_SET(s_datos[i], &conjunto);

        maximo = buscar_maximo(s_tcp, s_datos, MAXCLIENTES);

        // Esperar a que ocurra "algo"
        select(maximo + 1, &conjunto, NULL, NULL, NULL);
        printf("Ha ocurrido algo\n");

        // Averiguar que ocurrió
        if (FD_ISSET(s_tcp, &conjunto) && numclientes < MAXCLIENTES)
        {
            printf("Ha llegado un cliente al socket TCP\n");
            s_datos[buscar_pos_libre(s_datos, MAXCLIENTES)] = accept(s_tcp, NULL, NULL);
            numclientes++;
        }

        for(i = 0; i < MAXCLIENTES; i++)
            if (FD_ISSET(s_datos[i], &conjunto))
            {
                int n;
                printf("Han llegado datos por la conexión TCP\n");
                n = dar_servicio_TCP(s_datos[i]);
                if (n==0) {
                    printf("El cliente TCP ha desconectado\n");
                    close(s_datos[i]);
                    s_datos[i] = 0;
                    numclientes--;
                }
            }
    } // del while(1)
    return 0;   // Nunca se ejecuta, pero para que el compilador no proteste
} // de main()
