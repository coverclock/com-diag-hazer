/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_
#define _H_COM_DIAG_HAZER_

/**
 * @file
 *
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
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
 * Eric S. Raymond, "NMEA Revealed", 2.21, http://www.catb.org/gpsd/NMEA.html,
 * 2016-01
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
 * M. Grewal, et al., GLOBAL NAVIGATION SATELLITE SYSTEMS, INERTIAL NAVIGATION,
 * AND INTEGRATION, Wiley, 2013
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

/******************************************************************************
 * DEBUGGING
 ******************************************************************************/

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * hazer_debug(FILE *now);

/*******************************************************************************
 * STARTING UP AND SHUTTING DOWN
 ******************************************************************************/

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

/*******************************************************************************
 * COLLECTING AN NMEA OR UBLOX SENTENCE
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 *
 * SiRF NMEA, p. 2-2 has an example which appears to violate the
 * NMEA spec as to the length of the message ID.
 *
 * The USGlobalSat ND-105C routinely violates the NMEA spec as to
 * the maximum message length of 79 characters between the initial
 * '$' and the terminating \r\n by (so far) one character.
 *
 * The NaviSys GR-701W with the uBlox-7 chipset emits proprietary
 * PUBX messages longer than the NMEA spec.
 */
enum HazerConstantNmea {
    HAZER_CONSTANT_NMEA_SHORTEST    = sizeof("$ccccc*hh\r\n") - 1,
    HAZER_CONSTANT_NMEA_LONGEST     = 512, /* Adjusted. */
    HAZER_CONSTANT_NMEA_TALKER      = sizeof("GP") - 1,
    HAZER_CONSTANT_NMEA_MESSAGE     = sizeof("GGAXX") - 1, /* Adjusted. */
    HAZER_CONSTANT_NMEA_ID          = sizeof("$GPGGAXX") - 1, /* Adjusted. */
};

/**
 * NMEA 0183, 4.10, 5.3
 */
enum HazerConstantGps {
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
    HAZER_STATE_EOF					= 0,
    HAZER_STATE_START,
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
    HAZER_STIMULUS_HEXMIN_UC        = 'A',
    HAZER_STIMULUS_HEXMAX_UC        = 'F',
    HAZER_STIMULUS_HEXMIN_LC        = 'a',
    HAZER_STIMULUS_HEXMAX_LC        = 'f',
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
	HAZER_ACTION_FINAL,
} hazer_action_t;

/**
 * GNSS talkers.
 */
typedef enum HazerTalker {
    HAZER_TALKER_GPS				= 0,
    HAZER_TALKER_GLONASS,
    HAZER_TALKER_GALILEO,
    HAZER_TALKER_GNSS,
    HAZER_TALKER_RADIO,
	HAZER_TALKER_PUBX,
    HAZER_TALKER_TOTAL,
} hazer_talker_t;

/**
 * GNSS systems.
 *
 * N.B. Because the uBlox 7 receiver has only a single RF front-end, it cannot
 * track multiple systems (constellations) concurrently. Other inexpensive GPS
 * receivers I've used can routinely do this (but those receivers don't support
 * 1PPS like the uBlox 7-based GR-701W does).
 */
typedef enum HazerSystem {
    HAZER_SYSTEM_GPS				= 0,
    HAZER_SYSTEM_GLONASS,
    HAZER_SYSTEM_GALILEO,
    HAZER_SYSTEM_GNSS,
    HAZER_SYSTEM_TOTAL,
} hazer_system_t;

/**
 * Array of TALKER names indexed by talker enumeration.
 */
extern const char * HAZER_TALKER_NAME[/* hazer_talker_t */];

/**
 * Array of SYSTEM names indexed by system enumeration.
 */
extern const char * HAZER_SYSTEM_NAME[/* hazer_system_t */];

/**
 * This buffer is large enough to contain the largest NMEA sentence,
 * according to the NMEA spec, plus a trailing NUL (and then some).
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef unsigned char (hazer_buffer_t)[HAZER_CONSTANT_NMEA_LONGEST + 1]; /* plus NUL */

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
 * NMEA sentence resides in the buffer. The pointer state variable points
 * past the end of the NUL-terminated sentence, the size state variable
 * contrains the size of the sentence including the terminating NUL;
 * @param state is the prior state of the machine.
 * @param ch is the next character from the NMEA sentence stream.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param bp points to a character pointer state variable of no initial value.
 * @param sp points to a size state variable of no initial value.
 * @param lp points to the length state variable of no initial value.
 * @return the next state of the machine.
 */
extern hazer_state_t hazer_machine(hazer_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp, size_t * lp);

/*******************************************************************************
 * VALIDATING AN NMEA SENTENCE
 ******************************************************************************/

/**
 * Compute the eight-bit checksum of an NMEA sentence.
 * @param buffer points to the beginning of the sentence.
 * @param size is the size of the buffer in bytes.
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

/**
 * Return the length of the completed sentence in bytes.
 * @param buffer points to buffer containing the completed sentence.
 * @param size is the size of the buffer containing the sentence.
 * @return the length in bytes or <0 if an error occurred.
 */
extern ssize_t hazer_length(const void * buffer, size_t size);

/*******************************************************************************
 * BREAKING UP AN NMEA SENTENCE INTO FIELDS
 ******************************************************************************/

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

/**
 * Serialize an NMEA sentence by copying the fields from an argument vector
 * into a buffer. The buffer will be terminated with the '*' checksum field
 * prefix and a NUL character.
 * @param buffer points to the beginning of the sentence buffer.
 * @param size is the size of the buffer in bytes.
 * @param vector is an argument vector.
 * @param count is the size of the argument vector in array positions.
 * @return the number of bytes in the buffer including the final NUL character.
 */ 
extern ssize_t hazer_serialize(void * buffer, size_t size, char * vector[], size_t count);

/*******************************************************************************
 * PARSING INDIVIDUAL FIELDS IN AN NMEA SENTENCE
 ******************************************************************************/

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

/*******************************************************************************
 * IDENTIFYING SPECIFIC TALKERS
 ******************************************************************************/

/**
 * @def HAZER_NMEA_TALKER_GALILEO
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_TALKER_GALILEO "GA"

/**
 * @def HAZER_NMEA_TALKER_GLONASS
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_TALKER_GLONASS "GL"

/**
 * @def HAZER_NMEA_TALKER_GNSS
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_TALKER_GNSS "GN"

/**
 * @def HAZER_NMEA_TALKER_GPS
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_TALKER_GPS "GP"

/**
 * @def HAZER_NMEA_TALKER_RADIO
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_TALKER_RADIO "ZV"

/*******************************************************************************
 * IDENTIFYING SPECIFIC SENTENCES
 ******************************************************************************/

/**
 * @def HAZER_NMEA_GPS_MESSAGE_DTM
 * ublox7 Protocol Reference, p. vi, datum reference
 */
#define HAZER_NMEA_GPS_MESSAGE_DTM "DTM"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GBS
 * ublox7 Protocol Reference, p. vi, GNSS fault detection
 */
#define HAZER_NMEA_GPS_MESSAGE_GBS "GBS"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GGA
 * SiRF NMEA, Table 1-2, GPS fix data
 */
#define HAZER_NMEA_GPS_MESSAGE_GGA "GGA"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GLL
 * SiRF NMEA, Table 1-2, geographic position latitude/longitude
 */
#define HAZER_NMEA_GPS_MESSAGE_GLL "GLL"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GLQ
 * ublox7 Protocol Reference, p. vi, poll a standard message for talker GL
 */
#define HAZER_NMEA_GPS_MESSAGE_GLQ "GLQ"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GNQ
 * ublox7 Protocol Reference, p. vi, poll a standard message for talker GN
 */
#define HAZER_NMEA_GPS_MESSAGE_GNQ "GNQ"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GNS
 * ublox7 Protocol Reference, p. vi, GNSS fix data
 */
#define HAZER_NMEA_GPS_MESSAGE_GNS "GNS"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GPQ
 * ublox7 Protocol Reference, p. vi, poll a standard message for talker GP
 */
#define HAZER_NMEA_GPS_MESSAGE_GPQ "GPQ"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GRS
 * ublox7 Protocol Reference, p. vi, GNSS range residuals
 */
#define HAZER_NMEA_GPS_MESSAGE_GRS "GRS"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GSA
 * SiRF NMEA, Table 1-2, GPS DOP and active satellites
 */
#define HAZER_NMEA_GPS_MESSAGE_GSA "GSA"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GST
 * ublox7 Protocol Reference, p. vi, GNSS pseudo range error statistics
 */
#define HAZER_NMEA_GPS_MESSAGE_GRT "GST"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GSV
 * SiRF NMEA, Table 1-2, GPS satellites in view
 */
#define HAZER_NMEA_GPS_MESSAGE_GSV "GSV"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_MSS
 * SiRF NMEA, Table 1-2, beacon receiver status
 */
#define HAZER_NMEA_GPS_MESSAGE_MSS "MSS"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_RMC
 * SiRF NMEA, Table 1-2, recommended minimum navigation information message C
 */
#define HAZER_NMEA_GPS_MESSAGE_RMC "RMC"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_TXT
 * ublox7 Protocol Reference, p. vi, text
 */
#define HAZER_NMEA_GPS_MESSAGE_TXT "TXT"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_VTG
 * SiRF NMEA, Table 1-2, track made good and ground speed
 */
#define HAZER_NMEA_GPS_MESSAGE_VTG "VTG"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_ZDA
 * SiRF NMEA, Table 1-2, time & date
 */
#define HAZER_NMEA_GPS_MESSAGE_ZDA "ZDA"

/*******************************************************************************
 * IDENTIFYING PROPRIETARY SENTENCES
 ******************************************************************************/

/**
 * @def HAZER_NMEA_GPS_PROPRIETARY_PUBX
 * ublox7 Protocol Reference, p. vi, PUBX
 */
#define HAZER_PROPRIETARY_GPS_PUBX "PUBX"

/*******************************************************************************
 * DETERMINING TALKER
 ******************************************************************************/

/**
 * Determine if the talker is one in which we are interested. Return its
 * index if it is, <0 otherwise. We are only interested in certain talkers,
 * and even among those, only certain talkers are systems.
 * @param buffer points to the beginning or the first token of the sentence.
 * @return the index of the talker or TALKER TOTAL if N/A.
 */
extern hazer_talker_t hazer_parse_talker(const void * buffer);

/**
 * Return a system given a talker. Only some talkers are associated with
 * systems, or constellations, of satellites. Some talkers are devices other
 * than GNSS, some are specific to a particular GNSS receiver.
 * @param talker is talker index.
 * @return the index of the system or SYSTEM TOTAL if N/A.
 */
extern hazer_system_t hazer_parse_system(hazer_talker_t talker);

/*******************************************************************************
 * PARSING POSITION, HEADING, AND VELOCITY SENTENCES
 ******************************************************************************/

/**
 * THis structure maintains the time, position, altitude, speed, and bearing
 * derived from the NMEA stream. THIS OBJECT  SHOULD BE INITIALIZED TO ALL
 * ZEROS.
 */
typedef struct HazerPosition {
    uint64_t tot_nanoseconds;   /* Total nanoseconds. */
    uint64_t utc_nanoseconds;   /* Time in nanoseconds since 00:00 UTC. */
    uint64_t dmy_nanoseconds;   /* Date in nanoseconds since POSIX epoch. */
    int64_t lat_nanodegrees;    /* Latitude in nanodegrees. */
    int64_t lon_nanodegrees;    /* Longitude in nanodegrees. */
    int64_t alt_millimeters;    /* Altitude in millimeters. */
    int64_t sog_microknots;     /* Speed On Ground in microknots. */
    int64_t cog_nanodegrees;    /* Course On Ground in nanodegrees. */
    uint8_t sat_used;           /* Number of satellites used. */
    uint8_t lat_digits;         /* Significant digiits of latitude. */
    uint8_t lon_digits;         /* Signficant digits of longitute. */
    uint8_t alt_digits;         /* Significant digits of altitude. */
    uint8_t sog_digits;         /* Signficant digits of Speed On Ground. */
    uint8_t cog_digits;         /* Signficant digits of Course On Ground. */
    uint8_t unused[2];          /* Unused. */
} hazer_position_t;

/**
 * Parse a GGA NMEA sentence, updating the position.
 * @param datap points to the position structure (initialized to zeros).
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gga(hazer_position_t *datap, char * vector[], size_t count);

/**
 * Parse a RMC NMEA sentence, updating the position.
 * @param datap points to the position structure (initialized to zeros).
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_rmc(hazer_position_t *datap, char * vector[], size_t count);

/*******************************************************************************
 * PARSING SATELLITE ELEVATION, AZIMUTH, AND SIGNAL STRENGTH SENTENCES
 ******************************************************************************/

/**
 * This structure maintains the information on the satellites in any
 * constellation that were used in the position solution. THIS OBJECT
 * SHOULD BE INITIALIZED TO ALL ZEROS.
 */
typedef struct HazerSolution {
    double pdop;                /* Position Dilution Of Precision. */
    double hdop;                /* Horizontal Dilution Of Precisioin. */
    double vdop;                /* Vertical Diilution Of Precisioin. */
    uint8_t active;             /* Number of satellites active. */
    uint8_t id[HAZER_CONSTANT_GPS_SATELLITES];  /* Satellite IDentifiers. */
    uint8_t unused[3];          /* Unused. */
} hazer_solution_t;

/**
 * Parse a GSA NMEA sentence, updating the constellation.
 * @param datap points to the solution structure (initialized to zeros).
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gsa(hazer_solution_t * datap, char * vector[], size_t count);

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
 * have channels configured. THIS OBJECT SHOULD BE INITIALIZED TO ALL ZEROS.
 */
typedef struct HazerConstellation {
    hazer_satellite_t sat[HAZER_CONSTANT_GPS_CHANNELS]; /* Satellites viewed. */
    uint8_t view;               /* Number of satellites in view. */
    uint8_t channels;           /* Number of channels used in view. */
    uint8_t unused[6];          /* Unused. */
} hazer_constellation_t;

/**
 * Parse a GSV NMEA sentence, updating the constellation.
 * @param datap points to the constellation structure (initialized to zeros).
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success on final update of group, 1 for success, <0 otherwise.
 */
extern int hazer_parse_gsv(hazer_constellation_t * datap, char * vector[], size_t count);

/*******************************************************************************
 * FORMATTING DATA FOR OUTPUT
 ******************************************************************************/

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
extern const char * hazer_format_nanodegrees2compass32(int64_t nanodegrees);

/**
 * Format nanodegrees of compass bearing in a pointer to a name of a
 * compass point on an eight point compass.
 * @param nanodegrees is a bearing or heading in compass nanodegrees.
 * @return a compass point string in upper case.
 */
extern const char * hazer_format_nanodegrees2compass8(int64_t nanodegrees);

#endif
