#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <tapio.h>
#include <taptest.h>
#include <taputil.h>
#include <time.h>

#include "config.h"

size_t test_counter = 1;

void positive_tests(void) {
    struct {
        const char *name;
        struct tap_duration in;
        const char *out;
    } testcases[] = {
        /* Test base cases */
        {
            .name = "from 0 to 0",
            .in = {.t1 = {0}, .t0 = {0}},
            .out = "0",
        },
        {
            .name = "from 0 to 1",
            .in = {.t1 = {.tv_sec = 1}, .t0 = {0}},
            .out = "1",
        },
        {
            .name = "from 0 to 150",
            .in =
                {
                    .t1 = {.tv_sec = 150},
                    .t0 = {0},
                },
            .out = "150",
        },
        /* Test positive metric prefixes */
        {
            .name = "from 0 to 1k",
            .in =
                {
                    .t1 = {.tv_sec = 1000},
                    .t0 = {0},
                },
            .out = "1k",
        },
        {
            .name = "from 0 to 1M",
            .in =
                {
                    .t1 = {.tv_sec = 1000 * 1000},
                    .t0 = {0},
                },
            .out = "1M",
        },
        {
            .name = "from 0 to 1M + 1000n",
            .in =
                {
                    .t1 = {.tv_sec = 1000 * 1000, .tv_nsec = 1000},
                    .t0 = {0},
                },
            .out = "1M",
        },
        {
            .name = "from 1000n to 1M",
            .in =
                {
                    .t1 = {.tv_sec = 1000 * 1000},
                    .t0 = {.tv_nsec = 1000},
                },
            .out = "1M",
        },
        {
            .name = "from 1000n to 1M",
            .in =
                {
                    .t1 = {.tv_sec = 1000 * 1000},
                    .t0 = {.tv_nsec = 1000},
                },
            .out = "1M",
        },
        {
            .name = "from 7000 to 9M",
            .in =
                {
                    .t1 = {.tv_sec = 9 * 1000 * 1000},
                    .t0 = {.tv_sec = 7 * 1000},
                },
            .out = "8.99M",
        },
        {
            .name = "from 7000 to 11M",
            .in =
                {
                    .t1 = {.tv_sec = 11 * 1000 * 1000},
                    .t0 = {.tv_sec = 77 * 1000},
                },
            .out = "10.9M",
        },
        {
            .name = "from 7000 to 234M",
            .in =
                {
                    .t1 = {.tv_sec = 234 * 1000 * 1000},
                    .t0 = {.tv_sec = 777 * 1000},
                },
            .out = "233M",
        },
        /* Test negative metric prefixes */
        {
            .name = "from 0 to 375 + 402354658n",
            .in =
                {
                    .t1 = {.tv_sec = 375, .tv_nsec = 402354658},
                    .t0 = {0},
                },
            .out = "375",
        },
        {
            .name = "from (18077 + 29771188n) to (18077 + 328997539n)",
            .in =
                {
                    .t1 = {.tv_sec = 18077, .tv_nsec = 328997539},
                    .t0 = {.tv_sec = 18077, .tv_nsec = 29771188},
                },
            .out = "299m",
        },
        {
            .name = "from (7754 + 203207779n) to (7754 + 240335621n)",
            .in =
                {
                    .t1 = {.tv_sec = 7754, .tv_nsec = 240335621},
                    .t0 = {.tv_sec = 7754, .tv_nsec = 203207779},
                },
            .out = "37.1m",
        }};

    for (size_t idx = 0; idx < ARRAY_LEN(testcases); idx++) {
        size_t test_id = idx + test_counter;
        char dest[128] = {0};
        struct tap_seconds t;

        t = tap_duration_to_secs(&testcases[idx].in);

        snprintf(dest, ARRAY_LEN(dest), "%.3g%c", t.secs, t.mprefix);
        if (strcmp(testcases[idx].out, dest) != 0) {
            printf("not ok %zu - %s %s != %s\n", test_id, testcases[idx].name,
                   dest, testcases[idx].out);
            continue;
        }

        printf("ok %zu - %s\n", test_id, testcases[idx].name);
    }

    test_counter += ARRAY_LEN(testcases);
}

int main(void) {
    positive_tests();
    printf("1..%zu\n", test_counter - 1);
}
