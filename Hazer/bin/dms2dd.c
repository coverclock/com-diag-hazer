/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * ABSTRACT
 *
 * Simple utility to convert coordinates from degrees minutes seconds to
 * decimal degrees.
 *
 * USAGE
 *
 * dms2dd [ -? ] [ -d ] LATD LATM LATDS N|S LOND LONM LONDS E|W
 *
 * EXAMPLE
 *
 * dms2dd 39 43 28.76565 N 105 09 45.24156 W
 * 39.724657, -105.162567
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char * argv[])
{
	int xc = 1;
	const char * program = (const char *)0;
	int debug = 0;
	unsigned long latd = 0;
	unsigned long latm = 0;
	double latds = 0.0;
	char latdir = '\0';
	double lat = 0.0;
	unsigned long lond = 0;
	unsigned long lonm = 0;
	double londs = 0.0;
	char londir = '\0';
	double lon = 0.0;
	char * end = (char *)0;
	const char * arg = "";

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
    argv++;
    argc--;

    if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
		fprintf(stderr, "usage: %s [ -? ] [ -d ] LATD LATM LATDS N|S LOND LONM LONDS E|W\n", program);
		argv++;
		argc--;
    }

    if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
		debug = !0;
		argv++;
		argc--;
    }

	do {

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		latd = strtoul(arg, &end, 10);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0 <= latd) && (latd <= 90))) { break; }

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		latm = strtoul(arg, &end, 10);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0 <= latm) && (latm < 60))) { break; }

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		latds = strtold(arg, &end);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0.0 <= latds) && (latds < 60.0))) { break; }

		lat = latds;
		lat /= 60.0;
		lat += latm;
		lat /= 60.0;
		lat += latd;

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		if (*arg == 'N') {
			/* Do nothing. */
		} else if (*arg == 'S') {
			lat = -lat;
		} else if (*arg == 'n') {
			/* Do nothing. */
		} else if (*arg == 's') {
			lat = -lat;
		} else {
			break;
		}
		latdir = *arg;

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		lond = strtoul(arg, &end, 10);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0 <= lond) && (lond <= 180))) { break; }

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		lonm = strtoul(arg, &end, 10);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0 <= lonm) && (lonm < 60))) { break; }

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		londs = strtold(arg, &end);
		if (end == (char *)0) { break; }
		if (*end != '\0') { break; }
		if (!((0.0 <= londs) && (londs < 60.0))) { break; }

		lon = londs;
		lon /= 60.0;
		lon += lonm;
		lon /= 60.0;
		lon += lond;

		if ((argc--) <= 0) { break; }
		arg = *(argv++);
		if (*arg == 'E') {
			/* Do nothing. */
		} else if (*arg == 'W') {
			lon = -lon;
		} else if (*arg == 'e') {
			/* Do nothing. */
		} else if (*arg == 'w') {
			lon = -lon;
		} else {
			break;
		}
		londir = *arg;

		if (debug) {
			fprintf(stderr, "%s: %ld %ld %f %c %ld %ld %f %c\n", program, latd, latm, latds, latdir, lond, lonm, londs, londir);
		}

		printf("%.9f, %.9f\n", lat, lon);

		xc = 0;

	} while (0);

	if (xc != 0) {
		errno = EINVAL;
		perror(arg);
	}

	return xc;
}
