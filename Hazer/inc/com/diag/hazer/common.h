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
#include <wchar.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/calico.h"

#if !defined(COMMON_DEGREE_VALUE)
    /**
     * @def COMMON_DEGREE_VALUE
     * Define the value that is used for the degree symbol. By default this
     * is the wide character Unicode for the degree symbol, but can be
     * defined at compile time to be something else like '*'.
     */
#   define COMMON_DEGREE_VALUE 0x00b0
#endif

/**
 * This is the Unicode for the degree symbol.
 */
static const wchar_t COMMON_DEGREE = COMMON_DEGREE_VALUE;

#if !defined(COMMON_PLUSMINUS_VALUE)
    /**
     * @def COMMON_PLUSMINUS_VALUE
     * Define the value that is used for the plus minus symbol. By default this
     * is the wide character Unicode for the plus minus symbol, but can be
     * defined at compile time to be something else like '~'.
     */
#   define COMMON_PLUSMINUS_VALUE 0x00b1
#endif

/**
 * This is the Unicode for the plus minus symbol.
 */
static const wchar_t COMMON_PLUSMINUS = COMMON_PLUSMINUS_VALUE;

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
 * Return true if NMEA, UBX, RTCM, and CPO state machines are stalled.
 * @param nmea_state is the state of the NMEA state machine.
 * @param ubx_state is the state of the UBX state machine.
 * @param rtcm_state is the state of the RTCM state machine.
 * @param cpo_state is the state of the CPO state machine.
 * @return true if stalled, false otherwise.
 */
int common_machine_is_stalled(hazer_state_t nmea_state, yodel_state_t ubx_state, tumbleweed_state_t rtcm_state, calico_state_t cpo_state);

#endif
