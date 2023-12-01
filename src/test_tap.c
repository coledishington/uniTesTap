#include <tap.h>

int return_zero(void) { return 0; }

int main(void) {
    tap_register(return_zero);

    tap_runall();
}
