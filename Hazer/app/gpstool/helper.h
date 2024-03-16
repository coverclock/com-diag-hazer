/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_HELPER_
#define _H_COM_DIAG_HAZER_GPSTOOL_HELPER_

/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares and defines the gpstool Helper functionss.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * These are functions with no other good place to reside.
 */

#include "types.h"

/**
 * Track RTK updates by encoding each received RTCM message as a single
 * character in a shifting string.
 * @param number is the RTCM message number.
 * @param up points to the updates union.
 */
extern void helper_collect(int number, tumbleweed_updates_t * up);

/**
 * Allocate a buffer to hold a string, and copy the string into it.
 * The caller is responsible for deallocating the buffer.
 * @param string points to the string to copy.
 * @return a pointer to the buffer or NULL if an error occurred.
 */
extern char * helper_salloc(const char * string);

#endif
