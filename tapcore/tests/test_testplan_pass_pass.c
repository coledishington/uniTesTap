#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(NULL, pass, NULL);
    tap_register(NULL, pass, NULL);
    tap_runall_and_cleanup(NULL);
}
