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

static struct {
    const char name[8];
    unsigned int minimum;
    unsigned int maximum;
} POINT[] = {
    { "N", 354380, 5630 },
    { "NbE", 5630, 16880 },
    { "NNE", 16880, 28130 },
    { "NEbN", 28130, 39380 },
    { "NE", 39380, 50630 },
    { "NEbE", 50630, 61880 },
    { "ENE", 61880, 73130 },
    { "EbN", 73130, 84380 },
    { "E", 84380, 95630 },
    { "EbS", 95630, 106880 },
    { "ESE", 106880, 118130 },
    { "SEbE", 118130, 129380 },
    { "SE", 129380, 140630 },
    { "SEbS", 140630, 151880 },
    { "SSE", 151880, 163130 },
    { "SbE", 163130, 174380 },
    { "S", 174380, 185630 },
    { "SbW", 185630, 196880 },
    { "SSW", 196880, 208130 },
    { "SWbS", 208130, 219380 },
    { "SW", 219380, 230630 },
    { "SWbW", 230630, 241880 },
    { "WSW", 241880, 253130 },
    { "WbS", 253130, 264380 },
    { "W", 264380, 275630 },
    { "WbN", 275630, 286880 },
    { "WNW", 286880, 298130 },
    { "NWbW", 298130, 309380 },
    { "NW", 309380, 320630 },
    { "NWbN", 320630, 331880 },
    { "NNW", 331880, 343130 },
    { "NbW", 343130, 354380 },
};

int main(void)
{
    double degrees = 0.0;
    const char * name = (const char *)0;
    int index = 0;
    unsigned int millidegrees = 0;

    for (degrees = 0.00; degrees < 360.00; degrees += 0.01) {
        name = hazer_format_degrees2compass(degrees);
        millidegrees = degrees * 1000.0;
        for (index = 0; index < (sizeof(POINT) / sizeof(POINT[0])); ++index) {
            if (index == 0) {
                if ((POINT[index].minimum <= millidegrees) && (millidegrees < 360000)) {
                    printf("%.2lf %s %d (%.2lf <= %.2lf < %.2lf) %s\n", degrees, name, index, POINT[index].minimum / 1000.0, degrees, 360.00,  POINT[index].name);
                    break;
                }
                if ((0 <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                    printf("%.2lf %s %d (%.2lf <= %.2lf < %.2lf) %s\n", degrees, name, index, 0.00, degrees, POINT[index].maximum / 1000.0, POINT[index].name);
                    break;
                }
            } else {
                if ((POINT[index].minimum <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                    printf("%.2lf %s %d (%.2lf <= %.2lf < %.2lf) %s\n", degrees, name, index, POINT[index].minimum / 1000.0, degrees, POINT[index].maximum / 1000.0, POINT[index].name);
                    break;
                }
            }
        }
        if (index >= (sizeof(POINT) / sizeof(POINT[0]))) {
            printf("%.2lf %s\n", degrees, name);
        }
    }
}
