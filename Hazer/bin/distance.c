/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * ABSTRACT
 *
 * distance computes the distance between two points on the Earth identified
 * by their respective latitudes and longitudes expressed in decimal degrees.
 * (These arguments can be cut and pasted directly from the POS or HPP fields
 * in the gpstool output.) The computation is performed in double precision
 * floating point using the haversine formula based on spherical trigonometry.
 * The output is expressed in meters.
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
 * distance [ -? ] [ -d ] LAT1 LON1 LAT2 LON2
 *
 * EXAMPLE
 *
 * distance 39.794366985, -105.153063138 39.794237168, -105.153370541
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc, char * argv[])
{
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

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
    argv++;
    argc--;

    if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
		fprintf(stderr, "usage: %s [ -? ] [ -d ] LAT1 LON1 LAT2 LON2\n", program);
		argv++;
		argc--;
    }

    if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
		debug = !0;
		argv++;
		argc--;
    }

	if (argc < 4) {
		return 1;
	}

	lat1 = atof(*(argv++));
	lon1 = atof(*(argv++));

	lat2 = atof(*(argv++));
	lon2 = atof(*(argv++));

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

	printf("%lf\n", d);
}
