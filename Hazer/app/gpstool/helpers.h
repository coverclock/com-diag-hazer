/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_
#define _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_

/**
 * @file
 *
 * Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/tumbleweed.h"
#include "types.h"

/**
 * Return monotonic time in seconds.
 * @param frequency is the underlying clock frequency.
 * @return monotonic time in seconds.
 */
static inline long ticktock(diminuto_sticks_t frequency)
{
    return diminuto_time_elapsed() / frequency;
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
