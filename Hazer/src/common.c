/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Datagram module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/common.h"

/**
 * Return true if NMEA, UBX, RTCM, and DIS state machines are stalled.
 * @param nmea_state is the state of the NMEA state machine.
 * @param ubx_state is the state of the UBX state machine.
 * @param rtcm_state is the state of the RTCM state machine.
 * @param dis_state is the state of the DIS state machine.
 * @return true if stalled, false otherwise.
 */
int common_machine_is_stalled(hazer_state_t nmea_state, yodel_state_t ubx_state, tumbleweed_state_t rtcm_state, calico_state_t dis_state)
{
    int result = 0;

    if ((nmea_state == HAZER_STATE_START) && (ubx_state == YODEL_STATE_START) && (rtcm_state == TUMBLEWEED_STATE_START) && (dis_state == CALICO_STATE_START)) {
        /* Do nothing: all are scanning for beginning of frame. */
    } else if ((nmea_state != HAZER_STATE_START) && (nmea_state != HAZER_STATE_STOP)) {
        /* Do nothing: NMEA is processing. */
    } else if ((ubx_state != YODEL_STATE_START) && (ubx_state != YODEL_STATE_STOP)) {
        /* Do nothing: UBX is processing. */
    } else if ((rtcm_state != TUMBLEWEED_STATE_START) && (rtcm_state != TUMBLEWEED_STATE_STOP)) {
        /* Do nothing: RTCM is processing. */
    } else if ((dis_state != CALICO_STATE_START) && (dis_state != CALICO_STATE_STOP)) {
        /* Do nothing: DIS is processing. */
    } else {
        result = !0;
    }

    return result;
}
