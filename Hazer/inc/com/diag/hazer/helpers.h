/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_HELPERS_
#define _H_COM_DIAG_HAZER_HELPERS_

/**
 * @file
 *
 * Copyright 2020 Digital Aggregates Corporation, Colorado, USA.
 * Licensed under the terms in LICENSE.txt.
 */

#include <stdint.h>

/**
 * Return the absolute value of a signed thirty-two bit integer.
 * @param datum is a signed thirty-two bit integer.
 * @return an unsigned thirty-two bit integer.
 */
static inline uint32_t abs32(int32_t datum)
{
    return (datum >= 0) ? datum : -datum;
}

/**
 * Return the absolute value of a signed sixty-four bit integer.
 * @param datum is a signed sixty-four bit integer.
 * @return an unsigned sixty-four bit integer.
 */
static inline uint64_t abs64(int64_t datum)
{
    return (datum >= 0) ? datum : -datum;
}

#endif
