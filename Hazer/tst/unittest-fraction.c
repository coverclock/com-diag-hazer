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
    uint32_t numerator = 0;
    uint32_t denominator = 0;

    numerator = hazer_fraction("", &denominator);
    assert(numerator == 0);
    assert(denominator == 1);

    numerator = hazer_fraction("1", &denominator);
    assert(numerator == 1);
    assert(denominator == 10);

    numerator = hazer_fraction("12", &denominator);
    assert(numerator == 12);
    assert(denominator == 100);

    numerator = hazer_fraction("123", &denominator);
    assert(numerator == 123);
    assert(denominator == 1000);

    numerator = hazer_fraction("1234", &denominator);
    assert(numerator == 1234);
    assert(denominator == 10000);

    numerator = hazer_fraction("0000", &denominator);
    assert(numerator == 0);
    assert(denominator == 10000);

    numerator = hazer_fraction("0009", &denominator);
    assert(numerator == 9);
    assert(denominator == 10000);

    numerator = hazer_fraction("9000", &denominator);
    assert(numerator == 9000);
    assert(denominator == 10000);

    return 0;
}
