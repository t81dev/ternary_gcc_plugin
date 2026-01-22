#include <stdio.h>
#include <inttypes.h>
#include "ternary_runtime.h"

int main(void)
{
    t32_t packed = __ternary_tb2t_t32(5);
    t32_t literal = __ternary_bt_str_t32("+1 0 -1 0 1");
    int64_t decoded = __ternary_tt2b_t32(packed);

    printf("binary 5 -> packed (t32): %016" PRIX64 "\n", packed);
    printf("packed -> decoded: %" PRId64 "\n", decoded);
    printf("literal string -> decoded: %" PRId64 "\n", __ternary_tt2b_t32(literal));
    return 0;
}
