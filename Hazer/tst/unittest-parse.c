/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "com/diag/hazer/hazer.h"

int main(void)
{
    uint64_t numerator = 0;
    uint64_t denominator = 0;
    double latlon = 0;
    uint64_t nanoseconds = 0;

    numerator = hazer_parse_fraction("", &denominator);
    assert(numerator == 0);
    assert(denominator == 1);

    numerator = hazer_parse_fraction("1", &denominator);
    assert(numerator == 1);
    assert(denominator == 10);

    numerator = hazer_parse_fraction("12", &denominator);
    assert(numerator == 12);
    assert(denominator == 100);

    numerator = hazer_parse_fraction("123", &denominator);
    assert(numerator == 123);
    assert(denominator == 1000);

    numerator = hazer_parse_fraction("1234", &denominator);
    assert(numerator == 1234);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("0000", &denominator);
    assert(numerator == 0);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("0009", &denominator);
    assert(numerator == 9);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("9000", &denominator);
    assert(numerator == 9000);
    assert(denominator == 10000);

    nanoseconds = hazer_parse_utc("000000");
    assert(nanoseconds == 0LL);

    nanoseconds = hazer_parse_utc("235959");
    assert(nanoseconds == 86399000000000ULL);

    nanoseconds = hazer_parse_utc("235959.");
    assert(nanoseconds == 86399000000000ULL);

    nanoseconds = hazer_parse_utc("235959.0");
    assert(nanoseconds == 86399000000000ULL);

    nanoseconds = hazer_parse_utc("235959.125");
    assert(nanoseconds == 86399125000000ULL);

    latlon = hazer_parse_latlon("00000", 'E');
    assert(latlon == 0.0);

    latlon = hazer_parse_latlon("00000", 'S');
    assert(latlon == 0.0);

    latlon = hazer_parse_latlon("18000", 'E');
    assert(latlon == 180.0);

    latlon = hazer_parse_latlon("18000", 'S');
    assert(latlon == -180.0);

    latlon = hazer_parse_latlon("18030", 'E');
    assert(latlon == 180.5);

    latlon = hazer_parse_latlon("18030", 'S');
    assert(latlon == -180.5);

    latlon = hazer_parse_latlon("18030.", 'E');
    assert(latlon == 180.5);

    latlon = hazer_parse_latlon("18030.", 'W');
    assert(latlon == -180.5);

    latlon = hazer_parse_latlon("18030.60", 'E');
    assert(latlon == 180.51);

    latlon = hazer_parse_latlon("18030.60", 'W');
    assert(latlon == -180.51);

    latlon = hazer_parse_latlon("0000", 'N');
    assert(latlon == 0.0);

    latlon = hazer_parse_latlon("0000", 'S');
    assert(latlon == 0.0);

    latlon = hazer_parse_latlon("9000", 'N');
    assert(latlon == 90.0);

    latlon = hazer_parse_latlon("9000", 'S');
    assert(latlon == -90.0);

    latlon = hazer_parse_latlon("9030", 'N');
    assert(latlon == 90.5);

    latlon = hazer_parse_latlon("9030", 'S');
    assert(latlon == -90.5);

    latlon = hazer_parse_latlon("9030.", 'N');
    assert(latlon == 90.5);

    latlon = hazer_parse_latlon("9030.", 'S');
    assert(latlon == -90.5);

    latlon = hazer_parse_latlon("9030.0", 'N');
    assert(latlon == 90.5);

    latlon = hazer_parse_latlon("9030.0", 'S');
    assert(latlon == -90.5);

    latlon = hazer_parse_latlon("9030.60", 'N');
    assert(latlon == 90.51);

    latlon = hazer_parse_latlon("9030.60", 'S');
    assert(latlon == -90.51);

    return 0;
}
