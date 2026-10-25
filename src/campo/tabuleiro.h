#ifndef TABULEIRO_H
#define TABULEIRO_H

#include "../pecas/peca.h"
#include "../pecas/pecaInimiga.h"

typedef struct {
    Peca *aliada;
    PecaInimiga *inimiga;
} Tabuleiro;

Tabuleiro **criarTabuleiro(void);
int aumentarTabuleiro(Tabuleiro **T, int larguraAtual);
void liberarTabuleiro(Tabuleiro **T);

#endif
