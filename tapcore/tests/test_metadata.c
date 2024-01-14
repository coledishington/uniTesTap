#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    tap_easy_register(pass, "Test description");
    tap_easy_register(fail, "Test description");
    tap_easy_register(pass, "Test description with newline\n");
    tap_easy_register(pass, "Test description with two newlines\n\n");
    tap_easy_runall_and_cleanup();
}
