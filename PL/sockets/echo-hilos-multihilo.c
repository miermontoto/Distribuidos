#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

struct Cola {
    int size;
    int *q;
    int head;
    int tail;
    pthread_mutex_t m;
    sem_t hay_espacio, hay_algo;
};

typedef struct Cola cola_trabajos;

void inicializar_cola(cola_trabajos *c, int size)
{
    if (size <= 0) {
        perror("Tamaño de cola no válido");
        exit(EXIT_FAILURE);
    }

    c -> size = size;
    c -> q = malloc(size * sizeof(int));

    if (c -> q == NULL) {
        perror("Al reservar memoria para la cola");
        exit(EXIT_FAILURE);
    }

    c -> head = 0;
    c -> tail = 0;

    if (pthread_mutex_init(&c -> m, NULL) != 0) {
        perror("Al inicializar mutex");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&c -> hay_espacio, 0, size) < 0) {
        perror("Al inicializar semáforo");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&c -> hay_algo, 0, 0) < 0) {
        perror("Al inicializar semáforo");
        exit(EXIT_FAILURE);
    }
}

void destruir_cola(cola_trabajos *c)
{
    free(c -> q);
    if (c -> q == NULL) {
        perror("Al liberar memoria para la cola");
        exit(EXIT_FAILURE);
    }
    if (pthread_mutex_destroy(&c -> m) != 0) {
        perror("Al destruir mutex");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&c -> hay_espacio) < 0) {
        perror("Al destruir semáforo");
        exit(EXIT_FAILURE);
    }

    if (sem_destroy(&c -> hay_algo) < 0) {
        perror("Al destruir semáforo");
        exit(EXIT_FAILURE);
    }
}

int obtener_dato_cola(cola_trabajos *c)
{
    int dato;

    if (c == NULL) {
        perror("Cola no inicializada");
        exit(EXIT_FAILURE);
    }

    if (sem_wait(&c -> hay_algo) < 0) {
        perror("Al esperar semáforo");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_lock(&c -> m) != 0) {
        perror("Al bloquear mutex");
        exit(EXIT_FAILURE);
    }

    dato = c -> q[c -> tail];
    c -> tail = (c -> tail + 1) % c -> size;

    if (pthread_mutex_unlock(&c -> m) != 0) {
        perror("Al desbloquear mutex");
        exit(EXIT_FAILURE);
    }

    if (sem_post(&c -> hay_espacio) < 0) {
        perror("Al incrementar semáforo");
        exit(EXIT_FAILURE);
    }

    return dato;
}

void insertar_dato_cola(cola_trabajos *c, int dato)
{
    if (c == NULL) {
        perror("Cola no inicializada");
        exit(EXIT_FAILURE);
    }

    if (sem_wait(&c -> hay_espacio) < 0) {
        perror("Al esperar semáforo");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_lock(&c -> m) != 0) {
        perror("Al bloquear mutex");
        exit(EXIT_FAILURE);
    }

    c -> q[c -> head] = dato;
    c -> head = (c -> head + 1) % c -> size;

    if (pthread_mutex_unlock(&c -> m) != 0) {
        perror("Al desbloquear mutex");
        exit(EXIT_FAILURE);
    }

    if (sem_post(&c -> hay_algo) < 0) {
        perror("Al incrementar semáforo");
        exit(EXIT_FAILURE);
    }
}

int main() {


}
