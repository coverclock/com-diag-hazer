/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include <locale.h>
#include <wchar.h>
#include "com/diag/hazer/coordinates.h"

static const char FORMAT_HPP_PREFIX_SEPARATOR[] = "HPP " COORDINATES_SCANF_HPP ", " COORDINATES_SCANF_HPP;
static const char FORMAT_HPP_SEPERATOR[] = COORDINATES_SCANF_HPP ", " COORDINATES_SCANF_HPP;
static const char FORMAT_HPP[] = COORDINATES_SCANF_HPP " " COORDINATES_SCANF_HPP;

static const char FORMAT_POS_PREFIX_SEPARATOR[] = "POS " COORDINATES_SCANF_POS ", " COORDINATES_SCANF_POS;
static const char FORMAT_POS_SEPERATOR[] = COORDINATES_SCANF_POS ", " COORDINATES_SCANF_POS;
static const char FORMAT_POS[] = COORDINATES_SCANF_POS " " COORDINATES_SCANF_POS;

static const char FORMAT_NGS_PREFIX[] = "NGS " COORDINATES_SCANF_NGS " " COORDINATES_SCANF_NGS;
static const char FORMAT_NGS[] = COORDINATES_SCANF_NGS " " COORDINATES_SCANF_NGS;

static int debug = 0;

int coordinates_debug(int now) {
	int was = -1;

	was = debug;
	debug = now;

	return was;
}

int coordinates_parse(const char * string, double * latitudep, double * longitudep)
{
	coordinates_format_t rc = COORDINATES_FORMAT_INVALID;
	coordinates_format_t format = COORDINATES_FORMAT_UNSUPPORTED;
	double latitude = 0.0;
	double longitude = 0.0;
	unsigned int latitudedegrees = 0;
	unsigned int longitudedegrees = 0;
	unsigned int latitudeminutes = 0;
	unsigned int longitudeminutes = 0;
	double latitudeseconds = 0.0;
	double longitudeseconds = 0.0;
	char latitudedirection = '?';
	char longitudedirection = '?';

	/*
	 * The order here is important, since simpler scanf(3) formats can match
	 * the wrong longer input strings.
	 */

	if (sscanf(string, FORMAT_HPP_PREFIX_SEPARATOR, &latitude, &longitude) == 2) {
		format = COORDINATES_FORMAT_HPP_PREFIX_SEPERATOR;
	} else if (sscanf(string, FORMAT_POS_PREFIX_SEPARATOR, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) {
		format = COORDINATES_FORMAT_POS_PREFIX_SEPERATOR;
	} else if (sscanf(string, FORMAT_NGS_PREFIX, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) {
		format = COORDINATES_FORMAT_NGS_PREFIX;
	} else if (sscanf(string, FORMAT_NGS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) {
		format = COORDINATES_FORMAT_NGS;
	} else if (sscanf(string, FORMAT_POS_SEPERATOR, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) {
		format = COORDINATES_FORMAT_POS_SEPERATOR;
	} else if (sscanf(string, FORMAT_POS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection) == 8) {
		format = COORDINATES_FORMAT_POS;
	} else if (sscanf(string, FORMAT_HPP_SEPERATOR, &latitude, &longitude) == 2) {
		format = COORDINATES_FORMAT_HPP_SEPERATOR;
	} else if (sscanf(string, FORMAT_HPP, &latitude, &longitude) == 2) {
		format = COORDINATES_FORMAT_HPP;
	} else {
		/* Do nothing. */
	}

	switch (format) {

	case COORDINATES_FORMAT_HPP_PREFIX_SEPERATOR:
	case COORDINATES_FORMAT_HPP_SEPERATOR:
	case COORDINATES_FORMAT_HPP:

		if (debug) { fprintf(stderr, "%s[%d]: [%d] ( %.9lf , %.9lf )\n", __FILE__, __LINE__, format, latitude, longitude); }

		if ((!((-90.0 <= latitude) && (latitude <= 90.0)))) { break; }
		if ((!((-180.0 <= longitude) && (longitude <= 180.0)))) { break; }

		rc = format;

		break;

	case COORDINATES_FORMAT_POS_PREFIX_SEPERATOR:
	case COORDINATES_FORMAT_NGS_PREFIX:
	case COORDINATES_FORMAT_POS_SEPERATOR:
	case COORDINATES_FORMAT_NGS:
	case COORDINATES_FORMAT_POS:

		if (debug) { fprintf(stderr, "%s[%d]: [%d] ( %u %u %lf %c , %u %u %lf %c )\n", __FILE__, __LINE__, format, latitudedegrees, latitudeminutes, latitudeseconds, latitudedirection, longitudedegrees, longitudeminutes, longitudeseconds, longitudedirection); }

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
			rc = format;
			break;
		case 'S':
		case 's':
			latitude = -latitude;
			rc = format;
			break;
		}

		switch (longitudedirection) {
		case 'E':
		case 'e':
			rc = format;
			break;
		case 'W':
		case 'w':
			longitude = -longitude;
			rc = format;
			break;
		}

		break;

	default:

		/* Do nothing. */

		break;

	}

	if (rc > 0) {
		*latitudep = latitude;
		*longitudep = longitude;
	}

	return rc;
}
