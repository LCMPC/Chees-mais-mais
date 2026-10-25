#ifndef MOVIMENTO_H
#define MOVIMENTO_H

#include "tabuleiro.h"

enum {
    MOVIMENTO_INVALIDO = 0,
    MOVIMENTO_NORMAL = 1,
    MOVIMENTO_CAPTURA = 2,
    MOVIMENTO_BUFF_REI = 3
};

int movimentoTorre(int lo, int co, int ld, int cd);
int movimentoBispo(int lo, int co, int ld, int cd);
int movimentoCavalo(int lo, int co, int ld, int cd);
int movimentoRei(int lo, int co, int ld, int cd);
int movimentoRainha(int lo, int co, int ld, int cd);
int movimentoPeao(int lo, int co, int ld, int cd,
                  int direcao, int jaMoveu, int destinoOcupado);

/* Valida sem alterar a matriz; consulta jaMoveu na peca da origem.
 * Verifica protecao do chefe. Nao verifica turno, xeque ou efeitos de melhorias.
 */
int validarMovimento(Tabuleiro **T, int largura,
                     int linhaOrigem, int colunaOrigem,
                     int linhaDestino, int colunaDestino);

#endif
