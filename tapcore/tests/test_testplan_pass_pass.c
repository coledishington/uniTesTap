#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(pass, NULL);
    tap_register(pass, NULL);
    tap_runall();
}
