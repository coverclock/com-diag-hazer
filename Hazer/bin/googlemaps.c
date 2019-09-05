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
 * Converts coordinates in a string into a decimal degrees format that
 * Google Maps will understand.
 *
 * USAGE
 *
 * googlemaps [ -? ] [ -d ] STRING [ STRING ... ]
 *
 * EXAMPLES
 *
 * $ googlemaps "39.794212196, -105.153349930"
 * 39.794212196, -105.153349930
 *
 * $ googlemaps "39 47 39.16390(N) 105 09 12.05974(W)"
 * 39.794212194, -105.153349928
 *
 * $ googlemaps "39°47'39.163\"N, 105°09'12.060\"W"
 * 39.794211944, -105.153350000
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <errno.h>
#include "com/diag/hazer/coordinates.h"
#include "com/diag/diminuto/diminuto_log.h"

int main(int argc, char *argv[])
{
    const char * program = (const char *)0;
    double latitude = 0.0;
    double longitude = 0.0;

    setlocale(LC_ALL, "");

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
    argv++;
    argc--;

    if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
        fprintf(stderr, "usage: %s [ -? ] [ -d ] STRING [ STRING ... ]\n", program);
        argv++;
        argc--;
    }

    if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
        (void)coordinates_debug(!0);
        argv++;
        argc--;
    }

    while (argc-- > 0) {
        if (coordinates_parse(*argv, &latitude, &longitude) <= 0) {
            errno = EINVAL;
            diminuto_perror(*argv);
            return 1;
        } else {
            printf("%.9lf, %.9lf\n", latitude, longitude);
            argv++;
        }
    }

    return 0;
}
