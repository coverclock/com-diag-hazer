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
 * Start all state machines.
 * @param nsp points to the NMEA state.
 * @param usp points to the UBX state.
 * @param rsp points to the RTCM state.
 * @param csp points to the CPO state.
 */
void machine_start_all(hazer_state_t * nsp, yodel_state_t * usp, tumbleweed_state_t * rsp, calico_state_t * csp) {
    *nsp = HAZER_STATE_START;
    *usp = YODEL_STATE_START;
    *rsp = TUMBLEWEED_STATE_START;
    *csp = CALICO_STATE_START;
}

/**
 * Start NMEA state machine, stop others.
 * @param nsp points to the NMEA state.
 * @param usp points to the UBX state.
 * @param rsp points to the RTCM state.
 * @param csp points to the CPO state.
 */
void machine_start_nmea(hazer_state_t * nsp, yodel_state_t * usp, tumbleweed_state_t * rsp, calico_state_t * csp) {
    *nsp = HAZER_STATE_START;
    *usp = YODEL_STATE_STOP;
    *rsp = TUMBLEWEED_STATE_STOP;
    *csp = CALICO_STATE_STOP;
}

/**
 * Start UBX state machine, stop others.
 * @param nsp points to the NMEA state.
 * @param usp points to the UBX state.
 * @param rsp points to the RTCM state.
 * @param csp points to the CPO state.
 */
void machine_start_ubx(hazer_state_t * nsp, yodel_state_t * usp, tumbleweed_state_t * rsp, calico_state_t * csp) {
    *nsp = HAZER_STATE_STOP;
    *usp = YODEL_STATE_START;
    *rsp = TUMBLEWEED_STATE_STOP;
    *csp = CALICO_STATE_STOP;
}

/**
 * Start RTCM state machines, stop others.
 * @param nsp points to the NMEA state.
 * @param usp points to the UBX state.
 * @param rsp points to the RTCM state.
 * @param csp points to the CPO state.
 */
void machine_start_rtcm(hazer_state_t * nsp, yodel_state_t * usp, tumbleweed_state_t * rsp, calico_state_t * csp) {
    *nsp = HAZER_STATE_STOP;
    *usp = YODEL_STATE_STOP;
    *rsp = TUMBLEWEED_STATE_START;
    *csp = CALICO_STATE_STOP;
}

/**
 * Start CPO state machines, stop others.
 * @param nsp points to the NMEA state.
 * @param usp points to the UBX state.
 * @param rsp points to the RTCM state.
 * @param csp points to the CPO state.
 */
void machine_start_cpo(hazer_state_t * nsp, yodel_state_t * usp, tumbleweed_state_t * rsp, calico_state_t * csp) {
    *nsp = HAZER_STATE_STOP;
    *usp = YODEL_STATE_STOP;
    *rsp = TUMBLEWEED_STATE_STOP;
    *csp = CALICO_STATE_START;
}

/**
 * Return true if NMEA, UBX, RTCM, and CPO state machines are stalled.
 * @param ns is the state of the NMEA state machine.
 * @param us is the state of the UBX state machine.
 * @param rs is the state of the RTCM state machine.
 * @param cs is the state of the CPO state machine.
 * @return true if stalled, false otherwise.
 */
int machine_is_stalled(hazer_state_t ns, yodel_state_t us, tumbleweed_state_t rs, calico_state_t cs);

#endif
