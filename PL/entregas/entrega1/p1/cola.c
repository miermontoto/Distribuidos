#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cola.h"
#include "util.c"

// El contenido de este fichero implementa las funciones de la cola.
// Es prácticamente igual a la cola que tienes hecha de las prácticas
// de laboratorio pero adaptándola a la estructura de datos dato_cola
// usada en este ejercicio.
//
// Mira el fichero cola.h para ver las estructuras de datos a utilizar

void inicializar_cola(Cola *cola, int tam_cola)
{
    if (tam_cola <= 0) exit_error("Tamaño de cola inválido");

    cola -> datos = (dato_cola **) malloc(tam_cola * sizeof(dato_cola *));
    check_null(cola -> datos, "malloc cola->datos");

    cola -> head = 0;
    cola -> tail = 0;
    cola -> tam_cola = tam_cola;

    check_error(pthread_mutex_init(&(cola -> mutex_head), NULL), "Pthread init mutex_head");
    check_error(pthread_mutex_init(&(cola -> mutex_tail), NULL), "Pthread init mutex_tail");
    check_error(sem_init(&(cola -> num_huecos), 0, tam_cola), "Semaphore init num_huecos");
    check_error(sem_init(&(cola -> num_ocupados), 0, 0), "Semaphore init num_ocupados");
}


void destruir_cola(Cola *cola)
{
    free(cola -> datos);
    check_not_null(cola -> datos, "free cola->datos");
    check_error(pthread_mutex_destroy(&(cola -> mutex_head)), "Pthread destroy mutex_head");
    check_error(pthread_mutex_destroy(&(cola -> mutex_tail)), "Pthread destroy mutex_tail");
    check_error(sem_destroy(&(cola -> num_huecos)), "Semaphore destroy num_huecos");
    check_error(sem_destroy(&(cola -> num_ocupados)), "Semaphore destroy num_ocupados");
}

void insertar_dato_cola(Cola *cola, dato_cola * dato)
{
    //check_null(dato, "dato == NULL");
    check_null(cola, "cola == NULL");
    check_null(cola -> datos, "cola->datos == NULL");
    check_error(sem_wait(&(cola -> num_huecos)), "sem_wait num_huecos");
    check_error(pthread_mutex_lock(&(cola -> mutex_head)), "pthread_mutex_lock mutex_head");

    cola -> datos[cola -> head] = dato;
    cola -> head = (cola -> head + 1) % cola -> tam_cola;

    check_error(pthread_mutex_unlock(&(cola -> mutex_head)), "pthread_mutex_unlock mutex_head");
    check_error(sem_post(&(cola -> num_ocupados)), "sem_post num_ocupados");
}


dato_cola * obtener_dato_cola(Cola *cola)
{
    dato_cola *p;

    check_null(cola, "cola == NULL");
    check_null(cola -> datos, "cola->datos == NULL");
    check_error(sem_wait(&(cola -> num_ocupados)), "sem_wait num_ocupados");
    check_error(pthread_mutex_lock(&(cola -> mutex_tail)), "pthread_mutex_lock mutex_tail");

    p = cola -> datos[cola -> tail];
    cola -> tail = (cola -> tail + 1) % cola -> tam_cola;

    check_error(pthread_mutex_unlock(&(cola -> mutex_tail)), "pthread_mutex_unlock mutex_tail");
    check_error(sem_post(&(cola -> num_huecos)), "sem_post num_huecos");

    return(p);
}
