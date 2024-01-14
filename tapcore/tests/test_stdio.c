#include <stdio.h>
#include <stdlib.h>
#include <tap.h>

static int pass_output_with_no_newline() {
    printf("What is a line has no newline");
    return 0;
}

static int pass_output_with_one_newline(void) {
    printf("What is a line has one newline\n");
    return 0;
}

static int pass_output_with_two_newlines(void) {
    printf("What is a line has two newlines\n\n");
    return 0;
}

int main(void) {
    tap_easy_register(pass_output_with_no_newline, NULL);
    tap_easy_register(pass_output_with_one_newline, NULL);
    tap_easy_register(pass_output_with_two_newlines, NULL);
    tap_easy_runall_and_cleanup();
}
