/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_COMMON_
#define _H_COM_DIAG_HAZER_COMMON_

/**
 * @file
 * @copyright Copyright 2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Common facilities for other stuff.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdint.h>
#include <wchar.h>

#if !defined(COMMON_DEGREE_VALUE)
    /**
     * @def COMMON_DEGREE_VALUE
     * Define the value that is used for the degree symbol. By default this
     * is the wide character Unicode for the degree symbol, but can be
     * defined at compile time to be something else like '*' to avoid UTF-8.
     * (I'm not convinced this is correct for big endian architectures.)
     */
#   define COMMON_DEGREE_VALUE '\xb0'
#endif

/**
 * This is the Unicode for the degree symbol.
 */
static const wint_t COMMON_DEGREE = COMMON_DEGREE_VALUE;

#if !defined(COMMON_PLUSMINUS_VALUE)
    /**
     * @def COMMON_PLUSMINUS_VALUE
     * Define the value that is used for the plus minus symbol. By default this
     * is the wide character Unicode for the plus minus symbol, but can be
     * defined at compile time to be something else like '~' to avoid UTF-8.
     * (I'm not convinced this is correct for big endian architectures.)
     */
#   define COMMON_PLUSMINUS_VALUE '\xb1'
#endif

/**
 * This is the Unicode for the plus minus symbol.
 */
static const wint_t COMMON_PLUSMINUS = COMMON_PLUSMINUS_VALUE;

/**
 * Return the absolute value of a 64-bit integer.
 * @param x is the integer.
 * @return its absolute value.
 */
static inline int64_t common_abs64(int64_t x)
{
    return (x < 0) ? -x : x;
}

#if !defined(abs64)
#   define abs64 common_abs64
#endif

#endif
