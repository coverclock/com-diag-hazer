/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool time functions.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "globals.h"
#include "time.h"
#include "types.h"

int time_expired(seconds_t * wasp, seconds_t seconds)
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

void time_countdown(hazer_expiry_t * ep, seconds_t seconds)
{
    if (*ep == 0) {
        /* Do nothing. */
    } else if (seconds <= 0) {
        /* Do nothing. */
    } else if (*ep <= seconds) {
        *ep = 0;
    } else {
        *ep -= seconds;
    }
}
