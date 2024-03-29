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
 * wt901wired | serialtool -D /dev/ttyUSB0 -b 115200 -8 -1 -n -P | wt901tool -d -v -t -c
 *
 * [NEW] Device F3:C1:80:74:D6:3E WT901BLE68
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include "com/diag/diminuto/diminuto_ansi.h"
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_error.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_unicode.h"
#include "com/diag/hazer/dally.h"

int main(int argc, char * argv[])
{
    int xc = 0;
    int rc = 0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int text = 0;
    int csv = 0;
    int escape = 0;
    int error = 0;
    const char * program = (const char *)0;
    char * locale = (char *)0;
    char hostname[HOST_NAME_MAX] = { '\0', };
    int ch = -1;
    dally_state_t state = DALLY_STATE_START;
    dally_packet_t packet = { 0, };
    dally_context_t context = { 0, };
    dally_context_t * contextp = (dally_context_t *)0;
    dally_acceleration_t acceleration = { 0.0, };
    dally_magneticfield_t magneticfield = { 0.0, };
    dally_quaternion_t quaternion = { 0.0, };
    dally_temperature_t temperature = { 0.0, };
    diminuto_sticks_t clock = 0;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, "?Ecdtv")) >= 0) {
        switch (opt) {
        case 'E':
            escape = !0;
            text = !0;
            break;
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
            fprintf(stderr, "usage: %s [ -E ] [ -c ] [ -d ] [ -t ] [ -v ]\n", program);
            fprintf(stderr, "       -E              Emit text output using ANSI escape sequences.\n");
            fprintf(stderr, "       -c              Emit CSV output on standard output.\n");
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

    if (csv) {
        rc = gethostname(hostname, sizeof(hostname));
        diminuto_contract(rc >= 0);
        hostname[sizeof(hostname) - 1] = '\0';
    }

    contextp = dally_init(&context, &packet);
    diminuto_contract(contextp == &context);

    if (debug) {
        dally_debug(stderr);
    }

    if (verbose) {
        fprintf(stderr, "%s: init\n", program);
    }

    if (escape) {
        fputs(DIMINUTO_ANSI_POSITION_CURSOR(1,1), stdout);
        fputs(DIMINUTO_ANSI_ERASE_SCREEN, stdout);
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

        if (csv) {
            clock = diminuto_time_clock();
            diminuto_contract(clock > 0);
        }

        /*
         * N.B. The (text) printf statements below are formatted quite deliberately
         *      for purposes of column alignment when the -E (ANSI escape sequence)
         *      option is used. I find it makes the screen easier to read as the
         *      values change in real-time. Your mileage may vary.
         */

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

            if (escape) {
                fputs(DIMINUTO_ANSI_POSITION_CURSOR(1,1), stdout);
                fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
            }

            if (text) {
                printf("ACC ax %12.5f g  , ay %12.5f g  , az %12.5f g\n", acceleration.ax, acceleration.ay, acceleration.az);
            }

            if (escape) {
                fputs(DIMINUTO_ANSI_POSITION_CURSOR(2,1), stdout);
                fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
            }

            if (text) {
                printf("ANG wx %12.5f %lc/s, wy %12.5f %lc/s, wz %12.5f %lc/s\n", acceleration.wx, DIMINUTO_UNICODE_DEGREE, acceleration.wy, DIMINUTO_UNICODE_DEGREE, acceleration.wz, DIMINUTO_UNICODE_DEGREE);
            }

            if (escape) {
                fputs(DIMINUTO_ANSI_POSITION_CURSOR(3,1), stdout);
                fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
            }

            if (text) {
                printf("ROT ro %12.5f %lc  , pt %12.5f %lc  , yw %12.5f %lc\n", acceleration.roll, DIMINUTO_UNICODE_DEGREE, acceleration.pitch, DIMINUTO_UNICODE_DEGREE, acceleration.yaw, DIMINUTO_UNICODE_DEGREE);
            }

            if (csv) {
                printf("\"%s\",\"ACC\",%lld,%f,%f,%f\n", hostname, (long long int)clock, acceleration.ax, acceleration.ay, acceleration.az);
                printf("\"%s\",\"ANG\",%lld,%f,%f,%f\n", hostname, (long long int)clock, acceleration.wx, acceleration.wy, acceleration.wz);
                printf("\"%s\",\"ROT\",%lld,%f,%f,%f\n", hostname, (long long int)clock, acceleration.roll, acceleration.pitch, acceleration.yaw);
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

                if (escape) {
                    fputs(DIMINUTO_ANSI_POSITION_CURSOR(4,1), stdout);
                    fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
                }

                if (text) {
                    printf("MAG hx %12.5f mG , hy %12.5f mG , hz %12.5f mG\n", magneticfield.hx, magneticfield.hy, magneticfield.hz);
                }

                if (csv) {
                    printf("\"%s\",\"MAG\",%lld,%f,%f,%f\n", hostname, (long long int)clock, magneticfield.hx, magneticfield.hy, magneticfield.hz);
                }

                break;

            case DALLY_REGISTER_QUATERNION:

                quaternion.q0 = dally_value2quaternion(dally_word2value(packet.r.payload[0]));
                quaternion.q1 = dally_value2quaternion(dally_word2value(packet.r.payload[1]));
                quaternion.q2 = dally_value2quaternion(dally_word2value(packet.r.payload[2]));

                if (escape) {
                    fputs(DIMINUTO_ANSI_POSITION_CURSOR(5,1), stdout);
                    fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
                }

                if (text) {
                    printf("QUA q0 %12.5f    , q1 %12.5f    , q2 %12.5f\n", quaternion.q0, quaternion.q1, quaternion.q2);
                }

                if (csv) {
                    printf("\"%s\",\"QUA\",%lld,%f,%f,%f\n", hostname, (long long int)clock, quaternion.q0, quaternion.q1, quaternion.q2);
                }

                break;

            case DALLY_REGISTER_TEMPERATURE:

                temperature.t = dally_value2temperature(dally_word2value(packet.r.payload[0]));

                if (escape) {
                    fputs(DIMINUTO_ANSI_POSITION_CURSOR(6,1), stdout);
                    fputs(DIMINUTO_ANSI_ERASE_LINE, stdout);
                }

                if (text) {
                    printf("TEM    %12.5f %lcC ,    %12.5f %lcF\n", temperature.t, DIMINUTO_UNICODE_DEGREE, ((temperature.t * 9.0 / 5.0) + 32.0), DIMINUTO_UNICODE_DEGREE);
                }

                if (csv) {
                    printf("\"%s\",\"TEM\",%lld,%f,%f\n", hostname, (long long int)clock, temperature.t, ((temperature.t * 9.0 / 5.0) + 32.0));
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
