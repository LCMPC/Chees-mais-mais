#include <stdlib.h>
#include "tabuleiro.h"

Tabuleiro **criarTabuleiro(void) {
    Tabuleiro **T = malloc(5 * sizeof(Tabuleiro *));
    if (T == NULL) return NULL;
    for (int i = 0; i < 5; i++) {
        T[i] = malloc(5 * sizeof(Tabuleiro));
        if (T[i] == NULL) {
            for (int j = 0; j < i; j++) free(T[j]);
            free(T);
            return NULL;
        }
        for (int j = 0; j < 5; j++) {
            T[i][j].aliada = NULL;
            T[i][j].inimiga = NULL;
        }
    }
    return T;
}

int aumentarTabuleiro(Tabuleiro **T, int larguraAtual) {
    if (T == NULL || larguraAtual < 5 || larguraAtual >= 8) return 0;

    /* Prepara todas as linhas antes de substituir as antigas.
     * Se uma alocacao falhar, o tabuleiro original continua intacto. */
    Tabuleiro *novas[5];
    for (int i = 0; i < 5; i++) {
        novas[i] = malloc((larguraAtual + 1) * sizeof(Tabuleiro));
        if (novas[i] == NULL) {
            for (int j = 0; j < i; j++) free(novas[j]);
            return 0;
        }
        for (int j = 0; j < larguraAtual; j++) novas[i][j] = T[i][j];
        novas[i][larguraAtual].aliada = NULL;
        novas[i][larguraAtual].inimiga = NULL;
    }
    for (int i = 0; i < 5; i++) {
        free(T[i]);
        T[i] = novas[i];
    }
    return 1;
}

void liberarTabuleiro(Tabuleiro **T) {
    if (T == NULL) return;
    for (int i = 0; i < 5; i++) free(T[i]);
    free(T);
}
