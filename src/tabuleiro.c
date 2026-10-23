#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char peca;
} Tabuleiro;

Tabuleiro ** criarTabuleiro() {
    Tabuleiro **T = malloc(5 * sizeof(Tabuleiro *));

    if (T == NULL) return NULL;

    for (int i = 0; i < 5; i++) {
        T[i] = malloc(5 * sizeof(Tabuleiro));

        if (T[i] == NULL) {
            for (int j = 0; j < i; j++)
                free(T[j]);

            free(T);
            return NULL;
        }

        for (int j = 0; j < 5; j++) {
            T[i][j].peca = '.';
        }
    }

    return T;
}

int aumentarTabuleiro (Tabuleiro ** T, int larguraAtual) {
    if (larguraAtual >= 8) return 0;

    for (int i = 0; i < 5; i++) {
        Tabuleiro * tempT = realloc(T[i], (larguraAtual + 1) * sizeof(Tabuleiro));

        if (tempT == NULL) return 0;

        T[i] = tempT;
        T[i][larguraAtual].peca = '.';
    }

    return 1;
}