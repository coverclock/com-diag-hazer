/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Thread API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_criticalsection.h"
#include "com/diag/diminuto/diminuto_coherentsection.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "types.h"
#include "globals.h"
#include "threads.h"

/**
 * Implement a thread that polls for the data carrier detect (DCD) state for
 * 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
void * dcdpoller(void * argp)
{
    void * xc = (void *)1;
    poller_t * pollerp = (poller_t *)0;
    int done = 0;
    int rc = -1;
    int nowpps = 0;
    int waspps = 0;

    pollerp = (poller_t *)argp;

    while (!0) {
        DIMINUTO_COHERENT_SECTION_BEGIN;
            done = pollerp->done;
        DIMINUTO_COHERENT_SECTION_END;
        if (done) {
            xc = (void *)0;
            break;
        }
        rc = diminuto_serial_wait(fileno(pollerp->ppsfp));
        if (rc < 0) { break; }
        rc = diminuto_serial_status(fileno(pollerp->ppsfp));
        if (rc < 0) { break; }
        nowpps = !!rc;
        if (nowpps == waspps) {
            /* Do nothing. */
        } else if (nowpps) {
            if (pollerp->strobefp != (FILE *)0) {
                rc = diminuto_pin_set(pollerp->strobefp);
                if (rc < 0) { break; }
            }
            DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
                pollerp->onepps = (pollerp->onepps + 1) % 60;
            DIMINUTO_CRITICAL_SECTION_END;
        } else {
            if (pollerp->strobefp != (FILE *)0) {
                rc = diminuto_pin_clear(pollerp->strobefp);
                if (rc < 0) { break; }
            }
        }
        waspps = nowpps;
    }

    return xc;
}

/**
 * Implement a thread that polls for the general purpose input/output (GPIO)
 * state for 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
void * gpiopoller(void * argp)
{
    void * xc = (void *)1;
    poller_t * pollerp = (poller_t *)0;
    diminuto_mux_t mux = { 0 };
    int ppsfd = -1;
    int done = 0;
    int rc = -1;
    int fd = -1;
    int nowpps = 0;
    int waspps = 0;

    pollerp = (poller_t *)argp;

    diminuto_mux_init(&mux);
    ppsfd = fileno(pollerp->ppsfp);
    rc = diminuto_mux_register_interrupt(&mux, ppsfd);
    diminuto_contract(rc >= 0);

    while (!0) {
        DIMINUTO_COHERENT_SECTION_BEGIN;
            done = pollerp->done;
        DIMINUTO_COHERENT_SECTION_END;
        if (done) {
            xc = (void *)0;
            break;
        }
        rc = diminuto_mux_wait(&mux, -1);
        if (rc <= 0) { break; }
        while (!0) {
            fd = diminuto_mux_ready_interrupt(&mux);
            if (fd < 0) { break; }
            diminuto_contract(fd == ppsfd);
            rc = diminuto_pin_get(pollerp->ppsfp);
            if (rc < 0) { break; }
            nowpps = !!rc;
            if (nowpps == waspps) {
                /* Do nothing. */
            } else if (nowpps) {
                if (pollerp->strobefp != (FILE *)0) {
                    rc = diminuto_pin_set(pollerp->strobefp);
                    if (rc < 0) { break; }
                }
                DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
                    pollerp->onepps = (pollerp->onepps + 1) % 60;
                DIMINUTO_CRITICAL_SECTION_END;
            } else {
                if (pollerp->strobefp != (FILE *)0) {
                    rc = diminuto_pin_clear(pollerp->strobefp);
                    if (rc < 0) { break; }
                }
            }
            waspps = nowpps;
        }
        if (rc < 0) { break; }
    }

    rc = diminuto_mux_unregister_interrupt(&mux, ppsfd);
    diminuto_mux_fini(&mux);

    return xc;
}
