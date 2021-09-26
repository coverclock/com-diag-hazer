/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Helpers.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "types.h"
#include "globals.h"
#include "helpers.h"

int expired(seconds_t * wasp, seconds_t seconds)
{
    int result = 0;
    seconds_t now = 0;

    if (seconds < 0) {
        /* Do nothing. */
    } else if (seconds == 0) {
        result = !0;
    } else if ((result = ((now = Now / Frequency) >= (*wasp + seconds)))) {
        *wasp = now;
    } else {
        /* Do nothing. */
    }

    return result;
}

int expiring(const seconds_t * wasp, seconds_t seconds)
{
    int result = 0;
    seconds_t now = 0;

    if (seconds < 0) {
        /* Do nothing. */
    } else if (seconds == 0) {
        result = !0;
    } else {
        result = ((now = Now / Frequency) >= (*wasp + seconds));
    }

    return result;
}

void countdown(hazer_expiry_t * ep, diminuto_sticks_t elapsed)
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

void collect(int number, tumbleweed_updates_t * up)
{
    update_t update = UPDATE;

    switch (number) {

    case 1005:
        update = RTCM_TYPE_1005;
        break;

    case 1074:
        update = RTCM_TYPE_1074;
        break;

    case 1084:
        update = RTCM_TYPE_1084;
        break;

    case 1094:
        update = RTCM_TYPE_1094;
        break;

    case 1124:
        update = RTCM_TYPE_1124;
        break;

    case 1230:
        update = RTCM_TYPE_1230;
        break;

    case 9999:
        update = RTCM_TYPE_9999;
        break;

    default:
        /* Do nothing. */
        break;

    }

    up->word = (up->word << 8) | update;

}
