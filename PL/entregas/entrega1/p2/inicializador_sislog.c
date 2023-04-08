/*
Cliente de RPC que inicializa el servidor de sislog
*/
#include <rpc/rpc.h>
#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <pthread.h>
#include "sislog.h"
#include "util.h"


int main(int argc, char *argv[]) {
	CLIENT* cl;
	faclevel params;
	char* ip_sislog;

	int max_facilidades;
	int max_niveles;

	// Comprobamos que se han pasado los parámetros adecuados

	if(argc != 4) {
		fprintf(stderr, "Forma de uso: %s <max_facilidades> <max_levels> <ip_sislog>\n", argv[0]);
		exit(1);
	}

	if(!valida_numero(argv[1]) || !valida_numero(argv[2])) {
		fprintf(stderr, "Error: Los parámetros <max_facilidades> y <max_levels> deben ser números\n");
		exit(3);
	}

	max_facilidades = atoi(argv[1]);
	max_niveles = atoi(argv[2]);

	if(max_facilidades <= 0 || max_facilidades > MAXFACILITIES) {
		fprintf(stderr, "El parametro  <max_facilidades> debe ser >=0 y <=%d\n", MAXFACILITIES);
		exit(3);
	}

	if(max_niveles <= 0 || max_niveles > MAXLEVELS) {
		fprintf(stderr,"El parametro  <max_levels> debe ser >=0 y <=%d\n", MAXLEVELS);
		exit(3);
	}

	ip_sislog = strdup(argv[3]);
	if(!valida_ip(argv[3])) {
		fprintf(stderr, "Error: El parámetro IP no es valido\n");
		exit(4);
	}

	// Conectamos con el servidor RPC pasándole los parámetros apropiados
	// para que inicialice sus estructuras de datos con el tamaño requerido
	cl = clnt_create(ip_sislog, SISLOG, PRIMERA, "udp");
	check_null(cl, "clnt_create");

	params.facilidad = max_facilidades;
	params.nivel = max_niveles;

	// Llamamos al procedimiento remoto inicializar_sislog
	Resultado* r = inicializar_sislog_1(&params, cl);
	check_null(r, "Error al inicializar sislog");

	if(r -> caso == 1) {
		fprintf(stderr, "%s\n", r -> Resultado_u.msg);
		exit(6);
	}

	printf("Sislog inicializado\n");
	clnt_destroy(cl);
	return 0;
}
