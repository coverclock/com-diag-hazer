/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019-2022 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Converts a number into a UBX-friendly form.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Converts a number in octal, decimal, or hexidecimal into
 * hexadecimal little endian form usable by the -U command line
 * flag in gpstool to send to a UBX-speaking device. A number can
 * be a 1, 2, 4, or 8 byte integer, or a single (4) or double (8)
 * precision floating point.
 *
 * USAGE
 *
 * ubxval [ -1 | -2 | -4 | -8 | -s | -d | +1 | +2 | +4 | +8 | +s | +d ] NUMBER
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>

static void emit(void * pp, size_t ss)
{
    unsigned char * cc = (char *)0;

    for (cc = (unsigned char *)pp; ss > 0; --ss) {
        printf("\\x%02x", *(cc++));
    }

    putchar('\n');
}

int main(int argc, char * argv[])
{
    int xc = 1;
    int size = 0;
    uint64_t n8 = 0;
    uint32_t n4 = 0;
    uint16_t n2 = 0;
    uint8_t  n1 = 0;
    float f4 = 0.0;
    double f8 = 0.0;
    char * end = (char *)0;
    const char * name = (char *)0;
    int debug = 0;

    do {

        name = ((name = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : name + 1;

        if (argc < 2) {
            errno = EINVAL;
            perror(argv[0]);
            break;
        }

        if (argv[1][0] == '-') {
            debug = 0;
        } else if (argv[1][0] == '+') {
            debug = !0;
        } else {
            errno = EINVAL;
            perror(argv[1]);
            break;
        }

        if (argv[1][1] == '?') {
            fprintf(stderr, "usage: %s [ -1 | -2 | -4 | -8 | -s | -d | +1 | +2 | +4 | +8 | +s | +d ] NUMBER\n", name);
            xc = 0;
            break;
        }

        if (argc != 3) {
            errno = EINVAL;
            perror(argv[0]);
            break;
        }

        if (argv[1][1] == 's') {

            size = sscanf(argv[2], "%f", &f4);
            if (size != 1) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }
            if (debug) { fprintf(stderr, "%s: %f\n", name, f4); }
            emit(&f4, sizeof(f4));

        } else if (argv[1][1] == 'd') {

            size = sscanf(argv[2], "%lf", &f8);
            if (size != 1) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }
            if (debug) { fprintf(stderr, "%s: %lf\n", name, f8); }
            emit(&f8, sizeof(f8));

        } else {

            n8 = strtoll(argv[2], &end, 0);
            if (end == argv[2]) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }

            size = atoi(&(argv[1][1]));
            if (size == sizeof(n1)) {
                n1 = n8;
                if (debug) { fprintf(stderr, "%s: %02x\n", name, n1); }
                emit(&n1, sizeof(n1));
            } else if (size == sizeof(n2)) {
                n2 = n8;
                if (debug) { fprintf(stderr, "%s: %04x\n", name, n2); }
                n2 = htole16(n2);
                emit(&n2, sizeof(n2));
            } else if (size == sizeof(n4)) {
                n4 = n8;
                if (debug) { fprintf(stderr, "%s: %08lx\n", name, (unsigned long)n4); }
                n4 = htole32(n4);
                emit(&n4, sizeof(n4));
            } else if (size == sizeof(n8)) {
                if (debug) { fprintf(stderr, "%s: %016llx\n", name, (unsigned long long)n8); }
                n8 = htole64(n8);
                emit(&n8, sizeof(n8));
            } else {
                errno = EINVAL;
                perror(argv[1]);
                break;
            }

        }

        xc = 0;

    } while (0);

    return xc;
}
