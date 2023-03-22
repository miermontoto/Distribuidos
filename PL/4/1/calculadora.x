typedef char* Texto;

struct Operandos {
    int op1;
    int op2;
};

union Resultado switch(int caso) {
    case 1: int n;
    case 2: float x;
    case 3: Texto error;
};

program CALCULADORA {
    version BASICA {
        int sumar(Operandos) = 1;
        int restar(Operandos) = 2;
        int multiplicar(Operandos) = 3;
        Resultado dividir(Operandos) = 4;
    } = 1;
} = 0x40001234;
