/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "com/diag/hazer/hazer.h"

/**
 * Deriged from "Points of the compass" in Wikipedia except that, remarkably, I don't think
 * it's correct. The "32 cardinal points" table needs to have three fractional digits to be
 * accurate.
 */
static struct {
    const char name[8];
    unsigned int minimum;
    unsigned int maximum;
} POINT[] = {
    { "N", 354375, 5625 },
    { "NbE", 5625, 16875 },
    { "NNE", 16875, 28125 },
    { "NEbN", 28125, 39375 },
    { "NE", 39375, 50625 },
    { "NEbE", 50625, 61875 },
    { "ENE", 61875, 73125 },
    { "EbN", 73125, 84375 },
    { "E", 84375, 95625 },
    { "EbS", 95625, 106875 },
    { "ESE", 106875, 118125 },
    { "SEbE", 118125, 129375 },
    { "SE", 129375, 140625 },
    { "SEbS", 140625, 151875 },
    { "SSE", 151875, 163125 },
    { "SbE", 163125, 174375 },
    { "S", 174375, 185625 },
    { "SbW", 185625, 196875 },
    { "SSW", 196875, 208125 },
    { "SWbS", 208125, 219375 },
    { "SW", 219375, 230625 },
    { "SWbW", 230625, 241875 },
    { "WSW", 241875, 253125 },
    { "WbS", 253125, 264375 },
    { "W", 264375, 275625 },
    { "WbN", 275625, 286875 },
    { "WNW", 286875, 298125 },
    { "NWbW", 298125, 309375 },
    { "NW", 309375, 320625 },
    { "NWbN", 320625, 331875 },
    { "NNW", 331875, 343125 },
    { "NbW", 343125, 354375 },
};

int main(void)
{
    double degrees = 0.0;
    const char * name = (const char *)0;
    int index = 0;
    uint64_t nanodegrees = 0;
    uint32_t millidegrees = 0;

    for (degrees = 0.00; degrees < 360.00; degrees += 0.01) {
        nanodegrees = degrees * 1000000000.0;
        name = hazer_format_nanodegrees2compass32(nanodegrees);
        millidegrees = degrees * 1000.0;
        for (index = 0; index < (sizeof(POINT) / sizeof(POINT[0])); ++index) {
            if (index == 0) {
                if ((POINT[index].minimum <= millidegrees) && (millidegrees < 360000)) {
                    break;
                }
                if ((0 <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                    break;
                }
            } else {
                if ((POINT[index].minimum <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                    break;
                }
            }
        }
        assert(index < (sizeof(POINT) / sizeof(POINT[0])));
        assert(strcmp(name, POINT[index].name) == 0);
    }
}
