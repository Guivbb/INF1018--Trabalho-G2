#include "cria_func.h"
#include <stdint.h>

#define RAX 0
#define RDX 2
#define RSI 6
#define RDI 7
#define R8  8
#define R10 10
#define R11 11

static void emit1(unsigned char codigo[], int *i, unsigned char b) {
    codigo[(*i)++] = b;
}

static void emit_int32(unsigned char codigo[], int *i, int v) {
    uint32_t x = (uint32_t)v;

    emit1(codigo, i, x & 0xff);
    emit1(codigo, i, (x >> 8) & 0xff);
    emit1(codigo, i, (x >> 16) & 0xff);
    emit1(codigo, i, (x >> 24) & 0xff);
}

static void emit_uint64(unsigned char codigo[], int *i, uint64_t v) {
    int k;

    for (k = 0; k < 8; k++) {
        emit1(codigo, i, (v >> (8 * k)) & 0xff);
    }
}

/*
    Assembly AT&T:
    movq %src, %dst
*/
static void emit_mov_reg64(unsigned char codigo[], int *i, int src, int dst) {
    unsigned char rex = 0x48;

    if (src >= 8) {
        rex |= 0x04;
    }

    if (dst >= 8) {
        rex |= 0x01;
    }

    emit1(codigo, i, rex);
    emit1(codigo, i, 0x89);
    emit1(codigo, i, 0xC0 | ((src & 7) << 3) | (dst & 7));
}

/*
    Assembly AT&T:
    movl $imm, %dst
*/
static void emit_mov_imm32_to_reg(unsigned char codigo[], int *i, int imm, int dst) {
    if (dst >= 8) {
        emit1(codigo, i, 0x41);
    }

    emit1(codigo, i, 0xB8 + (dst & 7));
    emit_int32(codigo, i, imm);
}

/*
    Assembly AT&T:
    movabsq $imm, %dst
*/
static void emit_mov_imm64_to_reg(unsigned char codigo[], int *i, uint64_t imm, int dst) {
    unsigned char rex = 0x48;

    if (dst >= 8) {
        rex |= 0x01;
    }

    emit1(codigo, i, rex);
    emit1(codigo, i, 0xB8 + (dst & 7));
    emit_uint64(codigo, i, imm);
}

/*
    Assembly AT&T:
    movl (%rax), %dst
*/
static void emit_load_rax_to_reg32(unsigned char codigo[], int *i, int dst) {
    if (dst >= 8) {
        emit1(codigo, i, 0x44);
    }

    emit1(codigo, i, 0x8B);
    emit1(codigo, i, ((dst & 7) << 3));
}

/*
    Assembly AT&T:
    movq (%rax), %dst
*/
static void emit_load_rax_to_reg64(unsigned char codigo[], int *i, int dst) {
    unsigned char rex = 0x48;

    if (dst >= 8) {
        rex |= 0x04;
    }

    emit1(codigo, i, rex);
    emit1(codigo, i, 0x8B);
    emit1(codigo, i, ((dst & 7) << 3));
}

void cria_func(void* f, DescParam params[], int n, unsigned char codigo[]) {
    int i = 0;
    int p;
    int param_index = 0;

    int arg_regs[3] = {RDI, RSI, RDX};
    int temp_regs[3] = {R10, R11, R8};

    /*
        Prólogo:

        pushq %rbp
        movq %rsp, %rbp
    */
    emit1(codigo, &i, 0x55);

    emit1(codigo, &i, 0x48);
    emit1(codigo, &i, 0x89);
    emit1(codigo, &i, 0xE5);

    /*
        Primeiro salvamos os parâmetros recebidos pela função nova.

        Exemplo:
        Se a função nova recebe x, ele chega em %rdi.
        Mas talvez depois ele precise ir para %rsi ou %rdx.
    */
    for (p = 0; p < n; p++) {
        if (params[p].orig_val == PARAM) {
            emit_mov_reg64(codigo, &i, arg_regs[param_index], temp_regs[param_index]);
            param_index++;
        }
    }

    /*
        Agora montamos os parâmetros da função original.

        1º parâmetro original -> %rdi
        2º parâmetro original -> %rsi
        3º parâmetro original -> %rdx
    */
    param_index = 0;

    for (p = 0; p < n; p++) {
        int dst = arg_regs[p];

        if (params[p].orig_val == PARAM) {
            emit_mov_reg64(codigo, &i, temp_regs[param_index], dst);
            param_index++;
        }

        else if (params[p].orig_val == FIX) {
            if (params[p].tipo_val == INT_PAR) {
                emit_mov_imm32_to_reg(codigo, &i, params[p].valor.v_int, dst);
            } else {
                emit_mov_imm64_to_reg(codigo, &i, (uint64_t) params[p].valor.v_ptr, dst);
            }
        }

        else if (params[p].orig_val == IND) {
            /*
                Primeiro:
                movabsq $endereco, %rax

                Depois:
                movl (%rax), registrador    para inteiro
                ou
                movq (%rax), registrador    para ponteiro
            */
            emit_mov_imm64_to_reg(codigo, &i, (uint64_t) params[p].valor.v_ptr, RAX);

            if (params[p].tipo_val == INT_PAR) {
                emit_load_rax_to_reg32(codigo, &i, dst);
            } else {
                emit_load_rax_to_reg64(codigo, &i, dst);
            }
        }
    }

    /*
        Chama a função original:

        movabsq $f, %rax
        call *%rax
    */
    emit_mov_imm64_to_reg(codigo, &i, (uint64_t) f, RAX);

    emit1(codigo, &i, 0xFF);
    emit1(codigo, &i, 0xD0);

    /*
        Epílogo:

        leave
        ret
    */
    emit1(codigo, &i, 0xC9);
    emit1(codigo, &i, 0xC3);
}