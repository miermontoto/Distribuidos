#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

// Archivos de cabecera para manipulación de sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>

#include "cola.h"
#include "util.h"

#define CIERTO 1
#define FALSO 0

#define NUMFACILITIES 10
#define NUMLEVELS 8

// Estructura de datos para pasar los parametros a los hilos de atencion
struct param_hilo_aten
{
    int num_hilo;
    int s;
};
typedef struct param_hilo_aten param_hilo_aten;

// ====================================================================
// PROTOTIPOS FUNCIONES
// ====================================================================
static void handler(int signum); // Manejador de señal SIGINT

// ====================================================================
// VARIABLES GLOBALES
// ====================================================================

// Cola para sincronizar los hilos de atención de peticiones con los
// hilos trabajadores
Cola cola_eventos;

int puerto; // Cola par_t* hilos_aten;en el que esperamos los mensajes

// Variable booleana que indica si el socket es orientado a conexión o no
unsigned short es_stream = CIERTO;

// Variable que almacena el numero de hilos de atencion de peticiones
int num_hilos_aten;

// Variable que almacena el numero de hilos trabajadores
int num_hilos_work;

// Puntero a la dirección de comienzo del array de datos de hilo
// de los hilos de atencion de peticiones
pthread_t* hilos_aten = NULL;

// Puntero a la dirección de comienzo del array de datos de hilo
// de los hilos trabajadores
pthread_t* hilos_work = NULL;

// Arrays para la traducción de nombres de niveles y de facilities
// y para obtener los nombres de los ficheros de registro
char *facilities_names[NUMFACILITIES] = {
    "kern",
    "user",
    "mail",
    "daemon",
    "auth",
    "syslog",
    "lpr",
    "news",
    "uucp",
    "cron"
};

char *level_names[NUMLEVELS] = {
    "emerg",
    "alert",
    "crit",
    "err",
    "warning",
    "notice",
    "info",
    "debug"
};

char *facilities_file_names[NUMFACILITIES] = {
    "fac00.dat",
    "fac01.dat",
    "fac02.dat",
    "fac03.dat",
    "fac04.dat",
    "fac05.dat",
    "fac06.dat",
    "fac07.dat",
    "fac08.dat",
    "fac09.dat"
};

pthread_mutex_t mfp[NUMFACILITIES]; // Mutex de exclusión a los ficheros de registro

int tam_cola; // Tamaño de la cola circular

// ====================================================================
// Función handler de las señales recibidas por el hilo buque
// ====================================================================
static void handler(int signum) {
    register int i;
    switch(signum) {
        case SIGINT:
            destruir_cola(&cola_eventos); // Destruir cola
            free(hilos_work); // Liberar memoria reservada para objetos de datos de hilos
            free(hilos_aten);
            for(i = 0; i < NUMFACILITIES; i++) pthread_mutex_destroy(&mfp[i]); // Destruir mutex
            exit(EXIT_SUCCESS);
        default:
            pthread_exit(NULL);
    }
}

void procesa_argumentos(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Uso: %s <puerto> <t|u> <tam_cola> <num_hilos_aten> <num_hilos_work>\n", argv[0]);
        exit(EXIT_SUCCESS);
    }

    puerto = atoi(argv[1]);
    if (puerto < 1024 || puerto > 65535) exit_error("Número de puerto inválido");

    if (strcmp(argv[2], "t") == 0) es_stream = CIERTO;
    else if (strcmp(argv[2], "u") == 0) es_stream = FALSO;
    else exit_error("Tipo de socket incorrecto");

    tam_cola = atoi(argv[3]);
    check_value(tam_cola, "Tamaño de cola inválido", 1);

    num_hilos_aten = atoi(argv[4]);
    check_value(num_hilos_aten, "Número de hilos de atención inválido", 1);

    num_hilos_work = atoi(argv[5]);
    check_value(num_hilos_work, "Número de hilos trabajadores inválido", 1);
}

// ====================================================================
// Implementación de los hilos
// ====================================================================

void* Worker(int* id) {
    int id_worker;
    FILE* fp = NULL;
    dato_cola* evt = NULL;
    char msg[2048];
    char* fechahora = NULL;
    time_t timeraw;

    int facilidad;
    int nivel;

    id_worker = *id; // Hacemos copia del parámetro recibido
    free(id);        // y liberamos la memoria reservada para él

    // Mostramos información de depuración por pantalla
    sprintf(msg, "Comienza el Worker %d\n", id_worker);
    log_debug(msg);

    // Codigo del worker. Espera datos de la cola de sincronización,
    // genera en base a ellos la línea a escribir, y la escribe
    // en el fichero que corresponda. Mira "cola.h"
    // para recordar la estructura dato_cola que recibe de la cola
    while (1) {
        evt = (dato_cola*) obtener_dato_cola(&cola_eventos);
        p_check_null(evt, "Error al sacar de la cola de sincronización");

        facilidad = evt -> facilidad - '0';
        nivel = evt -> nivel - '0';

        p_check_error(pthread_mutex_lock(&mfp[facilidad]), "Error al bloquear el mutex");

        fp = fopen(facilities_file_names[facilidad], "a");
        p_check_null(fp, "Error al abrir el fichero de registro correspondiente");

        timeraw = time(NULL);
        fechahora = ctime(&timeraw);
        fechahora[strlen(fechahora) - 1] = '\0';

        evt -> msg[strcspn(evt -> msg, "\r\n")] = '\0';

        check_error(fprintf(fp,
            "%s:%s:%s:%s\n", facilities_names[facilidad], level_names[nivel], fechahora, evt -> msg),
                "Error al escribir al mensaje de log");
        check_error(fclose(fp), "Error al cerrar el fichero de registro correspondiente");

        check_error(pthread_mutex_unlock(&mfp[facilidad]), "Error al desbloquear el mutex");

        /*sprintf(msg, "Guardando %s:%s:%s:%s\n", facilities_names[facilidad], level_names[nivel], fechahora, evt -> msg);
        log_debug(msg);*/

        //free(evt -> msg);
        free(evt);
    }
}

void *AtencionPeticiones(param_hilo_aten *q) {
    int sock_dat, recibidos;
    struct sockaddr_in d_cliente;
    socklen_t l_dir = sizeof(d_cliente);
    char msg[100];
    char buffer[TAMMSG];
    char *token = NULL;
    char *loc = NULL;
    dato_cola *p = NULL;
    int s;        // Variable local para almacenar el socket que se recibe como parámetro
    int num_hilo; // Variable local para almacenar el numero de hilo que se recibe como parámetro
                  // (no usada, pero puedes usarla para imprimir mensajes de depuración)

    // Información de depuración
    sprintf(msg, "Comienza el Hilo de Atencion de Peticiones %d\n", q -> num_hilo);
    log_debug(msg);

    // Hacemos copia de los parámetros recibidos
    s = q -> s;
    num_hilo = q -> num_hilo;
    free(q); // y liberamos la memoria reservada para el parámetro

    while (1) { // Bucle infinito de atencion de mensajes
        // Primero, se recibe el mensaje del cliente. Cómo se haga depende
        // de si el socket es orientado a conexión o no
        if (es_stream) { // TCP
            // Aceptar el cliente, leer su mensaje hasta recibirlo entero, y cerrar la conexión
            p_check_error(listen(s, SOMAXCONN), "Error en el listen");
            sock_dat = accept(s, (struct sockaddr *) &d_cliente, &l_dir);
            p_check_error(sock_dat, "Error en el accept");
            recibidos = recv(sock_dat, buffer, TAMMSG, 0);
            p_check_error(recibidos, "Error en el recv");
            close(sock_dat);
        } else { // UDP
            // Recibir el mensaje del datagrama
            recibidos = recvfrom(s, buffer, TAMMSG, 0, (struct sockaddr *) &d_cliente, &l_dir);
            p_check_error(recibidos, "Error en el recvfrom");
        }

        // Una vez recibido el mensaje, es necesario separar sus partes,
        // guardarlos en la estructura adecuada, y poner esa estructura en la cola
        // de sincronización.
        p = (dato_cola *) malloc(sizeof(dato_cola));
        p_check_null(p, "Error al crear el objeto de datos del hilo");

        token = strtok_r(buffer, ":", &loc); // "facilidad" en token
        if(!valida_numero(token) || atoi(token) >= NUMFACILITIES || atoi(token) < 0) {
            log_debug("Número de facilidad inválido. Se ignora el mensaje.\n");
            continue;
        }
        p -> facilidad = *token;

        token = strtok_r(NULL, ":", &loc); // "nivel" en token
        if(!valida_numero(token) || atoi(token) >= NUMLEVELS || atoi(token) < 0) {
            log_debug("Número de nivel inválido. Se ignora el mensaje.\n");
            continue;
        }
        p -> nivel = *token;

        token = strtok_r(NULL, ":", &loc); // "mensaje" en token
        check_null(p -> msg, "Error p -> msg");
        if(strlen(token) >= TAMMAXMSG) exit_error("Tamaño de mensaje demasiado grande");
        strcpy(p -> msg, token);

        insertar_dato_cola(&cola_eventos, p);
    }
}

// ====================================================================
// PROGRAMA PRINCIPAL
// ====================================================================

// Su misión es crear e inicializar los recursos de sincronización globales,
// lanzar todos los hilos y esperar a que finalicen

int main(int argc, char *argv[]) {
    register int i; // Indice para bucles
    int *id = NULL; // Para pasar el identificador a cada hilo trabajador
    int sock_pasivo;
    struct sockaddr_in d_local;
    param_hilo_aten *q = NULL;

    procesa_argumentos(argc, argv);

    setbuf(stdout, NULL);    // quitamos el buffer de la salida estandar
    signal(SIGINT, handler); // establecemos el comportamiento ante la llegada asíncrona de la señal

    // Datos para asignar puerto al socket
    d_local.sin_family = AF_INET;
    d_local.sin_addr.s_addr = htonl(INADDR_ANY);
    d_local.sin_port = htons(puerto);

    if (es_stream) { // Preparar socket TCP
        sock_pasivo = socket(AF_INET, SOCK_STREAM, 0);
        check_error(sock_pasivo, "Error al crear el socket");
    } else { // Preparar socket UDP
        sock_pasivo = socket(AF_INET, SOCK_DGRAM, 0);
        check_error(sock_pasivo, "Error al crear el socket");
    }

    // Se establecen las opciones del socket para que el puerto pueda ser reutilizado.
    check_error(setsockopt(sock_pasivo, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int)), "Error al establecer opciones en el socket");

    // Asignamos el puerto al socket
    check_error(bind(sock_pasivo, (struct sockaddr *) &d_local, sizeof(d_local)), "Error al asignar el puerto al socket");

    // Creamos el espacio para los objetos de datos de hilo
    hilos_aten = (pthread_t *) malloc (num_hilos_aten * sizeof(pthread_t));
    check_null(hilos_aten, "Error al reservar memoria para los hilos de atención de peticiones");

    // Inicializamos los mutex de exclusión a los ficheros de log
    // en que escribirán los workers
    for (i = 0; i < NUMFACILITIES; i++)
        check_error(pthread_mutex_init(&mfp[i], NULL), "Error al inicializar el mutex");

    // Reservamos espacio para los objetos de datos de hilo de los hilos trabajadores
    hilos_work = (pthread_t *) malloc(num_hilos_work * sizeof(pthread_t));
    check_null(hilos_work, "Error al reservar memoria para los hilos trabajadores");

    inicializar_cola(&cola_eventos, tam_cola); // Inicializamos la cola

    // Creamos cada uno de los hilos de atención de peticiones
    for (i = 0; i < num_hilos_aten; i++) {
        // Creamos el objeto de datos de hilo
        q = (param_hilo_aten *) malloc(sizeof(param_hilo_aten));
        q -> s = sock_pasivo;
        q -> num_hilo = i;

        // Lanzamos el hilo
        check_error(pthread_create(&hilos_aten[i], NULL, (void *) AtencionPeticiones, (void *) q), "Error al crear el hilo");
    }

    // Y creamos cada uno de los hilos trabajadores
    for (i = 0; i < num_hilos_work; i++) {
        // Creamos el objeto de datos de hilo
        id = (int *) malloc(sizeof(int));
        *id = i;

        // Lanzamos el hilo
        check_error(pthread_create(&hilos_work[i], NULL, (void *) Worker, (void *) id), "Error al crear el hilo");
    }

    // Esperamos a que terminen todos los hilos
    for (i = 0; i < num_hilos_aten; i++) check_error(pthread_join(hilos_aten[i], NULL), "Error en el join");
    for (i = 0; i < num_hilos_work; i++) check_error(pthread_join(hilos_work[i], NULL), "Error en el join");

    return 0;
}
