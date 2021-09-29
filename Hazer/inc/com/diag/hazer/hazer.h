/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_HAZER_
#define _H_COM_DIAG_HAZER_HAZER_

/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Parse common NMEA strings from GNSS devices.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * THe Hazer module is a simple C-based parser of the National Marine
 * Electronics Association (NMEA) strings produced by typical consumer
 * GNSS devices.
 *
 * The Hazer repository also contains APIs for dealing with messages
 * in formats other than NMEA that are emitted by common GNSS devides.
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
 * "u-blox 7 Receiver Description Including Protocol Specification V14",
 * GPS.G7-SW-12001-B, ublox AG, 2013
 *
 * "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification",
 * UBX-13003221-R15, ublox AG, 2018-03-06
 *
 * "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification",
 * UBX-13003221-R24, ublox AG, 2021-06-22
 *
 * "u-blox ZED-F9P Interface Description*, UBX-18010854-R05, ublox AG,
 * 2018-12-20
 *
 * Eric S. Raymond, "NMEA Revealed", 2.21, http://www.catb.org/gpsd/NMEA.html,
 * 2016-01
 *
 * Richard B. Langley, "Dilution of Precision", GPS World, 1999-05, p. 52-59
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
 *
 * "Dilution of Precision", Wikipedia,
 * https://en.wikipedia.org/wiki/Dilution_of_precision_(navigation), 2018-08-03
 *
 * Gtop, "PMTK command reference", GlobalTop Tech Inc., 2012
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*******************************************************************************
 * TYPES
 ******************************************************************************/

typedef uint8_t hazer_expiry_t;

/******************************************************************************
 * DEBUGGING
 ******************************************************************************/

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * hazer_debug(FILE * now);

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
 * NMEA 0183, 4.10, 5.3
 */
enum HazerGnssConstants {
    HAZER_GNSS_SATELLITES   = 32,	/* Per constellation or system. */
    HAZER_GNSS_VIEWS        = 4,	/* Per NMEA GSV message. */
    HAZER_GNSS_ACTIVES		= 12,	/* Per NMEA GSA message. */
    HAZER_GNSS_SECONDS		= 255,	/* Maximum lifetime. */
    HAZER_GNSS_DOP			= 9999,	/* Maximum DOP in units * 100 */
};

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
 *
 * U-blox devices with PUBX,03 (SVSTATUS) enabled can produce a
 * sentence that is at least 474 characters with 148 fields.
 */
enum HazerNmeaConstants {
    HAZER_NMEA_SHORTEST    = sizeof("$GGAXX\r\n") - 1,
    HAZER_NMEA_LONGEST     = 512, /* Longer than spec. */
    HAZER_NMEA_TALKER      = sizeof("GP") - 1,
    HAZER_NMEA_MESSAGE     = sizeof("GGAXX") - 1, /* Adjusted. */
    HAZER_NMEA_ID          = sizeof("$GPGGAXX") - 1, /* Adjusted. */
};

/**
 * NMEA state machine states. The only state the application needs
 * to take action on is END (complete NMEA sentence in buffer). The
 * rest are transitory states. If the machine transitions from a non-START
 * state to the START state, that means the framing of the current sentence
 * failed; that might be of interest to the application.
 */
typedef enum HazerState {
    HAZER_STATE_STOP		= 'X',
    HAZER_STATE_START		= 'S',
    HAZER_STATE_BODY		= 'P',
    HAZER_STATE_MSN			= 'M',
    HAZER_STATE_LSN			= 'L',
    HAZER_STATE_CR			= 'R',
    HAZER_STATE_LF			= 'N',
    HAZER_STATE_END			= 'E',
} hazer_state_t;

/**
 * NMEA state machine stimuli. This is just the special characters that
 * the state machine must take different action on, not all possible
 * characters that may be in an NMEA sentence.
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
enum HazerStimulus {
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
    HAZER_ACTION_SKIP               = 'X',
    HAZER_ACTION_SAVE				= 'S',
    HAZER_ACTION_TERMINATE			= 'T',
} hazer_action_t;

/**
 * GNSS talkers.
 * These must be in the same order as the corresponding strings below.
 */
typedef enum HazerTalker {
    HAZER_TALKER_BEIDOU2			= 0,
    HAZER_TALKER_DSC,
    HAZER_TALKER_ECDIS,
    HAZER_TALKER_GALILEO,
    HAZER_TALKER_BEIDOU1,
    HAZER_TALKER_GLONASS,
    HAZER_TALKER_GNSS,
    HAZER_TALKER_GPS,
    HAZER_TALKER_INSTRUMENTATION,
    HAZER_TALKER_NAVIGATION,
    HAZER_TALKER_LORANC,
    HAZER_TALKER_PMTK,
    HAZER_TALKER_PSRF,
    HAZER_TALKER_PUBX,
    HAZER_TALKER_QZSS,
    HAZER_TALKER_RADIO,
    HAZER_TALKER_TOTAL,
} hazer_talker_t;

/**
 * @def HAZER_TALKER_NAME_INITIALIZER
 * Initialize the array of character strings that map from a Hazer talker
 * enumerated value to the printable name of the talker. These strings must
 * be in collating sequence order.
 */
#define HAZER_TALKER_NAME_INITIALIZER \
{ \
    "BD", \
    "CD", \
    "EC", \
    "GA", \
    "GB", \
    "GL", \
    "GN", \
    "GP", \
    "II", \
    "IN", \
    "LC", \
    "PMTK", \
    "PSRF", \
    "PUBX", \
    "QZ", \
    "ZV", \
    (const char *)0, \
}

/**
 * Array of TALKER names indexed by talker enumeration.
 */
extern const char * HAZER_TALKER_NAME[/* hazer_talker_t */];

/**
 * Internal GNSS system identifiers.
 * These must be in the same order as the corresponding strings below.
 */
typedef enum HazerSystem {
    HAZER_SYSTEM_GNSS		    = 0,
    HAZER_SYSTEM_GPS,
    HAZER_SYSTEM_GLONASS,
    HAZER_SYSTEM_GALILEO,
    HAZER_SYSTEM_BEIDOU,
    HAZER_SYSTEM_SBAS,
    HAZER_SYSTEM_IMES,
    HAZER_SYSTEM_QZSS,
    HAZER_SYSTEM_TOTAL,
} hazer_system_t;

/**
 * @def HAZER_SYSTEM_NAME_INITIALIZER
 * Initialize the array of character strings that map from a Hazer system
 * enumerated value to the printable name of the system. These strings should
 * be in order of preference for systems having (unlikely as it might be)
 * exactly the same dilution of precision (DOP). For example, you might prefer
 * GLONASS over GPS, or GPS over GNSS (which represents a solution using
 * multiple systems, which can be problematic).
 */
#define HAZER_SYSTEM_NAME_INITIALIZER \
    { \
        "GNSS", \
        "NAVSTAR", \
        "GLONASS", \
        "GALILEO", \
        "COMPASS", \
        "SBAS", \
        "IMES", \
        "QZSS", \
        (const char *)0, \
    }

/**
 * Array of SYSTEM names indexed by system enumeration.
 */
extern const char * HAZER_SYSTEM_NAME[/* hazer_system_t */];

/**
 * NMEA GNSS system identifiers.
 * NMEA 0183 4.10 table 20 p. 94-95.
 */
typedef enum HazerNmea {
    HAZER_NMEA_GPS              = 1,
    HAZER_NMEA_GLONASS          = 2,
    HAZER_NMEA_GALILEO          = 3,
    HAZER_NMEA_BEIDOU           = 4,
    HAZER_NMEA_SBAS             = 5,
    HAZER_NMEA_IMES             = 6,
    HAZER_NMEA_QZSS             = 15,
} hazer_nmea_t;

/**
 * Map the NMEA GNSS system id to the internal system id.
 * @param constellation is the NMEA system identifier.
 * @return an index of the system or SYSTEM TOTAL if N/A.
 */
extern hazer_system_t hazer_map_nmea_to_system(uint8_t constellation);

/**
 * GNSS satellite identifiers.
 * NMEA 0183 4.10 p. 94.
 * UBLOX8 R15 p. 373.
 * UBLOX8 R19 Appendix A p. 402.
 * There are some conflicts between these documents, and my most recent
 * receiver, the U-blox 9, doesn't match these anyway. Despite the
 * documentation, I don't consider these reliable.
 */
typedef enum HazerNmeaId {
    /*                        0,     */
    HAZER_NMEA_GPS_FIRST	= 1,
    HAZER_NMEA_GPS_LAST		= 32,
    HAZER_NMEA_SBAS_FIRST	= 33,
    HAZER_NMEA_SBAS_LAST	= 64,
    HAZER_NMEA_GLONASS_FIRST= 65,
    HAZER_NMEA_GLONASS_LAST	= 96,
    /*						  97,    */
    /*						   :     */
    /*						  151,   */
    HAZER_NMEA_SBASX_FIRST	= 152,
    HAZER_NMEA_SBASX_LAST	= 158,
    /*						  159,   */
    /*						   :     */
    /*						  172,   */
    HAZER_NMEA_IMES_FIRST	= 173,
    HAZER_NMEA_IMES_LAST	= 182,
    /*						  183,   */
    /*						   :     */
    /*						  192,   */
    HAZER_NMEA_QZSS_FIRST	= 193,
    HAZER_NMEA_QZSS_LAST	= 197,
    /*						  198,   */
    /*						   :     */
    /*						  200,   */
    HAZER_NMEA_BEIDOU1_FIRST= 201,
    HAZER_NMEA_BEIDOU1_LAST	= 235,
    /*						  236,   */
    /*						   :     */
    /*						  300,   */
    HAZER_NMEA_GALILEO_FIRST= 301,
    HAZER_NMEA_GALILEO_LAST	= 336,
    /*						  337,   */
    /*						   :     */
    /*						  400,   */
    HAZER_NMEA_BEIDOU2_FIRST= 401,
    HAZER_NMEA_BEIDOU2_LAST	= 437,
    /*						  438,   */
    /*						   :     */
    /*						  65535, */
} hazer_nmeaid_t;

/**
 * Map a single satellite identifier to a system. Using this is really a last
 * resort, and will likely only work in old receivers, and then maybe not
 * reliably.
 * @param id is the NMEA satellite identifier.
 * @return an index of the system or SYSTEM TOTAL if N/A.
 */
extern hazer_system_t hazer_map_nmeaid_to_system(uint16_t id);

/**
 * Proprietary UBX GNSS satellite identifiers used in NMEA-like PUBX sentences.
 * UBLOX R24 Appendix A p. 446.
 */
typedef enum HazerPubxId {
    /*                            0,    */
    HAZER_PUBX_GPS_FIRST        = 1,
    HAZER_PUBX_GPS_LAST         = 32,
    HAZER_PUBX_BEIDOU1_FIRST    = 33,
    HAZER_PUBX_BEIDOU1_LAST     = 64,
    HAZER_PUBX_GLONASS1_FIRST   = 65,
    HAZER_PUBX_GLONASS1_LAST    = 96,
    /*                            97,   */
    /*                             :    */
    /*                            119,  */
    HAZER_PUBX_SBAS_FIRST       = 120,
    HAZER_PUBX_SBAS_LAST        = 158,
    /*                            159,  */
    /*                             :    */
    /*                            210,  */
    HAZER_PUBX_GALILEO_FIRST    = 211,
    HAZER_PUBX_GALILEO_LAST     = 246,
    /*                            247,  */
    /*                             :    */
    /*                            158,  */
    HAZER_PUBX_BEIDOU2_FIRST    = 159,
    HAZER_PUBX_BEIDOU2_LAST     = 163,
    /*                            164,  */
    /*                             :    */
    /*                            172,  */
    HAZER_PUBX_IMES_FIRST       = 173,
    HAZER_PUBX_IMES_LAST        = 182,
    /*                            183,  */
    /*                             :    */
    /*                            192,  */
    HAZER_PUBX_QZSS_FIRST       = 193,
    HAZER_PUBX_QZSS_LAST        = 202,
    /*                            203,  */
    /*                             :    */
    /*                            254,  */
    HAZER_PUBX_GLONASS2_FIRST   = 255,
    HAZER_PUBX_GLONASS2_LAST    = 255,
} hazer_pubxid_t;

/**
 * Map a single satellite identifier to a system using the proprietary UBX
 * identifiers.
 * @param id is the UBX satellite identifier.
 * @return an index of the system or SYSTEM TOTAL if N/A.
 */
extern hazer_system_t hazer_map_pubxid_to_system(uint16_t id);

/**
 * This buffer is large enough to contain the largest NMEA sentence,
 * according to the NMEA spec, plus a trailing NUL (and then some).
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef uint8_t (hazer_buffer_t)[HAZER_NMEA_LONGEST + 1]; /* plus NUL */

/**
 * @def HAZER_BUFFER_INITIALIZER
 * Initialize a HazerBuffer type.
 */
#define HAZER_BUFFER_INITIALIZER  { '\0', }

/**
 * Hazer NMEA parser state machine context (which needs no initial value).
 */
typedef struct HazerContext {
    uint8_t * bp;		/* Current buffer pointer. */
    size_t sz;			/* Remaining buffer size in bytes. */
    size_t tot;			/* Total size once sentence is complete. */
    uint8_t cs;			/* Running checksum. */
    uint8_t msn;		/* Most significant checksum nibble character. */
    uint8_t lsn;		/* Least significant checksum nibble character. */
} hazer_context_t;

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
 * @param pp points to the context structure (which needs no initialization).
 * @return the next state of the machine.
 */
extern hazer_state_t hazer_machine(hazer_state_t state, uint8_t ch, void * buffer, size_t size, hazer_context_t * pp);

/**
 * Return the total size of the complete NMEA sentence as computed by the
 * parser.
 * @param pp points to the context structure.
 * @return the final size.
 */
static inline size_t hazer_size(const hazer_context_t * pp)
{
    return pp->tot;
}

/*******************************************************************************
 * VALIDATING AN NMEA SENTENCE
 ******************************************************************************/

/**
 * Update a running NMEA XOR checksum with the latest input character.
 * @param ch is the input character.
 * @param csp points to the running checksum character.
 */
static inline void hazer_checksum(uint8_t ch, uint8_t * csp)
{
    *csp ^= ch;
}

/**
 * Compute the eight-bit checksum of an NMEA sentence. The buffer points to the
 * beginning of the NMEA sentence, including the '$', not to the subset that
 * is checksummed. A pointer is returned pointing just past the checksummed
 * portion; this is where the '*' and the checksum will be stored in a correctly
 * formed packet.
 * @param buffer points to the beginning of the buffer.
 * @param size is the size of the buffer in bytes.
 * @param msnp points where the most significant nibble character is stored.
 * @param lsnp points where the least significant nibble character is stored.
 * @return a pointer just past the end of the checksummed portion, or NULL if an error occurred.
 */
extern const void * hazer_checksum_buffer(const void * buffer, size_t size, uint8_t * msnp, uint8_t * lsnp);

/**
 * Given two checksum characters, convert to an eight-bit checksum.
 * @param msn is the character representing the most significant nibble.
 * @param lsn is the character representing the least significant nibble.
 * @param ckp points to a variable in which the checksum is stored.
 * @return 0 for success, <0 if an error occurred.
 */
extern int hazer_characters2checksum(uint8_t msn, uint8_t lsn, uint8_t * ckp);

/**
 * Given an eight-bit checksum, concert into the two checksum characters.
 * @param ck is the checksum.
 * @param msnp points where the most significant character is stored.
 * @param lsnp points where the least significant character is stored.
 */
extern void hazer_checksum2characters(uint8_t ck, uint8_t * msnp, uint8_t * lsnp);

/**
 * Return the length of the completed sentence in bytes.
 * @param buffer points to buffer containing the completed sentence.
 * @param size is the number of bytes in the buffer.
 * @return the length in bytes or <0 if an error occurred.
 */
extern ssize_t hazer_length(const void * buffer, size_t size);

/**
 * Validate the contents of an buffer as a valid NMEA sentence. This combines
 * the hazer_length() and hazer_checksum_buffer() functions along with the
 * checksum comparison.
 * @param buffer points to the buffer.
 * @param size is the number of bytes in the buffer.
 * @return the length of the sentence in bytes or <0 if an error occurred.
 */
extern ssize_t hazer_validate(const void * buffer, size_t size);

/*******************************************************************************
 * BREAKING UP AN NMEA SENTENCE INTO FIELDS
 ******************************************************************************/

/**
 * This is an argument vector big enough to hold all possible sentences no
 * larger than those that can fit in the buffer type, plus a NULL pointer in
 * the last position.
 */
typedef char * (hazer_vector_t)[HAZER_NMEA_LONGEST - HAZER_NMEA_SHORTEST + 1]; /* plus NULL */

/**
 * @def HAZER_VECTOR_INITIALIZER
 * Initialize a HazerVector type.
 */
#define HAZER_VECTOR_INITIALIZER  { (char *)0, }

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
 * a signed integer number of nanominutes.
 * @param string points to the string.
 * @param direction is the NMEA direction: 'N', 'S', 'E', or 'W'.
 * @param digitsp points to where the number of digits is stored.
 * @return nanominutes.
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
 * Parse a string containing a speed in kilometers/h in NMEA format into a
 * signed integer number of millimeters/hour.
 * @param string points to the string.
 * @param digitsp points to where the number of digits is stored.
 * @return millimeters/hour.
 */
extern int64_t hazer_parse_smm(const char * string, uint8_t * digitsp);

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
 * Parse a dilution of precision into a value that is the DOP scaled by
 * multiplying it by 100. The result will be in the range of 0 to 9999 (DOP of
 * 99.99).
 * @param string points the string.
 * @return a DOP scaled by 100.
 */
extern uint16_t hazer_parse_dop(const char * string);

/*******************************************************************************
 * IDENTIFYING STANDARD SENTENCES
 ******************************************************************************/

/**
 * @def HAZER_NMEA_SENTENCE_DTM
 * ublox7 Protocol Reference, p. vi, datum reference
 */
#define HAZER_NMEA_SENTENCE_DTM "DTM"

/**
 * @def HAZER_NMEA_SENTENCE_GBS
 * ublox7 Protocol Reference, p. vi, GNSS fault detection
 */
#define HAZER_NMEA_SENTENCE_GBS "GBS"

/**
 * @def HAZER_NMEA_SENTENCE_GGA
 * SiRF NMEA, Table 1-2, GPS fix data
 */
#define HAZER_NMEA_SENTENCE_GGA "GGA"

/**
 * @def HAZER_NMEA_SENTENCE_GLL
 * SiRF NMEA, Table 1-2, geographic position latitude/longitude
 */
#define HAZER_NMEA_SENTENCE_GLL "GLL"

/**
 * @def HAZER_NMEA_SENTENCE_GNS
 * ublox7 Protocol Reference, p. vi, GNSS fix data
 */
#define HAZER_NMEA_SENTENCE_GNS "GNS"

/**
 * @def HAZER_NMEA_SENTENCE_GRS
 * ublox7 Protocol Reference, p. vi, GNSS range residuals
 */
#define HAZER_NMEA_SENTENCE_GRS "GRS"

/**
 * @def HAZER_NMEA_SENTENCE_GSA
 * SiRF NMEA, Table 1-2, GPS DOP and active satellites
 */
#define HAZER_NMEA_SENTENCE_GSA "GSA"

/**
 * @def HAZER_NMEA_SENTENCE_GRT
 * ublox7 Protocol Reference, p. vi, GNSS pseudo range error statistics
 */
#define HAZER_NMEA_SENTENCE_GRT "GST"

/**
 * @def HAZER_NMEA_SENTENCE_GSV
 * SiRF NMEA, Table 1-2, GPS satellites in view
 */
#define HAZER_NMEA_SENTENCE_GSV "GSV"

/**
 * @def HAZER_NMEA_SENTENCE_MSS
 * SiRF NMEA, Table 1-2, beacon receiver status
 */
#define HAZER_NMEA_SENTENCE_MSS "MSS"

/**
 * @def HAZER_NMEA_SENTENCE_RMC
 * SiRF NMEA, Table 1-2, recommended minimum navigation information message
 */
#define HAZER_NMEA_SENTENCE_RMC "RMC"

/**
 * @def HAZER_NMEA_SENTENCE_TXT
 * ublox7 Protocol Reference, p. vi, text
 */
#define HAZER_NMEA_SENTENCE_TXT "TXT"

/**
 * @def HAZER_NMEA_SENTENCE_VTG
 * SiRF NMEA, Table 1-2, track made good and ground speed
 */
#define HAZER_NMEA_SENTENCE_VTG "VTG"

/**
 * @def HAZER_NMEA_SENTENCE_ZDA
 * SiRF NMEA, Table 1-2, time & date
 */
#define HAZER_NMEA_SENTENCE_ZDA "ZDA"

/*******************************************************************************
 * IDENTIFYING PROPRIETARY SENTENCES
 ******************************************************************************/

/**
 * @def HAZER_PROPRIETARY_SENTENCE_PUBX
 * ublox7 Protocol Reference, p. vi, PUBX
 */
#define HAZER_PROPRIETARY_SENTENCE_PUBX "PUBX"

/**
 * @def HAZER_PROPRIETARY_SENTENCE_PUBX_POSITION
 * ublox8 M8 Receiver description, p. 138, PUBX,00
 */
#define HAZER_PROPRIETARY_SENTENCE_PUBX_POSITION "00"

/**
 * @def HAZER_PROPRIETARY_SENTENCE_PUBX_SVSTATUS
 * ublox8 M8 Receiver description, p. 140, PUBX,03
 */
#define HAZER_PROPRIETARY_SENTENCE_PUBX_SVSTATUS "03"

/**
 * @def HAZER_PROPRIETARY_SENTENCE_PUBX_TIME
 * ublox8 M8 Receiver description, p. 141, PUBX,04
 */
#define HAZER_PROPRIETARY_SENTENCE_PUBX_TIME "04"

/**
 * @def HAZER_PROPRIETARY_SENTENCE_PMTK
 * GTop PMTK command packet reference
 */
#define HAZER_PROPRIETARY_SENTENCE_PMTK "PMTK"

/**
 * @define HAZER_PROPRIETARY SENTENCE_PSRF
 * SiRF NMEA Reference Manual, 2-1, Input Messages
 */
#define HAZER_PROPRIETARY_SENTENCE_PSRF "PSRF"

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
extern hazer_system_t hazer_map_talker_to_system(hazer_talker_t talker);

/*******************************************************************************
 * PARSING POSITION, HEADING, VELOCITY, AND TIME SENTENCES
 ******************************************************************************/

/**
 * This structure maintains the time, position, altitude, speed, and bearing
 * derived from the NMEA stream.
 */
typedef struct HazerPosition {
    uint64_t old_nanoseconds;	    /* Prior total nanoseconds. */
    uint64_t tot_nanoseconds;       /* Total nanoseconds. */
    uint64_t utc_nanoseconds;       /* Time in nanoseconds since 00:00 UTC. */
    uint64_t dmy_nanoseconds;       /* Date in nanoseconds since POSIX epoch. */
    int64_t lat_nanominutes;        /* Latitude in nanominutes. */
    int64_t lon_nanominutes;        /* Longitude in nanominutes. */
    int64_t alt_millimeters;        /* Altitude above MSL in millimeters. */
    int64_t sep_millimeters;        /* Geoid seperation in millimeters. */
    int64_t sog_microknots;         /* Speed Over Ground in microknots. */
    int64_t sog_millimetersperhour; /* Speed Over Ground in millimeters per hour. */
    int64_t cog_nanodegrees;        /* Course Over Ground true in nanodegrees. */
    int64_t mag_nanodegrees;        /* Magnetic bearing in nanodegrees. */
    const char * label;			    /* Label for sentence. */
    uint8_t sat_used;               /* Number of satellites used. */
    uint8_t lat_digits;             /* Significant digits of latitude. */
    uint8_t lon_digits;             /* Significant digits of longitude. */
    uint8_t alt_digits;             /* Significant digits of altitude. */
    uint8_t sep_digits;             /* Significant digits of seperation. */
    uint8_t sog_digits;             /* Significant digits of Speed On Ground. */
    uint8_t smm_digits;			    /* Significant digits of SOG mm/h. */
    uint8_t cog_digits;             /* Significant digits of Course On Ground. */
    uint8_t mag_digits;             /* Significant digits of Magnetic bearing. */
    hazer_expiry_t ticks;		    /* Lifetime in application-defined ticks. */
    uint8_t unused[1];              /* Unused. */
} hazer_position_t;

/**
 * @def HAZER_POSITION_INITIALIZER
 * Initialize a HazerPosition structure.
 */
#define HAZER_POSITION_INITIALIZER \
    { \
        0, 0, 0, 0, \
        0, 0, 0, 0, \
        0, 0, 0, 0, \
        (const char *)0, \
        0, \
        0, 0, 0, 0, 0, 0, 0, 0, \
        0, \
        { 0, } \
    }

/**
 * Parse a GGA NMEA sentence, updating the position.
 * @param positionp points to the position structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gga(hazer_position_t * positionp, char * vector[], size_t count);

/**
 * Parse a RMC NMEA sentence, updating the position.
 * @param positionp points to the position structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_rmc(hazer_position_t * positionp, char * vector[], size_t count);

/**
 * Parse a GLL NMEA sentence, updating the position.
 * @param positionp points to the position structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gll(hazer_position_t * positionp, char * vector[], size_t count);

/**
 * Parse a VTG NMEA sentence, updating the position.
 * @param positionp points to the position structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_vtg(hazer_position_t * positionp, char * vector[], size_t count);

/*******************************************************************************
 * PARSING SATELLITE ELEVATION, AZIMUTH, AND SIGNAL STRENGTH SENTENCES
 ******************************************************************************/

/**
 * Various encodings for the fix mode. Note that larger numbers do not necessarily
 * indicate a better fix. The values were chosen mostly to preserve the encoding
 * specified by NMEA while capturing other possibilities of UBX PUBX.
 * NMEA 0183 4.10 p. 94.
 * UBX M8 R24 p. 164.
 */
typedef enum HazerMode {
    HAZER_MODE          = 0,
    HAZER_MODE_NOFIX    = 1,
    HAZER_MODE_2D       = 2,
    HAZER_MODE_3D       = 3,
    HAZER_MODE_COMBINED = 4,
    HAZER_MODE_DGNSS2D  = 5,
    HAZER_MODE_DGNSS3D  = 6,
    HAZER_MODE_TIME     = 7,
    HAZER_MODE_IMU      = 8,
    HAZER_MODE_ZERO     = 9,
    HAZER_MODE_TOTAL    = 10,
} hazer_mode_t;

/**
 * @def HAZER_MODE_NAME_INITIALIZER
 * Initialize the array of character strings that map from a Hazer mode
 * enumerated value to the printable name of the mode.
 */
#define HAZER_MODE_NAME_INITIALIZER \
{ \
    "--", \
    "NF", \
    "2D", \
    "3D", \
    "RK", \
    "D2", \
    "D3", \
    "TT", \
    "DR", \
    "NS", \
    "??", \
}

/**
 * Array of MODE names indexed by mode enumeration.
 */
extern const char * HAZER_MODE_NAME[/* hazer_mode_t */];

/**
 * This structure maintains the information on the satellites in any
 * constellation that were used in the position solution.
 */
typedef struct HazerActive {
    const char * label;			        /* Label for sentence. */
    uint16_t id[HAZER_GNSS_ACTIVES];    /* Satellites active. */
    uint16_t pdop;				        /* Position Dilution Of Precision * 100. */
    uint16_t hdop;				        /* Horizontal Dilution Of Precision * 100. */
    uint16_t vdop;				        /* Vertical Dilution Of Precision * 100. */
    uint16_t tdop;				        /* Time Dilution Of Precision * 100. */
    uint8_t system;				        /* GNSS System ID (HAZER_SYSTEM_TOTAL == unused). */
    uint8_t active;                     /* Number of satellites active. */
    uint8_t mode;                       /* Navigation mode: see HazerMode. */
    hazer_expiry_t ticks;		        /* Lifetime in application-defined ticks. */
} hazer_active_t;

#define HAZER_ACTIVE_

/**
 * @def HAZER_ACTIVE_INITIALIZER
 * Initialize a HazerActive structure.
 */
#define HAZER_ACTIVE_INITIALIZER \
    { \
        (const char *)0, \
        { 0, }, \
        HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, \
        HAZER_SYSTEM_TOTAL, \
        0, \
        HAZER_MODE, \
        0, \
    }

/**
 * Parse a GSA NMEA sentence, updating the constellation.
 * @param activep points to the active structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_gsa(hazer_active_t * activep, char * vector[], size_t count);

/**
 * Return a system given a list of active satellites. This is based on the
 * NMEA conventions for satellite numbering for GPS, GLONASS, and WAAS.
 * It is sometimes useful for GPS devices that emit multiple GSA sentences all
 * under the GNSS talker. But I recommend using it only as a last resort, as I
 * find the documented NMEA conventions for satellite numbering unreliable.
 * @param activep points to the active structure.
 * @return the index of the system or SYSTEM TOTAL if N/A.
 */
extern hazer_system_t hazer_map_active_to_system(const hazer_active_t * activep);

/**
 * This structure maintains the elevation, azimuth, and signal strength of a
 * single satellite. (The phantom field was introduced due to the Ublox 8
 * equipped BU353W10 reporting a GPS PRN 4 satellite within view, with an
 * empty string for both elevation and azimuth, but a reasonable SNR. At that
 * time, there is no PRN 4, that vehicle having been decommisioned and the
 * pseudo-random number code #4 not yet reassigned.)
 */
typedef struct HazerSatellite {
    uint16_t id;                /* Satellite IDentifier. */
    int16_t elv_degrees;        /* Elevation in whole degrees. */
    int16_t azm_degrees;        /* Azimuth in whole degrees. */
    int8_t snr_dbhz;            /* Signal/Noise Ratio in dBHz. */
    uint8_t signal;             /* Signal band identifier. */
    uint8_t phantom;			/* If true, elevation or azimuth were empty. */
    uint8_t untracked;          /* If true, signal strength was empty. */
    uint8_t unused;             /* If true, unused. */
} hazer_satellite_t;

/**
 * @def HAZER_SATELLITE_INITIALIZER
 * Initialize a HazerSatellite structure.
 */
#define HAZER_SATELLITE_INITIALIZER \
    { \
        0, \
        0, 0, \
        0, \
        0, \
        0, \
        0, \
        0 \
    }

/**
 * This structure maintains the information on as many satellites as we
 * have channels configured.
 */
typedef struct HazerView {
    const char * label;			/* Label for sentence. */
    hazer_satellite_t sat[HAZER_GNSS_SATELLITES]; /* Satellites viewed. */
    uint8_t view;               /* Number of satellites in view. */
    uint8_t channels;           /* Number of channels used in view. */
    uint8_t pending;			/* Number of updates pending. */
    hazer_expiry_t ticks;		/* Lifetime in application-defined ticks. */
} hazer_view_t;

/**
 * @def HAZER_VIEW_INITIALIZER
 * Initialize a HazerView structure.
 */
#define HAZER_VIEW_INITIALIZER \
    { \
        (const char *)0, \
        { \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
            HAZER_SATELLITE_INITIALIZER, \
        }, \
        0, \
        0, \
        0, \
        0, \
    }

/**
 * Parse a GSV NMEA sentence, updating the constellation.
 * @param viewp points to the view structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success on final update of group, 1 for success, <0 otherwise.
 */
extern int hazer_parse_gsv(hazer_view_t * viewp, char * vector[], size_t count);

/*******************************************************************************
 * PARSING TEXT SENTENCES
 ******************************************************************************/

/**
 * Parse a TXT NMEA sentence.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success, <0 otherwise.
 */
extern int hazer_parse_txt(char * vector[], size_t count);

/*******************************************************************************
 * PARSING PROPRIETARY U-BLOX PUBX SENTENCES
 ******************************************************************************/

/**
 * Parse a U-blox PUBX,00 (POSITION) message. This message contains information
 * both about position and the fix itself (e.g. the dilution of precision values
 * and fix mode indicator).
 * @param positionp points to the position structure.
 * @param activep points to the active structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success on final update of group, 1 for success, <0 otherwise.
 */
extern int hazer_parse_pubx_position(hazer_position_t * positionp, hazer_active_t * activep, char * vector[], size_t count);

/**
 * Parse a U-blox PUBX,03 (SVSTATUS) message. The arrays are used instead
 * of pointers to a single element in the array because this PUBX message
 * contains Space Vehicle (SV) statuses for satellites across all systems.
 * @param view is the view ARRAY (not a pointer to a single element).
 * @param active is the active ARRAY (not a pointer to a single element).
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return a bit mask with (1<<system) bit set for every system reported.
 */
extern int hazer_parse_pubx_svstatus(hazer_view_t view[], hazer_active_t active[], char * vector[], size_t count);

/**
 * Parse a U-blox PUBX,04 (TIME) message. Note that should the fix be lost,
 * at least the U-blox generation 8 device will continue to issue this
 * messgae with the time updated according (apparently) to its internal clock.
 * This can be misleadning, IMO.
 * @param positionp points to the position structure.
 * @param vector contains the words in the NMEA sentence.
 * @param count is size of the vector in slots including the null pointer.
 * @return 0 for success on final update of group, 1 for success, <0 otherwise.
 */
extern int hazer_parse_pubx_time(hazer_position_t * positionp, char * vector[], size_t count);

/*******************************************************************************
 * PARSING HELPERS
 ******************************************************************************/

/**
 * Return true if the NMEA sentence name following the talker matches
 * the specified three letter name.
 * @param vector is the Hazer parsed vector (a 2D array).
 * @param name is the nul-terminated three letter name.
 * @return true if the name matches the field.
 */
static inline int hazer_is_nmea_name(const hazer_vector_t vector, const char name[4])
{
    return ((vector[0][0] == HAZER_STIMULUS_START) && (strcmp(&(vector[0][3]), name) == 0));
}

/**
 * Return true if the second field in the vector matches the specified
 * PUBX message id.
 * @param vector is the Hazer parsed vector (a 2D array).
 * @param id is the nul-terminated two letter message identifier.
 * @return true if the id matches the field.
 */
static inline int hazer_is_pubx_id(const hazer_vector_t vector, const char id[3])
{
    return ((vector[0][0] == HAZER_STIMULUS_START) && (strcmp(&(vector[0][1]), "PUBX") == 0) && (strcmp(vector[1], id) == 0));
}

/**
 * Returns true if there are GSV views pending for any constellation. Can be
 * applied by a single constellation by passing pointer to single view and
 * using a count of one.
 * @param va points to the array of all satellites being viewed.
 * @param count is the number of entries in the view array.
 * @return true if there are GSV views pending for any constellation.
 */
extern int hazer_has_pending_gsv(const hazer_view_t va[], size_t count);

/**
 * Returns true if there is any position that has a valid clock, which
 * requires that at least one constellation has both the time and date and
 * a monotonically increasing clock.
 * @param pa points to the array of all positions being computed.
 * @param count is the number of entries in the position array.
 * @return true if time, date, and monotonic clock are true for some position.
 */
extern int hazer_has_valid_time(const hazer_position_t pa[], size_t count);

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

/*
 * At least on later devices, NMEA sentences report about ten significant
 * digits for latitude and longitude in the form of whole degrees and decimal
 * minutes. I try to do the same below.
 */

/**
 * Format nanominutes of latitude or longitude into position values.
 * @param nanominutes is a longitude or latitude in nanominutes.
 * @param degreesp points to where the integral degrees (e.g. 180) is stored.
 * @param minutesp points to where the minutes (0..59) are stored.
 * @param secondsp points to where the seconds (0..59) are stored.
 * @param thousanthsp points to there the fractional seconds (0..999) are stored.
 * @param directionp points to where 1 (N or E) or -1 (S or W) is stored.
 */
extern void hazer_format_nanominutes2position(int64_t nanominutes, int * degreesp, int * minutesp, int * secondsp, int * thousanthsp, int * directionp);

/**
 * Format nanominutes of latitude or longitude into decimal degrees.
 * @param nanominutes is a longitude or latitude in nanominutes.
 * @param degreesp points to where the signed integral degrees (e.g. 180) is stored.
 * @param nanodegreesp points to where the unsigned fractional nanodegrees are stored.
 */
extern void hazer_format_nanominutes2degrees(int64_t nanominutes, int * degreesp, uint64_t * nanodegreesp);

/**
 * Format nanodegrees of compass bearing in a pointer to a name of a
 * compass point on a thirty-two point compass.
 * @param nanodegrees is a bearing or heading in compass nanodegrees.
 * @return a compass point string in upper case.
 */
extern const char * hazer_format_nanodegrees2compass32(int64_t nanodegrees);

/**
 * Format nanodegrees of compass bearing in a pointer to a name of a
 * compass point on a sixteen point compass.
 * @param nanodegrees is a bearing or heading in compass nanodegrees.
 * @return a compass point string in upper case.
 */
extern const char * hazer_format_nanodegrees2compass16(int64_t nanodegrees);

/**
 * Format nanodegrees of compass bearing in a pointer to a name of a
 * compass point on an eight point compass.
 * @param nanodegrees is a bearing or heading in compass nanodegrees.
 * @return a compass point string in upper case.
 */
extern const char * hazer_format_nanodegrees2compass8(int64_t nanodegrees);

#endif
