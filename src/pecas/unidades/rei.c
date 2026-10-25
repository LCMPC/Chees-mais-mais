#include <stdlib.h>
#include "../../campo/movimento.h"

int movimentoRei(int lo, int co, int ld, int cd) {
    if (lo == ld && co == cd) return 0;
    return abs(ld - lo) <= 1 && abs(cd - co) <= 1;
}
