#include <assert.h>
#include <stdlib.h>
#include <tap.h>

int return_zero(void) { return 0; }

int assert_zero(void) {
    assert(0);
    return 0;
}

int infinite_recursion(int start) {
    int i = 1;
    int j = 2;
    int k = i + j;

    return infinite_recursion(start + i) + infinite_recursion(start + j) +
           infinite_recursion(start + k);
}

int exhaust_stack_frame(void) {
    infinite_recursion(1);
    return 0;
}

int dereference_null_ptr(void) {
    int *priv = NULL;
    return *priv == 0;
}

int main(void) {
    tap_register(return_zero, "This test does nothing but return zero");
    tap_register(assert_zero, NULL);
    tap_register(exhaust_stack_frame, NULL);
    tap_register(dereference_null_ptr, NULL);

    tap_runall();
}
