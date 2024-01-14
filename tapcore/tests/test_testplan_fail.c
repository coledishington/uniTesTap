#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_easy_register(fail, NULL);
    tap_easy_runall_and_cleanup();
}
