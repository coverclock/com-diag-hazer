/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes the WGS84 geodesic distance between, two points.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * geodesic computes the azimuths for, and the distance between, two points on
 * the Earth identified by their respective latitudes and longitudes expressed
 * in decimal degrees. (These arguments can be cut and pasted directly from the
 * POS or HPP fields in the gpstool output.) The computation is performed in
 * double precision floating point using the geodesic algorithms from
 * GeographicLib authored by Charles Karney. The output is expressed in meters.
 * The geodesic distance is based on the elliptical shape of the Earth as
 * defined by WGS84. Compare this with the result produced by the great
 * circle computation performed by the haversine utility. The code to implement
 * this computation was extracted from Mr. Karney's library, is licensed under
 * the MIT license, and is NOT part of the Digital Aggregates intellectual
 * property.
 *
 * USAGE
 * 
 * geodesic [ -? ] LATDD1 LONDD1 LATDD2 LONDD2
 *
 * REFERENCES
 *
 * Charles F. F. Karney, "Algorithms for geodesics", *Journal for Geodesy*,
 * 2013-01, 87.1, pp. 43..55
 *
 * <https://geographiclib.sourceforge.io>
 *
 * <https://geographiclib.sourceforge.io/html/C/inverse_8c_source.html>
 *
 * EXAMPLE
 *
 * > geodesic 39.794366985, -105.153063138 39.794237168, -105.153370541
 * 30.0160979302
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "geodesic.h"

int main(int argc, char ** argv) {
    int xc = 1;
    const char * program = (const char *)0;
    int debug = 0;
    double a = 6378137; /* WGS84 equatorial radius in meters. */
    double f = 1.0 / 298.257223563; /* WGS84 ellipsoidal flattening. */
    double lat1 = 0.0;
    double lon1 = 0.0;
    double azi1 = 0.0;
    double lat2 = 0.0;
    double lon2 = 0.0;
    double azi2 = 0.0;
    double s12 = 0.0;
    struct geod_geodesic g = { 0 };
    char * end = (char *)0;
    char * arg = "";

    do {

        if ((argc--) <= 0) { break; }
        arg = *(argv++);
        program = ((program = strrchr(arg, '/')) == (char *)0) ? arg : program + 1;

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

        if (debug) { fprintf(stderr, "%s: a=%.15f f=%.15f\n", program, a, f); }

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

        if (debug) { fprintf(stderr, "%s: lat1=%.15f lon1=%.15f lat2=%.15f lon2=%.15f\n", program, lat1, lon1, lat2, lon2); }

        geod_init(&g, a, f);

        geod_inverse(&g, lat1, lon1, lat2, lon2, &s12, &azi1, &azi2);

        if (debug) { fprintf(stderr, "%s: azi1=%.15f azi2=%.15f\n", program, azi1, azi2); }

        printf("%.10f\n", s12);

        xc = 0;

    } while (0);

    return xc;
}
