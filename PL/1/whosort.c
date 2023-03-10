/*
EPI GIJÓN
GRADO EN INGENIERIA INFORMATICA
SISTEMAS DISTRIBUIDOS - CURSO 3º
MATERIAL DE LA ASIGNATURA
-------------------------
MECANISMO   : PIPES
FICHERO     : whosort.c
DESCRIPCION : Implementacion del proceso de usuario who | sort empleando pipes.
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void)
{
    int fds[2]; // array de enteros de dos posiciones donde se almacenan los extremos de
                // lectura y escritura del pipe
    pipe(fds);  // crea un pipe

    switch(fork()) {
        case -1:
            perror("fork hijo1");
            exit(1);
        case 0:
            // hijo1 reconecta su entrada estandar (stdin) al flujo de salida del pipe
            // y cierra su descriptor de la entrada del pipe
            dup2(fds[0], 0);
            close(fds[1]);
            execlp("sort", "sort", NULL);
            break;
        default:
            break;
    }

    switch(fork()) {
        case -1:
            perror("fork hijo2");
            exit(1);
        case 0:
            // hijo2 reconecta su salida estandar (stdout) a la entrada del pipe
            // y cierra el descriptor de la salida del pipe

            /* Se duplican los datos del extremo de entrada del pipe en el descriptor
               correspondiente a la salida estandar (posicion 1 de la tabla de descriptores)*/
            dup2(fds[1], 1);

            close(fds[0]); // se cierra el extremo que no se va a utilizar.
            execlp("who", "who", NULL);
            break;
        default:
            close(fds[0]);
            close(fds[1]);
            wait(0);
            wait(0);
            break;
    }
    return 0;
}
