#include <stdlib.h>
#include "../../campo/movimento.h"

int movimentoCavalo(int lo, int co, int ld, int cd) {
    int linhas = abs(ld - lo);
    int colunas = abs(cd - co);
    return (linhas == 2 && colunas == 1) || (linhas == 1 && colunas == 2);
}
