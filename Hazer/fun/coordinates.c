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
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <wchar.h>
#undef NDEBUG
#include <assert.h>
#include "com/diag/diminuto/diminuto_dump.h"

static const wchar_t DEGREE = 0x00B0;

const char EXAMPLE_HPP_LATITUDE[] = "39.794234216";
const char EXAMPLE_HPP_LONGITUDE[] = "-105.153377669";
const char EXAMPLE_NGS_LATITUDE[] = "39 47 39.24317(N)";
const char EXAMPLE_NGS_LONGITUDE[] = "105 09 12.15960(W)";
const char EXAMPLE_POS_LATITUDE[] = "39°47'39.243\"N";
const char EXAMPLE_POS_LONGITUDE[] = "105°09'12.159\"W";

const char COM_DIAG_HAZER_SCANF_HPP[] = "%lf";
const char COM_DIAG_HAZER_SCANF_NGS[] = "%u %u %lf(%c)";
const char COM_DIAG_HAZER_SCANF_POS[] = "%u%lc%u'%lf\"%c";

int main(int argc, char * argv[])
{
	{
		int latituderc = 0;
		int longituderc = 0;
		double latitude = 0.0;
		double longitude = 0.0;
		latituderc = sscanf(EXAMPLE_HPP_LATITUDE, COM_DIAG_HAZER_SCANF_HPP, &latitude);
		longituderc = sscanf(EXAMPLE_HPP_LONGITUDE, COM_DIAG_HAZER_SCANF_HPP, &longitude);
		fprintf(stderr, "HPP LATITUDE=\"%s\"[%d] latitude=%.9f LONGITUDE=\"%s\"[%d] longitude=%.9f\n", EXAMPLE_HPP_LATITUDE, latituderc, latitude, EXAMPLE_HPP_LONGITUDE, longituderc, longitude);
		assert(latituderc == 1);
		assert(latitude == 39.794234216);
		assert(longituderc == 1);
		assert(longitude == -105.153377669);
	}

	{
		int latituderc = 0;
		int longituderc = 0;
		unsigned int latitudedegrees = 0;
		unsigned int longitudedegrees = 0;
		unsigned int latitudeminutes = 0;
		unsigned int longitudeminutes = 0;
		double latitudeseconds = 0.0;
		double longitudeseconds = 0.0;
		char latitudedirection = '\0';
		char longitudedirection = '\0';
		wchar_t degreesymbol = 0;
		latituderc = sscanf(EXAMPLE_NGS_LATITUDE, COM_DIAG_HAZER_SCANF_NGS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection);
		longituderc = sscanf(EXAMPLE_NGS_LONGITUDE, COM_DIAG_HAZER_SCANF_NGS, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection);
		fprintf(stderr, "NGS LATITUDE=\"%s\"[%d] latitude=%u %02u %012.9lf(%c) LONGITUDE=\"%s\"[%d] longitude=%u %02u %012.9lf(%c)\n", EXAMPLE_NGS_LATITUDE, latituderc, latitudedegrees, latitudeminutes, latitudeseconds, latitudedirection, EXAMPLE_NGS_LONGITUDE, longituderc, longitudedegrees, longitudeminutes, longitudeseconds, longitudedirection);
		assert(latituderc == 4);
		assert(latitudedegrees == 39);
		assert(latitudeminutes == 47);
		assert(latitudeseconds == 39.24317);
		assert(latitudedirection == 'N');
		assert(longituderc == 4);
		assert(longitudedegrees == 105);
		assert(longitudeminutes == 9);
		assert(longitudeseconds == 12.15960);
		assert(longitudedirection == 'W');
	}

	{
		int latituderc = 0;
		int longituderc = 0;
		unsigned int latitudedegrees = 0;
		unsigned int longitudedegrees = 0;
		unsigned int latitudeminutes = 0;
		unsigned int longitudeminutes = 0;
		double latitudeseconds = 0.0;
		double longitudeseconds = 0.0;
		char latitudedirection = '\0';
		char longitudedirection = '\0';
		wchar_t latitudesymbol = 0;
		wchar_t longitudesymbol = 0;
		char * locale = (char *)0;
	    locale = setlocale(LC_ALL, "");
	    latituderc = sscanf(EXAMPLE_POS_LATITUDE, COM_DIAG_HAZER_SCANF_POS, &latitudedegrees, &latitudesymbol, &latitudeminutes, &latitudeseconds, &latitudedirection);
		longituderc = sscanf(EXAMPLE_POS_LONGITUDE, COM_DIAG_HAZER_SCANF_POS, &longitudedegrees, &longitudesymbol, &longitudeminutes, &longitudeseconds, &longitudedirection);
		fprintf(stderr, "POS LATITUDE=\"%s\"[%d] latitude=%u%lc%02u'%012.9lf\"%c LONGITUDE=\"%s\"[%d] longitude=%u%lc%02u'%012.9lf\"%c\n", EXAMPLE_POS_LATITUDE, latituderc, latitudedegrees, latitudesymbol, latitudeminutes, latitudeseconds, latitudedirection, EXAMPLE_POS_LONGITUDE, longituderc, longitudedegrees, longitudesymbol, longitudeminutes, longitudeseconds, longitudedirection);
		assert(latituderc == 5);
		assert(latitudedegrees == 39);
		assert(latitudesymbol == DEGREE);
		assert(latitudeminutes == 47);
		assert(latitudeseconds == 39.243);
		assert(latitudedirection == 'N');
		assert(longituderc == 5);
		assert(longitudedegrees == 105);
		assert(longitudesymbol == DEGREE);
		assert(longitudeminutes == 9);
		assert(longitudeseconds == 12.159);
		assert(longitudedirection == 'W');
	}

	return 0;
}
