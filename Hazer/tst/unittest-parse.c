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
    double value = 0;
    uint64_t number = 0;
    uint8_t digits = 0;

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

    value = hazer_parse_latlon("00000", 'E', &digits);
    assert(value == 0.0);
    assert(digits == 5);

    value = hazer_parse_latlon("00000", 'S', &digits);
    assert(value == 0.0);
    assert(digits == 5);

    value = hazer_parse_latlon("18000", 'E', &digits);
    assert(value == 180.0);
    assert(digits == 5);

    value = hazer_parse_latlon("18000", 'S', &digits);
    assert(value == -180.0);
    assert(digits == 5);

    value = hazer_parse_latlon("18030", 'E', &digits);
    assert(value == 180.5);
    assert(digits == 5);

    value = hazer_parse_latlon("18030", 'S', &digits);
    assert(value == -180.5);
    assert(digits == 5);

    value = hazer_parse_latlon("18030.", 'E', &digits);
    assert(value == 180.5);
    assert(digits == 5);

    value = hazer_parse_latlon("18030.", 'W', &digits);
    assert(value == -180.5);
    assert(digits == 5);

    value = hazer_parse_latlon("18030.60", 'E', &digits);
    assert(value == 180.51);
    assert(digits == 7);

    value = hazer_parse_latlon("18030.60", 'W', &digits);
    assert(value == -180.51);
    assert(digits == 7);

    value = hazer_parse_latlon("0000", 'N', &digits);
    assert(value == 0.0);
    assert(digits == 4);

    value = hazer_parse_latlon("0000", 'S', &digits);
    assert(value == 0.0);
    assert(digits == 4);

    value = hazer_parse_latlon("9000", 'N', &digits);
    assert(value == 90.0);
    assert(digits == 4);

    value = hazer_parse_latlon("9000", 'S', &digits);
    assert(value == -90.0);
    assert(digits == 4);

    value = hazer_parse_latlon("9030", 'N', &digits);
    assert(value == 90.5);
    assert(digits == 4);

    value = hazer_parse_latlon("9030", 'S', &digits);
    assert(value == -90.5);
    assert(digits == 4);

    value = hazer_parse_latlon("9030.", 'N', &digits);
    assert(value == 90.5);
    assert(digits == 4);

    value = hazer_parse_latlon("9030.", 'S', &digits);
    assert(value == -90.5);
    assert(digits == 4);

    value = hazer_parse_latlon("9030.0", 'N', &digits);
    assert(value == 90.5);
    assert(digits == 5);

    value = hazer_parse_latlon("9030.0", 'S', &digits);
    assert(value == -90.5);
    assert(digits == 5);

    value = hazer_parse_latlon("9030.60", 'N', &digits);
    assert(value == 90.51);
    assert(digits == 6);

    value = hazer_parse_latlon("9030.60", 'S', &digits);
    assert(value == -90.51);
    assert(digits == 6);

    value = hazer_parse_number("");
    assert(value == 0.0);

    value = hazer_parse_number("0");
    assert(value == 0.0);

    value = hazer_parse_number("0.");
    assert(value == 0.0);

    value = hazer_parse_number("0.0");
    assert(value == 0.0);

    value = hazer_parse_number("521.125");
    assert(value == 521.125);

    value = hazer_parse_alt("521.125", 'M');
    assert(value == 521.125);

    number = hazer_parse_dmy("310117");
    assert(number == 1485820800000000000ULL);

    return 0;
}
