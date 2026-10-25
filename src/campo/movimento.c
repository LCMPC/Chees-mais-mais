#include <ctype.h>
#include <stddef.h>
#include "movimento.h"

static int tipoValido(char peca) {
    int tipo = toupper((unsigned char)peca);
    return tipo == 'P' || tipo == 'T' || tipo == 'B'
        || tipo == 'C' || tipo == 'R' || tipo == 'D';
}

static int casaVazia(Tabuleiro casa) {
    return casa.aliada == NULL && casa.inimiga == NULL;
}

static int casaInvalida(Tabuleiro casa) {
    return casa.aliada != NULL && casa.inimiga != NULL;
}

static int caminhoLivre(Tabuleiro **T, int lo, int co, int ld, int cd) {
    int passoLinha = 0, passoColuna = 0;
    if (ld > lo) passoLinha = 1;
    if (ld < lo) passoLinha = -1;
    if (cd > co) passoColuna = 1;
    if (cd < co) passoColuna = -1;

    int linha = lo + passoLinha;
    int coluna = co + passoColuna;
    while (linha != ld || coluna != cd) {
        if (!casaVazia(T[linha][coluna])) return 0;
        linha += passoLinha;
        coluna += passoColuna;
    }
    return 1;
}

int validarMovimento(Tabuleiro **T, int largura,
                     int lo, int co, int ld, int cd) {
    if (T == NULL || largura < 5 || largura > 8) return MOVIMENTO_INVALIDO;
    if (lo < 0 || lo >= 5 || ld < 0 || ld >= 5
        || co < 0 || co >= largura || cd < 0 || cd >= largura) {
        return MOVIMENTO_INVALIDO;
    }
    if (lo == ld && co == cd) return MOVIMENTO_INVALIDO;

    for (int i = 0; i < 5; i++) {
        if (T[i] == NULL) return MOVIMENTO_INVALIDO;
    }
    Tabuleiro casaOrigem = T[lo][co];
    Tabuleiro casaDestino = T[ld][cd];
    if (casaVazia(casaOrigem) || casaInvalida(casaOrigem)
        || casaInvalida(casaDestino)) return MOVIMENTO_INVALIDO;

    int aliada = casaOrigem.aliada != NULL;
    char origem;
    int jaMoveu;
    if (aliada) {
        origem = casaOrigem.aliada->inicial;
        jaMoveu = casaOrigem.aliada->jaMoveu;
    } else {
        origem = casaOrigem.inimiga->inicial;
        jaMoveu = casaOrigem.inimiga->jaMoveu;
    }
    int destinoOcupado = !casaVazia(casaDestino);
    if (!tipoValido(origem)) return MOVIMENTO_INVALIDO;

    int tipo = toupper((unsigned char)origem);
    if (destinoOcupado && aliada == (casaDestino.aliada != NULL)) {
        if (tipo == 'R' && movimentoRei(lo, co, ld, cd)) return MOVIMENTO_BUFF_REI;
        return MOVIMENTO_INVALIDO;
    }

    int valido = 0;
    switch (tipo) {
        case 'T': valido = movimentoTorre(lo, co, ld, cd); break;
        case 'B': valido = movimentoBispo(lo, co, ld, cd); break;
        case 'C': valido = movimentoCavalo(lo, co, ld, cd); break;
        case 'R': valido = movimentoRei(lo, co, ld, cd); break;
        case 'D': valido = movimentoRainha(lo, co, ld, cd); break;
        case 'P': {
            // Aliados sobem na matriz; inimigos descem.
            int direcao = aliada ? -1 : 1;
            valido = movimentoPeao(lo, co, ld, cd, direcao, jaMoveu, destinoOcupado);
            break;
        }
    }
    if (!valido) return MOVIMENTO_INVALIDO;
    if (tipo != 'C' && !caminhoLivre(T, lo, co, ld, cd)) return MOVIMENTO_INVALIDO;
    if (destinoOcupado && casaDestino.inimiga != NULL && casaDestino.inimiga->ehChefe) {
        // O chefe so pode ser capturado quando for o ultimo inimigo.
        for (int l = 0; l < 5; l++) {
            for (int c = 0; c < largura; c++) {
                if (T[l][c].inimiga != NULL && (l != ld || c != cd)) {
                    return MOVIMENTO_INVALIDO;
                }
            }
        }
    }
    return destinoOcupado ? MOVIMENTO_CAPTURA : MOVIMENTO_NORMAL;
}
