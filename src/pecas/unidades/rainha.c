#include "../../campo/movimento.h"

int movimentoRainha(int lo, int co, int ld, int cd) {
    return movimentoTorre(lo, co, ld, cd) || movimentoBispo(lo, co, ld, cd);
}
