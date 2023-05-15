#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "filodist.h"

/* variables globales */
int idfilo;								   // Identificador del filósofo
int numfilo;							   // Número de filósofos en la simulación
char siguiente_chain[45];				   // IP o nombre FQDN del siguiente filósofo en el anillo lógico
unsigned short int puerto_siguiente_chain; // Puerto donde enviar el testigo al siguiente filosofo
unsigned short int puerto_local;		   // Puerto local en donde deberemos recibir el testigo

// Delay incial antes de conectar con el siguiente
// filosofo en el anillo lógico. Este delay permite
// que el siguiente filósofo haya creado, vinculado(bind)
// y hecho el listen en su socket servidor
int delay;

int cuchara; // APARTADO 1: almacena la cuchara que sostiene el filósofo cuando condimenta.
estado_filosofo estado;
pthread_mutex_t mestado; // Mutex que protege las modificaciones al valor del estado del filosofo

// Variable condicional que permite suspender al filosofo
// hasta que se produce el cambio de estado efectivo
pthread_cond_t condestado;

/* prototipos funciones */
void procesaLineaComandos(int numero, char *lista[]);
void inicializaciones(void);
void* filosofo(void);
void esperarPalillos(void);
void soltarPalillos(void);
void cambiarEstado(estado_filosofo nuevoestado);
char palillosLibres(unsigned char token);
void alterarToken(unsigned char *tok, estado_filosofo nuevoestado);
void* comunicaciones(void);
void* esperarConexion(void);
void check_error(int ret, char *msg, int ret_value);
void printlog(char* msg); // APARTADO 0.2
char* estado2str(estado_filosofo estado); // APARTADO 0.2
void print_token(unsigned char token[2], estado_filosofo estado); // APARTADO 0.2
int cucharaLibre(unsigned char tok); // APARTADO 1
void esperarCuchara(void); // APARTADO 1
void soltarCuchara(void); // APARTADO 1


void printlog(char* msg) { // APARTADO 0.2
  // Imprime en stderr el mensaje recibido, precedido de un timestamp entero con precisión de milisegundos
  struct timeval tv;
  gettimeofday(&tv, NULL);
  fprintf(stderr, "[%ld.%06ld] Filosofo %d (%s): %s\n", tv.tv_sec, tv.tv_usec, idfilo, estado2str(estado), msg);
}


void print_token(unsigned char token[2], estado_filosofo estado) {
	// El tipo estado_filosofo es un enum declarado en filodist.h
	char buff[2][9];
	unsigned char tok;
	char msg[100];

	for (int byte = 0; byte < 2; byte++) {  // Para cada byte del token
		tok = token[byte];
		for (int i = 7; i>=0; i--) {        // Sacamos sus bits
			buff[byte][i] = (tok&1) ? '1' : '0';
			tok = tok >> 1;
		}
		buff[byte][8] = 0;                  // Añadir terminador de cadena
	}

	sprintf(msg, "transmite token = %s|%s", buff[0], buff[1]);
	printlog(msg);
}


char* estado2str(estado_filosofo estado) { // APARTADO 0.2
	switch(estado) {
		case no_sentado:
			return "NO_SENTADO";
		case queriendo_comer:
			return "QUERIENDO_COMER";
		case comiendo:
			return "COMIENDO";
		case dejando_comer:
			return "DEJANDO_COMER";
		case pensando:
			return "PENSANDO";
		case queriendo_condimentar: // APARTADO 1
			return "QUERIENDO_CONDIMENTAR";
		case condimentando: // APARTADO 1
			return "CONDIMENTANDO";
		case dejando_condimentar: // APARTADO 1
			return "DEJANDO_CONDIMENTAR";
		case hablando: // APARTADO 1
			return "HABLANDO";
		case esperando_irse: // APARTADO 2
			return "ESPERANDO_IRSE";
		case levantado: // APARTADO 2
			return "LEVANTADO";
		default:
			return "UNKNOWN";
	}
}


void check_error(int ret, char *msg, int ret_value) {
	if (ret < 0) {
		char msg_err[256]; // APARTADO 0.2
		sprintf(msg_err, "%s(%s)", idfilo, msg, strerror(errno)); // APARTADO 0.2
		printlog(msg_err); // APARTADO 0.2
		exit(ret_value);
	}
}


int main(int argc, char *argv[]) {
	pthread_t h1, h2; // objetos de datos de hilo

	procesaLineaComandos(argc, argv);
	inicializaciones();

	// Lanzamiento del hilo de comunicaciones del filósofo
	check_error(pthread_create(&h1, NULL, (void *)comunicaciones, (void *)NULL),
		"Falló al lanzar el hilo de comunicaciones", 10);

	// Lanzamiento del hilo principal de funcionamiento del filósofo
	check_error(pthread_create(&h2, NULL, (void *)filosofo, (void *)NULL),
		"Falló al lanzar el hilo filosofo", 10);

	// Sincronización con la terminación del hilo de comunicaciones y el
	// hilo que ejecuta la función filósofo
	check_error(pthread_join(h1, NULL), "pthread_join h1", 11);
	check_error(pthread_join(h2, NULL), "pthread_join h2", 11);

	return 0;
}


// Procesa la linea de comandos, almacena los valores leidos en variables
// globales e imprime los valores leidos
void procesaLineaComandos(int numero, char *lista[]) {
	char* msg[1024]; // APARTADO 0.2
	if (numero != 7) {
		sprintf(msg, "Forma de uso: %s id_filosofo num_filosofos " // APARTADO 0.2
			"ip_siguiente puerto_siguiente "
			"puerto_local delay_conexion",
			lista[0]);
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "Donde id_filosofo es un valor de 0 a n. " // APARTADO 0.2
			"El iniciador del anillo debe ser "
			"el filosofo con id=0");
		printlog(msg); // APARTADO 0.2
		exit(1);
	} else {
		idfilo = atoi(lista[1]);
		numfilo = atoi(lista[2]);
		strcpy(siguiente_chain, lista[3]);
		puerto_siguiente_chain = (unsigned short)atoi(lista[4]);
		puerto_local = (unsigned short)atoi(lista[5]);
		delay = atoi(lista[6]);
		if ((numfilo < 2) || (numfilo > 8)) {
			sprintf(msg, "El numero de filosofos debe ser >=2 y <8"); // APARTADO 0.2
			printlog(msg); // APARTADO 0.2
			exit(2);
		}

		if(delay <= 0) {
			sprintf(msg, "El delay de conexión debe ser >0 o error en atoi"); // APARTADO 0.2
			printlog(msg); // APARTADO 0.2
			exit(12);
		}

		if((idfilo < 0) || (idfilo >= numfilo)) {
			sprintf(msg, "El id del filósofo debe ser >=0 y <numfilo"); // APARTADO 0.2
			printlog(msg); // APARTADO 0.2
			exit(3);
		}

		if((puerto_siguiente_chain < 1024) || (puerto_siguiente_chain > 65535)) {
			sprintf(msg, "El puerto del siguiente filosofo debe ser >=1024 y <=65535"); // APARTADO 0.2
			printlog(msg); // APARTADO 0.2
			exit(4);
		}

		if((puerto_local < 1024) || (puerto_local > 65535)) {
			sprintf(msg, "El puerto local debe ser >=1024 y <=65535"); // APARTADO 0.2
			printlog(msg); // APARTADO 0.2
			exit(5);
		}

		sprintf(msg, "Valores leidos:"); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "\tNumero filosofos: %d\n", numfilo); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "\tDir. IP siguiente filosofo: %s", siguiente_chain); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "\tPuerto siguiente filosofo: %d", puerto_siguiente_chain); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "\tPuerto local: %d", puerto_local); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sprintf(msg, "\tDelay conexion: %d", delay); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
	}
}


// Inicializa el mutex, la variable condicional y el estado del filósofo
void inicializaciones(void) {
	check_error(pthread_mutex_init(&mestado, NULL), "pthread_mutex_init", 14);
	check_error(pthread_cond_init(&condestado, NULL), "pthread_cond_init", 14);
	estado = no_sentado;
}


// hilo principal del filosofo
void* filosofo(void) {
	int numbocados = 0;
	char msg[1024]; // APARTADO 0.2

	while (numbocados < MAX_BOCADOS) {
		sprintf(msg, "Ronda %d/%d", numbocados+1, MAX_BOCADOS);
		printlog(msg);
		cambiarEstado(queriendo_condimentar);
		sprintf(msg, "Cambiando estado a queriendo condimentar"); // APARTADO 1
		printlog(msg); // APARTADO 1
		esperarCuchara();
		// condimentando
		sprintf(msg, "Cambiando estado a condimentando"); // APARTADO 1
		printlog(msg); // APARTADO 1
		sleep(1);
		cambiarEstado(dejando_condimentar);
		soltarCuchara();
		sprintf(msg, "Cambiando estado a hablando"); // APARTADO 1
		printlog(msg); // APARTADO 1
		sleep(2);
		cambiarEstado(queriendo_comer);
		sprintf(msg, "Cambiando estado a queriendo comer"); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		esperarPalillos();
		// comiendo
		sprintf(msg, "Cambiando estado a comiendo"); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sleep(3);
		numbocados++;
		cambiarEstado(dejando_comer);
		soltarPalillos();
		sprintf(msg, "Cambiando estado a pensando"); // APARTADO 0.2
		printlog(msg); // APARTADO 0.2
		sleep(2);
	}

	cambiarEstado(levantado);
	sprintf(msg, "Levantandose de la mesa"); // APARTADO 0.2
	printlog(msg); // APARTADO 0.2

	return NULL; // Suprime el warning de compilación
}


// Sincronización con el cambio de estado a "comiendo"
void esperarPalillos(void) {
	check_error(pthread_mutex_lock(&mestado), "pthread_mutex_lock (esperarPalillos)", 15);
	while (estado != comiendo)
		check_error(pthread_cond_wait(&condestado, &mestado), "pthread_cond_wait (esperarPalillos)", 15);
	check_error(pthread_mutex_unlock(&mestado), "pthread_mutex_unlock (esperarPalillos)", 15);
}


// Sincronización con el cambio de estado a "pensando"
void soltarPalillos(void) {
	check_error(pthread_mutex_lock(&mestado), "pthread_mutex_lock (soltarPalillos)", 16);
	while (estado != pensando)
		check_error(pthread_cond_wait(&condestado, &mestado), "pthread_cond_wait (soltarPalillos)", 16);
	check_error(pthread_mutex_unlock(&mestado), "pthread_mutex_unlock (soltarPalillos)", 16);
}


void esperarCuchara() { // APARTADO 1
	check_error(pthread_mutex_lock(&mestado), "pthread_mutex_lock (esperarCuchara)", 15);
	while (estado != condimentando)
		check_error(pthread_cond_wait(&condestado, &mestado), "pthread_cond_wait (esperarCuchara)", 15);
	check_error(pthread_mutex_unlock(&mestado), "pthread_mutex_unlock (esperarCuchara)", 15);
}


void soltarCuchara() { // APARTADO 1
	check_error(pthread_mutex_lock(&mestado), "pthread_mutex_lock (soltarCuchara)", 16);
	while (estado != hablando)
		check_error(pthread_cond_wait(&condestado, &mestado), "pthread_cond_wait (soltarCuchara)", 16);
	check_error(pthread_mutex_unlock(&mestado), "pthread_mutex_unlock (soltarCuchara)", 16);
}


void cambiarEstado(estado_filosofo nuevoestado) {
	check_error(pthread_mutex_lock(&mestado), "pthread_mutex_lock (cambiarEstado)", 17);
	estado = nuevoestado;
	check_error(pthread_mutex_unlock(&mestado), "pthread_mutex_unlock (cambiarEstado)", 17);
}


// Comprueba el estado de los palillos necesarios
// para que el filósofo pueda comer
char palillosLibres(unsigned char token) {
	int pos;
	unsigned char ocupado = 1;
	unsigned char tokenorg = token;

	pos = idfilo;
	// Desplazamiento a la derecha
	// Se rellena con ceros por la izquierda
	token = token >> pos;
	ocupado &= token;
	if (!ocupado) {
		ocupado = 1;
		if (idfilo > 0) pos = idfilo - 1;
		else pos = numfilo - 1;
		token = tokenorg >> pos;
		ocupado &= token;
	}
	return (!ocupado);
}


/*
 * Devuelve el valor de la primera cuchara por la derecha que
 * está libre en ese determinado momento.
 *
 * Para obtenerlo, se desplaza el token a la derecha hasta
 * dejar en el byte tan solo los dos bits que indican el estado
 * de las cucharas.
 *
 * Estos bits pueden almacenar un valor entre 0 y 3, que indican
 * las cucharas que quedan libres. Cuando la cuchara derecha está
 * libre, el valor es o 0 o 2. Cuando la cuchara izquierda está
 * libre, el valor es 1. Cuando el valor es 3, ninguna de las
 * dos cucharas está libre.
*/
int cucharaLibre(unsigned char tok) { // APARTADO 1
	switch(tok >> 6) {
		case 0b00: return 1;
		case 0b01: return 2;
		case 0b10: return 1;
		case 0b11: return 0;
	}
}


// Cambia el token reservando o liberando los recursos que el filósofo
// utiliza en función del nuevo estado al que pasa
void alterarToken(unsigned char *tok, estado_filosofo nuevoestado) {
	int pos;
	unsigned char bit;
	unsigned char tokenaux;

	switch (nuevoestado) {
		case comiendo:
			pos = idfilo;
			bit = 1;
			bit = bit << pos;
			*tok |= bit;
			if (idfilo > 0) pos = idfilo - 1;
			else pos = numfilo - 1;
			bit = 1;
			bit = bit << pos;
			*tok |= bit;
			break;
		case pensando:
			tokenaux = ~*tok;
			pos = idfilo;
			bit = 1;
			bit = bit << pos;
			tokenaux |= bit;
			if (idfilo > 0) pos = idfilo - 1;
			else pos = numfilo - 1;
			bit = 1;
			bit = bit << pos;
			tokenaux |= bit;
			*tok = ~tokenaux;
			break;
		case condimentando: // APARTADO 1
			cuchara = cucharaLibre(*tok); // Se almacena la cuchara que se va a usar
			*tok = *tok | (1 << (5 + cuchara));
			break;
		case hablando: // APARTADO 1
			*tok = *tok - (1 << (5 + cuchara));
			break;
		case esperando_irse: // APARTADO 2
			*tok = *tok + 1;
			break;
		default:;
	}
}


// Hilo de comunicaciones
void *comunicaciones(void) {
	int ret;
	unsigned char token[2] = {0, 0}; // APARTADO 0.1
	unsigned char old_token[2]; // APARTADO 0.2
	struct sockaddr_in next;
	struct hostent *host_info;
	int sockserver, sockant;
	int socknext;
	struct sockaddr_in servidor, anterior;
	int anterior_len;
	char* msg[1024]; // APARTADO 0.2

	// 1. Crear socket de comunicación con el y listen
	sockserver = socket(AF_INET, SOCK_STREAM, 0);
	check_error(sockserver, "No se pudo crear el socket de comunicación con el anterior en el anilo", 3);

	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	servidor.sin_port = htons(puerto_local);
	check_error(bind(sockserver, (struct sockaddr *)&servidor, sizeof(servidor)),
		"Error vinculando el socket de comunicación con el anterior en el anillo", 4);
	check_error(listen(sockserver, SOMAXCONN), "Error en el listen", 18);

	// 2. Delay para permitir que el resto de procesos
	// se lancen y lleguen a crear su socket servidor
	check_error(sleep(delay), "Error en el sleep", 19);

	// 3. Conectar con el siguiente
	socknext = socket(AF_INET, SOCK_STREAM, 0);
	check_error(socknext, "Error creando el socket de conexion con el siguiente", 5);

	sprintf(msg, "Dirección de conexión " // APARTADO 0.2
		"del siguiente filosofo: %s, puerto: %d",
		siguiente_chain, puerto_siguiente_chain
	);
	printlog(msg); // APARTADO 0.2

	host_info = gethostbyname(siguiente_chain);
	if (host_info == NULL) {
		sprintf(msg, "nombre de host desconocido: %s", // APARTADO 0.2
			siguiente_chain);
		printlog(msg); // APARTADO 0.2
		exit(3);
	}

	next.sin_family = host_info -> h_addrtype;
	memcpy((char*) &next.sin_addr, host_info -> h_addr, host_info -> h_length);
	next.sin_port = htons(puerto_siguiente_chain);

	check_error(connect(socknext, (struct sockaddr *)&next, sizeof(next)),
		"Error conectando con el filosofo siguiente", 7);

	// 4. Esperar a que se haya aceptado la conexión del anterior
	anterior_len = sizeof(anterior);
	sockant = accept(sockserver, (struct sockaddr *)&anterior,
					 (socklen_t *)&anterior_len);
	sprintf(msg, "Llega conexión con valor %d", sockant); // APARTADO 0.2
	printlog(msg); // APARTADO 0.2

	// Si se llega a este punto el ciclo está completo
	// 5. Si filósofo = 0, inyectar token.
	if (idfilo == 0) {
		if(write(socknext, token, (size_t) sizeof(unsigned char) * 2) < 0) {
			sprintf(msg, "Error de escritura " // APARTADO 0.2
				"en el socket de conexion con el siguiente nodo (Ret=%d)",
				idfilo, ret
			);
			printlog(msg); // APARTADO 0.2
		} // APARTADO 0.1
	}

	while (1) {
		// 6. Esperar token
		ret = read(sockant, token, sizeof(unsigned char) * 2); // APARTADO 0.1
		if (ret != 2) { // APARTADO 0.1
			sprintf(msg, "Error de lectura " // APARTADO 0.2
				"en el socket de conexion con el anterior nodo (Ret=%d)",
				idfilo, ret
			);
			printlog(msg); // APARTADO 0.2
		}

		check_error(pthread_mutex_lock(&mestado), "Error en el lock", 19);
		memcpy(old_token, token, sizeof(unsigned char) * 2); // APARTADO 0.2

		if (estado == queriendo_comer) {
			// Alterar token cuando esten libres y avanzar
			// cambiar estado a comiendo y señalar la condición
			if (palillosLibres(token[1])) { // APARTADO 0.1
				alterarToken(&token[1], comiendo); // APARTADO 0.1
				estado = comiendo;
				check_error(pthread_cond_signal(&condestado), "Error en el signal", 20);
			}
		} else if (estado == dejando_comer) {
			// Alterar token y avanzar
			// Cambiar estado a pensando y señalar la condicion
			alterarToken(&token[1], pensando); // APARTADO 0.1
			estado = pensando;
			check_error(pthread_cond_signal(&condestado), "Error en el signal", 20);
		} else if (estado == queriendo_condimentar) { // APARTADO 1
			if (cucharaLibre(token[0])) {
				alterarToken(&token[0], condimentando); // APARTADO 0.1
				estado = condimentando;
				check_error(pthread_cond_signal(&condestado), "Error en el signal", 20);
			}
		} else if (estado == dejando_condimentar) { // APARTADO 1
			alterarToken(&token[0], hablando); // APARTADO 0.1
			estado = hablando;
			check_error(pthread_cond_signal(&condestado), "Error en el signal", 20);
		} else if (estado == levantado) { // APARTADO 2
			alterarToken(&token[0], esperando_irse);
			estado = esperando_irse;
			check_error(pthread_cond_signal(&condestado), "Error en el signal", 20);
		}

		if (idfilo == 0 && (token[0] & 0b00000111) == 0b00000111) { // APARTADO 2
			sprintf(msg, "Enviando token especial de finalización");
			printlog(msg);
			token[0] = 0b11111111;
		}

		check_error(pthread_mutex_unlock(&mestado), "Error en el unlock", 21);
		if (ret == 2) { // APARTADO 0.1
			ret = write(socknext, token, sizeof(char) * 2); // APARTADO 0.1
			if (ret != 2) { // APARTADO 0.1
				sprintf(msg, "Error de escritura " // APARTADO 0.2
					"en el socket de conexion con el siguiente nodo");
				printlog(msg); // APARTADO 0.2
			}

			if(memcmp(token, old_token, sizeof(unsigned char) * 2) != 0) { // APARTADO 0.2
				print_token(token, estado);
			}

			if (token[0] == 0b11111111) { // APARTADO 2
				sprintf(msg, "Saliendo");
				printlog(msg);
				exit(EXIT_SUCCESS);
			}
		}
		usleep(1000);
	}
}
