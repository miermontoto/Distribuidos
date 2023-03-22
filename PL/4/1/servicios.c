#include "calculadora.h"

int* sumar_1_svc(Operandos *op, struct svc_req *rqstp) {
    static int res;
    res = op->op1 + op->op2;
    return &res;
}

int* restar_1_svc(Operandos *op, struct svc_req *rqstp) {
    static int res;
    res = op->op1 - op->op2;
    return &res;
}

int* multiplicar_1_svc(Operandos *op, struct svc_req *rqstp) {
    static int res;
    res = op->op1 * op->op2;
    return &res;
}

Resultado* dividir_1_svc(Operandos *op, struct svc_req *rqstp) {
    static Resultado res;

    res.Resultado_u.error = NULL;
    if (op->op2 == 0) {
        res.caso = 3;
        res.Resultado_u.error = "Division por cero";
    } else {
        if(op->op1 % op->op2 == 0) {
            res.caso = 1;
            res.Resultado_u.n = op->op1 / op->op2;
        } else {
            res.caso = 2;
            res.Resultado_u.x = (float)op->op1 / op->op2;
        }
    }
    return &res;
}
