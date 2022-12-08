/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2020-2022 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Filter that determines the boundaries of the solutions in a CSV file.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Filter that determines the boundaries of the solutions in a CSV file.
 * Also useful as a model for other code that reads the CSV file.
 *
 * USAGE
 *
 * csvlimits [ -? ] [ -d ] [ -v ]
 *
 * EXAMPLE
 *
 * csvlimits < data.csv
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <values.h>

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
    int fix = 0;
    int system = 0;
    int satellites = 0;
    double clock = 0.0;
    double time = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;
    double haccuracy = 0.0;
    double msl = 0.0;
    double geo = 0.0;
    double vaccuracy = 0.0;
    double speed = 0.0;
    double course = 0.0;
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
    double raccuracy = 0.0;
    double paccuracy = 0.0;
    double yaccuracy = 0.0;
    int observations = 0;
    double maccuracy = 0.0;
    double minimum_latitude = MAXDOUBLE;
    double maximum_latitude = -MAXDOUBLE;
    double minimum_longitude = MAXDOUBLE;
    double maximum_longitude = -MAXDOUBLE;
    double minimum_msl = MAXDOUBLE;
    double maximum_msl = -MAXDOUBLE;
    double minimum_geo = MAXDOUBLE;
    double maximum_geo = -MAXDOUBLE;
    
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

    // fprintf(stderr, "%.9lf %.9lf %.9lf\n", MINDOUBLE, -MAXDOUBLE, MAXDOUBLE);

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

        here = buffer;

        if (*here != '"') { 
            if (verbose) {
                fprintf(stderr, "%s", here);
            }
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

        if (sscanf(here, "%d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %d, %lf\n", &observation, &fix, &system, &satellites, &clock, &time, &latitude, &longitude, &haccuracy, &msl, &geo, &vaccuracy, &speed, &course, &roll, &pitch, &yaw, &raccuracy, &paccuracy, &yaccuracy, &observations, &maccuracy) != 22) {
            continue;
        }

        // fputs(here, stdout);

        if (fix < 3) {
            continue;
        }

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

        if (msl < minimum_msl) {
            minimum_msl = msl;
        }

        if (msl > maximum_msl) {
            maximum_msl = msl;
        }

        if (geo < minimum_geo) {
            minimum_geo = geo;
        }

        if (geo > maximum_geo) {
            maximum_geo = geo;
        }

        if (verbose) {
            fprintf(stderr, "\"%s\", %d, %d, %d, %d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %d, %lf\n", hostname, observation, fix, system, satellites, clock, time, latitude, longitude, haccuracy, msl, geo, vaccuracy, speed, course, roll, pitch, yaw, raccuracy, paccuracy, yaccuracy, observations, maccuracy);
        }

    }

    if (count == 0) {
        return -1;
    }

    printf("%s: [%d] %.9lf, %.9lf %.9lf, %.9lf %.9lf %.9lf %.9lf %.9lf\n", program, count, minimum_latitude, minimum_longitude, maximum_latitude, maximum_longitude, minimum_msl, maximum_msl, minimum_geo, maximum_geo);

    return 0;
}
