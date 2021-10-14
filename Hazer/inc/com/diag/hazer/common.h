/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_COMMON_
#define _H_COM_DIAG_HAZER_COMMON_

/**
 * @file
 * @copyright Copyright 2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Common facilities for other stuff.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdint.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"

/**
 * Return the absolute value of a 64-bit integer.
 * @param x is the integer.
 * @return its absolute value.
 */
static inline int64_t common_abs64(int64_t x)
{
    return (x < 0) ? -x : x;
}

#if !defined(abs64)
#   define abs64 common_abs64
#endif

/**
 * Return true if the character is the first of an NMEA sentence.
 * @param ch is the character.
 * @return true if NMEA, false otherwise.
 */
static inline int common_machine_is_nmea(int ch)
{
    return ((ch == HAZER_STIMULUS_START) || (ch == HAZER_STIMULUS_ENCAPSULATION));
}

/**
 * Return true if the character is the first of a UBX packet.
 * @param ch is the character.
 * @return true if UBX, false otherwise.
 */
static inline int common_machine_is_ubx(int ch)
{
    return (ch == YODEL_STIMULUS_SYNC_1);
}

/**
 * Return true if the character is the first of an RTCM message.
 * @param ch is the character.
 * @return true if RTCM, false otherwise.
 */
static inline int common_machine_is_rtcm(int ch)
{
    return (ch == TUMBLEWEED_STIMULUS_PREAMBLE);
}

/**
 * Return true if NMEA, UBX, and RTCM state machines are stalled.
 * @param nmea_state is the state of the NMEA state machine.
 * @param ubx_state is the state of the UBX state machine.
 * @param rtcm_state is the state of the RTCM state machine.
 * @return true if stalled, false otherwise.
 */
static inline int common_machine_is_stalled(hazer_state_t nmea_state, yodel_state_t ubx_state, tumbleweed_state_t rtcm_state)
{
    int result = 0;

    if ((nmea_state == HAZER_STATE_START) && (ubx_state == YODEL_STATE_START) && (rtcm_state == TUMBLEWEED_STATE_START)) {
        /* Do nothing: all are scanning for beginning of frame. */
    } else if ((nmea_state != HAZER_STATE_START) && (nmea_state != HAZER_STATE_STOP)) {
        /* Do nothing: NMEA is processing. */
    } else if ((ubx_state != YODEL_STATE_START) && (ubx_state != YODEL_STATE_STOP)) {
        /* Do nothing: UBX is processing. */
    } else if ((rtcm_state != TUMBLEWEED_STATE_START) && (rtcm_state != TUMBLEWEED_STATE_STOP)) {
        /* Do nothing: RTCM is processing. */
    } else {
        result = !0;
    }

    return result;
}

#endif
