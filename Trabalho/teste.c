#include <stdio.h>
#include <string.h>
#include "cria_func.h"

typedef int (*func_int_0)();
typedef int (*func_int_1)(int);
typedef int (*func_int_2)(int, int);
typedef int (*func_memcmp)(void*, int);

int mult(int x, int y) {
    return x * y;
}

int soma3(int a, int b, int c) {
    return a + b + c;
}

char fixa[] = "quero saber se a outra string e um prefixo dessa";

int main(void) {
    unsigned char codigo[500];

    printf("===== TESTE 1: mult(x, 10) =====\n");
    {
        DescParam params[2];
        func_int_1 f_mult;

        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;

        params[1].tipo_val = INT_PAR;
        params[1].orig_val = FIX;
        params[1].valor.v_int = 10;

        cria_func(mult, params, 2, codigo);
        f_mult = (func_int_1) codigo;

        printf("f_mult(5) = %d\n", f_mult(5));
        printf("f_mult(7) = %d\n", f_mult(7));
    }

    printf("\n===== TESTE 2: mult(10, x) =====\n");
    {
        DescParam params[2];
        func_int_1 f_mult;

        params[0].tipo_val = INT_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_int = 10;

        params[1].tipo_val = INT_PAR;
        params[1].orig_val = PARAM;

        cria_func(mult, params, 2, codigo);
        f_mult = (func_int_1) codigo;

        printf("f_mult(5) = %d\n", f_mult(5));
        printf("f_mult(7) = %d\n", f_mult(7));
    }

    printf("\n===== TESTE 3: mult(i, 10), com IND =====\n");
    {
        DescParam params[2];
        func_int_0 f_mult;
        int i;

        params[0].tipo_val = INT_PAR;
        params[0].orig_val = IND;
        params[0].valor.v_ptr = &i;

        params[1].tipo_val = INT_PAR;
        params[1].orig_val = FIX;
        params[1].valor.v_int = 10;

        cria_func(mult, params, 2, codigo);
        f_mult = (func_int_0) codigo;

        i = 3;
        printf("i = 3  -> f_mult() = %d\n", f_mult());

        i = 8;
        printf("i = 8  -> f_mult() = %d\n", f_mult());
    }

    printf("\n===== TESTE 4: soma3(x, 100, y) =====\n");
    {
        DescParam params[3];
        func_int_2 f_soma;

        params[0].tipo_val = INT_PAR;
        params[0].orig_val = PARAM;

        params[1].tipo_val = INT_PAR;
        params[1].orig_val = FIX;
        params[1].valor.v_int = 100;

        params[2].tipo_val = INT_PAR;
        params[2].orig_val = PARAM;

        cria_func(soma3, params, 3, codigo);
        f_soma = (func_int_2) codigo;

        printf("f_soma(1, 2) = %d\n", f_soma(1, 2));
        printf("f_soma(10, 20) = %d\n", f_soma(10, 20));
    }

    printf("\n===== TESTE 5: memcmp(fixa, candidata, n) =====\n");
    {
        DescParam params[3];
        func_memcmp mesmo_prefixo;
        char s[] = "quero saber tudo";
        int tam;

        params[0].tipo_val = PTR_PAR;
        params[0].orig_val = FIX;
        params[0].valor.v_ptr = fixa;

        params[1].tipo_val = PTR_PAR;
        params[1].orig_val = PARAM;

        params[2].tipo_val = INT_PAR;
        params[2].orig_val = PARAM;

        cria_func(memcmp, params, 3, codigo);
        mesmo_prefixo = (func_memcmp) codigo;

        tam = 12;
        printf("prefixo de tamanho 12: %s\n",
               mesmo_prefixo(s, tam) == 0 ? "SIM" : "NAO");

        tam = strlen(s);
        printf("prefixo de tamanho strlen(s): %s\n",
               mesmo_prefixo(s, tam) == 0 ? "SIM" : "NAO");
    }

    return 0;
}