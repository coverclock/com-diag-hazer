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
 * EXAMPLES
 *
 */

#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "com/diag/hazer/coordinates.h"

int coordinates_parse(const char * string, double * latitudep, double * longitudep)
{
	int rc = -1;
	double latitude = 0.0;
	double longitude = 0.0;
	unsigned int latitudedegrees = 0;
	unsigned int longitudedegrees = 0;
	unsigned int latitudeminutes = 0;
	unsigned int longitudeminutes = 0;
	double latitudeseconds = 0.0;
	double longitudeseconds = 0.0;
	char latitudedirection = '\0';
	char longitudedirection = '\0';

	if (sscanf(string, COORDINATES_FORMAT_HPP ", " COORDINATES_FORMAT_HPP, &latitude, &longitude) == 2) {

		do {

			if ((!((-90.0 <= latitude) && (latitude <= 90.0)))) { break; }
			if ((!((-180.0 <= longitude) && (longitude <= 180.0)))) { break; }

			rc = 0;

		} while (0);

	} else if ((sscanf(string, COORDINATES_FORMAT_NGS ", " COORDINATES_FORMAT_NGS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) ||
		       (sscanf(string, COORDINATES_FORMAT_POS ", " COORDINATES_FORMAT_POS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8)) {

		do {

			if (!((0 <= latitudedegrees) && (latitudedegrees <= 90))) { break; }
			if (!((0 <= longitudedegrees) && (longitudedegrees <= 180))) { break; }
			if (!((0 <= latitudeminutes) && (latitudeminutes < 60))) { break; }
			if (!((0 <= longitudeminutes) && (longitudeminutes < 60))) { break; }
			if (!((0.0 <= latitudeseconds) && (latitudeseconds < 60.0))) { break; }
			if (!((0.0 <= longitudeseconds) && (longitudeseconds < 60.0))) { break; }

			latitude = latitudeseconds;
			latitude /= 60.0;
			latitude += latitudeminutes;
			latitude /= 60.0;
			latitude += latitudedegrees;

			longitude = longitudeseconds;
			longitude /= 60.0;
			longitude += longitudeminutes;
			longitude /= 60.0;
			longitude += longitudedegrees;

			if ((!((0.0 <= latitude) && (latitude <= 90.0)))) { break; }
			if ((!((0.0 <= longitude) && (longitude <= 180.0)))) { break; }

			switch (latitudedirection) {
			case 'N':
			case 'n':
				rc = 0;
				break;
			case 'S':
			case 's':
				latitude = -latitude;
				rc = 0;
				break;
			}

			switch (longitudedirection) {
			case 'E':
			case 'e':
				rc = 0;
				break;
			case 'W':
			case 'w':
				longitude = -longitude;
				rc = 0;
				break;
			}

		} while (0);

	} else {

		/* Do nothing. */

	}

	if (rc == 0) {
		*latitudep = latitude;
		*longitudep = longitude;
	}

	return rc;
}
