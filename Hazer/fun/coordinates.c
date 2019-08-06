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

const char EXAMPLE_HPP[] = "39.794234216, -105.153377669";
const char EXAMPLE_NGS[] = "39 47 39.24317(N) 105 09 12.15960(W)";
const char EXAMPLE_POS[] = "39°47'39.243\"N, 105°09'12.159\"W";

const char COM_DIAG_HAZER_SCANF_HPP[] = "%lf, %lf";
const char COM_DIAG_HAZER_SCANF_NGS[] = "%u %u %lf(%c) %u %u %lf(%c)";
const char COM_DIAG_HAZER_SCANF_POS[] = "%u%lc%u'%lf\"%c, %u%lc%u'%lf\"%c";

int main(int argc, char * argv[])
{
	{
		int rc = 0;
		double latitude = 0.0;
		double longitude = 0.0;
		rc = sscanf(EXAMPLE_HPP, COM_DIAG_HAZER_SCANF_HPP, &latitude, &longitude);
		fprintf(stderr, "HPP string=\"%s\" rc=%d latitude=%.9f longitude=%.9f\n", EXAMPLE_HPP, rc, latitude, longitude);
		assert(rc == 2);
		assert(latitude == 39.794234216);
		assert(longitude == -105.153377669);
	}

	{
		int rc = 0;
		unsigned int latitudedegrees = 0;
		unsigned int longitudedegrees = 0;
		unsigned int latitudeminutes = 0;
		unsigned int longitudeminutes = 0;
		double latitudeseconds = 0.0;
		double longitudeseconds = 0.0;
		char latitudedirection = '\0';
		char longitudedirection = '\0';
		wchar_t degreesymbol = 0;
		rc = sscanf(EXAMPLE_NGS, COM_DIAG_HAZER_SCANF_NGS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection);
		fprintf(stderr, "NGS string=\"%s\" rc=%d latitude=%u %02u %lf(%c) longitude=%u %02u %lf(%c)\n", EXAMPLE_NGS, rc, latitudedegrees, latitudeminutes, latitudeseconds, latitudedirection, longitudedegrees, longitudeminutes, longitudeseconds, longitudedirection);
		assert(rc == 8);
		assert(latitudedegrees == 39);
		assert(latitudeminutes == 47);
		assert(latitudeseconds == 39.24317);
		assert(latitudedirection == 'N');
		assert(longitudedegrees == 105);
		assert(longitudeminutes == 9);
		assert(longitudeseconds == 12.15960);
		assert(longitudedirection == 'W');
	}

	{
		int rc = 0;
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
		rc = sscanf(EXAMPLE_POS, COM_DIAG_HAZER_SCANF_POS, &latitudedegrees, &latitudesymbol, &latitudeminutes, &latitudeseconds, &latitudedirection, &longitudedegrees, &longitudesymbol, &longitudeminutes, &longitudeseconds, &longitudedirection);
		fprintf(stderr, "POS string=\"%s\" rc=%d latitude=%u%lc%u'%lf\"%c longitude=%u%lc%u'%lf\"%c\n", EXAMPLE_POS, rc, latitudedegrees, latitudesymbol, latitudeminutes, latitudeseconds, latitudedirection, longitudedegrees, longitudesymbol, longitudeminutes, longitudeseconds, longitudedirection);
		assert(rc == 10);
		assert(latitudedegrees == 39);
		assert(latitudesymbol == DEGREE);
		assert(latitudeminutes == 47);
		assert(latitudeseconds == 39.243);
		assert(latitudedirection == 'N');
		assert(longitudedegrees == 105);
		assert(longitudesymbol == DEGREE);
		assert(longitudeminutes == 9);
		assert(longitudeseconds == 12.159);
		assert(longitudedirection == 'W');
	}

	return 0;
}
