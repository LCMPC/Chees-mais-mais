#include "../../campo/movimento.h"

int movimentoTorre(int lo, int co, int ld, int cd) {
    if (lo == ld && co == cd) return 0;
    return lo == ld || co == cd;
}
