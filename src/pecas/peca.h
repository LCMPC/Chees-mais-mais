#ifndef PECA_H
#define PECA_H

typedef struct Peca {
    char inicial;
    int posicaoInicial[2];
    int melhorias[10];
    int jaMoveu; /* Primeiro movimento do peao; nao reiniciar entre rodadas. */
    struct Peca *proximo;
} Peca;

Peca *criarPeca(Peca **head, char inicial, int melhorias[10]);

#endif
