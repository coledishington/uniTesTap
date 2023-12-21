#include <assert.h>
#include <stdlib.h>
#include <tap.h>

static int early_exit_success(void) {
    exit(0);
    return -1;
}

static int early_exit_fail(void) {
    exit(-1);
    return 0;
}

static int assert_zero(void) {
    assert(0);
    return 0;
}

static int infinite_recursion(int start) {
    int i = 1;
    int j = 2;
    int k = i + j;

    return infinite_recursion(start + i) + infinite_recursion(start + j) +
           infinite_recursion(start + k);
}

static int exhaust_stack_frame(void) {
    infinite_recursion(1);
    return 0;
}

static int dereference_null_ptr(void) {
    int *priv = NULL;
    return *priv == 0;
}

int main(void) {
    tap_register(early_exit_success, NULL);
    tap_register(early_exit_fail, NULL);
    tap_register(assert_zero, NULL);
    tap_register(exhaust_stack_frame, NULL);
    tap_register(dereference_null_ptr, NULL);
    tap_runall();
}
