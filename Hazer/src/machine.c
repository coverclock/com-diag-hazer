/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Machine module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/machine.h"

int machine_is_stalled(hazer_state_t ns, yodel_state_t us, tumbleweed_state_t rs, calico_state_t cs)
{
    int result = 0;

    if (
        (ns == HAZER_STATE_START) &&
        (us == YODEL_STATE_START) &&
        (rs == TUMBLEWEED_STATE_START) &&
        (cs == CALICO_STATE_START)
    ) {
        /* Do nothing: all are scanning for beginning of frame. */
    } else if (
        (ns != HAZER_STATE_START) &&
        (ns != HAZER_STATE_STOP)
    ) {
        /* Do nothing: NMEA is processing. */
    } else if (
        (us != YODEL_STATE_START) &&
        (us != YODEL_STATE_STOP)
    ) {
        /* Do nothing: UBX is processing. */
    } else if (
        (rs != TUMBLEWEED_STATE_START) &&
        (rs != TUMBLEWEED_STATE_STOP)
    ) {
        /* Do nothing: RTCM is processing. */
    } else if (
        (cs != CALICO_STATE_START) &&
        (cs != CALICO_STATE_STOP)
    ) {
        /* Do nothing: CPO is processing. */
    } else {
        result = !0;
    }

    return result;
}
