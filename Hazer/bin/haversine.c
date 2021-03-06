/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes the Haversine distance between two points.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * haversine computes the distance between two points on the Earth identified
 * by their respective latitudes and longitudes expressed in decimal degrees.
 * (These arguments can be cut and pasted directly from the POS or HPP fields
 * in the gpstool output.) The computation is performed in double precision
 * floating point using the haversine formula based on great circle routes and
 * spherical trigonometry. The output is expressed in meters. It does *not*
 * take into account the elliptical shape of the Earth as does WGS84.
 *
 * REFERENCES
 *
 * Chris Veness, "Calculate distance, bearing and more between
 * Latitude/Longitude points",
 * <https://www.movable-type.co.uk/scripts/latlong.html>, 2019-07-11
 *
 * Wikipedia, "Haversine forumula",
 * <https://en.wikipedia.org/wiki/Haversine_formula>, 2019-07-10
 *
 * Wikipedia, "Earth radius",
 * <https://en.wikipedia.org/wiki/Earth_radius>, 2019-07-08
 *
 * USAGE
 *
 * haversine [ -? ] [ -d ] LATDD1 LONDD1 LATDD2 LONDD2
 *
 * EXAMPLE
 *
 * > haversine 39.794366985, -105.153063138 39.794237168, -105.153370541
 * 30.002282
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc, char * argv[])
{
    int xc = 1;
    const char * program = (const char *)0;
    int debug = 0;
    double lat1 = 0.0;
    double lon1 = 0.0;
    double lat2 = 0.0;
    double lon2 = 0.0;
    double theta1 = 0.0;
    double theta2 = 0.0;
    double deltatheta = 0.0;
    double deltalambda = 0.0;
    double a = 0.0;
    double b = 0.0;
    double c = 0.0;
    double d = 0.0;
    static const double R = 6378100;
    char * end = (char *)0;
    char * arg = "";

    do {

        program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
        argv++;
        argc--;

        if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
            fprintf(stderr, "usage: %s [ -? ] [ -d ] LATDD1 LONDD1 LATDD2 LONDD2\n", program);
            argv++;
            argc--;
        }

        if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
            debug = !0;
            argv++;
            argc--;
        }

        if (argc < 4) { break; }

        if ((argc--) <= 0) { break; }
        arg = *(argv++);
        lat1 = strtold(arg, &end);
        if (end == (char *)0) { break; }
        if ((*end != '\0') && (*end != ',')) { break; }
        if (!((-90.0 <= lat1) && (lat1 <= 90.0))) { break; }

        if ((argc--) <= 0) { break; }
        arg = *(argv++);
        lon1 = strtold(arg, &end);
        if (end == (char *)0) { break; }
        if (*end != '\0') { break; }
        if (!((-180.0 <= lon1) && (lon1 <= 180.0))) { break; }

        if ((argc--) <= 0) { break; }
        arg = *(argv++);
        lat2 = strtold(arg, &end);
        if (end == (char *)0) { break; }
        if ((*end != '\0') && (*end != ',')) { break; }
        if (!((-90.0 <= lat2) && (lat2 <= 90.0))) { break; }

        if ((argc--) <= 0) { break; }
        arg = *(argv++);
        lon2 = strtold(arg, &end);
        if (end == (char *)0) { break; }
        if (*end != '\0') { break; }
        if (!((-180.0 <= lon2) && (lon2 <= 180.0))) { break; }

        if (debug) {
            fprintf(stderr, "%s: ( %.10f , %.10f ) ( %.10f , %.10f )\n", program, lat1, lon1, lat2, lon2);
        }

        theta1 = (lat1 * M_PI) / 180.0;
        theta2 = (lat2 * M_PI) / 180.0;

        deltatheta = ((lat2 - lat1) * M_PI) / 180.0;
        deltalambda = ((lon2 - lon1) * M_PI) / 180.0;

        b = sin(deltalambda / 2.0);
        b *= b;

        a = sin(deltatheta / 2.0);
        a *= a;

        a += cos(theta1) * cos(theta2) * b;

        c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

        d = R * c;

        printf("%.10f\n", d);

        xc = 0;

    } while (0);

    return xc;
}
