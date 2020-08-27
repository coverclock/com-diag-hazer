/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 * Converts a decimal bearing to a 8, 16, or 32 compass point.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "com/diag/hazer/hazer.h"

int main(int argc, char *argv[])
{
    const char * program = (const char *)0;
    double bearing = 0.0;
    int64_t nanodegrees = 0;
    int points = 0;
    const char * point = "";
    const char * (*compass)(int64_t) = ((const char *(*)(int64_t))0);
    char * end = (char *)0;
    int debug = 0;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
    argv++;
    argc--;

    if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
        fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -e | -s | -t ] BEARING\n", program);
        argv++;
        argc--;
    }

    if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
        debug = !0;
        argv++;
        argc--;
    }

    if (argc <= 0) {
        /* Do nothing. */
    } else if (strcmp(*argv, "-e") == 0) {
        points = 8;
        compass = &hazer_format_nanodegrees2compass8;
        argv++;
        argc--;
    } else if (strcmp(*argv, "-s") == 0) {
        points = 16;
        compass = &hazer_format_nanodegrees2compass16;
        argv++;
        argc--;
    } else if (strcmp(*argv, "-t") == 0) {
        points = 32;
        compass = &hazer_format_nanodegrees2compass32;
        argv++;
        argc--;
    } else {
        points = 16;
        compass = &hazer_format_nanodegrees2compass16;
    }

    if (argc > 0) {
        bearing = strtod(*argv, &end);
        nanodegrees = bearing * 1000000000.0;
        while (nanodegrees < 0) { nanodegrees += 360000000000; }
        if (nanodegrees > 360000000LL) { nanodegrees = nanodegrees % 360000000000LL; }
        point = (*compass)(nanodegrees);
        if (debug) {
            fprintf(stderr, "%s: [%d] %7.3lf %lld %s\n", program, points, bearing, (long long int)nanodegrees, point);
        }
        printf("%s\n", point);
    }

    return 0;
}

