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
#include <values.h>
#include "./gpstool.h"

/*
HOSTNAME, OBSERVATION, CLOCK, TIME, LATITUDE, LONGITUDE, HORIZONTAL, MSL, WGS84, VERTICAL, SPEED, COURSE
"cadmium", 1, 1589566960.035580274, 1589566960.000000000, 39.794248017, -105.153353729, 10.4399, 1705.4152, 1683.9153, 14.7053, 0.602000, 0.000000000
*/

int main(int argc, char *argv[])
{
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

    //fprintf(stderr, "%.9lf %.9lf %.9lf %.9lf\n", minimum_latitude, maximum_latitude, minimum_longitude, maximum_longitude);

    while (fgets(buffer, sizeof(buffer), stdin) != (char *)0) {

        // fputs(here, stdout);

        if (strncmp(buffer, HEADINGS[0], sizeof(HEADINGS[0])) == 0) {
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

    }

    if (count == 0) {
        return -1;
    }

    printf("%.9lf, %.9lf <= %.9lf, %.9lf\n", minimum_latitude, minimum_longitude, maximum_latitude, maximum_longitude);

    return 0;
}
