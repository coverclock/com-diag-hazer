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
 * Simple little program to convert lattitude-longitude as expressed on
 * the National Geodetic Survey datasheets into a form palatable to
 * Google Maps. This isn't intended to reject all possible malformed
 * NGS coordinates; it's just not that smart.
 *
 * From 39 43 28.76565(N)    105 09 45.24156(W)
 * To   39°43'28.76545"N, 105°09'45.24078"W
 *
 * USAGE
 *
 * ngs2gmaps STRING
 *
 * EXAMPLE
 *
 * ngs2gmaps '39 43 28.76565(N)    105 09 45.24156(W)'
 * 39°43'28.76565"N, 105°09'45.24156"W
 */

#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>

static const wchar_t DEGREE = 0x00B0; /* %lc */

int main(int argc, char * argv[])
{
	char * ss;
	char * latdeg = (char *)0;
	char * latmin = (char *)0;
	char * latsec = (char *)0;
	char * latdir = (char *)0;
	char * londeg = (char *)0;
	char * lonmin = (char *)0;
	char * lonsec = (char *)0;
	char * londir = (char *)0;

    setlocale(LC_ALL, "");

	while (--argc > 0) {

		ss = *(++argv);

		do {

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if (!isdigit(*ss)) { break; }

			latdeg = ss++;

			while ((*ss != '\0') && (isdigit(*ss))) { ++ss; }
			if (*ss == '\0') { break; }

			*(ss++) = '\0';

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if (!isdigit(*ss)) { break; }

			latmin = ss++;

			while ((*ss != '\0') && (isdigit(*ss))) { ++ss; }
			if (*ss == '\0') { break; }

			*(ss++) = '\0';

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if ((!isdigit(*ss)) && (*ss != '.')) { break; }

			latsec = ss++;

			while ((*ss != '\0') && (isdigit(*ss) || (*ss == '.'))) { ++ss; }
			if (*ss == '\0') { break; }

			if (*ss != '(') { break; }
			*(ss++) = '\0';
			if (*ss == '\0') { break; }
			latdir = ss++;
			if (*ss == '\0') { break; }
			if (*ss != ')') { break; }
			*(ss++) = '\0';

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if (!isdigit(*ss)) { break; }

			londeg = ss++;

			while ((*ss != '\0') && (isdigit(*ss))) { ++ss; }
			if (*ss == '\0') { break; }

			*(ss++) = '\0';

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if (!isdigit(*ss)) { break; }

			lonmin = ss++;

			while ((*ss != '\0') && (isdigit(*ss))) { ++ss; }
			if (*ss == '\0') { break; }

			*(ss++) = '\0';

			while ((*ss != '\0') && (isspace(*ss))) { ++ss; }
			if (*ss == '\0') { break; }
			if ((!isdigit(*ss)) && (*ss != '.')) { break; }

			lonsec = ss++;

			while ((*ss != '\0') && (isdigit(*ss) || (*ss == '.'))) { ++ss; }
			if (*ss == '\0') { break; }

			if (*ss != '(') { break; }
			*(ss++) = '\0';
			if (*ss == '\0') { break; }
			londir = ss++;
			if (*ss == '\0') { break; }
			if (*ss != ')') { break; }
			*(ss++) = '\0';

			printf("%s%lc%s'%s\"%s, %s%lc%s'%s\"%s", latdeg, DEGREE, latmin, latsec, latdir, londeg, DEGREE, lonmin, lonsec, londir);

		} while (0);

		fputc('\n', stdout);

	}
}
