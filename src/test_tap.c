#include <assert.h>
#include <tap.h>

int return_zero(void) { return 0; }

int assert_zero(void) {
    assert(0);
    return 0;
}

int main(void) {
    tap_register(return_zero);
    tap_register(assert_zero);

    tap_runall();
}
