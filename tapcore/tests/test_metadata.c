#include <tap.h>

#include "internal.h"

int main(void) {
    tap_register(pass, "Test description");
    tap_register(fail, "Test description");
    tap_runall();
}
