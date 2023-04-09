
#include <time.h>
#include "util.h"
#include "sislog.h"

// Funciones auxiliares para inicializar arrays
int inicializado = 0;

// Matriz de enteros donde se va
// a guardar la contabilidad de eventos por
// facilidad y nivel
int** contabilidad_eventos;

// Nombres de ficheros, de facilidades y de niveles
int numfacilities;
int numlevels;

char* facilities_file_names[MAXFACILITIES] = {
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

char* facilities_names[MAXFACILITIES] = {
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

char* level_names[MAXLEVELS] = {
    "emerg",
    "alert",
    "crit",
    "err",
    "warning",
    "notice",
    "info",
    "debug"
};

// Implementación de los servicios
// -------------------------------

// Esta función es llamada internamente, no por el cliente
// por lo que no hace falta verificar si los parámetros están
// dentro de los límites
void inicializar_matriz_eventos(int **p, int numfac, int numlevels) {
    register int i, j;

    for (i = 0; i < numfac; i++) {
        for (j = 0; j < numlevels; j++) p[i][j] = 0;
    }
}

int init() {
    register int i;

    // Si no estaba ya inicializada, se reserva espacio de memoria para
    // la matriz de eventos y se llama a la función anterior para incializarla
    // con ceros.
    if (!inicializado) {
        contabilidad_eventos = (int**) malloc(numfacilities * sizeof(int*));
        for (i = 0; i < numfacilities; i++) {
            contabilidad_eventos[i] = (int*) malloc(numlevels * sizeof(int));
        }
        inicializar_matriz_eventos(contabilidad_eventos, numfacilities, numlevels);
        inicializado = 1;
        return CIERTO;
    }
    return FALSO;
}


// Comprueba si el numero de facilidades y niveles no excede el valor maximo permitido
// y si no lo está se devuelve un mensaje de error.
// En caso de estar dentro de los limites, llama a la función anterior para incializar
// la matriz de recuento de eventos y se devuelve un resultado de exito
Resultado* inicializar_sislog_1_svc(faclevel* q, struct svc_req* pet) {
    static Resultado r;

    if (q -> facilidad > MAXFACILITIES) {
        r.caso = 1;
        r.Resultado_u.msg = "ERROR: Al inicializar sislog. El numero maximo de facilidades no puede ser <=0";
        return &r;
    }

    if (q -> nivel > MAXLEVELS) {
        r.caso = 1;
        r.Resultado_u.msg = "ERROR: Al inicializar sislog. El numero maximo de niveles no puede ser <=0";
        return &r;
    }

    if (q -> facilidad <= 0) {
        r.caso = 1;
        r.Resultado_u.msg = "ERROR: Al inicializar sislog. El numero maximo de facilidades no puede ser <=0";
        return &r;
    }

    if (q -> nivel <= 0) {
        r.caso = 1;
        r.Resultado_u.msg = "ERROR: Al inicializar sislog. El numero maximo de niveles no puede ser <=0";
        return &r;
    }

    numfacilities = q -> facilidad;
    numlevels = q -> nivel;

    if(init()) {
        // Antes de retornar, para depuración, mostramos por pantalla la matriz de
        // contadores llamando a la función mostrar_recuento_eventos (util.c)
        mostrar_recuento_eventos(q -> facilidad, q -> nivel,
            facilities_names, level_names, contabilidad_eventos);
        r.caso = 0;
        r.Resultado_u.valor = 0;
    } else {
        r.caso = 1;
        r.Resultado_u.msg = "ERROR: Al inicializar sislog. Ya estaba inicializado";
    }

    return &r;
}


// Comprueba si el evento que se desea registrar tiene un numero de facilidad
// y nivel dentro de los admisibles (retorna un mensaje de error si no)
// Si todo es correcto debe crear la línea de registro en el fichero correspondiente
// (lo que implica también la hora de recepción del evento, según el formato
// especificado en el enunciado)
// Tras ello, incrementará el contador correspondiente en la matriz de eventos y devolverá
// un resultado de exito
Resultado* registrar_evento_1_svc(eventsislog* evt, struct svc_req* peticion) {
    static Resultado res;
    FILE* fp;
    char* fechahora;
    time_t timeraw;

    // Comprobar que el número de facilidad y nivel están dentro de los límites
    if ((evt -> facilidad < 0) || (evt -> facilidad > (numfacilities - 1))) {
        res.caso = 1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de facilidad esta fuera de rango", evt -> facilidad);
    } else if ((evt -> nivel < 0) || (evt -> nivel > (numlevels - 1))) {
        res.caso = 1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de nivel esta fuera de rango", evt -> nivel);
    } else {
        // Abrir el fichero correspondiente a la facilidad
        fp = fopen(facilities_file_names[evt -> facilidad], "a");
        check_null(fp, "Error al abrir el fichero de registro");

        // Obtener la fecha y hora actual
        timeraw = time(NULL);
        fechahora = ctime(&timeraw);
        fechahora[strlen(fechahora) - 1] = '\0';

        // Escribir la línea de registro en el fichero
        fprintf(fp, "%s:%s:%s:%s\n", facilities_names[evt -> facilidad],
            level_names[evt -> nivel], fechahora, evt -> msg);
        check_error(fclose(fp), "Error al cerrar el fichero de registro");

        // Incrementar el contador de eventos para la facilidad y nivel dados
        contabilidad_eventos[evt -> facilidad][evt -> nivel]++;
        res.caso = 0;
    }

    // Puesto que `estadis` no funciona y no se podría observar
    // el estado de la matriz de contadores después de la inserción,
    // se muestra manualmente la matriz por pantalla después de
    // limpiar la consola.
    printf("\e[1;1H\e[2J");
    mostrar_recuento_eventos(numfacilities, numlevels,
        facilities_names, level_names, contabilidad_eventos);

    return(&res);
}

// Funciones de apoyo para las estadísticas
// ----------------------------------------

// Devuelve el total de eventos contabilizados para una facilidad determinada
Resultado* obtener_total_facilidad_1_svc(int* fac, struct svc_req* peticion) {
    static Resultado res;
    register int i;

    if ((*fac < 0) || (*fac > (numfacilities - 1)))  {
        res.caso = 1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de facilidad esta fuera de rango", *fac);
    } else {
        res.caso = 0;
        res.Resultado_u.valor = 0;
        // Computar la suma de los contadores para la facilidad dada
        for(i = 0; i < numlevels; i++) {
            res.Resultado_u.valor += contabilidad_eventos[*fac][i];
        }
    }

    return(&res);
}

// Devuelve el número de eventos contabilizados para un nivel determinado
// independientemente de su origen (facilidad)
Resultado* obtener_total_nivel_1_svc(int* level, struct svc_req* peticion) {
    static Resultado res;
    register int i;

    if ((*level < 0) || (*level > (numlevels - 1))) {
        res.caso = 1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de nivel esta fuera de rango", *level);
    } else {
        // Computar la suma de los contadores para la facilidad dada
        res.caso = 0;
        for(i = 0; i < numfacilities; i++) {
            res.Resultado_u.valor += contabilidad_eventos[i][*level];
        }
    }

    return(&res);
}


// Devuelve el total de eventos contabilizados para una facilidad y un nivel dados
Resultado* obtener_total_facilidadnivel_1_svc(faclevel* q, struct svc_req* peticion) {
    static Resultado res;

    if ((q -> facilidad < 0) || (q -> facilidad > (numfacilities - 1))) {
        res.caso = 1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de facilidad esta fuera de rango", q -> facilidad);
    }
    else if ((q -> nivel < 0) || (q -> nivel > (numlevels - 1))) {
        res.caso=1;
        res.Resultado_u.msg = (char *) malloc(sizeof(char)*MAXMSG);
        sprintf(res.Resultado_u.msg, "ERROR: El valor %d de nivel esta fuera de rango", q -> nivel);
    } else {
        // Obtener el contador solicitado
        res.caso = 0;
        res.Resultado_u.valor = contabilidad_eventos[q -> facilidad][q -> nivel];
    }

    return(&res);
}

// El resto de funciones se suministran y no necesitas modificarlas

Resultado* obtener_num_facilidades_1_svc(void* q, struct svc_req* peticion) {
    static Resultado res;

    res.caso = 0;
    res.Resultado_u.valor = numfacilities;

    return(&res);
}


Resultado * obtener_num_niveles_1_svc(void* q, struct svc_req* peticion) {
    static Resultado res;

    res.caso = 0;
    res.Resultado_u.valor = numlevels;

    return(&res);
}


// Obtiene el nombre de una facilidad dado su número de facilidad
Resultado* obtener_nombre_facilidad_1_svc(int* n, struct svc_req* peticion) {
    static Resultado res;

    if ((*n < 0) || (*n > (numfacilities - 1))) res.Resultado_u.msg = "ERROR en id facilidad";
    else res.Resultado_u.msg = facilities_names[*n];
    res.caso = 1;

    return(&res);
}


// Obtiene el nombre de una facilidad dado su número de facilidad
Resultado* obtener_nombre_nivel_1_svc(int* n, struct svc_req* peticion) {
    static Resultado res;

    if ((*n < 0) || (*n > (numlevels - 1))) res.Resultado_u.msg = "ERROR en id nivel";
    else res.Resultado_u.msg = level_names[*n];
    res.caso = 1;

    return(&res);
}
