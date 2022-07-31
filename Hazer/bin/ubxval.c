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
 * Converts a number in octal, decimal, hexidecimal, or float into
 * hexadecimal little endian form usable by the -U command line
 * flag in gpstool to send to a UBX-speaking device. A number can
 * be a 1, 2, 4, or 8 byte integer value, or a Single- (4 byte) or
 * Double- (8 byte) precision floating-point value.
 *
 * Using a '+' instead of a '-' as the option prefix turns on
 * debugging output to standard error.
 *
 * USAGE
 *
 * ubxval [ -1 | -2 | -4 | -8 | -S | -D | +1 | +2 | +4 | +8 | +S | +D ] STRING
 *
 * REFERENCES
 * 
 * Wikipedia, "IEEE 754", 2022-07-07,
 * <https://en.wikipedia.org/wiki/IEEE_754>
 *
 * Wikipedia, "Double-precision floating-point format", 2022-06-28,
 * <https://en.wikipedia.org/wiki/Double-precision_floating-point_format>
 *
 * EXAMPLES
 *
 * $ ubxval +1 65
 * ubxval: 65 0x41
 * \x41
 *
 * $ ubxval +2 65
 * ubxval: 65 0x0041
 * \x41\x00
 * 
 * $ ubxval +4 65
 * ubxval: 65 0x00000041
 * \x41\x00\x00\x00
 *
 * $ ubxval +8 65
 * ubxval: 65 0x0000000000000041
 * \x41\x00\x00\x00\x00\x00\x00\x00
 * 
 * $ ubxval +S 0.15625
 * ubxval: 0.156250 0x3e200000
 * \x00\x00\x20\x3e
 * 
 * $ ubxval +D 0.333333333333333314829616256247390992939472198486328125
 * ubxval: 0.333333 0x3fd5555555555555
 * \x55\x55\x55\x55\x55\x55\xd5\x3f
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
    int64_t s8 = 0;
    int32_t s4 = 0;
    int16_t s2 = 0;
    int8_t  s1 = 0;
    float f4 = 0.0;
    double f8 = 0.0;
    union { uint32_t u4; float f4; } x4;
    union { uint64_t u8; double f8; } x8;
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
            fprintf(stderr, "usage: %s [ -1 | -2 | -4 | -8 | -S | -D | +1 | +2 | +4 | +8 | +S | +D ] NUMBER\n", name);
            xc = 0;
            break;
        }

        if (argc != 3) {
            errno = EINVAL;
            perror(argv[0]);
            break;
        }

        if (argv[1][1] == 'S') {

            size = sscanf(argv[2], "%f", &f4);
            if (size != 1) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }
            x4.f4 = f4;
            if (debug) { fprintf(stderr, "%s: %f 0x%08lx\n", name, f4, (unsigned long)x4.u4); }
            s4 = htole32(x4.u4);
            emit(&s4, sizeof(s4));

        } else if (argv[1][1] == 'D') {

            size = sscanf(argv[2], "%lf", &f8);
            if (size != 1) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }
            x8.f8 = f8;
            if (debug) { fprintf(stderr, "%s: %lf 0x%016llx\n", name, f8, (unsigned long long)x8.u8); }
            s8 = htole64(x8.u8);
            emit(&s8, sizeof(s8));

        } else {

            s8 = strtoll(argv[2], &end, 0);
            if (end == argv[2]) {
                errno = EINVAL;
                perror(argv[2]);
                break;
            }

            size = atoi(&(argv[1][1]));
            if (size == sizeof(s1)) {
                s1 = s8;
                if (debug) { fprintf(stderr, "%s: %d 0x%02x\n", name, s1, s1); }
                emit(&s1, sizeof(s1));
            } else if (size == sizeof(s2)) {
                s2 = s8;
                if (debug) { fprintf(stderr, "%s: %d 0x%04x\n", name, s2, s2); }
                s2 = htole16(s2);
                emit(&s2, sizeof(s2));
            } else if (size == sizeof(s4)) {
                s4 = s8;
                if (debug) { fprintf(stderr, "%s: %ld 0x%08lx\n", name, (long)s4, (unsigned long)s4); }
                s4 = htole32(s4);
                emit(&s4, sizeof(s4));
            } else if (size == sizeof(s8)) {
                if (debug) { fprintf(stderr, "%s: %lld 0x%016llx\n", name, (long long)s8, (unsigned long long)s8); }
                s8 = htole64(s8);
                emit(&s8, sizeof(s8));
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
