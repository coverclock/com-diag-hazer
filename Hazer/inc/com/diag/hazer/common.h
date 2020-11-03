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

/**
 * Return the absolute value of a 64-bit integer.
 * @param x is the integer.
 * @return its absolute value.
 */
static inline int64_t abs64(int64_t x) { return (x < 0) ? -x : x; }

#endif
