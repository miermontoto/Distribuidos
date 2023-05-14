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

estado_filosofo estado;
pthread_mutex_t mestado; // Mutex que protege las modificaciones al valor del estado del filosofo

// variable condicional que permite suspender al filosofo
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
void check_error(int ret, char *msg);
void check_error(int ret, char *msg, int ret_value);

void check_error(int ret, char *msg) {
	check_error(ret, msg, 1);
}

void check_error(int ret, char *msg, int ret_value) {
	if (ret < 0) {
		fprintf(stderr, "Filosofo %d: %s(%s)\n", idfilo, msg, strerror(errno));
		exit(ret_value);
	}
}


int main(int argc, char *argv[]) {
	pthread_t h1, h2; // objetos de datos de hilo

	procesaLineaComandos(argc, argv);
	inicializaciones();

	// Lanzamiento del hilo de comunicaciones del filósofo
	check_error(pthread_create(&h1, NULL, (void *)comunicaciones, (void *)NULL),
		"Falló al lanzar el hilo de comuni", 10);

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
	if (numero != 7) {
		fprintf(stderr, "Forma de uso: %s id_filosofo num_filosofos "
			"ip_siguiente puerto_siguiente "
			"puerto_local delay_conexion\n",
			lista[0]);
		fprintf(stderr, "Donde id_filosofo es un valor de 0 a n. "
			"El iniciador del anillo debe ser "
			"el filosofo con id=0\n");
		exit(1);
	} else {
		idfilo = atoi(lista[1]);
		numfilo = atoi(lista[2]);
		strcpy(siguiente_chain, lista[3]);
		puerto_siguiente_chain = (unsigned short)atoi(lista[4]);
		puerto_local = (unsigned short)atoi(lista[5]);
		delay = atoi(lista[6]);
		if ((numfilo < 2) || (numfilo > 8)) {
			fprintf(stderr, "El numero de filosofos debe ser >=2 y <8\n");
			exit(2);
		}

		if(delay == 0) {
			fprintf(stderr, "El delay de conexion debe ser >0 o error en atoi\n");
			exit(12);
		}

		if((idfilo < 0) || (idfilo >= numfilo)) {
			fprintf(stderr, "El id del filosofo debe ser >=0 y <numfilo\n");
			exit(3);
		}

		if((puerto_siguiente_chain < 1024) || (puerto_siguiente_chain > 65535)) {
			fprintf(stderr, "El puerto del siguiente filosofo debe ser >=1024 y <=65535\n");
			exit(4);
		}

		if((puerto_local < 1024) || (puerto_local > 65535)) {
			fprintf(stderr, "El puerto local debe ser >=1024 y <=65535\n");
			exit(5);
		}

		printf("Filosofo %d Valores leidos:\n", idfilo);
		printf("Filosofo %d   Numero filosofos: %d\n", idfilo, numfilo);
		printf("Filosofo %d   Dir. IP siguiente filosofo: %s\n",
			idfilo, siguiente_chain);
		printf("Filosofo %d   Puerto siguiente filosofo: %d\n",
			idfilo, puerto_siguiente_chain);
		printf("Filosofo %d   Puerto local: %d\n", idfilo, puerto_local);
		printf("Filosofo %d   Delay conexion: %d\n", idfilo, delay);
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

	while (numbocados < MAX_BOCADOS) {
		fprintf(stderr, "Filosofo %d: cambiando estado a "
			"queriendo comer\n",
			idfilo);
		cambiarEstado(queriendo_comer);
		esperarPalillos();
		// comiendo
		fprintf(stderr, "Filosofo %d: Comiendo\n", idfilo);
		sleep(5);
		numbocados++;
		cambiarEstado(dejando_comer);
		soltarPalillos();
		fprintf(stderr, "Filosofo %d: Pensando\n", idfilo);
		sleep(10);
	}
	fprintf(stderr, "Filosofo %d: Levantandose de la mesa\n", idfilo);

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
		default:;
	}
}


// Hilo de comunicaciones
void *comunicaciones(void) {
	int ret;
	unsigned char token[2] = {0, 0}; // APARTADO 0.1
	struct sockaddr_in next;
	struct hostent *host_info;
	int sockserver, sockant;
	int socknext;
	struct sockaddr_in servidor, anterior;
	int anterior_len;

	// 1. Crear socket de comunicación con el y listen
	sockserver = socket(AF_INET, SOCK_STREAM, 0);
	if (sockserver < 0) {
		fprintf(stderr, "Filosofo %d: No se pudo crear "
			"el socket de comunicación con el anterior "
			"en el anillo.\n",
			idfilo
		);
		exit(3);
	}
	servidor.sin_family = AF_INET;
	servidor.sin_addr.s_addr = htonl(INADDR_ANY);
	servidor.sin_port = htons(puerto_local);
	if (bind(sockserver, (struct sockaddr *)&servidor, sizeof(servidor)) < 0) {
		fprintf(stderr, "Filosofo %d: Error vinculando el "
			"socket de comunicación con el anterior en el anillo.\n",
			idfilo
		);
		exit(4);
	}
	check_error(listen(sockserver, SOMAXCONN), "Error en el listen", 18);

	// 2. Delay para permitir que el resto de procesos
	// se lancen y lleguen a crear su socket servidor
	check_error(sleep(delay), "Error en el sleep", 19);

	// 3. Conectar con el siguiente
	socknext = socket(AF_INET, SOCK_STREAM, 0);
	if (socknext < 0) {
		fprintf(stderr, "Filosofo %d: Error creando el"
			"socket de conexion con el siguiente. \n",
			idfilo
		);
		exit(5);
	}

	fprintf(stderr, "Filosofo %d: Direccion de conexion "
		"del siguiente filosofo %s  puerto: %d\n",
		idfilo, siguiente_chain, puerto_siguiente_chain
	);

	host_info = gethostbyname(siguiente_chain);
	if (host_info == NULL) {
		fprintf(stderr, "Filosofo %d: nombre de host desconocido: %s\n",
			idfilo, siguiente_chain);
		exit(3);
	}
	next.sin_family = host_info -> h_addrtype;
	memcpy((char*) &next.sin_addr, host_info -> h_addr, host_info -> h_length);
	next.sin_port = htons(puerto_siguiente_chain);
	if (connect(socknext, (struct sockaddr *)&next, sizeof(next)) < 0) {
		fprintf(stderr, "Filosofo %d: Error %d conectando "
			"con el filosofo siguiente\n",
			idfilo, errno
		);
		perror("Error conectando");
		exit(7);
	}

	// 4. Esperar a que se haya aceptado la conexión del anterior
	anterior_len = sizeof(anterior);
	sockant = accept(sockserver, (struct sockaddr *)&anterior,
					 (socklen_t *)&anterior_len);
	fprintf(stderr, "Filosofo %d: Llega conexion valor %d\n", idfilo, sockant);

	// Si se llega a este punto el ciclo está completo
	// 5. Si filósofo = ', inyectar token.
	if (idfilo == 0) {
		write(socknext, token, (size_t) sizeof(unsigned char) * 2); // APARTADO 0.1
	}

	while (1) {
		// 6. Esperar token
		ret = read(sockant, token, sizeof(unsigned char) * 2); // APARTADO 0.1
		if (ret != 2) { // APARTADO 0.1
			fprintf(stderr, "Filosofo %d: Error de lectura "
				"en el socket de conexion con el anterior nodo Ret=%d\n",
				idfilo, ret
			);
		}
		check_error(pthread_mutex_lock(&mestado), "Error en el lock", 19);
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
		}
		check_error(pthread_mutex_unlock(&mestado), "Error en el unlock", 21);
		if (ret == 2) { // APARTADO 0.1
			ret = write(socknext, token, sizeof(char) * 2); // APARTADO 0.1
			if (ret != 2) { // APARTADO 0.1
				fprintf(stderr, "Error de escritura "
					"en el socket de conexion con el siguiente nodo\n"
				);
			}
		}
	}
}
