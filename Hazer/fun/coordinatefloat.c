/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is a functional test of coordinate conversion.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * This is an experiment to compare the difference between
 * float (32-bits) and double (64-bits) in coordinate conversion.
 * It was inspired by some weirdness in how Google Maps does its
 * own conversion (which still eludes me).
 *
 * USAGE
 *
 * coordinatefloat SIGNEDDEGREES UNSIGNEDMINUTES DECIMALSECONDS
 *
 * EXAMPLE
 *
 * $ coordinatefloat -32 47 39.4
 * "-32" "47" "39.4" [4] -32.794277191 [8] -32.794277777777779193
 *
 * NOTES
 *
 * Google Maps converts 39.794272981, -105.153413763
 * to 39.794273, -105.153414 (which seems reasonable)
 * which it displays as 39°47'39.4"N 105°09'12.3"W.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char * argv[])
{
    float decimal32 = 0.0;
    double decimal64 = 0.0;
    unsigned int minutes = 0;
    int degrees = 0;
    int sign = 1;

    if (argc != 4) {
        errno = EINVAL;
        perror("4");
        return 1;
    }

    if (sscanf(argv[3], "%f", &decimal32) != 1) {
        errno = EINVAL;
        perror(argv[3]);
        return 2;
    }

    if (sscanf(argv[3], "%lf", &decimal64) != 1) {
        errno = EINVAL;
        perror(argv[3]);
        return 3;
    }

    if (sscanf(argv[2], "%u", &minutes) != 1) {
        errno = EINVAL;
        perror(argv[2]);
        return 4;
    }

    decimal32 = decimal32 / 60.0;
    decimal32 += minutes;

    decimal64 = decimal64 / 60.0;
    decimal64 += minutes;

    if (sscanf(argv[1], "%d", &degrees) != 1) {
        errno = EINVAL;
        perror(argv[1]);
        return 5;
    }

    if (degrees < 0) {
        degrees = -degrees;
        sign = -sign;
    }

    decimal32 = decimal32 / 60.0;
    decimal32 += degrees;
    decimal32 *= sign;

    decimal64 = decimal64 / 60.0;
    decimal64 += degrees;
    decimal64 *= sign;

    printf("\"%s\" \"%s\" \"%s\" [%zu] %.9f [%zu] %.18lf\n", argv[1], argv[2], argv[3], sizeof(decimal32), decimal32, sizeof(decimal64), decimal64);

    return 0;
}
