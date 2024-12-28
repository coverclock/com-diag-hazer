/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2024 Digital Aggregates Corporation, Colorado, USA.
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
#include "com/diag/diminuto/diminuto_line.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "constants.h"
#include "globals.h"
#include "threads.h"
#include "types.h"

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
        rc = diminuto_serial_wait(pollerp->ppsfd);
        if (rc < 0) { break; }
        rc = diminuto_serial_status(pollerp->ppsfd);
        if (rc < 0) { break; }
        nowpps = !!rc;
        if (nowpps == waspps) {
            /* Do nothing. */
        } else if (nowpps) {
            if (pollerp->strobefd >= 0) {
                rc = diminuto_line_set(pollerp->strobefd);
                if (rc < 0) { break; }
            }
            DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
                pollerp->onepps %= MODULO;  /* 0..(MODULO-1) */
                pollerp->onepps += 1;       /* 1..MODULO */
                pollerp->onehz = 0;         /* 0..TOLERANCE */
            DIMINUTO_CRITICAL_SECTION_END;
        } else {
            if (pollerp->strobefd >= 0) {
                rc = diminuto_line_clear(pollerp->strobefd);
                if (rc < 0) { break; }
            }
        }
        waspps = nowpps;
    }

    return xc;
}

void * gpiopoller(void * argp)
{
    void * xc = (void *)1;
    poller_t * pollerp = (poller_t *)0;
    diminuto_mux_t mux = { 0 };
    int done = 0;
    int rc = -1;
    int fd = -1;
    int nowpps = 0;
    int waspps = 0;

    pollerp = (poller_t *)argp;

    diminuto_mux_init(&mux);
    rc = diminuto_mux_register_read(&mux, pollerp->ppsfd);
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
            fd = diminuto_mux_ready_read(&mux);
            if (fd < 0) { break; }
            diminuto_contract(fd == pollerp->ppsfd);
            rc = diminuto_line_read(pollerp->ppsfd);
            if (rc < 0) { break; }
            nowpps = !!rc;
            /*
             * THe strobe, if it exists, follows the value of 1PPS
             * as closely as possible. But we only change strobe when
             * we know that 1PPS has changed.
             */
            if (nowpps == waspps) {
                /* Do nothing. */
            } else if (nowpps) {
                if (pollerp->strobefd >= 0) {
                    rc = diminuto_line_set(pollerp->strobefd);
                    if (rc < 0) { break; }
                }
                DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
                    pollerp->onepps %= MODULO;  /* 0..(MODULO-1) */
                    pollerp->onepps += 1;       /* 1..MODULO */
                    pollerp->onehz = 0;         /* 0..TOLERANCE */
                DIMINUTO_CRITICAL_SECTION_END;
            } else {
                if (pollerp->strobefd >= 0) {
                    rc = diminuto_line_clear(pollerp->strobefd);
                    if (rc < 0) { break; }
                }
            }
            waspps = nowpps;
        }
        if (rc < 0) { break; }
    }

    rc = diminuto_mux_unregister_read(&mux, pollerp->ppsfd);
    diminuto_mux_fini(&mux);

    return xc;
}

void * timerservice(void * argp)
{
    void * xc = (void *)0;
    poller_t * pollerp = (poller_t *)0;

    pollerp = (poller_t *)argp;

    DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
        if (pollerp->onehz < TOLERANCE) {
            pollerp->onehz += 1;            /* 0..TOLERANCE */
        }
    DIMINUTO_CRITICAL_SECTION_END;

    return xc;
}
