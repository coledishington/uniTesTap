#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <tapio.h>
#include <time.h>

#include "config.h"

static double diff_timespec(struct timespec *t1, struct timespec *t0) {
    return (t1->tv_sec - t0->tv_sec) + (t1->tv_nsec - t0->tv_nsec) / 1e9;
}

struct tap_seconds tap_duration_to_secs(struct tap_duration *d) {
    char mprefix = 0;
    int exponent = 0;
    double elapsed;

    elapsed = diff_timespec(&d->t1, &d->t0);
    if (fabs(elapsed) < 1e-9) {
        elapsed = 0.0;
        goto done;
    }

    /* Find the closest metric prefix */
    if (elapsed > 1) {
        for (; fabs(round(elapsed)) > (1e3 - 1e-9); elapsed *= 1e-3, exponent++)
            ;
    } else {
        for (; fabs(round(elapsed)) < (1 - 1e-9); elapsed *= 1e3, exponent--)
            ;
    }

    if (exponent > 0) {
        char prefixes[] = {'k', 'M', 'G', 'T', 'G', 'T', 'P', 'E', 'Z'};

        mprefix = prefixes[exponent - 1];
    } else if (exponent < 0) {
        char prefixes[] = {'m', 'u', 'n', 'p', 'f', 'a', 'z'};

        mprefix = prefixes[exponent * -1 - 1];
    }

done:
    return (struct tap_seconds){
        .secs = elapsed,
        .mprefix = mprefix,
    };
}
