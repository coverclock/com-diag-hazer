/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 *
 * EXAMPLE
 *
 * serialtool -D /dev/ttyUSB0 -b 4800 -8 -1 -n -l | unittest-hazer
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/hazer/hazer.h"
#include "../src/hazer.h"

int main(int argc, char * argv[])
{
    const char * program = (const char *)0;
	hazer_buffer_t buffer = { 0 };
    char * bb = (char *)0;
    ssize_t size = 0;
    ssize_t ss = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    int bad = 0;
    int gga = 0;
    int gll = 0;
    int gsa = 0;
    int gsv = 0;
    int rmc = 0;
    int vtg = 0;
    int oth = 0;
    const char * type = (const char *)0;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    if (argc > 1) {
        hazer_debug(stderr);
    }

    while (!0) {

        size = hazer_nmea_read(stdin, buffer, sizeof(buffer));

        if (size < 0) {
            fprintf(stderr, "%s: ERR\n", program);
            return 1;
        }

         if (size == 0) {
            fprintf(stderr, "%s: EOF\n", program);
            return 0;
        }

        check = hazer_nmea_check(buffer, size);

        if (check != size) {
            type = "BAD";
            ++bad;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_GGA, sizeof(HAZER_NMEA_MESSAGE_GGA) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_GGA;
            ++gga;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_GLL, sizeof(HAZER_NMEA_MESSAGE_GLL) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_GLL;
            ++gll;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_GSA, sizeof(HAZER_NMEA_MESSAGE_GSA) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_GSA;
            ++gsa;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_GSV, sizeof(HAZER_NMEA_MESSAGE_GSV) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_GSV;
            ++gsv;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_RMC, sizeof(HAZER_NMEA_MESSAGE_RMC) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_RMC;
            ++rmc;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_MESSAGE_VTG, sizeof(HAZER_NMEA_MESSAGE_VTG) - 1) == 0) {
            type = HAZER_NMEA_MESSAGE_VTG;
            ++vtg;
        } else {
            type = "OTH";
            ++oth;
        }

        for (bb = buffer, ss = size; ss > 0; --ss) {
            diminuto_phex_emit(stderr, *(bb++), ~0, 0, 0, 0, &current, &end, 0);
        }
        fprintf(stderr, " [%ld] %s %d %d %d %d %d %d %d %d\n", size, type, bad, gga, gll, gsa, gsv, rmc, vtg, oth);

        if (bad || oth) {
            return 2;
        }

        if (gga && gll && gsa && gsv && rmc && vtg) {
            break;
        }
    }

    return 0;
}
