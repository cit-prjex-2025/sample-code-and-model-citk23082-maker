#ifdef __cplusplus
extern "C" {
#endif

/**
 * Common header files
 */
#include "ev3api.h"
#include "target_test.h"

/**
 * Task priorities (smaller number has higher priority)
 */
#define MAIN_PRIORITY	5

/**
 * Task periods in ms
 */
#define MAIN_PERIOD	2

/**
 * Default task stack size in bytes
 */
#define STACK_SIZE	4096

/**
 * Prototypes for configuration
 */
#ifndef TOPPERS_MACRO_ONLY

extern void	main_task(intptr_t);

#endif /* TOPPERS_MACRO_ONLY */

#ifdef __cplusplus
}
#endif