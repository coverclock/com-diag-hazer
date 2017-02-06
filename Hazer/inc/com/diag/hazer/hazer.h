/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_
#define _H_COM_DIAG_HAZER_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 * Hazer is a simple C-based parser of the National Marine Electronics
 * Association (NMEA) strings produced by typical consumer GPS devices.
 *
 * This code deliberately tries to avoid using floating poing arithmetic.
 * Some of the smaller embedded platforms I work on don't have floating
 * point hardware, relying instead on library-based software emulation
 * with a significant performance impact. Also, most of the time it just
 * isn't necessary. If the calling application wants to use floating point,
 * I'm okay with that.
 *
 * REFERENCES
 *
 * "NMEA 0183 Standard for Interfacing Marine Electronic Devices", version 4.10,
 * NMEA 0183, National Marine Electronics Association, 2012-06
 *
 * "BU-353S4 GPS Receiver Data Sheet", BU353S4-DS08212013B, USGLobalSat Inc.,
 * 2013
 *
 * "NMEA Reference Manual", Revision 2.2, 1050-0042, SiRF Technology, Inc.,
 * 2008-11
 *
 * "SiRF Binary Protocol Reference Manual", revision 2.4, 1040-0041, SiRF
 * Technology, Inc., 2008-11
 *
 * "GP-2106 SiRF Star IV GPS module with antenna", version 0.2, ADH Technology
 * Co. Ltd., 2010-12-08
 *
 * Electronic Doberman, "Modern GPS Teardown - GlobalSat BU-353S4 SiRF Star
 * IV USB GPS", https://www.youtube.com/watch?v=8xn8FspJDnY
 *
 * E. Kaplan, ed., UNDERSTANDING GPS PRINCIPLES AND APPLICATIONS, Artech House,
 * 1996
 *
 * "Geographic coordinate system", Wikipedia,
 * https://en.wikipedia.org/wiki/Geographic_coordinate_system, 2017-01-24
 *
 * "Decimal degrees", Wikipedia,
 * https://en.wikipedia.org/wiki/Decimal_degrees, 2016-11-04
 *
 * "Points of the compass", Wikipedia,
 * https://en.wikipedia.org/wiki/Points_of_the_compass, 2017-01-17
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/*********************************************************************************
 * DEBUGGING
 ********************************************************************************/

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * hazer_debug(FILE *now);

/*********************************************************************************
 * STARTING UP AND SHUTTING DOWN
 ********************************************************************************/

/**
 * Perform any necessary initialization.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_initialize(void);

/**
 * Perform any necessary finalization.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_finalize(void);

/*********************************************************************************
 * COLLECTING AN NMEA SENTENCE
 ********************************************************************************/

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 * NMEA 0183 4.10, 5.3
 *
 * SiRF NMEA, p. 2-2 has an example which appears to violate the
 * NMEA spec as to the length of the message ID.
 *
 * The USGlobalSat ND-105C routinely violates the NMEA spec as to
 * the maximum message length of 79 characters between the initial
 * '$' and the terminating \r\n by (so far) one character.
 */
enum HazerConstant {
    HAZER_CONSTANT_NMEA_SHORTEST    = sizeof("$ccccc*hh\r\n") - 1,
    HAZER_CONSTANT_NMEA_LONGEST     = 83, /* Adjusted. */
    HAZER_CONSTANT_NMEA_TALKER      = sizeof("GP") - 1,
    HAZER_CONSTANT_NMEA_MESSAGE     = sizeof("GGAXX") - 1, /* Adjusted. */
    HAZER_CONSTANT_NMEA_ID          = sizeof("$GPGGAXX") - 1, /* Adjusted. */
    HAZER_CONSTANT_GPS_CHANNELS     = 48,
    HAZER_CONSTANT_GPS_VIEWS        = 4,
    HAZER_CONSTANT_GPS_SATELLITES   = 12,
};

/**
 * NMEA state machine states. The only states the application needs
 * to take action on are START (to initialize the state), EOF (end of file
 * on the input stream), and END (complete NMEA sentence in buffer). The
 * rest are transitory states. If the machine transitions from a non_START
 * state to the START state, that means the framing of the current sentence
 * failed; that might be of interest to the application.
 */
typedef enum HazerState {
    HAZER_STATE_EOF                 = -1,
    HAZER_STATE_START               = 0,
    HAZER_STATE_BODY,
    HAZER_STATE_MSN,
    HAZER_STATE_LSN,
    HAZER_STATE_CR,
    HAZER_STATE_LF,
    HAZER_STATE_END,
} hazer_state_t;

/**
 * NMEA state machine stimuli. This is just the special characters that
 * the state machine must take different action on, not all possible
 * characters that may be in an NMEA sentence.
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
enum HazerStimulus {
    HAZER_STIMULUS_NUL              = '\0',
    HAZER_STIMULUS_MINIMUM          = ' ',
    HAZER_STIMULUS_ENCAPSULATION    = '!',
    HAZER_STIMULUS_START            = '$',
    HAZER_STIMULUS_DELIMITER        = ',',
    HAZER_STIMULUS_TAG              = '\\',
    HAZER_STIMULUS_HEXADECIMAL      = '^',
    HAZER_STIMULUS_DECIMAL          = '.',
    HAZER_STIMULUS_CHECKSUM         = '*',
    HAZER_STIMULUS_DECMIN           = '0',
    HAZER_STIMULUS_DECMAX           = '9',
    HAZER_STIMULUS_HEXMIN           = 'A',
    HAZER_STIMULUS_HEXMAX           = 'F',
    HAZER_STIMULUS_GNSS             = 'G',
    HAZER_STIMULUS_EAST             = 'E',
    HAZER_STIMULUS_WEST             = 'W',
    HAZER_STIMULUS_NORTH            = 'N',
    HAZER_STIMULUS_SOUTH            = 'S',
    HAZER_STIMULUS_CR               = '\r',
    HAZER_STIMULUS_LF               = '\n',
    HAZER_STIMULUS_MAXIMUM          = '}',
    HAZER_STIMULUS_RESERVED         = '~',
};

/**
 * NMEA state machine actions.
 */
typedef enum HazerAction {
    HAZER_ACTION_SKIP               = 0,
    HAZER_ACTION_SAVE,
    HAZER_ACTION_SAVESPECIAL,
    HAZER_ACTION_TERMINATE,
} hazer_action_t;

/**
 * This buffer is large enough to contain the largest NMEA sentence,
 * according to the NMEA spec, plus a trailing NUL.
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef char (hazer_buffer_t)[HAZER_CONSTANT_NMEA_LONGEST + 1]; /* plus NUL */

/**
 * Process a single character of stimulus for the state machine that is
 * assembling a single NMEA sentence in the caller provided buffer. State
 * is maintained in a character pointer and a size variable, pointers to
 * which are passed to the function, which will be initialized by the function
 * itself. The function returns the new state, which must be used in the
 * subsequent call. The initial state should be the START state. Of interest
 * to the application are the EOF and END staes. The EOF state indicates
 * that the input stream has ended, detected by virtue of the stimulus character
 * being equal to the standard I/O EOF. The END state indicates that a complete
 * NMEA sentence resides in the buffer; the pointer state variable points
 * past the end of the NUL-terminated sentence, and the size state variable
 * contrains the size of the sentence including the terminating NUL.
 * @param state is the prior state of the machine.
 * @param ch is the next character from the NMEA sentence stream.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param bp points to a character pointer state variable.
 * @param sp points to a size state variable.
 * @return the next state of the machine.
 */
extern hazer_state_t hazer_machine(hazer_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp);

/**
 * Compute the checksum of an NMEA sentence.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @return the checksum.
 */
extern uint8_t hazer_checksum(const void * buffer, size_t size);

/**
 * Given two checksum characters, convert to an eight-bit checksum.
 * @param msn is the character representing the most significant nibble.
 * @param lsn is the character representing the least significant nibble.
 * @param ckp points to a variable in which the checksum is stored.
 * @return 0 for success, <0 if an error occurred.
 */
extern int hazer_characters2checksum(char msn, char lsn, uint8_t * ckp);

/**
 * Given an eight-bit checksum, concert into the two checksum characters.
 * @param ck is the checksum.
 * @param mnsp points where the most significant character is stored.
 * @param lnsp points where the least significant character is stored.
 * @return 0 for success, <0 if an error occurred.
 */
extern int hazer_checksum2characters(uint8_t ck, char * msnp, char * lsnp);

/*********************************************************************************
 * BREAKING UP AN NMEA SENTENCE INTO FIELDS
 ********************************************************************************/

/**
 * THis is an argument vector big enough to hold all possible sentences no
 * larger than those that can fit in the buffer type, plus a NULL pointer in
 * the last position.
 */
typedef char * (hazer_vector_t)[HAZER_CONSTANT_NMEA_LONGEST - HAZER_CONSTANT_NMEA_SHORTEST + 1]; /* plus NULL */

/**
 * Tokenize an NMEA sentence by splitting it into substrings whose pointers
 * are placed in an argument vector. This is a descructive operation on the
 * sentence since delimiers are replaced with NULs to terminate the substrings.
 * A null pointer terminates the argument vector.
 * @param vector is an argument vector in which the substring pointers are stored.
 * @param count is the size of the argument vector in array positions.
 * @param buffer points to the beginning of the sentence buffer.
 * @param size is the size of the sentence in bytes.
 * @return the number of arguments including the final null pointer.
 */
extern ssize_t hazer_tokenize(char * vector[], size_t count, void * buffer, size_t size);

/*********************************************************************************
 * PARSING INDIVIDUAL FIELDS IN AN NMEA SENTENCE
 ********************************************************************************/

/**
 * Parse a string containing an integer representing the fractional portion of
 * a floating point value into a numerator representing the magnitude and a
 * denominator that will be a power of ten. This is done to defer the floating
 * point conversion.
 * @param string points to the string (just past the decimal point).
 * @param denominatorp points to where the denominator is stored.
 * @return the numerator.
 */
extern uint64_t hazer_parse_fraction(const char * string, uint64_t * denominatorp);

/**
 * Parse a string containing the time in UTC in NMEA format into an integer
 * number of nanoseconds since the start of the day.
 * @param string points to the string.
 * @return an integer number of nanoseconds.
 */
extern uint64_t hazer_parse_utc(const char * string);

/**
 * Parse a string containing the date in NMEA format into an integer
 * number of nanoseconds since the start of the POSIX epoch.
 * @param string points to the string.
 * @return an integer number of microseconds.
 */
extern uint64_t hazer_parse_dmy(const char * string);

/**
 * Parse a string containing the latitude or longitude in NMEA format into
 * a signed integer number of nanodegrees.
 * @param string points to the string.
 * @param direction is the NMEA direction: 'N', 'S', 'E', or 'W'.
 * @param digitsp points to where the number of digits is stored.
 * @return nanodegrees.
 */
extern int64_t hazer_parse_latlon(const char * string, char direction, uint8_t * digitsp);

/**
 * Parse a string containing a heading (bearing) in degrees in NMEA format
 * into a signed integer number of nanodegrees.
 * @param string points to the string.
 * @param digitsp points to where the number of digits is stored.
 * @return nanodegrees.
 */
extern int64_t hazer_parse_cog(const char * string, uint8_t * digitsp);

/**
 * Parse a string containing a speed in knots in NMEA format into a
 * signed integer number of microknots.
 * @param string points to the string.
 * @param digitsp points to where the number of digits is stored.
 * @return microknots.
 */
extern int64_t hazer_parse_sog(const char * string, uint8_t * digitsp);

/**
 * Parse a decimal number representing altitude above Mean Sea Level (MSL)
 * into integer millimeters. (Currently the units field is ignored and the
 * units are assumed to be meters.)
 * @param string points to the string.
 * @param units is the units ('M' for meters).
 * @param digitsp points to where the number of digits is stored.
 * @return millimeters.
 */
extern int64_t hazer_parse_alt(const char * string, char units, uint8_t * digitsp);

/**
 * Parse any decimal number with or without a fractional part into a
 * double precision floating point value.
 * @param string points the string.
 * @return a double precision floating point value.
 */
extern double hazer_parse_num(const char * string);

/*********************************************************************************
 * PARSING POSITION, HEADING, AND VELOCITY SENTENCES
 ********************************************************************************/

/**
 * THis structure maintains the time, position, altitude, speed, and bearing
 * derived from the NMEA stream.
 */
typedef struct HazerPosition {
    uint64_t utc_nanoseconds;   /* Time in nanoseconds since 00:00 UTC. */
    uint64_t dmy_nanoseconds;   /* Date in nanoseconds since POSIX epoch. */
    int64_t lat_nanodegrees;    /* Latitude in nanodegrees. */
    int64_t lon_nanodegrees;    /* Longitude in nanodegrees. */
    int64_t alt_millimeters;    /* Altitude in millimeters. */
    int64_t sog_microknots;     /* Speed On Ground in microknots. */
    int64_t cog_nanodegrees;    /* Course On Ground in nanodegrees. */
    uint8_t lat_digits;         /* Significant digiits of latitude. */
    uint8_t lon_digits;         /* Signficant digits of longitute. */
    uint8_t alt_digits;         /* Significant digits of altitude. */
    uint8_t sog_digits;         /* Signficant digits of Speed On Ground. */
    uint8_t cog_digits;         /* Signficant digits of Course On Ground. */
    uint8_t unused[3];          /* Unused. */
} hazer_position_t;

/**
 * Parse a GGA NMEA sentence, updating the position.
 * @param datap points to the position structure.
 * @Param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gga(hazer_position_t *datap, char * vector[], size_t count);

/**
 * Parse a RMC NMEA sentence, updating the position.
 * @param datap points to the position structure.
 * @Param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_rmc(hazer_position_t *datap, char * vector[], size_t count);

/*********************************************************************************
 * PARSING SATELLITE ELEVATION, AZIMUTH, AND SIGNAL STRENGTH SENTENCES
 ********************************************************************************/

/**
 * This structure maintains the elevation, azimuth, and signal strength of a
 * single satellite.
 */
typedef struct HazerSatellite {
    uint16_t elv_degrees;       /* Elevation in whole dregrees. */
    uint16_t azm_degrees;       /* Azimuth in whole degrees. */
    uint8_t id;                 /* Satellite IDentifier. */
    uint8_t snr_dbhz;           /* Signal/Noise Ratio in dBHz. */
    uint8_t unused[2];          /* Unused. */
} hazer_satellite_t;

/**
 * This structure maintains the information on as many satellites as we
 * have channels configured.
 */
typedef struct HazerConstellation {
    double pdop;                /* Position Dilution Of Precision. */
    double hdop;                /* Horizontal Dilution Of Precisioin. */
    double vdop;                /* Vertical Diilution Of Precisioin. */
    uint8_t satellites;         /* Number of satellites used in soluton. */
    uint8_t id[HAZER_CONSTANT_GPS_SATELLITES];  /* Satellite IDentifiers. */
    uint8_t channels;           /* Number of channels used in view. */
    uint8_t unused[2];          /* Unused. */
    hazer_satellite_t sat[HAZER_CONSTANT_GPS_CHANNELS]; /* Satellites viewed. */
} hazer_constellation_t;

/**
 * Parse a GSV NMEA sentence, updating the constellation.
 * @param datap points to the constellation structure.
 * @Param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success on final update of group, 1 for success, <0 otherwise.
 */
extern int hazer_parse_gsv(hazer_constellation_t * datap, char * vector[], size_t count);

/**
 * Parse a GSA NMEA sentence, updating the constellation.
 * @param datap points to the constellation structure.
 * @Param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gsa(hazer_constellation_t * datap, char * vector[], size_t count);

/*********************************************************************************
 * FORMATTING DATA FOR OUTPUT
 ********************************************************************************/

/**
 * Format nanoseconds (the sum of the UTC and DMY fields) in to separate values.
 * @param nanoseconds is the total nanoseconds since the POSIX epoch.
 * @param yearp points to where the year (e.g. 2017) is stored.
 * @param monthp points to where the month (1..12) is stored.
 * @param dayp points to where the day of the month (1..31) is stored.
 * @param hourp points to where the hour (0..23) is stored.
 * @param minutep points to where the minute (0..59) is stored.
 * @param secondp points to where the second (0..59) is stored.
 * @param nanosecondsp points to where the fractional nanoseconds is stored.
 */
extern void hazer_format_nanoseconds2timestamp(uint64_t nanoseconds, int * yearp, int * monthp, int * dayp, int * hourp, int * minutep, int * secondp, uint64_t * nanosecondsp);

/**
 * Format nanodegrees of latitude or longitude into separate values.
 * @param nanodegrees is a longitude or latitude in nanodegrees.
 * @param degreesp points to where the integral degrees (e.g. 180) is stored.
 * @param minutesp points to where the minutes (0..59) are stored.
 * @param secondsp points to where the seconds (0..59) are stored.
 * @param hundredsthp points to there the fractional seconds (0..99) are stored.
 * @param direction points to where 1 (N or E) or -1 (S or W) is stored.
 */
extern void hazer_format_nanodegrees2position(int64_t nanodegrees, int * degreesp, int * minutesp, int * secondsp, int * hundredsthp, int * directionp);

/**
 * Format nanodegrees of compass bearing in a pointer to a name of a
 * compass point on a thirty-two point compass.
 * @param nanodegrees is a bearing or heading in compass nanodegrees.
 * @return a compass point string in upper case.
 */
extern const char * hazer_format_nanodegrees2compass(int64_t nanodegrees);

#endif
