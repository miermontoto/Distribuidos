#include <stdio.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include "calculadora.h"

int main(int argc, char* argv[]) {
    CLIENT *clnt;

    Operandos op;
    int *res1;
    Resultado *res2;

    clnt = clnt_create(argv[1], CALCULADORA, BASICA, "udp");
    if (clnt == NULL) {
        clnt_pcreateerror("No se puede inicializar cliente");
        exit(EXIT_FAILURE);
    }

    printf("Introduce el primer operando: ");
    scanf("%d", &op.op1);
    printf("Introduce el segundo operando: ");
    scanf("%d", &op.op2);

    res1 = sumar_1(&op, clnt);
    if (res1 == NULL) {
        clnt_perror(clnt, "Error en la llamada remota");
        exit(EXIT_FAILURE);
    }
    printf("El resultado de la suma es: %d\n", *res1);

    res1 = restar_1(&op, clnt);
    if (res1 == NULL) {
        clnt_perror(clnt, "Error en la llamada remota");
        exit(EXIT_FAILURE);
    }
    printf("El resultado de la resta es: %d\n", *res1);

    res1 = multiplicar_1(&op, clnt);
    if (res1 == NULL) {
        clnt_perror(clnt, "Error en la llamada remota");
        exit(EXIT_FAILURE);
    }
    printf("El resultado de la multiplicacion es: %d\n", *res1);

    res2 = dividir_1(&op, clnt);
    if (res2 == NULL) {
        clnt_perror(clnt, "Error en la llamada remota");
        exit(EXIT_FAILURE);
    }
    switch (res2 -> caso) {
        case 1:
            printf("El resultado de la division es: %d\n", res2 -> Resultado_u.n);
            break;
        case 2:
            printf("El resultado de la division es: %f\n", res2 -> Resultado_u.x);
            break;
        case 3:
            printf("%s\n", res2 -> Resultado_u.error);
            break;
        default:
            printf("Error en la llamada remota");
            exit(EXIT_FAILURE);
    }

    clnt_destroy(clnt);
    return 0;
}
