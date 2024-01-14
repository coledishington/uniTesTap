#ifndef __TAP_H__
#define __TAP_H__

/**
 * @var TAP
 *
 * A tap handle for use with all tap functions.
 */
typedef struct TAP TAP;

/**
 * @var TAP_OPTION
 *
 * Options to libtaps behaviour.
 */
typedef enum {
    TAP_OPTION_N_RUNNERS, /**< Set the number of test runners that libtap will
                               run tests with. The default is physical
                               cpu cores - 1. */
} TAP_OPTION;

/**
 * @var test_t
 *
 * Type of TAP test function.
 */
typedef int (*test_t)(void);

/**
 * @fn tap_init
 *
 * Allocates and returns a TAP handle that should be passed to tap functions.
 *
 * @param d_tap double pointer to TAP handle. If tap function calls do not
 *              need to be re-entrant, NULL can be passed.
 *
 * @return 0 on success, errno-like value otherwise.
 */
int tap_init(TAP **d_tap);

/**
 * @fn tap_set_option
 *
 * Allocates and returns a TAP handle that should be passed to tap functions.
 *
 * @param d_tap double pointer to TAP handle. If tap function calls do not
 *              need to be re-entrant, NULL can be passed.
 *
 * @return 0 on success, errno-like value otherwise.
 */
int tap_set_option(TAP *tap, TAP_OPTION option, ...);

/**
 * @fn tap_cleanup
 *
 * @param tap a TAP handle allocated by tap_init().
 *
 * Cleanup any resources allocated by libtap associated with the passed
 * handle.
 */
void tap_cleanup(TAP *tap);

/**
 * @fn tap_register
 *
 * Register a test to run in tap_runall().
 *
 * @param tap a tap handle allocated by tap_init().
 * @param test the test function to register.
 * @param description an optional description of the registered test.
 *
 * @return 0 on success, errno-like value otherwise.
 */
int tap_register(TAP *tap, test_t test, const char *description);

/**
 * @fn tap_easy_register
 *
 * Register a test to run in tap_runall(). This function is not re-entrant,
 * see tap_register() for a re-entrant version.
 *
 * @param test the test function to register.
 * @param description an optional description of the registered test.
 *
 * @return 0 on success, errno-like value otherwise.
 */
static inline int tap_easy_register(test_t test, const char *description) {
    return tap_register(NULL, test, description);
}

/**
 * @fn tap_runall
 *
 * Run all tests registered in the global TAP tests list.
 *
 * @param tap a tap handle allocated by tap_init().
 *
 * @return 0 on success, errno-like value otherwise.
 */
int tap_runall(TAP *tap);

/**
 * @fn tap_easy_runall_and_cleanup
 *
 * Run all tests registered in the global TAP tests list and cleanup. This
 * function is not re-entrant, see tap_runall() and tap_cleanup() for
 * re-entrant usecases.
 *
 * @return 0 on success, errno-like value otherwise.
 */
static inline int tap_easy_runall_and_cleanup(void) {
    int ret;

    ret = tap_runall(NULL);
    tap_cleanup(NULL);
    return ret;
}

#endif /* __TAP_H__ */