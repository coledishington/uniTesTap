#ifndef __TAP_H__
#define __TAP_H__

/**
 * @var test_t
 *
 * Type of TAP test function.
 */
typedef int (*test_t)(void);

/**
 * @fn tap_register
 *
 * Register a test to run in tap_runall().
 */
void tap_register(test_t test);

/**
 * @fn tap_runall
 *
 * Run all tests registered in the global TAP tests list.
 */
void tap_runall(void);

#endif /* __TAP_H__ */