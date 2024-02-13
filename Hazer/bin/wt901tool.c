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
 * wt901setup | serialtool -D /dev/ttyUSB0 -T -b 115200 -8 -1 -n -P | wt901tool -d -v -t -c
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
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
    static const wint_t DEGREE = 0xb0U;
    int xc = 0;
    int rc = 0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int text = 0;
    int csv = 0;
    int error = 0;
    const char * program = (const char *)0;
    char * locale = (char *)0;
    int ch = -1;
    dally_state_t state = DALLY_STATE_START;
    dally_packet_t packet = { 0, };
    dally_context_t context = { 0, };
    dally_context_t * contextp = (dally_context_t *)0;
    dally_acceleration_t acceleration = { 0.0, };
    dally_magneticfield_t magneticfield = { 0.0, };
    dally_quaternion_t quaternion = { 0.0, };
    dally_temperature_t temperature = { 0.0, };

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, "?cdtv")) >= 0) {
        switch (opt) {
        case 'c':
            csv = !0;
            break;
        case 'd':
            debug = !0;
            break;
        case 't':
            text = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -c ] [ -d ] [ -t ] [ -v ]\n", program);
            fprintf(stderr, "       -c              Emit CSV output on standard error.\n");
            fprintf(stderr, "       -d              Display debug output on standard error.\n");
            fprintf(stderr, "       -t              Emit text output on standard output.\n");
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

    rc = setenv("LC_ALL", "en_US.UTF-8", 0);
    diminuto_contract(rc >= 0);

    locale = setlocale(LC_ALL, "");
    diminuto_contract(locale != (char *)0);

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

        switch (packet.d.flag) {
        case DALLY_FLAG_DATA:
            acceleration.ax = dally_value2acceleration(dally_word2value(packet.d.payload[0]));
            acceleration.ay = dally_value2acceleration(dally_word2value(packet.d.payload[1]));
            acceleration.az = dally_value2acceleration(dally_word2value(packet.d.payload[2]));
            acceleration.wx = dally_value2angularvelocity(dally_word2value(packet.d.payload[3]));
            acceleration.wy = dally_value2angularvelocity(dally_word2value(packet.d.payload[4]));
            acceleration.wz = dally_value2angularvelocity(dally_word2value(packet.d.payload[5]));
            acceleration.roll = dally_value2angle(dally_word2value(packet.d.payload[6]));
            acceleration.pitch = dally_value2angle(dally_word2value(packet.d.payload[7]));
            acceleration.yaw = dally_value2angle(dally_word2value(packet.d.payload[8]));
            if (text) {
                printf("%s ACC ax %7.3fg, ay %7.3fg, az %7.3fg\n", program, acceleration.ax, acceleration.ay, acceleration.az);
                printf("%s ANG wx %8.2f%lc/s, wy %8.2f%lc/s, wz %8.2f%lc/s\n", program, acceleration.wx, DEGREE, acceleration.wy, DEGREE, acceleration.wz, DEGREE);
                printf("%s POS rol %7.2f%lc, pit %7.2f%lc, yaw %7.2f%lc\n", program, acceleration.roll, DEGREE, acceleration.pitch, DEGREE, acceleration.yaw, DEGREE);
            }
            if (csv) {
                printf("\"%s\",\"ACC\",%f,%f,%f\n", program, acceleration.ax, acceleration.ay, acceleration.az);
                printf("\"%s\",\"ANG\",%f,%f,%f\n", program, acceleration.wx, acceleration.wy, acceleration.wz);
                printf("\"%s\",\"POS\",%f,%f,%f\n", program, acceleration.roll, acceleration.pitch, acceleration.yaw);
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
                magneticfield.hx = dally_value2magneticfield(dally_word2value(packet.r.payload[0]));
                magneticfield.hy = dally_value2magneticfield(dally_word2value(packet.r.payload[1]));
                magneticfield.hz = dally_value2magneticfield(dally_word2value(packet.r.payload[2]));
                if (text) {
                    printf("%s MAG hx %7.2fmG, hy %7.2fmG, hz %7.2fmG\n", program, magneticfield.hx, magneticfield.hy, magneticfield.hz);
                }
                if (csv) {
                    printf("\"%s\",\"MAG\",%f,%f,%f\n", program, magneticfield.hx, magneticfield.hy, magneticfield.hz);
                }
                break;
            case DALLY_REGISTER_QUATERNION:
                quaternion.q0 = dally_value2quaternion(dally_word2value(packet.r.payload[0]));
                quaternion.q1 = dally_value2quaternion(dally_word2value(packet.r.payload[1]));
                quaternion.q2 = dally_value2quaternion(dally_word2value(packet.r.payload[2]));
                if (text) {
                    printf("%s QUA q0 %7.4f, q1 %7.4f, q2 %7.4f\n", program, quaternion.q0, quaternion.q1, quaternion.q2);
                }
                if (csv) {
                    printf("\"%s\",\"QUA\",%f,%f,%f\n", program, quaternion.q0, quaternion.q1, quaternion.q2);
                }
                break;
            case DALLY_REGISTER_TEMPERATURE:
                temperature.t = dally_value2temperature(dally_word2value(packet.r.payload[0]));
                if (text) {
                    printf("%s TEM %7.2f%lcC %7.2f%lcF\n", program, temperature.t, DEGREE, ((temperature.t * 9.0 / 5.0) + 32.0), DEGREE);
                }
                if (csv) {
                    printf("\"%s\",\"TEM\",%f,%f\n", program, temperature.t, ((temperature.t * 9.0 / 5.0) + 32.0));
                }
                break;
            default:
                fprintf(stderr, "%s: Register 0x%x\n", program, packet.r.reg);
                break;
            }
            break;
        default:
            fprintf(stderr, "%s: Flag 0x%x\n", program, packet.d.flag);
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
