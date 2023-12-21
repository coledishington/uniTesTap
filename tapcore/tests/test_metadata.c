#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(NULL, pass, "Test description");
    tap_register(NULL, fail, "Test description");
    tap_runall_and_cleanup(NULL);
}
