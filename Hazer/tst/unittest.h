/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_TST_UNITTEST_
#define _H_COM_DIAG_HAZER_TST_UNITTEST_

/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
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

#define BEGIN(_MESSAGE_) \
	do { \
		const char * string = (const char *)0; \
		size_t length = 0; \
		unsigned char * message = (unsigned char *)0; \
		size_t size = 0; \
		string = (_MESSAGE_); \
		length = strlen(string) + 1; \
		message = (char *)malloc(length); \
		size = diminuto_escape_collapse(message, string, length); \
		size -= 1; \
		do { \
			(void)0

#define END \
		} while (0); \
		free(message); \
	} while (0)

#endif
