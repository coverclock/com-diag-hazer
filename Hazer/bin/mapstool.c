/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019-2022 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Convert coordinates into a Google Maps friendly form.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Converts coordinates in a string into a decimal degrees format that
 * Google Maps will understand. The output format can be either in decimal
 * degrees (-D, the default), or in DDDMMSS.FFFFFFC position format (-P).
 * To make cutting and pasting from gpstool output simpler, the strings can
 * be prefixed by the corresponding gpstool tags.
 *
 * USAGE
 *
 * maps [ -? ] [ -d ] [ -D | -P ] STRING [ [ -D | -P ] STRING ... ]
 *
 * EXAMPLES
 *
 * N.B. On MacOS, the degree symbol is typed via: shift option 8
 *
 * $ mapstool "39.794212196, -105.153349930"
 * 39.794212196, -105.153349930
 *
 * $ mapstool "39 47 39.16390(N) 105 09 12.05974(W)"
 * 39.794212194, -105.153349928
 *
 * $ mapstool "39°47'39.163\"N, 105°09'12.060\"W"
 * 39.794211944, -105.153350000
 *
 * $ mapstool -D "39.794212196, -105.153349930"
 * 39.794212196, -105.153349930
 *
 * $ mapstool -D "39 47 39.16390(N) 105 09 12.05974(W)"
 * 39.794212194, -105.153349928
 *
 * $ mapstool -D "39°47'39.163\"N, 105°09'12.060\"W"
 * 39.794211944, -105.153350000
 *
 * $ mapstool -P "39.794212196, -105.153349930"
 * 39°47'39.163905"N, 105°09'12.059748"W
 *
 * $ mapstool -P "39 47 39.16390(N) 105 09 12.05974(W)"
 * 39°47'39.163899"N, 105°09'12.059740"W
 *
 * $ mapstool -P "39°47'39.163\"N, 105°09'12.060\"W"
 * 39°47'39.162999"N, 105°09'12.060000"W
 *
 * $ mapstool "HPP   39.794212196, -105.153349930"
 * 39.794212196, -105.153349930
 *
 * $ mapstool "NGS  39 47 39.16390(N) 105 09 12.05974(W)"
 * 39.794212194, -105.153349928
 *
 * $ mapstool "POS 39°47'39.163\"N, 105°09'12.060\"W"
 * 39.794211944, -105.153350000
 *
 * $ mapstool -D "HPP   39.794212196, -105.153349930"
 * 39.794212196, -105.153349930
 *
 * $ mapstool -D "NGS  39 47 39.16390(N) 105 09 12.05974(W)"
 * 39.794212194, -105.153349928
 *
 * $ mapstool -D "POS 39°47'39.163\"N, 105°09'12.060\"W"
 * 39.794211944, -105.153350000
 *
 * $ mapstool -P "HPP    39.794212196, -105.153349930"
 * 39°47'39.163905"N, 105°09'12.059748"W
 *
 * $ mapstool -P "NGS  39 47 39.16390(N) 105 09 12.05974(W)"
 * 39°47'39.163899"N, 105°09'12.059740"W
 *
 * $ mapstool -P "POS 39°47'39.163\"N, 105°09'12.060\"W"
 * 39°47'39.162999"N, 105°09'12.060000"W
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <wchar.h>
#include <errno.h>
#include "com/diag/hazer/common.h"
#include "com/diag/hazer/coordinates.h"
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_log.h"

int main(int argc, char *argv[])
{
    const char * program = (const char *)0;
    const char * locale = (const char *)0;
    int rc = 0;
    int position = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;
    int millionths = 0;
    int direction = 0;
    int debug = 0;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;
    argv++;
    argc--;

    if ((argc > 0) && (strcmp(*argv, "-?") == 0)) {
        fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -D | -P ] STRING [ STRING ... ]\n", program);
        argv++;
        argc--;
    }

    if ((argc > 0) && (strcmp(*argv, "-d") == 0)) {
        (void)coordinates_debug(!0);
        debug = !0;
        argv++;
        argc--;
    }

    rc = setenv("LC_ALL", "en_US.UTF-8", 0);
    if (rc < 0) {
        diminuto_perror("setenv");
        return 1;
    }
    locale = setlocale(LC_ALL, "");
    if (locale == (const char *)0) {
        diminuto_perror("setlocale");
        return 1;
    }

    if (debug) {
        static const wchar_t DEGREE = 0x00b0;
        static const wchar_t PLUSMINUS = 0x00b1;
        fprintf(stderr, "%s[%d]: sizeof(wchar_t)=%zu\n", __FILE__, __LINE__, sizeof(wchar_t));
        fprintf(stderr, "%s[%d]: sizeof(wint_t)=%zu\n", __FILE__, __LINE__, sizeof(wint_t));
        fprintf(stderr, "%s[%d]: DEGREE=0x%x=\'%lc\'", __FILE__, __LINE__, DEGREE, (wint_t)DEGREE);
        fputc('\n', stderr);
        fprintf(stderr, "%s[%d]: PLUSMINUS=0x%x=\'%lc\'", __FILE__, __LINE__, PLUSMINUS, (wint_t)PLUSMINUS);
        fputc('\n', stderr);
        fprintf(stderr, "%s[%d]: COMMON_DEGREE=0x%x=\'%lc\'", __FILE__, __LINE__, COMMON_DEGREE, (wint_t)COMMON_DEGREE);
        fputc('\n', stderr);
        fprintf(stderr, "%s[%d]: COMMON_PLUSMINUS=0x%x=\'%lc\'", __FILE__, __LINE__, COMMON_PLUSMINUS, (wint_t)COMMON_PLUSMINUS);
        fputc('\n', stderr);
    }

    while (argc-- > 0) {
        if (strcmp(*argv, "-D") == 0) {
            position = 0;
            argv++;
        } else if (strcmp(*argv, "-P") == 0) {
            position = !0;
            argv++;
        } else if (coordinates_parse(*argv, &latitude, &longitude) <= 0) {
            errno = EINVAL;
            diminuto_perror(*argv);
            return 1;
        } else if (position) {
            degrees = 0;
            minutes = 0;
            seconds = 0;
            millionths = 0;
            direction = 0;
            coordinates_format_decimaldegrees2position(latitude, &degrees, &minutes, &seconds, &millionths, &direction);
            diminuto_assert((0 <= degrees) && (degrees <= 90));
            diminuto_assert((0 <= minutes) && (minutes <= 59));
            diminuto_assert((0 <= seconds) && (seconds <= 59));
            diminuto_assert((0 <= millionths) && (millionths <= 999999));
            printf("%2d%lc%02d'%02d.%06d\"%c,", degrees, (wint_t)COMMON_DEGREE, minutes, seconds, millionths, (direction < 0) ? 'S' : 'N');
            degrees = 0;
            minutes = 0;
            seconds = 0;
            millionths = 0;
            direction = 0;
            coordinates_format_decimaldegrees2position(longitude, &degrees, &minutes, &seconds, &millionths, &direction);
            diminuto_assert((0 <= degrees) && (degrees <= 180));
            diminuto_assert((0 <= minutes) && (minutes <= 59));
            diminuto_assert((0 <= seconds) && (seconds <= 59));
            diminuto_assert((0 <= millionths) && (millionths <= 999999));
            printf(" %3d%lc%02d'%02d.%06d\"%c\n", degrees, (wint_t)COMMON_DEGREE, minutes, seconds, millionths, (direction < 0) ? 'W' : 'E');
            argv++;
        } else {
            printf("%.9lf, %.9lf\n", latitude, longitude);
            argv++;
        }
    }

    return 0;
}
