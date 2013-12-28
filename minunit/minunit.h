#ifndef __UNIT_TEST
#define __UNIT_TEST

#include <stdio.h>
#include "arm_math.h"
#include "main.h"
#include "minunit.h"
 

#define mu_assert(message, test) do { if (!(test)) return message; } while (0)
#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
extern int tests_run;

int test_run(void);

#endif
