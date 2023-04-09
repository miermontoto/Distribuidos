// Archivos de cabecera para manipulación de sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include "util.h"

#define TAMLINEA 1024
#define FALSO 0
#define CIERTO 1
#define SINASIGNAR -1

// tipo de datos que recibiran los hilos lectores
struct datos_hilo
{
	FILE* fp;
	struct sockaddr* dserv;
};
typedef struct datos_hilo datos_hilo;

//
// VARIABLES GLOBALES
//

// IP del proceso syslog
char* ip_syslog = NULL;

// Puerto en el que espera el proceso syslog los
int puerto_syslog;

// Numero de hilos lectores
int nhilos;

pthread_t* th = NULL;

// Es o no orientado a conexion
unsigned short es_stream = CIERTO;

// nombre del fichero fuente de eventos
char* fich_eventos = NULL;

// handler de archivo
FILE* fp = NULL;

pthread_mutex_t file_read_mutex;

void procesa_argumentos(int argc, char* argv[]) {
	if (argc != 6) {
		printf("Uso: %s <ip_sislog> <puerto_sislog> <t|u> <nhilos> <fich_eventos>\n", argv[0]);
		exit(EXIT_SUCCESS);
	}

	ip_syslog = argv[1];
	if(!valida_ip(ip_syslog)) exit_error("IP inválida");

	if(!valida_numero(argv[2])) exit_error("Puerto inválido");
	puerto_syslog = atoi(argv[2]);
	if(puerto_syslog < 1024 || puerto_syslog > 65535) exit_error("Puerto inválido");

	if(strcmp(argv[3], "t") == 0) es_stream = CIERTO;
	else if(strcmp(argv[3], "u") == 0) es_stream = FALSO;
	else exit_error("Tipo de socket inválido");

	if(!valida_numero(argv[4])) exit_error("Número de hilos inválido");
	nhilos = atoi(argv[4]);
	check_value(nhilos, "Número de hilos inválido", 1);

	fich_eventos = argv[5];
	fp = fopen(fich_eventos, "r");
	check_null(fp, "Error al abrir el fichero de eventos");
}

void salir_bien(int s) { // No se comprueban errores porque se está saliendo
	pthread_mutex_destroy(&file_read_mutex);
	fclose(fp);
	free(th);
	exit(EXIT_SUCCESS);
}

void* hilo_lector(datos_hilo *p) {
	//int enviados;
	char buffer[TAMLINEA];
	char *s = NULL;
	int sock_dat;

	do {
		bzero(buffer, TAMLINEA);
		// Leer la siguiente linea del fichero con fgets
		// (haciendo exclusión mutua con otros hilos)
		// El fichero (ya abierto por main) se recibe en uno de los parámetros
		check_error(pthread_mutex_lock(&file_read_mutex), "Error en pthread_mutex_lock");
		s = fgets(buffer, TAMLINEA, p -> fp);
		check_error(pthread_mutex_unlock(&file_read_mutex), "Error en pthread_mutex_unlock");

		if (s != NULL) {
			// La IP y puerto del servidor están en una estructura sockaddr_in
			// que se recibe en uno de los parámetros
			if (es_stream) { // Enviar la línea por un socket TCP
				sock_dat = socket(AF_INET, SOCK_STREAM, 0);
				check_error(sock_dat, "Error en socket TCP");
				check_error(connect(sock_dat, p -> dserv, sizeof(struct sockaddr_in)), "Error en connect");
				check_error(send(sock_dat, s, strlen(s), 0), "Error en el send");
			} else { // Enviar la línea por un socket UDP
				sock_dat = socket(AF_INET, SOCK_DGRAM, 0);
				check_error(sock_dat, "Error en socket UDP");
				check_error(sendto(sock_dat, s, strlen(s), 0, p -> dserv, sizeof(struct sockaddr_in)), "Error en sendto");
			}

			check_error(close(sock_dat), "Error al cerrar el socket de datos");
			printf("%s", s); // Para depuración, imprimimos la línea que hemos enviado
		}
	} while (s); // Mientras no se llegue al final del fichero

	return NULL;
}


// La función main crea los hilos lector, pasándoles los parámetros necesarios,
// y espera a que terminen
int main(int argc, char* argv[]) {
	register int i;
	datos_hilo q;
	struct sockaddr_in d_serv;
	//socklen_t ldir;
	//char buffer[50];

	signal(SIGINT, salir_bien); // Instalar la rutina de tratamiento de la señal SIGINT
	procesa_argumentos(argc, argv); // Procesar los argumentos de la línea de comandos

	printf("IP servidor %s, es_stream=%d\n", ip_syslog, es_stream);
	if ((fp = fopen(fich_eventos, "r")) == NULL) {
		perror("Error al abrir el fichero de eventos");
		exit(6);
	}

	// Creamos espacio para los objetos de datos de hilo
	th = (pthread_t *) malloc(nhilos * sizeof(pthread_t));

	// Incicializamos los datos que le vamos a pasar como parámetro a los hilo_lector
	// (se pasa a todos el mismo parámetro)
	// Se utiliza el inet_pton tanto para guardar la IP como para comprobar que es válida.
	q.fp = fp;
	q.dserv = (struct sockaddr *) &d_serv;
	check_value(inet_pton(AF_INET, ip_syslog, &d_serv.sin_addr), "Error en inet_pton", 1);
	d_serv.sin_port = htons(puerto_syslog);
	d_serv.sin_family = AF_INET;

	check_error(pthread_mutex_init(&file_read_mutex, NULL), "Error en pthread_mutex_init");

	for (i = 0; i < nhilos; i++) { // Lanzar hilos lectores
		check_error(pthread_create(&th[i], NULL, (void *) hilo_lector, (void *) &q), "Error al lanzar el hilo lector");
	}

	// Una vez lanzados todos, hacemos un join sobre cada uno de ellos
	for (i = 0; i < nhilos; i++) pthread_join(th[i], NULL);

	// Al llegar aquí, todos los hilos han terminado, se limpia y se sale.
	salir_bien(0);
	return 0;
}
