#include <stdio.h>
#include <stdlib.h>
#include <tap.h>

#include "internal.h"

static int pass_skipped_newline(void) {
    printf(":SKIP don't need this test\n");
    return 0;
}

int main(void) {
    tap_easy_register(pass_skipped_newline,
                      "This is a description over\nmultiple lines\n");
    tap_easy_runall_and_cleanup();
}
