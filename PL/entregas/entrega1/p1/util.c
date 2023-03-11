#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "util.h"

// Función de utilidad que determina si los caracteres de una cadena son todos numericos
int valida_numero(char *str) {
    register int i = 0;
	while (str[i] != '\0') {
		if (str[i] < '0' || str[i] > '9') return FALSO;
		i++;
	}
	return CIERTO;
}

// Función de utilidad que valida si una cadena de caracteres representa una IPv4 valida
int valida_ip(char *ip)
{
    ip = strtok(ip, ".");
	register int l = 0;
	while(ip != NULL) {
		if (!valida_numero(ip)) return FALSO;
		if (atoi(ip) < 0 || atoi(ip) > 255) return FALSO;
		ip = strtok(NULL, ".");
		l++;
	}
	return l == 4;
}

// Función de utilidad, para generar los tiempos aleatorios entre un
// min y un max
double randRange(double min, double max)
{
	return min + (rand() / (double) RAND_MAX * (max - min + 1));
}


// Función de utilidad para depuración. Emite por pantalla el mensaje
// que se le pasa como parámetro, pero pone delante del mensaje un
// timestamp, para poder ordenar la salida por si saliera desordenada
//
// Ejemplo de uso:
//
//  log_debug("Mensaje a mostrar por pantalla")
//
void log_debug(char *msg) {
	struct timespec t;
	clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);
	printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}

void check_error(int ret, char *msg) {
	if (ret < 0) exit_error(msg);
}

void exit_error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void check_null(void *ptr, char *msg) {
	if (ptr == NULL) exit_error(msg);
}

void check_not_null(void *ptr, char *msg) {
	if (ptr != NULL) exit_error(msg);
}

void check_not_natural(int ret, char *msg) {
	if (ret <= 0) exit_error(msg);
}

void p_check_null(void *ptr, char *msg) {
	if (ptr == NULL) p_exit_error(msg);
}

void p_exit_error(char *msg) {
	perror(msg);
	pthread_exit(NULL);
}
