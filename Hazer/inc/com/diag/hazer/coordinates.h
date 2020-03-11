/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_COORDINATES_
#define _H_COM_DIAG_HAZER_COORDINATES_

/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA.
 * Licensed under the terms in LICENSE.txt.
 *
 * The Coordinates Feature provides some common facilities for parsing and
 * displaying latitude and longitude coordinates in a variety of useful
 * formats.
 */

/*
 * POS 39*47'39.216"N, 105*09'12.106"W    39.7942268, -105.1533628        GNSS
 * HPP   39.794226865, -105.153362915 ~     1.1993m                       GNSS
 * NGS  39 47 39.21671(N) 105 09 12.10649(W)                              GNSS
 *
 * N.B. Non-ASCII characters replaced to make IDE happy.
 */

/*
 * N.B. These are preprocessor symbols and not static const variables for a
 * reason.
 */

/**
 * @def COORDINATES_SCANF_HPP
 * Each number in the HPP format looks like this to sscanf(3).
 */
#define COORDINATES_SCANF_HPP "%lf"

/**
 * @def COORDINATES_SCANF_NGS
 * Each number in the NGS format looks like this to sscanf(3).
 */
#define COORDINATES_SCANF_NGS "%u %u %lf(%c)"

/**
 * @def COORDINATES_SCANF_POS
 * Each number in the POS format looks like this to sscanf(3).
 */
#define COORDINATES_SCANF_POS "%u\u00B0%u'%lf\"%c"

/**
 * These are the coordinate formats that are supported. An "invalid" value
 * is one which seemed to be a valid format, but the actual data was not
 * valid.
 */
typedef enum CoordinatesFormat {
    COORDINATES_FORMAT_INVALID					= -1,
    COORDINATES_FORMAT_UNSUPPORTED				= 0,
    COORDINATES_FORMAT_HPP_PREFIX_SEPERATOR		= 1,
    COORDINATES_FORMAT_HPP_SEPERATOR			= 2,
    COORDINATES_FORMAT_HPP						= 3,
    COORDINATES_FORMAT_POS_PREFIX_SEPERATOR		= 4,
    COORDINATES_FORMAT_POS_SEPERATOR			= 5,
    COORDINATES_FORMAT_POS						= 6,
    COORDINATES_FORMAT_NGS_PREFIX				= 7,
    COORDINATES_FORMAT_NGS						= 8,
} coordinates_format_t;

/**
 * Enable or disable debug output from the coordinates function(s).
 * @param now is the new debug value (true or false).
 * @return the old debug value (true or false);
 */
extern int coordinates_debug(int now);

/**
 * Parse a character string containing any one of a number of supported
 * latitude and longitude coordinate formats and convert it into binary
 * values in double precision floating point degrees. N.B.: this requires
 * that the locale be set, for example via setlocale(LC_ALL, "").
 * @param string points to the input string.
 * @param latitudep points to where the latitude result (if any) is stored.
 * @param longitudep points to where the longitude result (if any) is stored.
 * @return a value indicating what format the input data was in.
 */
extern int coordinates_parse(const char * string, double * latitudep, double * longitudep);

/**
 * Format decimal degrees of latitude or longitude into position values.
 * @param decimaldegrees is a longitude or latitude in floating point.
 * @param degreesp points to where the integral degrees (e.g. 180) is stored.
 * @param minutesp points to where the minutes (0..59) are stored.
 * @param secondsp points to where the seconds (0..59) are stored.
 * @param millionthsp points to there the fractional seconds (0..999999) are stored.
 * @param directionp points to where 1 (N or E) or -1 (S or W) is stored.
 */
extern void coordinates_format_decimaldegrees2position(double decimaldegrees, int * degreesp, int * minutesp, int * secondsp, int * millionthsp, int * directionp);

#endif
