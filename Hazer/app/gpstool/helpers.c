/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Helpers.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "types.h"
#include "helpers.h"

/**
 * Track RTK updates by encoding each received RTCM message as a single
 * character in a shifting string.
 * @param number is the RTCM message number.
 * @param up points to the updates union.
 */
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
