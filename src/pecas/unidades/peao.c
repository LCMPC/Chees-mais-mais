#include <stdlib.h>
#include "../../campo/movimento.h"

int movimentoPeao(int lo, int co, int ld, int cd,
                  int direcao, int jaMoveu, int destinoOcupado) {
    int linhas = ld - lo;
    int colunas = abs(cd - co);
    if (direcao != -1 && direcao != 1) return 0;

    if (destinoOcupado) return linhas == direcao && colunas == 1;
    if (colunas != 0) return 0;
    if (linhas == direcao) return 1;
    return !jaMoveu && linhas == 2 * direcao;
}
