/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_TIME_
#define _H_COM_DIAG_HAZER_GPSTOOL_TIME_

/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares and defines the gpstool time functions.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "types.h"
#include "globals.h"

/**
 * Return true if specified number of seconds has elapsed, and if so
 * update the previous elapsed seconds value-result variable.
 * @param wasp points to the variable containing previous elapsed seconds.
 * @param seconds is the number of seconds desired to elapse, <0 for never, 0 for always.
 * @return true if the specified number of seconds has elapsed.
 */
extern int time_expired(seconds_t * wasp, seconds_t seconds);

/**
 * Count down an expiration fields in the database. This doesn't return
 * anything because the expiration field is interrogated separately by
 * the appropriate print function.
 * @param ep points to the expiration field to count down.
 * @param seconds is the number of seconds to count down.
 */
extern void time_countdown(hazer_expiry_t * ep, seconds_t seconds);

#endif
