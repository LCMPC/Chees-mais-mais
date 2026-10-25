#include <stdlib.h>
#include "../../campo/movimento.h"

int movimentoBispo(int lo, int co, int ld, int cd) {
    if (lo == ld && co == cd) return 0;
    return abs(ld - lo) == abs(cd - co);
}
