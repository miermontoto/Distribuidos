/*
Cliente de RPC que simula las operaciones de varios clientes del servidor de log
*/
#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "util.h"
#include "sislog.h"

#define TAMLINEA 1024
#define TAMMSG   1024

// --- Variables globales ---

// IP del proceso sislog
char *ip_sislog;

// numero de clientes
int num_clientes;

// tipo de datos que recibiran los hilos cliente
struct datos_hilo {
	FILE *fp;
	int id_cliente;
};
typedef struct datos_hilo datos_hilo;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *Cliente(datos_hilo *p) {
    // Implementación del hilo que genera los eventos que se envían al
    // servidor via RPC
    CLIENT* cl;
    FILE* fp;
    int id_cliente;
    char buffer[TAMLINEA];  // Buffer de lectura de lineas del fichero de eventos
    char msg[TAMLINEA * 2];
    char aux[TAMLINEA * 2 / 3];

    Resultado *res;
    char* s;
    char* token;
    char* loc;

    eventsislog evt;
    evt.msg = (char*) malloc(sizeof(char) * TAMMSG);
    check_null(evt.msg, "malloc del mensaje de evento");

    id_cliente = p -> id_cliente; // Capturar el id del cliente en una variable local
    fp = p -> fp;
    free(p); // Ya no necesitamos el parámetro recibido, lo liberamos

    // Creamos un handler de conexión con el servidor RPC
    // y comprobamos que se ha creado correctamente
    //
    // NOTA: se crea un único handler para cada hilo cliente, de
    // forma que se crea y se destruye el handler una única vez
    // para evitar pérdidas de memoria.
    cl = clnt_create(ip_sislog, SISLOG, PRIMERA, "tcp");
    if (cl == NULL) {
        clnt_pcreateerror(ip_sislog);
        clnt_destroy(cl);
        exit(1);
    }

    // Bucle de lectura de eventos
    do {
        // Leemos mediante exclusión la siguiente línea del fichero cuyo *FILE
        // recibimos en uno de los campos de de la estructura datos_hilo
        bzero(buffer, TAMLINEA);
        check_error(pthread_mutex_lock(&mutex), "pthread_mutex_lock");
        s = fgets(buffer, TAMLINEA, fp);
        check_error(pthread_mutex_unlock(&mutex), "pthread_mutex_unlock");

        // Si la cadena leida del fichero no es nula, tokenizamos de manera
        // reentrante la línea para extraer sus tokens e ir componiendo
        // el mensaje de la invocación remota al sislog
        if (s != NULL) {
            // Tokenizar cadena y rellenado de la estructura de datos
            token = strtok_r(s, ":", &loc);
            evt.facilidad = atoi(token);
            token = strtok_r(NULL, ":", &loc);
            evt.nivel = atoi(token);
            token = strtok_r(NULL, ":", &loc);
            strcpy(evt.msg, token);

            // Mensaje de depuración
            sprintf(msg, "Cliente %d envia evento. Facilidad: %d, Nivel: %d, Texto: %s\n", id_cliente, evt.facilidad, evt.nivel, evt.msg);
            log_debug(msg);

            // Enviar evento por RPC e imprimir el velor retornado
            // y liberar seguidamente las estructuras de datos utilizadas
            res = registrar_evento_1(&evt, cl);
            check_null(res, "Error al llamar al servidor RPC");
            sprintf(aux, "Cliente %d recibe respuesta. Caso: %d", id_cliente, res -> caso);
            switch(res -> caso) {
                case 0:
                    sprintf(msg, "%s, Valor: %d\n", aux, res -> Resultado_u.valor);
                    break;
                case 1:
                    sprintf(msg, "%s, Mensaje: %s\n", aux, res -> Resultado_u.msg);
                    break;
                default:
                    sprintf(msg, "%s, Error desconocido\n", aux);
                    break;
            }
            log_debug(msg);
        }
    } while(s);
    free(evt.msg);
    clnt_destroy(cl);
    return NULL;
}

int main(int argc,char *argv[]) {
    register int i;
    pthread_t* th;
    datos_hilo* q;
    FILE* fp;
    char msg[TAMLINEA * 2];

    if (argc != 4) {
        fprintf(stderr, "Forma de uso: %s <numero_clientes> <ip_serv_sislog> <fich_eventos>\n", argv[0]);
        exit(1);
    }

    if (atoi((char *) argv[1]) <= 0) {
        fprintf(stderr, "El parametro <numero_clientes> debe ser un entero positivo\n");
        exit(3);
    }

    ip_sislog = argv[2];
    if (!valida_ip(ip_sislog)) {
        fprintf(stderr, "La IP introducida no es valida\n");
        exit(4);
    }
    num_clientes = atoi(argv[1]);

    // Reservamos memoria para los objetos de datos de hilo
    th = (pthread_t*) malloc(sizeof(pthread_t) * num_clientes);
    if (th == NULL) {
        sprintf(msg, "Error: no hay memoria suficiente para los objetos de datos de hilo\n");
        log_debug(msg);
        exit(5);
    }

    // fp = fopen(argv[3], "r");
    // check_null(fp, "Error al abrir el fichero de eventos");
    if ((fp = fopen(argv[3], "r")) == NULL) {
        perror("Error al abrir el fichero de eventos");
        exit(6);
    }

    // Creación de un hilo para cada cliente. Estos sí reciben como parámetro
    // un puntero a entero que será su id_cliente. Se crea dinámicamente uno
    // para cada hilo y se le asigna el contador del bucle
    for (i = 0; i < num_clientes; i++) {
        q = (datos_hilo*) malloc(sizeof(datos_hilo));
        check_null(q, "No hay memoria suficiente para los datos del hilo");
        q -> fp = fp;
        q -> id_cliente = i;
        check_error(pthread_create(&th[i], NULL, (void *) Cliente, q), "pthread_create");
    }

    // main espera a que terminen todos los buques
    for (i = 0; i < num_clientes; i++) {
        pthread_join(th[i], NULL);
    }
    free(th);
}
