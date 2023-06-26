/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_MACHINE_
#define _H_COM_DIAG_HAZER_MACHINE_

/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Functions for managing all of the state machines.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/calico.h"

/**
 * Return true if NMEA, UBX, RTCM, and CPO state machines are stalled.
 * @param nmea_state is the state of the NMEA state machine.
 * @param ubx_state is the state of the UBX state machine.
 * @param rtcm_state is the state of the RTCM state machine.
 * @param cpo_state is the state of the CPO state machine.
 * @return true if stalled, false otherwise.
 */
int machine_is_stalled(hazer_state_t nmea_state, yodel_state_t ubx_state, tumbleweed_state_t rtcm_state, calico_state_t cpo_state);

#endif
