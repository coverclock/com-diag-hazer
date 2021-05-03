/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_
#define _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares and defines the gpstool Helpers.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "types.h"

/**
 * Return monotonic time in seconds.
 * @return monotonic time in seconds.
 */
static inline seconds_t ticktock(void)
{
    return diminuto_time_elapsed() / diminuto_frequency();
}

/**
 * Return true if specified number of seconds has elapsed, and if so
 * update the previous elapsed seconds value-result variable.
 * @param wasp points to the variable containing previous elapsed seconds.
 * @param seconds is the number of seconds desired to elapse.
 * @return true if the specified number of seconds has elapsed.
 */
static inline int dingdong(seconds_t * wasp, seconds_t seconds)
{
    int result;
    seconds_t now;

    if ((result = ((now = ticktock()) >= (*wasp + seconds)))) {
        *wasp = now;
    }

    return result;
}

/**
 * Common function to count down the expiration fields in the database.
 * @param ep points to the expiration field to count down.
 * @param elapsed is the number of ticks to count down.
 */
static inline void countdown(hazer_expiry_t * ep, diminuto_sticks_t elapsed)
{
    if (*ep == 0) {
        /* Do nothing. */
    } else if (elapsed <= 0) {
        /* Do nothing. */
    } else if (*ep <= elapsed) {
        *ep = 0;
    } else {
        *ep -= elapsed;
    }
}

/**
 * Track RTK updates by encoding each received RTCM message as a single
 * character in a shifting string.
 * @param number is the RTCM message number.
 * @param up points to the updates union.
 */
extern void collect(int number, tumbleweed_updates_t * up);

#endif
