/*
 * Copyright(c) 2019 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#ifndef ___CREATEWRITER_H
#define ___CREATEWRITER_H

#include <stdio.h>
#include <string.h>

#include "dds/dds.h"

#if defined (__cplusplus)
extern "C" {
#endif

/*
 * Test asserts.
 * Printing is supported eg TST_ASSERT_EQ(a, b, "foo: %s", bar")
 */
#define TST_FATAL_YES 1
#define TST_FATAL_NO  0

#ifdef _WIN32
/* Microsoft Visual Studio does not expand __VA_ARGS__ correctly. */
#define TST__ASSERT__(...)     TST__ASSERT____((__VA_ARGS__))
#define TST__ASSERT____(tuple) TST__ASSERT___ tuple
#else
#define TST__ASSERT__(...) TST__ASSERT___(__VA_ARGS__)
#endif /* _WIN32 */

#define TST__ASSERT___(cond, fatal, ...)                  \
  do {                                                    \
    if (!(cond)) {                                        \
      printf("TST_FAIL(%s, %d):\n", __FILE__, __LINE__);  \
      printf(__VA_ARGS__);                                \
      printf("\n");                                       \
      CU_ASSERT(0);                                       \
    }                                                     \
  } while(0)

#define TST_ASSERT(cond, ...)   TST__ASSERT__((cond), TST_FATAL_NO, __VA_ARGS__)

#define TST_ASSERT_FAIL(...)    TST_ASSERT(0, __VA_ARGS__)

#define TST_ASSERT_EQ(value, expected, ...)  TST_ASSERT(((value) == (expected)), __VA_ARGS__)
#define TST_ASSERT_NEQ(value, expected, ...) TST_ASSERT(((value) != (expected)), __VA_ARGS__)
#define TST_ASSERT_LEQ(value, expected, ...) TST_ASSERT(((value) <= (expected)), __VA_ARGS__)
#define TST_ASSERT_GEQ(value, expected, ...) TST_ASSERT(((value) >= (expected)), __VA_ARGS__)
#define TST_ASSERT_LT(value, expected, ...)  TST_ASSERT(((value)  < (expected)), __VA_ARGS__)
#define TST_ASSERT_GT(value, expected, ...)  TST_ASSERT(((value)  > (expected)), __VA_ARGS__)

#define TST_ASSERT_EQUAL TST_ASSERT_EQ

#define TST_ASSERT_NULL(value, ...)     TST_ASSERT(((value) == NULL), __VA_ARGS__)
#define TST_ASSERT_NOT_NULL(value, ...) TST_ASSERT(((value) != NULL), __VA_ARGS__)

#define TST_ASSERT_STR_EQ(value, expected, ...)  TST_ASSERT((TST_STRCMP((value), (expected), 1) == 0), __VA_ARGS__)
#define TST_ASSERT_STR_NEQ(value, expected, ...) TST_ASSERT((TST_STRCMP((value), (expected), 0) != 0), __VA_ARGS__)
#define TST_ASSERT_STR_EMPTY(value, ...)         TST_ASSERT((TST_STRLEN((value), 1) == 0), __VA_ARGS__)
#define TST_ASSERT_STR_NOT_EMPTY(value, ...)     TST_ASSERT((TST_STRLEN((value), 0)  > 0), __VA_ARGS__)


/* Fatal just means that control is returned to the parent function. */
#define TST_ASSERT_FATAL(cond, ...)   TST__ASSERT__((cond), TST_FATAL_YES, __VA_ARGS__)

#define TST_ASSERT_FATAL_FAIL(...)    TST_ASSERT_FATAL(0, __VA_ARGS__)

#define TST_ASSERT_FATAL_EQ(value, expected, ...)  TST_ASSERT_FATAL(((value) == (expected)), __VA_ARGS__)
#define TST_ASSERT_FATAL_NEQ(value, expected, ...) TST_ASSERT_FATAL(((value) != (expected)), __VA_ARGS__)
#define TST_ASSERT_FATAL_LEQ(value, expected, ...) TST_ASSERT_FATAL(((value) <= (expected)), __VA_ARGS__)
#define TST_ASSERT_FATAL_GEQ(value, expected, ...) TST_ASSERT_FATAL(((value) >= (expected)), __VA_ARGS__)
#define TST_ASSERT_FATAL_LT(value, expected, ...)  TST_ASSERT_FATAL(((value)  < (expected)), __VA_ARGS__)
#define TST_ASSERT_FATAL_GT(value, expected, ...)  TST_ASSERT_FATAL(((value)  > (expected)), __VA_ARGS__)

#define TST_ASSERT_EQUAL_FATAL TST_ASSERT_FATAL_EQ

#define TST_ASSERT_FATAL_NULL(value, ...)     TST_ASSERT_FATAL(((value) == NULL), __VA_ARGS__)
#define TST_ASSERT_FATAL_NOT_NULL(value, ...) TST_ASSERT_FATAL(((value) != NULL), __VA_ARGS__)

#define TST_ASSERT_FATAL_STR_EQ(value, expected, ...)  TST_ASSERT_FATAL((TST_STRCMP((value), (expected), 1) == 0), __VA_ARGS__)
#define TST_ASSERT_FATAL_STR_NEQ(value, expected, ...) TST_ASSERT_FATAL((TST_STRCMP((value), (expected), 0) != 0), __VA_ARGS__)
#define TST_ASSERT_FATAL_STR_EMPTY(value, ...)         TST_ASSERT_FATAL((TST_STRLEN((value), 1) == 0), __VA_ARGS__)
#define TST_ASSERT_FATAL_STR_NOT_EMPTY(value, ...)     TST_ASSERT_FATAL((TST_STRLEN((value), 0)  > 0), __VA_ARGS__)

void createwriter_init_md(void);
void createwriter_init_sd(void);
void createwriter_fini(void);

#if defined (__cplusplus)
}
#endif

#endif /* ___CREATEWRITER_H */
