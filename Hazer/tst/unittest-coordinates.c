/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
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
#include "com/diag/hazer/coordinates.h"

const char EXAMPLE_HPP_LATITUDE[] = "39.794234216";
const char EXAMPLE_HPP_LONGITUDE[] = "-105.153377669";

const char EXAMPLE_NGS_LATITUDE[] = "39 47 39.24317(N)";
const char EXAMPLE_NGS_LONGITUDE[] = "105 09 12.15960(W)";

const char EXAMPLE_POS_LATITUDE[] = "39°47'39.243\"N";
const char EXAMPLE_POS_LONGITUDE[] = "105°09'12.159\"W";

const char EXAMPLE_HPP_DATA[] = "39.794212196, -105.153349930";
const char EXAMPLE_NGS_DATA[] = "39 47 39.16390(N) 105 09 12.05974(W)";
const char EXAMPLE_POS_DATA[] = "39°47'39.163\"N, 105°09'12.060\"W";

const char EXAMPLE_HPP_LINE[] = "HPP   39.794217657, -105.153375607 ±     1.0647m                       GNSS";
const char EXAMPLE_NGS_LINE[] = "NGS  39 47 39.18356(N) 105 09 12.15218(W)                              GNSS";
const char EXAMPLE_POS_LINE[] = "POS 39°47'39.183\"N, 105°09'12.152\"W    39.7942176, -105.1533756        GNSS";

const char EXAMPLE_HPP[] = "39.794212196 -105.153349930";
const char EXAMPLE_POS[] = "39°47'39.163\"N 105°09'12.060\"W";

const char EXAMPLE_HPP_OTHER[] = "-39.794212196, 105.153349930";
const char EXAMPLE_NGS_OTHER[] = "39 47 39.16390(S) 105 09 12.05974(E)";

int main(int argc, char * argv[])
{
    if (argc > 1) { (void)coordinates_debug(!0); }

    {
        fputs("COORDINATES_SCANF_HPP=\"" COORDINATES_SCANF_HPP "\"\n", stderr);
        fputs("COORDINATES_SCANF_NGS=\"" COORDINATES_SCANF_NGS "\"\n", stderr);
        fputs("COORDINATES_SCANF_POS=\"" COORDINATES_SCANF_POS "\"\n", stderr);
    }

    {
        fprintf(stderr, "COORDINATES_SCANF_HPP: " COORDINATES_SCANF_HPP "\n", -179.999999 );
        fprintf(stderr, "COORDINATES_SCANF_NGS: " COORDINATES_SCANF_NGS "\n", 179, 59, 59.999999, 'W');
        fprintf(stderr, "COORDINATES_SCANF_POS: " COORDINATES_SCANF_POS "\n", 179, 59, 59.999999, 'W');
    }

    {
        int latituderc = 0;
        int longituderc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        latituderc = sscanf(EXAMPLE_HPP_LATITUDE, COORDINATES_SCANF_HPP, &latitude);
        longituderc = sscanf(EXAMPLE_HPP_LONGITUDE, COORDINATES_SCANF_HPP, &longitude);
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
        latituderc = sscanf(EXAMPLE_NGS_LATITUDE, COORDINATES_SCANF_NGS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection);
        longituderc = sscanf(EXAMPLE_NGS_LONGITUDE, COORDINATES_SCANF_NGS, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection);
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
        char * locale = (char *)0;
        locale = setlocale(LC_ALL, "");
        latituderc = sscanf(EXAMPLE_POS_LATITUDE, COORDINATES_SCANF_POS, &latitudedegrees, &latitudeminutes, &latitudeseconds, &latitudedirection);
        longituderc = sscanf(EXAMPLE_POS_LONGITUDE, COORDINATES_SCANF_POS, &longitudedegrees, &longitudeminutes, &longitudeseconds, &longitudedirection);
        fprintf(stderr, "POS LATITUDE=\"%s\"[%d] latitude=%u°%02u'%012.9lf\"%c LONGITUDE=\"%s\"[%d] longitude=%u°%02u'%012.9lf\"%c\n", EXAMPLE_POS_LATITUDE, latituderc, latitudedegrees, latitudeminutes, latitudeseconds, latitudedirection, EXAMPLE_POS_LONGITUDE, longituderc, longitudedegrees, longitudeminutes, longitudeseconds, longitudedirection);
        assert(latituderc == 4);
        assert(latitudedegrees == 39);
        assert(latitudeminutes == 47);
        assert(latitudeseconds == 39.243);
        assert(latitudedirection == 'N');
        assert(longituderc == 4);
        assert(longitudedegrees == 105);
        assert(longitudeminutes == 9);
        assert(longitudeseconds == 12.159);
        assert(longitudedirection == 'W');
    }

    /*
     * Comparing for exact values of computed floating point variables in a
     * platform and target independent way is problematic. That's why the Hazer
     * keeps NMEA coordinates integer units. Hazer uses nanominutes because the
     * NMEA spec uses degrees, minutes, and fractional minutes. Yodel uses
     * nanodegrees because that's what UBX uses for its high precision position
     * (HPP). Below, we just check for the first few fractional digits. Above,
     * we may check for more, because there is no computation; we're effectively
     * checking the correctness of the sscanf(3) function.
     */

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_HPP_DATA;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "HPP DATA=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_HPP_SEPERATOR);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_HPP_OTHER;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "HPP DATA=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_HPP_SEPERATOR);
        assert(((int)(latitude * 1000)) == -39794);
        assert(((int)(longitude * 1000)) == 105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_HPP_LINE;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "HPP LINE=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_HPP_PREFIX_SEPERATOR);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_HPP;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "HPP PAIR=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_HPP);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_NGS_DATA;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "NGS DATA=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_NGS);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_NGS_OTHER;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "NGS DATA=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_NGS);
        assert(((int)(latitude * 1000)) == -39794);
        assert(((int)(longitude * 1000)) == 105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_NGS_LINE;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "NGS LINE=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_NGS_PREFIX);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_POS_DATA;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "POS DATA=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_POS_SEPERATOR);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_POS_LINE;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "POS LINE=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_POS_PREFIX_SEPERATOR);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    {
        int rc = 0;
        double latitude = 0.0;
        double longitude = 0.0;
        const char * string = EXAMPLE_POS;
        rc = coordinates_parse(string, &latitude, &longitude);
        fprintf(stderr, "POS PAIR=\"%s\" rc=%d latitude=%.9lf longitude=%.9f\n", string, rc, latitude, longitude);
        assert(rc == COORDINATES_FORMAT_POS);
        assert(((int)(latitude * 1000)) == 39794);
        assert(((int)(longitude * 1000)) == -105153);
    }

    return 0;
}
