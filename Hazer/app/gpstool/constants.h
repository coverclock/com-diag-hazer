/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_CONSTANTS_
#define _H_COM_DIAG_HAZER_GPSTOOL_CONSTANTS_

/**
 * @file
 * @copyright Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This defines the gpstool Constants.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <sys/types.h>
#include <wchar.h>
#include "types.h"

/**
 * Encodings for fix type.
 */
static const char FIXES[] = {
    '-',    /* no fix */
    '!',    /* dead reckoning only */
    '2',    /* 2D-fix */
    '3',    /* 3D-fix */
    '+',    /* GNSS + dead reckoning combined */
    '*',    /* time only fix */
    '?',    /* error */
};

/**
 * Headings for CSV columns.
 */
static const char * const HEADINGS[] = {
    "NAM",
    "NUM",
    "FIX",
    "SYS",
    "SAT",
    "CLK",
    "TIM",
    "LAT",
    "LON",
    "HAC",
    "MSL",
    "GEO",
    "VAC",
    "SOG",
    "COG",
    "ROL",
    "PIT",
    "YAW",
    "RAC",
    "PAC",
    "YAC",
    "OBS",
    "MAC",
};

/**
 * Encoding for empty CSV field.
 */
static const char const EMPTY[] = ", 0.";

/**
 * If we're displaying in real-time using full screen control, we try to limit
 * our output lines to this many bytes.
 */
static const size_t LIMIT = 80 - (sizeof("OUT ") - 1) - (sizeof("[123] ") - 1) - (sizeof("\r\n") - 1) - 1;

/**
 * If we're just scrolling our output continuously, we don't limit the line
 * length.
 */
static const size_t UNLIMITED = ~(size_t)0;

/**
 * This is the Unicode for the degree symbol.
 */
static const wchar_t DEGREE = 0x00B0;

/**
 * This is the Unicode for the plus&minus symbol.
 */
static const wchar_t PLUSMINUS = 0x00B1;

#endif
