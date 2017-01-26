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
#include "com/diag/hazer/hazer_nmea_gps.h"

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
    const char * type = (const char *)0;
    hazer_vector_t vector = { 0 };
    ssize_t tokens = 0;
    char ** vv = (char **)0;

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

        for (bb = buffer, ss = size; ss > 0; --ss) {
            diminuto_phex_emit(stderr, *(bb++), ~0, 0, 0, 0, &current, &end, 0);
        }
        fprintf(stderr, "[%ld]\n", size);

        check = hazer_nmea_check(buffer, size);
        if (check != size) {
            type = "BAD";
        } else if (strncmp(&buffer[1], HAZER_NMEA_GPS_TALKER, sizeof(HAZER_NMEA_GPS_TALKER) - 1) != 0) {
            type = "IGN";
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_GGA, sizeof(HAZER_NMEA_GPS_MESSAGE_GGA) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_GGA;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_GLL, sizeof(HAZER_NMEA_GPS_MESSAGE_GLL) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_GLL;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_GSA, sizeof(HAZER_NMEA_GPS_MESSAGE_GSA) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_GSA;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_GSV, sizeof(HAZER_NMEA_GPS_MESSAGE_GSV) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_GSV;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_RMC, sizeof(HAZER_NMEA_GPS_MESSAGE_RMC) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_RMC;
        } else if (strncmp(&(buffer[3]), HAZER_NMEA_GPS_MESSAGE_VTG, sizeof(HAZER_NMEA_GPS_MESSAGE_VTG) - 1) == 0) {
            type = HAZER_NMEA_GPS_MESSAGE_VTG;
        } else {
            type = "OTH";
        }

        fprintf(stderr, "%s\n", type);

        tokens = hazer_nmea_tokenize(vector, sizeof(vector) / sizeof(vector[0]),  buffer, size);

        for (vv = vector; *vv != (char *)0; ++vv) {
            fprintf(stderr, "[%ld]=\"%s\"\n", vv - vector, *vv);
        }
        fprintf(stderr, "[%ld]=NULL\n", tokens);

    }

    return 0;
}
