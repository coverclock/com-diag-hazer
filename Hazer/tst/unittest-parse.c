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
    int64_t nanodegrees = 0;
    int64_t millimeters = 0;
    int64_t microknots = 0;
    double value = 0;
    uint64_t number = 0;
    uint8_t digits = 0;

    /**************************************************************************/

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

    /**************************************************************************/

    number = hazer_parse_utc("000000");
    assert(number == 0LL);

    number = hazer_parse_utc("235959");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.0");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.125");
    assert(number == 86399125000000ULL);

    /**************************************************************************/

    number = hazer_parse_dmy("310117");
    assert(number == 1485820800000000000ULL);

    /**************************************************************************/

    nanodegrees = hazer_parse_latlon("00000", 'E', &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("00000", 'S', &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18000", 'E', &digits);
    assert(nanodegrees == 180000000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18000", 'S', &digits);
    assert(nanodegrees == -180000000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18030", 'E', &digits);
    assert(nanodegrees == 180500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18030", 'S', &digits);
    assert(nanodegrees == -180500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18030.", 'E', &digits);
    assert(nanodegrees == 180500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18030.", 'W', &digits);
    assert(nanodegrees == -180500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("18030.60", 'E', &digits);
    assert(nanodegrees == 180510000000LL);
    assert(digits == 7);

    nanodegrees = hazer_parse_latlon("18030.60", 'W', &digits);
    assert(nanodegrees == -180510000000LL);
    assert(digits == 7);

    nanodegrees = hazer_parse_latlon("0000", 'N', &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("0000", 'S', &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9000", 'N', &digits);
    assert(nanodegrees == 90000000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9000", 'S', &digits);
    assert(nanodegrees == -90000000000);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9030", 'N', &digits);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9030", 'S', &digits);
    assert(nanodegrees == -90500000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9030.", 'N', &digits);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9030.", 'S', &digits);
    assert(nanodegrees == -90500000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_latlon("9030.0", 'N', &digits);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("9030.0", 'S', &digits);
    assert(nanodegrees == -90500000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_latlon("9030.60", 'N', &digits);
    assert(nanodegrees == 90510000000LL);
    assert(digits == 6);

    nanodegrees = hazer_parse_latlon("9030.60", 'S', &digits);
    assert(nanodegrees == -90510000000);
    assert(digits == 6);

    nanodegrees = hazer_parse_latlon("9030.66", 'N', &digits);
    assert(nanodegrees == 90511000000LL);
    assert(digits == 6);

    /**************************************************************************/

    nanodegrees = hazer_parse_cog("0", &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 1);

    nanodegrees = hazer_parse_cog("360", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("360.", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("360.0", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_cog("360.00", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_cog("90.5", &digits);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("90.25", &digits);
    assert(nanodegrees == 90250000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_cog("90.125", &digits);
    assert(nanodegrees == 90125000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_cog("-90.125", &digits);
    assert(nanodegrees == -90125000000LL);
    assert(digits == 5);

    /**************************************************************************/

    microknots = hazer_parse_sog("15.5", &digits);
    assert(microknots == 15500000LL);
    assert(digits == 3);

    microknots = hazer_parse_sog("-15.5", &digits);
    assert(microknots == -15500000LL);
    assert(digits == 3);

    /**************************************************************************/

    millimeters = hazer_parse_alt("", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 0);

    millimeters = hazer_parse_alt("0", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 1);

    millimeters = hazer_parse_alt("0.", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 1);

    millimeters = hazer_parse_alt("0.0", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 2);

    millimeters = hazer_parse_alt("521.125", 'M', &digits);
    assert(millimeters == 521125000ULL);
    assert(digits == 6);

    millimeters = hazer_parse_alt("-521.125", 'M', &digits);
    assert(millimeters == -521125000ULL);
    assert(digits == 6);

    /**************************************************************************/

    value = hazer_parse_num("");
    assert(value == 0.0);

    value = hazer_parse_num("0");
    assert(value == 0.0);

    value = hazer_parse_num("0.");
    assert(value == 0.0);

    value = hazer_parse_num("0.0");
    assert(value == 0.0);

    value = hazer_parse_num("521.125");
    assert(value == 521.125);

    /**************************************************************************/

    return 0;
}
