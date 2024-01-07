#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <tapio.h>
#include <time.h>

#include "config.h"

static double diff_timespec(struct timespec *t1, struct timespec *t0) {
    return (t1->tv_sec - t0->tv_sec) + (t1->tv_nsec - t0->tv_nsec) / 1e9;
}

double tap_duration_to_secs(struct tap_duration *d) {
    double elapsed;

    elapsed = diff_timespec(&d->t1, &d->t0);
    if (fabs(elapsed) < 1e-9) {
        elapsed = 0.0;
    }
    return elapsed;
}
