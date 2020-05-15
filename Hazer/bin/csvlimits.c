/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 * Filter that etermines the boundaries of the solutions in a CSV file.
 * Also useful as a model for other code that reads the CSV file.
 * E.G. csvlimits < data.csv
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <values.h>
#include "./gpstool.h"

/*
HOSTNAME, OBSERVATION, CLOCK, TIME, LATITUDE, LONGITUDE, HORIZONTAL, MSL, WGS84, VERTICAL, SPEED, COURSE
"cadmium", 1, 1589566960.035580274, 1589566960.000000000, 39.794248017, -105.153353729, 10.4399, 1705.4152, 1683.9153, 14.7053, 0.602000, 0.000000000
*/

int main(int argc, char *argv[])
{
    const char * program = (const char *)0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    char buffer[512] = { '\0', };
    int count = 0;
    char * here = (char *)0;
    char * hostname = (char *)0;
    int observation = 0;
    double clock = 0.0;
    double time = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    double horizontal = 0.0;
    double msl = 0.0;
    double wgs84 = 0.0;
    double vertical = 0.0;
    double speed = 0.0;
    double course = 0.0;
    double minimum_latitude = MAXDOUBLE;
    double maximum_latitude = -MAXDOUBLE;
    double minimum_longitude = MAXDOUBLE;
    double maximum_longitude = -MAXDOUBLE;
    
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    while ((opt = getopt(argc, argv, "?dv")) >= 0) {
        switch (opt) {
        case '?':
            fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -v ]\n", program);
            return 0;
            break;
        case 'd':
            debug = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        default:
            fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -v ]\n", program);
            return 1;
            break;
        }
    }

    //fprintf(stderr, "%.9lf %.9lf %.9lf %.9lf\n", minimum_latitude, maximum_latitude, minimum_longitude, maximum_longitude);

    while (fgets(buffer, sizeof(buffer), stdin) != (char *)0) {

        if (debug) {
            fputs(buffer, stderr);
        }

        // fputs(here, stdout);

        if (strncmp(buffer, HEADINGS[0], sizeof(HEADINGS[0])) == 0) {
            if (verbose) {
                fprintf(stderr, "%s", buffer);
            }
            continue;
        }

        // fputs(here, stdout);

        here = buffer;

        if (*here != '"') { 
            continue;
        }

        // fputs(here, stdout);

        here += 1;
        hostname = here;

        if ((here = strchr(here, '"')) == (char *)0) {
            continue;
        }

        // fputs(here, stdout);

        *here = '\0';
        here += 1;

        if (*here != ',') {
            continue;
        }

        // fputs(here, stdout);

        here += 1;

        if (*here != ' ') {
            continue;
        }

        // fputs(here, stdout);

        here += 1;

        if (sscanf(here, "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", &observation, &clock, &time, &latitude, &longitude, &horizontal, &msl, &wgs84, &vertical, &speed, &course) != 11) {
            continue;
        }

        // fputs(here, stdout);

        count += 1;

        if (latitude < minimum_latitude) {
            minimum_latitude = latitude;
        }

        if (latitude > maximum_latitude) {
            maximum_latitude = latitude;
        }

        if (longitude < minimum_longitude) {
            minimum_longitude = longitude;
        }

        if (longitude > maximum_longitude) {
            maximum_longitude = longitude;
        }

        if (verbose) {
            fprintf(stderr, "\"%s\", %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n", hostname, observation, clock, time, latitude, longitude, horizontal, msl, wgs84, vertical, speed, course);
        }

    }

    if (count == 0) {
        return -1;
    }

    printf("%s: [%d] ( %.9lf, %.9lf ) ( %.9lf, %.9lf )\n", program, count, minimum_latitude, minimum_longitude, maximum_latitude, maximum_longitude);

    return 0;
}
