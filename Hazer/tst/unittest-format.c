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
    const char name[4];
    double minimum;
    double maximum;
} POINT[] = {
    { "N", 354.38, 5.63 },
    { "NbE", 5.63, 16.88 },
    { "NNE", 16.88, 28.13 },
    { "NEbN", 28.13, 39.38 },
    { "NE", 39.38, 50.63 },
    { "NEbE", 50.63, 61.88 },
    { "ENE", 61.88, 73.13 },
    { "EbN", 73.13, 84.38 },
    { "E", 84.38, 95.63 },
    { "EbS", 95.63, 106.88 },
    { "ESE", 106.88, 118.13 },
    { "SEbE", 118.13, 129.38 },
    { "SE", 129.38, 140.63 },
    { "SEbS", 140.63, 151.88 },
    { "SSE", 151.88, 163.13 },
    { "SbE", 163.13, 174.38 },
    { "S", 174.38, 185.63 },
    { "SbW", 185.63, 196.88 },
    { "SSW", 196.88, 208.13 },
    { "SWbS", 208.13, 219.38 },
    { "SW", 219.38, 230.63 },
    { "SWbW", 230.63, 241.88 },
    { "WSW", 241.88, 253.13 },
    { "WbS", 253.13, 264.38 },
    { "W", 264.38, 275.63 },
    { "WbN", 275.63, 286.88 },
    { "WNW", 286.88, 298.13 },
    { "NWbW", 298.13, 309.38 },
    { "NW", 309.38, 320.63 },
    { "NWbN", 320.63, 331.88 },
    { "NNW", 331.88, 343.13 },
    { "NbW", 343.13, 354.38 },
};

int main(void)
{
    double degrees = 0.0;
    const char * name = (const char *)0;
    int index = 0;

    for (degrees = 0.00; degrees < 360.00; degrees += 0.01) {
        name = hazer_format_degrees2compass(degrees);
        for (index = 0; index < (sizeof(POINT) / sizeof(POINT[0])); ++index) {
            if (index == 0) {
                if ((POINT[index].minimum <= degrees) && (degrees < 360.00)) {
                    printf("%.3lf %s %d (%.3lf <= %.3lf <= %.3lf) %s\n", degrees, name, index, POINT[index].minimum, degrees, 360.00,  POINT[index].name);
                    break;
                }
                if ((0.00 <= degrees) && (degrees < POINT[index].maximum)) {
                    printf("%.3lf %s %d (%.3lf <= %.3lf <= %.3lf) %s\n", degrees, name, index, 0.00, degrees, POINT[index].maximum, POINT[index].name);
                    break;
                }
            } else {
                if ((POINT[index].minimum <= degrees) && (degrees < POINT[index].maximum)) {
                    printf("%.3lf %s %d (%.3lf <= %.3lf <= %.3lf) %s\n", degrees, name, index, POINT[index].minimum, degrees, POINT[index].maximum, POINT[index].name);
                    break;
                }
            }
        }
        if (index >= (sizeof(POINT) / sizeof(POINT[0]))) {
            printf("%.3lf %s\n", degrees, name);
        }
    }
}
