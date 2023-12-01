#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <tap.h>

#define MAX_TESTS 10

struct cfg {
    size_t n_tests;
    test_t tests[MAX_TESTS];
};

struct cfg cfg = {
    .n_tests = 0,
};

void tap_register(test_t test) {
    assert(cfg.n_tests + 1 < MAX_TESTS);

    cfg.tests[cfg.n_tests] = test;
    cfg.n_tests++;
}

void tap_evaluate(size_t test_id, test_t test) {
    int res;

    res = test();
    if (res == 0) {
        printf("ok %zu\n", test_id);
    } else {
        printf("not ok %zu\n", test_id);
    }
}

void tap_runall(void) {
    printf("1..%zu\n", cfg.n_tests);
    for (size_t test_idx = 0; test_idx < cfg.n_tests; test_idx++) {
        test_t test = cfg.tests[test_idx];
        size_t test_id = test_idx + 1;
        tap_evaluate(test_id, test);
    }
}
