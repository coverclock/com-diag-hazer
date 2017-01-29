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
 * Association (NMEA) strings produced by the USGlobalSat BU-353S4 Global
 * Positioning System (GPS) device, a tiny little external GPS receiver that
 * emits NMEA strings over its built-in USB-to-serial adaptor. The BU-353S4
 * is based on the SiRF Star IV chipset. If you want to futz around with
 * satellite geolocation, the BU-353S4 is a cheap and easy way to do it.
 * Hazer may be useful with other GPS devices that produce NMEA sentences,
 * but the BU-353S4 is what it was tested with.
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
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 * NMEA 0183 4.10, 5.3
 * N.B. SiRF NMEA, p. 2-2 has an example which appears to violate the
 * NMEA spec as to the length of the message ID.
 */
enum HazerConstant {
    HAZER_NMEA_CONSTANT_SHORTEST    = sizeof("$ccccc*hh\r\n") - 1,
    HAZER_NMEA_CONSTANT_LONGEST     = 82,
    HAZER_NMEA_CONSTANT_TALKER      = sizeof("GP") - 1,
    HAZER_NMEA_CONSTANT_MESSAGE     = sizeof("GGA") - 1,
    HAZER_NMEA_CONSTANT_ID          = sizeof("$GPGGA") - 1,
};

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * hazer_debug(FILE *now);

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
    HAZER_STATE_TALKER_1,
    HAZER_STATE_TALKER_2,
    HAZER_STATE_MESSAGE_1,
    HAZER_STATE_MESSAGE_2,
    HAZER_STATE_MESSAGE_3,
    HAZER_STATE_DELIMITER,
    HAZER_STATE_CHECKSUM,
    HAZER_STATE_CHECKSUM_1,
    HAZER_STATE_CHECKSUM_2,
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
typedef char (hazer_buffer_t)[HAZER_NMEA_CONSTANT_LONGEST + 1]; /* plus NUL */

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
 * Compute the checksum of an NMEA sentence. If the first character is the
 * start character, it is skipped. The message is checked for legal characters.
 * The computation stops when the checksum character is encountered.
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

/**
 * THis is an argument vector big enough to hold all possible sentences no
 * larger than those that can fit in the buffer type, plus a NULL pointer in
 * the last position.
 */
typedef char * (hazer_vector_t)[HAZER_NMEA_CONSTANT_LONGEST - HAZER_NMEA_CONSTANT_SHORTEST + 1]; /* plus NULL */

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

extern uint64_t hazer_parse_fraction(const char * string, uint64_t * denominator);

extern uint64_t hazer_parse_utc(const char * string);

extern double hazer_parse_latlon(const char * string, char direction);

typedef struct HazerPosition {
    uint64_t utc_nanoseconds;
    double lat_degrees;
    double lon_degrees;
} hazer_position_t;

#endif
