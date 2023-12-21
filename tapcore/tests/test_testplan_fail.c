#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(NULL, fail, NULL);
    tap_runall_and_cleanup(NULL);
}
