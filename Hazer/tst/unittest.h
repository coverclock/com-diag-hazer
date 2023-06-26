/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_TST_UNITTEST_
#define _H_COM_DIAG_HAZER_TST_UNITTEST_

/**
 * @file
 *
 * Copyright 2019-2022 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://github.com/coverclock/com-diag-hazer<BR>
 */

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_dump.h"

/**
 * @def BEGIN
 * Begin a unit test using a test string that may contain escape sequences.
 * string points to the expanded _MESSAGE_. size is the number of bytes in the
 * expanded _MESSAGE_ including the terminating NUL. message points to the
 * collapsed and perhaps unprintable message. size is the number of bytes in
 * the collapsed message and does not include any terminating NUL.
 */
#define BEGIN(_MESSAGE_) \
    do { \
        const char * string = (const char *)0; \
        size_t length = 0; \
        uint8_t  * message = (uint8_t *)0; \
        size_t size = 0; \
        string = (const char *)(_MESSAGE_); \
        length = strlen(string) + 1; \
        message = (uint8_t *)malloc(length); \
        size = diminuto_escape_collapse((char *)message, (const char *)string, length); \
        size -= 1; \
        do { \
            (void)0

/**
 * @def END
 * End a unit test using a test string.
 */
#define END \
        } while (0); \
        free(message); \
    } while (0)

#undef NDEBUG

#endif
