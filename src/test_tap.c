#include <assert.h>
#include <stdio.h>
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

int ok_skipped(void) {
    printf(":SKIP don't need this test");
    return 0;
}

int not_ok_skipped(void) {
    printf(":SKIP don't need this test");
    return 1;
}

int ok_todo(void) {
    printf(":TODO will this ever be done?");
    return 0;
}

int not_ok_todo(void) {
    printf(":TODO will this ever be done?");
    return 1;
}

int main(void) {
    tap_register(return_zero, "This test does nothing but return zero");
    tap_register(assert_zero, NULL);
    tap_register(exhaust_stack_frame, NULL);
    tap_register(dereference_null_ptr, NULL);
    tap_register(ok_skipped, NULL);
    tap_register(not_ok_skipped, NULL);
    tap_register(ok_todo, NULL);
    tap_register(not_ok_todo, NULL);

    tap_runall();
}
