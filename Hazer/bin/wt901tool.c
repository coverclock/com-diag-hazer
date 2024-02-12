/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements a filter to process WT901 IMU data.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * wt901setup | serialtool -D /dev/ttyUSB0 -T -b 115200 -8 -1 -n -P | wt901tool -v
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_error.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/hazer/dally.h"

int main(int argc, char * argv[])
{
    int xc = 0;
    int rc = 0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int error = 0;
    const char * program = (const char *)0;
    dally_state_t state = DALLY_STATE_START;
    dally_packet_t packet __attribute__ ((aligned (16))) = { 0, };
    dally_context_t context = { 0, };
    dally_context_t * contextp = (dally_context_t *)0;
    int ch = -1;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, "?dv")) >= 0) {
        switch (opt) {
        case 'd':
            debug = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -d ] [ -v ]\n", program);
            fprintf(stderr, "       -d              Display debug output on standard error.\n");
            fprintf(stderr, "       -v              Display verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    if (error) {
        return 1;
    }

    /*
     * INIT
     */

    rc = diminuto_hangup_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_interrupter_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_pipe_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_terminator_install(0);
    diminuto_contract(rc >= 0);

    contextp = dally_init(&context, &packet);
    diminuto_contract(contextp == &context);

    if (debug) {
        dally_debug(stderr);
    }

    if (verbose) {
        fprintf(stderr, "%s: init\n", program);
    }

    /*
     * PROCESS
     */

    while (!0) {

        if (diminuto_hangup_check()) {
            /* Do nothing. */
        }

        if (diminuto_interrupter_check()) {
            break;
        }

        if (diminuto_pipe_check()) {
            break;
        }

        if (diminuto_terminator_check()) {
            break;
        }

        ch = fgetc(stdin);
        if (ch < 0) {
            break;
        }

        state = dally_machine(&context, ch);
        diminuto_contract(state != DALLY_STATE_ERROR);

        if (state != DALLY_STATE_FINAL) {
            continue;
        }

        switch (packet.p.flag) {
        case DALLY_FLAG_DATA:
            if (verbose) {
                fprintf(stderr, "%s: Accelerometer\n", program);
            }
            break;
        case DALLY_FLAG_REGISTER:
            switch (packet.r.reg) {
            case DALLY_REGISTER_YEARMONTH:
                if (verbose) {
                    fprintf(stderr, "%s: YearMonth\n", program);
                }
                break;
            case DALLY_REGISTER_DATEHOUR:
                if (verbose) {
                    fprintf(stderr, "%s: DateHour\n", program);
                }
                break;
            case DALLY_REGISTER_MINUTESECOND:
                if (verbose) {
                    fprintf(stderr, "%s: MinuteSecond\n", program);
                }
                break;
            case DALLY_REGISTER_MILLISECOND:
                if (verbose) {
                    fprintf(stderr, "%s: Millisecond\n", program);
                }
                break;
            case DALLY_REGISTER_MAGNETICFIELD:
                if (verbose) {
                    fprintf(stderr, "%s: Compass\n", program);
                }
                break;
            case DALLY_REGISTER_TEMPERATURE:
                if (verbose) {
                    fprintf(stderr, "%s: Thermometer\n", program);
                }
                break;
            case DALLY_REGISTER_QUATERNION:
                if (verbose) {
                    fprintf(stderr, "%s: Quaternion\n", program);
                }
                break;
            default:
                fprintf(stderr, "%s: Register 0x%x\n", program, packet.r.reg);
                break;
            }
            break;
        default:
            fprintf(stderr, "%s: Flag 0x%x\n", program, packet.p.flag);
            break;
        }

        if (verbose) {
            diminuto_dump(stderr, &packet, sizeof(packet));
        }

        contextp = dally_reset(&context);
        diminuto_contract(contextp == &context);

    }

    /*
     * FINI
     */

    if (verbose) {
        fprintf(stderr, "%s: fini\n", program);
    }

    if (debug) {
        dally_debug((FILE *)0);
    }

    contextp = dally_fini(&context);
    diminuto_contract(contextp == (dally_context_t *)0);

    fflush(stderr);

    return xc;
}
