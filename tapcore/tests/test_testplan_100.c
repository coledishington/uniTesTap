#include <stdlib.h>
#include <tap.h>

#include "internal.h"

int main(void) {
    for (int i = 1; i < 101; i++) {
        if ((i < 20 && i % 3 == 0) || i % 11 == 0) {
            tap_register(NULL, fail, NULL);
        } else {
            tap_register(NULL, pass, NULL);
        }
    }
    tap_runall_and_cleanup(NULL);
}
