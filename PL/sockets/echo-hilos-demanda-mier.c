#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>


#define PUERTO 2007
#define MAX_LINEA 80

int createTCPsocket(int port);
int createUDPsocket(int port);
int t_accept(int sock, struct sockaddr *addr, socklen_t *addrlen);
int t_recv(int sock, void *buf, size_t len);
int t_send(int sock, const void *buf, size_t len);
int t_close(int sock);

/* --- */

struct conex {
    char *src_ip;           // IP origen
    int src_port;           // Puerto de origen
    int sock;               // Socket de conexión con el cliente
    unsigned int threadnum; // Ordinal del hilo que atiende la petición
};

typedef struct conex datos_conexion;

int sockEscucha, sockDatos;

void sigint_handler(int sig)
{
    printf("Interrupción desde teclado. Terminando servidor. \n");
    exit(t_close(sockEscucha));
}

int createTCPsocket(int port)
{
    struct sockaddr_in dir;

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock <= -1) {
        perror("Al crear socket");
        exit(EXIT_FAILURE);
    }
    dir.sin_family = AF_INET;
    dir.sin_port = htons(port);
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &dir, sizeof(dir)) <= -1) {
        perror("Al asignar direccion");
        close(sock);
        exit(EXIT_FAILURE);
    }
    if (listen(sock, 5) <= -1) {
        perror("Al poner a escuchar");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return sock;
}

int createUDPsocket(int port)
{
    struct sockaddr_in dir;

    int sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock <= 1) {
        perror("Al crear socket");
        exit(EXIT_FAILURE);
    }
    dir.sin_family = AF_INET;
    dir.sin_port = htons(port);
    dir.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *) &dir, sizeof(dir)) <= -1) {
        perror("Al asignar direccion");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return sock;
}

int t_accept(int sock, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret = accept(sock, addr, addrlen);
    if (ret <= -1) {
        perror("Al aceptar conexión");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return ret;
}

int t_recv(int sock, void *buf, size_t len)
{
    int ret = recv(sock, buf, len, 0);
    if (ret == -1) {
        perror("Al recibir datos");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return ret;
}

int t_send(int sock, const void *buf, size_t len)
{
    int ret = send(sock, buf, len, 0);
    if (ret == -1) {
        perror("Al enviar datos");
        close(sock);
        exit(EXIT_FAILURE);
    }
    return ret;
}

int t_close(int sock)
{
    int ret = close(sock);
    if (ret == -1) {
        perror("Al cerrar socket");
        exit(EXIT_FAILURE);
    }
    return ret;
}

void* servidor_echo(datos_conexion *s)
{
    int recibidos;
    char buffer[MAX_LINEA];

    printf("Hilo %d: Recibida conexión desde %s(%d)\n",
        s -> threadnum, s -> src_ip, s -> src_port);
    // Leemos datos del socket mientras que el cliente no cierre la conexión
    do {
        recibidos = t_recv(s -> sock, buffer, MAX_LINEA);
        printf("Hilo %d: Recibida un mensaje de longitud %d\n",
            s -> threadnum, recibidos);
        buffer[recibidos] = 0;  // Añadir terminador
        printf("Hilo %d: Contenido: %s\n", s -> threadnum, buffer);
        t_send(s -> sock, buffer, strlen(buffer));
    } while (recibidos != 0);
    printf("Hilo %d: El cliente ha cerrado. Muero.\n", s -> threadnum);
    t_close(s -> sock);
    //Liberamos la memoria reservada para pasar los datos de la conexión
    //al hilo
    free(s -> src_ip);
    free(s);
    return NULL; // El hilo debe retornar un puntero al terminar
}

int main(int argc, char *argv[])
{
    int port;
    char *src_ip;
    int src_port;
    int numthreads = 0;
    pthread_t tid;
    datos_conexion *con;

    if (argc < 2) {
        printf("Uso: %s [puerto]\n", argv[0]);
        printf("     puerto por defecto = % d\n", PUERTO);
    }

    if (argc>1) port = atoi(argv[1]);
    else port = PUERTO;

    // Tratar el Ctrl-C para cerrar bien el socket de escucha
    signal(SIGINT, sigint_handler);

    sockEscucha = createTCPsocket(port);
    while (1) {
        //Esperamos la llegada de nuevas conexiones
        sockDatos = t_accept(sockEscucha, &src_ip, &src_port);
        //Reservamos espacio para almacenar los datos de la nueva conexión
        con = (datos_conexion *) malloc(sizeof(datos_conexion));
        if (con == NULL) {
            perror("Error al reservar memoria para los datos de la conexión");
            exit(EXIT_FAILURE);
        }
        con-> src_ip = (char *) malloc(strlen(src_ip)+1);
        strcpy(con -> src_ip,src_ip);
        con -> src_port = src_port;
        con -> sock = sockDatos;
        con -> threadnum = numthreads;
        pthread_create(&tid,NULL,(void *) servidor_echo, (void *) con); // Creamos un nuevo hilo que atenderá la nueva conexión
        pthread_detach(tid); // Hacemos que el hilo sea independiente
        printf("Servidor principal: Lanzado el hijo %d\n", numthreads);
        numthreads++; //Incrementamos el contador de hilos lanzados
    }
    return 0;  // Nunca se alcanza, pero evita warning al compilar
}
