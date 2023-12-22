#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(NULL, pass, "Test description");
    tap_register(NULL, fail, "Test description");
    tap_register(NULL, pass, "Test description with newline\n");
    tap_register(NULL, pass, "Test description with two newlines\n\n");
    tap_runall_and_cleanup(NULL);
}
