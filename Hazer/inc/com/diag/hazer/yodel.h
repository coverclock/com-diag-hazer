/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_YODEL_
#define _H_COM_DIAG_HAZER_YODEL_

/**
 * @file
 * @copyright Copyright 2018-2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Support for proprietary UBX messaging as used by ublox AG devices.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * The Yodel module provides support for the proprietary UBX binary packet
 * format that is optionally produced by GNSS devices manufactured by ublox AG.
 * Yodel can be used in parallel with Hazer for devices that produce both
 * output formats in the same data stream.
 *
 * REFERENCES
 *
 * "u-blox 7 Receiver Description Including Protocol Specification V14",
 * GPS.G7-SW-12001-B, ublox AG, 2013
 *
 * "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification",
 * UBX-13003221-R15, ublox AG, 2018-03-06
 *
 * "u-blox ZED-F9P Interface Description", v27.11, UBX-18010854-R07, ublox AG,
 * 2019-07-10
 *
 * "ZED-F9P u-blox F9 high precision GNSS module Integration Manual",
 * UBX-18010802-R05, ublox AG, 2019-07-11
 *
 * "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification",
 * UBX-13003221-R19, ublox AG, 2020-05-20
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
extern FILE * yodel_debug(FILE * now);

/*******************************************************************************
 * STARTING UP AND SHUTTING DOWN
 ******************************************************************************/

/**
 * Perform any necessary initialization.
 * @return 0 for success, <0 otherwise.
 */
extern int yodel_initialize(void);

/**
 * Perform any necessary finalization.
 * @return 0 for success, <0 otherwise.
 */
extern int yodel_finalize(void);

/*******************************************************************************
 * COLLECTING A UBLOX PACKET
 ******************************************************************************/

/**
 * Yodel UBX constants.
 * UBlox 7, p.73
 * UBlox 8 R15, p. 134
 */
enum YodelUbxConstants {
    YODEL_UBX_UNSUMMED	= 2,	/* SYNC1[1], SYNC2[1] */
    YODEL_UBX_SUMMED	= 4,	/* CLASS[1], ID[1], LENGTH[2] ... */
    YODEL_UBX_SHORTEST	= 8,	/* UNSUMMED[2], SUMMED[4], CK_A[1], CK_B[1] */
    YODEL_UBX_CHECKSUM	= 2,	/* CK_A[1], CK_B[1] */
    YODEL_UBX_LONGEST	= 1024,	/* Rounded up from SHORTEST + (64 * (4 + 8)). */
};

/**
 * This buffer is large enough to contain the largest UBX packet, plus a
 * trailing NUL, and then some. The NUL at the end is useless in the UBX binary
 * protocol, but is useful in some edge cases in which the data format has not
 * yet been determined (e.g. incoming UDP datagrams).
 */
typedef uint8_t (yodel_buffer_t)[YODEL_UBX_LONGEST + 1];

/**
 * @def YODEL_BUFFER_INITIALIZER
 * Initialize a YodelBuffer type.
 */
#define YODEL_BUFFER_INITIALIZER  \
    { '\0', }

/**
 * Yodel UBX offsets.
 * UBlox 7, p.73
 * UBlox 8 R15, p. 134
 */
enum YodelUbxOffsets {
    YODEL_UBX_SYNC_1		= 0,	/* Always 0xb5. */
    YODEL_UBX_SYNC_2		= 1,	/* Always 0x62. */
    YODEL_UBX_CLASS			= 2,
    YODEL_UBX_ID			= 3,
    YODEL_UBX_LENGTH_LSB	= 4,	/* 16-bit, little endian (LSB). */
    YODEL_UBX_LENGTH_MSB 	= 5,	/* 16-bit, little endian (MSB). */
    YODEL_UBX_PAYLOAD		= 6,
};

/**
 * This is the structure of the header on every UBX packet. Its size is
 * awkward because it's not a multiple of four bytes, but many of the UBX
 * payloads begin with a four byte integer or worse.
 * UBlox 7, p.73
 * UBlox 8 R15, p. 134
 */
typedef struct YodelUbxHeader {
    uint8_t sync_1;		/* 0xb5 */
    uint8_t sync_2;		/* 0x62 */
    uint8_t classx;
    uint8_t	id;
    uint16_t length;	/* little endian */
    uint8_t payload[0];
} yodel_ubx_header_t __attribute__((__aligned__(2)));

/**
 * @def YODEL_UBX_HEADER_INITIALIZER
 * Initialize a YodelUbxHeader structure.
 */
#define YODEL_UBX_HEADER_INITIALIZER \
    { 0, }

/**
 * UBX state machine states. The only states the application needs
 * to take action on is END (complete UBX packet in buffer). The
 * rest are transitory states. If the machine transitions from a non-START
 * state to the START state, that means the framing of the current packet
 * failed; that might be of interest to the application.
 */
typedef enum YodelState {
    YODEL_STATE_STOP		= 'X',
    YODEL_STATE_START		= 'S',
    YODEL_STATE_SYNC_2		= 'Y',
    YODEL_STATE_CLASS		= 'C',
    YODEL_STATE_ID			= 'I',
    YODEL_STATE_LENGTH_1	= 'L',
    YODEL_STATE_LENGTH_2	= 'M',
    YODEL_STATE_PAYLOAD		= 'P',
    YODEL_STATE_CK_A		= 'A',
    YODEL_STATE_CK_B		= 'B',
    YODEL_STATE_END			= 'E',
} yodel_state_t;

/**
 * UBX state machine stimuli.
 */
enum YodelStimulus {
    YODEL_STIMULUS_SYNC_1		= 0xb5,	/* ISO 8859.1 for 'mu'. */
    YODEL_STIMULUS_SYNC_2		= 0x62,	/* 'b' but in hex in doc. */
};

/**
 * UBX state machine actions.
 */
typedef enum YodelAction {
    YODEL_ACTION_SKIP		= 'X',
    YODEL_ACTION_SAVE		= 'S',
    YODEL_ACTION_TERMINATE	= 'T',
} yodel_action_t;

/**
 * Yodel UBX parser state machine context (which needs no initial value).
 */
typedef struct YodelContext {
    uint8_t * bp;		/* Current buffer pointer. */
    size_t sz;			/* Remaining buffer size in bytes. */
    size_t tot;			/* Total size once packet is complete. */
    uint16_t ln;		/* Payload length in bytes. */
    uint8_t csa;		/* Running Fletcher checksum A. */
    uint8_t csb;		/* Running Fletcher checksum B. */
} yodel_context_t;

/**
 * Process a single character of stimulus for the state machine that is
 * assembling a single UBX packet in the caller provided buffer. State
 * is maintained in a character pointer and a size variable, pointers to
 * which are passed to the function, which will be initialized by the function
 * itself. The function returns the new state, which must be used in the
 * subsequent call. The initial state should be the START state. Of interest
 * to the application are the EOF and END states. The EOF state indicates
 * that the input stream has ended, detected by virtue of the stimulus character
 * being equal to the standard I/O EOF. The END state indicates that a complete
 * NMEA sentence resides in the buffer. The pointer state variable points
 * past the end of the NUL-terminated sentence, the size state variable
 * constrains the size of the sentence including the terminating NUL;
 * @param state is the prior state of the machine.
 * @param ch is the next character from the UBX packet stream.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param pp points to the context structure (which needs no initialization).
 * @return the next state of the machine.
 */
extern yodel_state_t yodel_machine(yodel_state_t state, uint8_t ch, void * buffer, size_t size, yodel_context_t * pp);

/**
 * Return the total size of the complete UBX message as computed by the parser.
 * @param pp points to the context structure.
 * @return the final size.
 */
static inline size_t yodel_size(const yodel_context_t * pp)
{
    return pp->tot;
}

/*******************************************************************************
 * VALIDATING A UBX PACKET
 ******************************************************************************/

/**
 * Update a running UBX Fletcher checksum with the latest input character.
 * @param ch is the input character.
 * @param csap points to the A running checksum character.
 * @param csbp points to the B running checksum character.
 */
static inline void yodel_checksum(uint8_t ch, uint8_t * csap, uint8_t * csbp)
{
    *csap += ch;
    *csbp += *csap;
}

/**
 * Compute the Fletcher checksum used by UBX for the specified buffer. The
 * buffer points to the beginning of the UBX packet, not to the subset that
 * is checksummed, and the sentence must contain a valid length field. A pointer
 * is returned pointing just past the checksummed portion; this is where the
 * checksum will be stored in a correctly formed packet.
 * @param buffer points to the beginning of the buffer.
 * @param size is the size of the buffer in bytes.
 * @param csap points to where the ck_a value will be stored.
 * @param csbp points to where the ck_b value will be stored.
 * @return a pointer just past the end of the checksummed portion, or NULL if an error occurred.
 */
extern const void * yodel_checksum_buffer(const void * buffer, size_t size, uint8_t * csap, uint8_t * csbp);

/**
 * Return the length of the completed packet in bytes.
 * @param buffer points to buffer containing the completed packet.
 * @param size is the size of the buffer containing the packet.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t yodel_length(const void * buffer, size_t size);

/**
 * Validate the contents of an buffer as a valid UBX packet.
 * @param buffer points to the buffer. This combines
 * the yodel_length() and yodel_checksum_buffer() functions along with the
 * checksum comparison.
 * @param size is the number of bytes in the buffer.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t yodel_validate(const void * buffer, size_t size);

/******************************************************************************
 *
 ******************************************************************************/

/**
 * UBX system identifiers.
 * UBLOX9 R05, p. 242.
 * These must be in the same order as the corresponding strings below.
 */
typedef enum YodelSystem {
    YODEL_SYSTEM_GPS				= 0,
    YODEL_SYSTEM_SBAS				= 1,
    YODEL_SYSTEM_GALILEO			= 2,
    YODEL_SYSTEM_BEIDOU				= 3,
/*  UNUSED                            4, */
    YODEL_SYSTEM_QZSS				= 5,
    YODEL_SYSTEM_GLONASS			= 6,
    YODEL_SYSTEM_IMES,
    YODEL_SYSTEM_GNSS,
    YODEL_SYSTEM_TOTAL,
} yodel_system_t;

/**
 * @def YODEL_SYSTEM_NAME_INITIALIZER
 * Initialize the array of character strings that map from a Hazer system
 * enumerated value to the printable name of the system. These strings should
 * be in order of preference for systems having (unlikely as it might be)
 * exactly the same dilution of precision (DOP). For example, you might prefer
 * GLONASS over GPS, or GPS over GNSS (which represents a solution using
 * multiple systems, which can be problematic).
 */
#define YODEL_SYSTEM_NAME_INITIALIZER \
    { \
        "GPS", \
        "SBAS", \
        "GALILEO", \
        "BEIDOU", \
        "", \
        "QZSS", \
        "GLONASS", \
        "IMES", \
        "GNSS", \
        (const char *)0, \
    }

/**
 * GNSS satellite identifiers.
 * UBLOX9 R05 p. 242.
 */
typedef enum YodelId {
    /*                        0,     */
    YODEL_ID_GPS_FIRST		= 1,
    /*                        :      */
    YODEL_ID_GPS_LAST		= 32,
    YODEL_ID_SBAS1_FIRST	= 33,
    /*                        :      */
    YODEL_ID_SBAS1_LAST		= 64,
    YODEL_ID_GLONASS1_FIRST	= 65,
    /*                        :      */
    YODEL_ID_GLONASS1_LAST	= 96,
    /*						  97,    */
    /*						   :     */
    /*						  119,   */
    YODEL_ID_SBAS2_FIRST	= 120,
    /*                        :      */
    YODEL_ID_SBAS2_LAST		= 158,
    YODEL_ID_BEIDOU1_FIRST	= 159,
    /*                        :      */
    YODEL_ID_BEIDOU1_LAST	= 163,
    /*						  164,   */
    /*						   :     */
    /*						  172,   */
    YODEL_ID_IMES_FIRST		= 173,
    /*                        :      */
    YODEL_ID_IMES_LAST		= 182,
    /*						  183,   */
    /*						   :     */
    /*						  192,   */
    YODEL_ID_QZSS_FIRST		= 193,
    /*                        :      */
    YODEL_ID_QZSS_LAST		= 197,
    /*						  198,   */
    /*						   :     */
    /*						  254,   */
    YODEL_ID_GLONASS2_FIRST	= 255,
    YODEL_ID_GLONASS2_LAST	= 255,
    /*						  256,   */
    /*						   :     */
    /*						  300,   */
    YODEL_ID_GALILEO_FIRST	= 301,
    /*                        :      */
    YODEL_ID_GALILEO_LAST	= 336,
    /*						  337,   */
    /*						   :     */
    /*						  400,   */
    YODEL_ID_BEIDOU2_FIRST	= 401,
    /*                        :      */
    YODEL_ID_BEIDOU2_LAST	= 437,
    /*						  438,   */
    /*						   :     */
    /*						  65535, */
} yodel_id_t;

/*******************************************************************************
 * PROCESSING UBX-NAV-HPPOSLLH MESSAGES
 ******************************************************************************/

/**
 * UBX-NAV-HPPOSLLH (0x01, 0x14) [36] offers a high precision geodetic position
 * solution.
 */
typedef struct YodelUbxNavHpposllh {
    uint8_t version;
    uint8_t reserved[3];
    uint32_t iTOW;
    int32_t lon;
    int32_t lat;
    int32_t height;
    int32_t hMSL;
    int8_t lonHp;
    int8_t latHp;
    int8_t heightHp;
    int8_t hMSLHp;
    uint32_t hAcc;
    uint32_t vAcc;
} yodel_ubx_nav_hpposllh_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_NAV_HPPOSLLH_INITIALIZER
 * Initialize a YodelUbxNavHpposllh structure.
 */
#define YODEL_UBX_NAV_HPPOSLLH_INITIALIZER \
    { 0, }

/**
 * UBX-NAV-HPPOSLLH constants.
 */
enum YodelUbxNavHpposllhConstants {
    YODEL_UBX_NAV_HPPOSLLH_Class	= 0x01,
    YODEL_UBX_NAV_HPPOSLLH_Id		= 0x14,
    YODEL_UBX_NAV_HPPOSLLH_Length	= 36,
};

/**
 * Process a possible UBX-NAV-HPPOSLLH message.
 * @param mp points to a UBX-NAV-HPPOSLLH structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_hpposllh(yodel_ubx_nav_hpposllh_t * mp, const void * bp, ssize_t length);

/*
 * At least on later devices, UBX HPPOS packets report about twelve significant
 * digits for latitude and longitude using a resolution of billionths of a
 * degree if both the standard and the high precision fields are taken into
 * account. I try to do the same below.
 */

/**
 * Format a high precision position into decimal degrees.
 * @param whole is the lat or lon value in the UBX-NAV-HPPOSLLH structure in 10^-7 degrees.
 * @param fraction is the corresponding latHp or lonHp value in the UBX-NAV-HPPOSLLH structure in 10^-9 degrees.
 * @param degreesp points to where the signed integral degrees (e.g. 180) is stored.
 * @param billionthsp points to where the unsigned fractional degrees (0..999999999) is stored.
 */
extern void yodel_format_hppos2degrees(int32_t whole, int8_t fraction, int32_t * degreesp, uint64_t * billionthsp);

/**
 * Format a high precision position into position values with a fractional
 * part in 10^-5 seconds conforming to the NGS format.
 * @param whole is the lat or lon value in the UBX-NAV-HPPOSLLH structure in 1/10^7 degrees.
 * @param fraction is the corresponding latHp or lonHp value in the UBX-NAV-HPPOSLLH structure in 1/10^9 degrees.
 * @param degreesp points to where the degrees (0..360) are stored.
 * @param minutesp points to where the minutes (0..59) are stored.
 * @param secondsp points to where the seconds (0..59) are stored.
 * @param onehundredthousandsthp points to there the fractional seconds (0..99999) are stored.
 * @param directionp points to where 1 (N or E) or -1 (S or W) is stored.
 */
extern void yodel_format_hppos2position(int32_t whole, int8_t fraction, uint32_t * degreesp, uint32_t * minutesp, uint32_t * secondsp, uint32_t * onehundredthousandsthp, int * directionp);

/**
 * Format a high precision height into an altitude value with a fractional part
 * in 10^-4 meters. This can format either the altitude above Mean Sea Level
 * (MSL) value, or the height above the WGS84 ellipse value.
 * @param whole is the height value in 10^-3 meters (millimeters).
 * @param fraction is the fractional part in 10^-4 meters.
 * @param metersp points to where the meters value is stored.
 * @param tenthousandthsp points to there the 10^-4 fractional part is stored.
 */
extern void yodel_format_hpalt2aaltitude(int32_t whole, int8_t fraction, int32_t * metersp, uint32_t * tenthousandthsp);

/**
 * Format a high precision accuracy into a plus/minus value with a fractional
 * part in 10^-4 meters.
 * @param whole is the accuracy value in 10^-4 meters.
 * @param metersp points to where the meters value is stored.
 * @param tenthousandthsp points to there the 10^-4 fractional part is stored.
 */
extern void yodel_format_hpacc2accuracy(int32_t whole,  int32_t * metersp, uint32_t * tenthousandthsp);

/*******************************************************************************
 * PROCESSING UBX-MON-HW MESSAGES
 ******************************************************************************/

/**
 * UBX-MON-HW (0x0A, 0x09) [60] can be used to detect jamming.
 * Ublox 8 R15, p. 285-286.
 */
typedef struct YodelUbxMonHw {
    uint32_t pinSel;
    uint32_t pinBank;
    uint32_t pinDir;
    uint32_t pinVal;
    uint16_t noisePerMS;
    uint16_t agcCnt;
    uint8_t aStatus;
    uint8_t aPower;
    uint8_t flags;
    uint8_t reserved1;
    uint32_t usedMask;
    uint8_t VP[17];
    uint8_t jamInd;
    uint8_t reserved2[2];
    uint32_t pinIrq;
    uint32_t pullH;
    uint32_t pullL;
} yodel_ubx_mon_hw_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_MON_HW_INITIALIZER
 * Initialize a YodelUbxMonHw structure.
 */
#define YODEL_UBX_MON_HW_INITIALIZER \
    { 0, }

/**
 * UBX-MON-HW constants.
 */
enum YodelUbxMonHwConstants {
    YODEL_UBX_MON_HW_Class	= 0x0a,
    YODEL_UBX_MON_HW_Id		= 0x09,
    YODEL_UBX_MON_HW_Length	= 60,
};

/**
 * UBX-MON-HW.flags masks.
 */
enum YodelUbxMonHwFlagsMasks {
    YODEL_UBX_MON_HW_flags_rtcCalib_MASK		= 0x1,
    YODEL_UBX_MON_HW_flags_safeBoot_MASK		= 0x1,
    YODEL_UBX_MON_HW_flags_jammingState_MASK	= 0x3,
    YODEL_UBX_MON_HW_flags_xtalAbsent_MASK		= 0x1,
};

/**
 * UBX-MON-HW.flags left shifts.
 */
enum YodelUbxMonHwFlagsShifts {
    YODEL_UBX_MON_HW_flags_rtcCalib_SHIFT		= 0,
    YODEL_UBX_MON_HW_flags_safeBoot_SHIFT		= 1,
    YODEL_UBX_MON_HW_flags_jammingState_SHIFT	= 2,
    YODEL_UBX_MON_HW_flags_xtalAbsent_SHIFT		= 4,
};

/**
 * UBX-MON-HW.Flags.JammingState values.
 */
enum YodelUbxMonHwFlagsJammingState {
    YODEL_UBX_MON_HW_flags_jammingState_unknown		= 0,
    YODEL_UBX_MON_HW_flags_jammingState_none		= 1,
    YODEL_UBX_MON_HW_flags_jammingState_warning		= 2,
    YODEL_UBX_MON_HW_flags_jammingState_critical	= 3,
};

/**
 * Process a possible UBX-MON-HW message.
 * @param mp points to a UBX-MON-HW structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_mon_hw(yodel_ubx_mon_hw_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-NAV-STATUS MESSAGES
 ******************************************************************************/

/**
 * UBX-NAV-STATUS (0x01, 0x03) [16] can be used to detect spoofing.
 * Ublox 8 R15, p. 316-318.
 */
typedef struct YodelUbxNavStatus {
    uint32_t iTOW;
    uint8_t gpsFix;
    uint8_t flags;
    uint8_t fixStat;
    uint8_t flags2;
    uint32_t ttff;
    uint32_t msss;
} yodel_ubx_nav_status_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_NAV_STATUS_INITIALIZER
 * Initialize a YodelUbxNavStatus structure.
 */
#define YODEL_UBX_NAV_STATUS_INITIALIZER \
    { 0, }

/**
 * UBX-NAV-STATUS constants.
 */
enum YodelUbxNavStatusConstants {
    YODEL_UBX_NAV_STATUS_Class	= 0x01,
    YODEL_UBX_NAV_STATUS_Id		= 0x03,
    YODEL_UBX_NAV_STATUS_Length	= 16,
};

/**
 * UBX-NAV-STATUS.flags masks.
 */
enum YodelUbxNavStatusFlagsMasks {
    YODEL_UBX_NAV_STATUS_flags_gpsFixOk_MASK	= 0x1,
    YODEL_UBX_NAV_STATUS_flags_diffSoln_MASK	= 0x1,
    YODEL_UBX_NAV_STATUS_flags_wknSet_MASK		= 0x1,
    YODEL_UBX_NAV_STATUS_flags_towSet_MASK		= 0x1,
};

/**
 * UBX-NAV-STATUS.flags left shifts.
 */
enum YodelUbxNavStatusFlagsShifts {
    YODEL_UBX_NAV_STATUS_flags_gpsFixOk_SHIFT	= 0,
    YODEL_UBX_NAV_STATUS_flags_diffSoln_SHIFT	= 1,
    YODEL_UBX_NAV_STATUS_flags_wknSet_SHIFT		= 2,
    YODEL_UBX_NAV_STATUS_flags_towSet_SHIFT		= 3,
};

/**
 * UBX-NAV-STATUS.fixStat masks.
 */
enum YodelUbxNavStatusFixStatMasks {
    YODEL_UBX_NAV_STATUS_fixStat_diffCorr_MASK		= 0x1,
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_MASK	= 0x3,
};

/**
 * UBX-NAV-STATUS.fixStat left shifts.
 */
enum YodelUbxNavStatusFixStatShifts {
    YODEL_UBX_NAV_STATUS_fixStat_diffCorr_SHIFT		= 0,
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_SHIFT	= 6,
};

/**
 * UBX-NAV-STATUS.fixStat.mapMatching values.
 */
enum YodelUbxNavStatusFixStatMapMatching {
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_none			= 0,
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_unused			= 1,
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_applied		= 2,
    YODEL_UBX_NAV_STATUS_fixStat_mapMatching_deadreckoning	= 3,
};

/**
 * UBX-NAV-STATUS.flags2 masks.
 */
enum YodelUbxNavStatusFlags2Masks {
    YODEL_UBX_NAV_STATUS_flags2_psmState_MASK		= 0x3,
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_MASK	= 0x3,
};

/**
 * UBX-NAV-STATUS.flags2 left shifts.
 */
enum YodelUbxNavStatusFlags2Shifts {
    YODEL_UBX_NAV_STATUS_flags2_psmState_SHIFT			= 0,
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_SHIFT		= 3,
};

/**
 * UBX-NAV-STATUS.flags2.psmState values.
 */
enum YodelUbxNavStatusFlags2PsmState {
    YODEL_UBX_NAV_STATUS_flags2_psmState_acquisition	= 0,
    YODEL_UBX_NAV_STATUS_flags2_psmState_nospoofing		= 1,
    YODEL_UBX_NAV_STATUS_flags2_psmState_tracking		= 2,
    YODEL_UBX_NAV_STATUS_flags2_psmState_inactive		= 3,
};

/**
 * UBX-NAV-STATUS.flags2.spoofDetState values.
 */
enum YodelUbxNavStatusFlags2SpoolDetState {
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_unknown	= 0,
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_none		= 1,
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_one		= 2,
    YODEL_UBX_NAV_STATUS_flags2_spoofDetState_many		= 3,
};

/**
 * Process a possible UBX-NAV-STATUS message.
 * @param mp points to a UBX-NAV-STATUS structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_status(yodel_ubx_nav_status_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-ACK-ACK and UBX-ACK-NAK MESSAGES
 ******************************************************************************/

/**
 * UBX-ACK-ACK (0x05, 0x01) [2] and UBX-ACK-NAK (0x05, 0x00) [2] are used to
 * indicate the success or failure of UBX messages sent to the device.
 * Ublox 8 R15, p. 145.
 */
typedef struct YodelUbxAck {
    uint8_t clsID;		/* Class of packet ACKed or NAKed. */
    uint8_t msgID;		/* Message of packet ACKed or NAKed. */
    uint8_t state;		/* True if ACK, false if NAK. */
} yodel_ubx_ack_t;

/**
 * @def YODEL_UBX_ACK_INITIALIZER
 * Initialize a YodelUbxAck structure.
 */
#define YODEL_UBX_ACK_INITIALIZER \
    { \
        ~0, \
        ~0, \
        ~0, \
    }

/**
 * UBX-ACK constants.
 */
enum YodelUbxAckConstants {
    YODEL_UBX_ACK_Class		= 0x05,
    YODEL_UBX_ACK_Length	= 2,
    YODEL_UBX_ACK_NAK_Id	= 0x00,
    YODEL_UBX_ACK_ACK_Id	= 0x01,
};

/**
 * Process a possible UBX-ACK-ACK or UBX-ACK-NAK message.
 * @param mp points to a UBX-ACK structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_ack(yodel_ubx_ack_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-CFG-VALGET MESSAGES
 ******************************************************************************/

/**
 * UBX-CFG-VALGET is how generation 9 handles device configuration queries.\
 * N.B. The layer field here is an enumeration, but in a UBX-CFG-VALSET message
 * it is a bit mask. You can only VALGET one layer at a time, but you can VALSET
 * multiple layers in one message.
 * Ublox 9, p. 85.
 */
typedef struct YodelUbxCfgValget {
    uint8_t version;		/* Message version: send 0, receive 1. */
    uint8_t layer;			/* 0: RAM, 1: Battery Backed RAM, 2: Flash, 3: ROM. */
    uint8_t reserved[2];	/* Reserved. */
    uint8_t cfgData[0];		/* Beginning of variable number key/value pairs. */
} yodel_ubx_cfg_valget_t;

/**
 * @def YODEL_UBX_CFG_VALGET_INITIALIZER
 * Initialize the fixed portion of a YodelUbxCfgValget structure.
 */
#define YODEL_UBX_CFG_VALGET_INITIALIZER \
    { 0, }

/**
 * UBX-CFG-VALGET constants.
 */
enum YodelUbxCfgValgetConstants {
    YODEL_UBX_CFG_VALGET_Class			= 0x06,
    YODEL_UBX_CFG_VALGET_Id				= 0x8b,
    YODEL_UBX_CFG_VALGET_Length			= 4,
    YODEL_UBX_CFG_VALGET_Key_Size_SHIFT	= 28,
    YODEL_UBX_CFG_VALGET_Key_Size_MASK	= 0x7,
};

/**
 * Note that the way UBX-CFG-VALGET encodes the layer (as an enumeration)
 * is NOT how UBX-CFG-VALSET encodes the layer (as a bit mask so that multiple
 * layers can be modified with a single VALSET command). Unless there is a
 * compelling reason not to, I stick with RAM so that a power cycle can
 * restore the default to defaults.
 * Ublox 9, p. 86
 */
enum YodelUbxCfgValgetLayer {
    YODEL_UBX_CFG_VALGET_Layer_RAM	= 0,
    YODEL_UBX_CFG_VALGET_Layer_BBR	= 1,
    YODEL_UBX_CFG_VALGET_Layer_NVM	= 2,
    YODEL_UBX_CFG_VALGET_Layer_ROM	= 7,
};

/**
 * Ublox 9, p. 191
 */
enum YodelUbxCfgValgetSize {
    YODEL_UBX_CFG_VALGET_Size_BIT	= 0x01,
    YODEL_UBX_CFG_VALGET_Size_ONE	= 0x02,
    YODEL_UBX_CFG_VALGET_Size_TWO	= 0x03,
    YODEL_UBX_CFG_VALGET_Size_FOUR	= 0x04,
    YODEL_UBX_CFG_VALGET_Size_EIGHT	= 0x05,
};

/**
 * UBX configuration key identifiers are four bytes little endian.
 */
typedef uint32_t yodel_ubx_cfg_valget_key_t;

/**
 * Process a possible UBX-CFG-VALGET message. The buffer is passed as non-const
 * because the byte-swapping of the variable length payload, both key IDs and
 * their values, is performed in-place.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_cfg_valget(void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-MON-VER MESSAGES
 ******************************************************************************/

/**
 * UBX-MON-VER constants.
 */
enum YodelUbxMonVerConstants {
    YODEL_UBX_MON_VER_Class				= 0x0a,
    YODEL_UBX_MON_VER_Id				= 0x04,
    YODEL_UBX_MON_VER_swVersion_LENGTH	= 30,
    YODEL_UBX_MON_VER_hwVersion_LENGTH	= 10,
    YODEL_UBX_MON_VER_extension_LENGTH	= 30,
};

/**
 * Process a possible UBX-MON-VER message. The UBX-MON-VER message is variable
 * length containing a variable number of character strings.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_mon_ver(const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-NAV-SVIN MESSAGES
 ******************************************************************************/

/**
 * UBX-NAV-SVIN (0x01, 0x3b) [40] indicates the state of the Survey-In,
 * typically by the stationary Base.
 */
typedef struct YodelUbxNavSvin {
    uint8_t version;
    uint8_t reserved[3];
    uint32_t iTOW;
    uint32_t dur;
    int32_t meanX;
    int32_t meanY;
    int32_t meanZ;
    int8_t meanXHP;
    int8_t meanYHP;
    int8_t meanZHP;
    int8_t reserved2[1];
    uint32_t meanAcc;
    int32_t obs;
    int8_t valid;
    int8_t active;
    int8_t reserved3[2];
} yodel_ubx_nav_svin_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_RXM_RTCM_INITIALIZER
 * Initialize a YodelUbxAck structure.
 */
#define YODEL_UBX_NAV_SVIN_INITIALIZER \
    { 0, }

/**
 * UBX-NAV-SVIN constants.
 */
enum YodelUbxNavSvinConstants {
    YODEL_UBX_NAV_SVIN_Class	= 0x01,
    YODEL_UBX_NAV_SVIN_Id		= 0x3b,
    YODEL_UBX_NAV_SVIN_Length	= 40,
};

/**
 * Process a possible UBX-NAV-SVIN message.
 * @param mp points to a UBX-NAV-SVIN structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_svin(yodel_ubx_nav_svin_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-RXM-RTCM MESSAGES
 ******************************************************************************/

/**
 * UBX-RXM-RTCM (0x02, 0x32) [8] indicates the reception of RTCM messages,
 * typically by the mobile Rover.
 * Ublox 9 R05, p. 181.
 */
typedef struct YodelUbxRxmRtcm {
    uint8_t version;		/* Message version. */
    uint8_t flags;			/* If true, crcFailed. */
    uint16_t subType;		/* Message sub type if RTCM 4072. */
    uint16_t refStation;	/* Reference station identification. */
    uint16_t msgType;		/* Message type. */
} yodel_ubx_rxm_rtcm_t __attribute__((aligned(2)));

/**
 * @def YODEL_UBX_RXM_RTCM_INITIALIZER
 * Initialize a YodelUbxRxmRtcm structure.
 */
#define YODEL_UBX_RXM_RTCM_INITIALIZER \
    { 0, }

/**
 * UBX-RXM-RTCM constants.
 */
enum YodelUbxRxmRtcmConstants {
    YODEL_UBX_RXM_RTCM_Class	= 0x02,
    YODEL_UBX_RXM_RTCM_Id		= 0x32,
    YODEL_UBX_RXM_RTCM_Length	= 8,
};

/**
 * Process a possible UBX-RXM-RTCM message.
 * @param mp points to a UBX-RXM-RTCM structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_rxm_rtcm(yodel_ubx_rxm_rtcm_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-MON-COMMS MESSAGES
 ******************************************************************************/

/**
 * UBX-MON-COMMS port indices.
 * Ublox 9 "Integration Manual" R05, p. 34..35.
 */
enum YodelUbxMonCommsPort {
    YODEL_UBX_MON_COMMS_PORT_FIRST  = 0,
    YODEL_UBX_MON_COMMS_PORT_I2C    = YODEL_UBX_MON_COMMS_PORT_FIRST,
    YODEL_UBX_MON_COMMS_PORT_UART1,
    YODEL_UBX_MON_COMMS_PORT_UART2,
    YODEL_UBX_MON_COMMS_PORT_USB,
    YODEL_UBX_MON_COMMS_PORT_SPI,
    YODEL_UBX_MON_COMMS_PORT_LAST   = YODEL_UBX_MON_COMMS_PORT_SPI,
};

/**
 * UBX-MON-COMMS (0x0A, 0x36) [8 + 40 * nPorts] reports the communication ports
 * utilization.
 * Ublox 9 "Interface Description" R07, p. 131..132.
 */
typedef struct YodelUbxMonComms {
    struct {
        uint8_t version;        /* Message version. */
        uint8_t nPorts;         /* Number of ports included. */
        uint8_t txErrors;       /* TX error bitmask. */
        uint8_t reserved1[1];   /* Reserved. */
        uint8_t protIds[4];     /* Protocol identifiers. */
    } prefix;
    struct {
        uint16_t portId;        /* Port identifier. */
        uint16_t txPending;     /* Number of bytes pending in TX buffer. */
        uint32_t txBytes;       /* Number of bytes ever sent. */
        uint8_t txUsage;        /* Percentage recent usage TX buffer. */
        uint8_t txPeakUsage;    /* Percentage maximum usage TX buffer. */
        uint16_t rxPending;     /* Number of bytes pending in RX buffer. */
        uint32_t rxBytes;       /* Number of bytes ever received. */
        uint8_t rxUsage;        /* Percentage recent usage RX buffer. */
        uint8_t rxPeakUsage;    /* Percentage maximum usage RX buffer. */
        uint16_t overrunErrs;   /* Number of 100ms timeslots with overrun. */
        uint16_t msgs[4];       /* Number of parsed message per protocol. */
        uint8_t reserved2[8];   /* Reserved. */
        uint32_t skipped;       /* Number of bytes skipped. */
    } port[YODEL_UBX_MON_COMMS_PORT_LAST + 1];
} yodel_ubx_mon_comms_t  __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_MON_COMMS_INITIALIZER
 * Initialize a YodelUbxMonComms structure.
 */
#define YODEL_UBX_MON_COMMS_INITIALIZER \
    { 0, }

/**
 * UBX-MON-COMMS port identifiers.
 * Ublox 9 "Integration Manual" R05, p. 34..35.
 */
enum YodelUbxMonCommsPortId {
    YODEL_UBX_MON_COMMS_PORTID_I2C      = 0x0000,
    YODEL_UBX_MON_COMMS_PORTID_UART1    = 0x0001,
    YODEL_UBX_MON_COMMS_PORTID_UART2    = 0x0102, /* (sic) */
    YODEL_UBX_MON_COMMS_PORTID_USB      = 0x0003,
    YODEL_UBX_MON_COMMS_PORTID_SPI      = 0x0004,
};

/**
 * UBX-MON-COMMS protocol identifiers.
 * Ublox 9 "Interface Description" R07, p. 131..132.
 */
enum YodelUbxMonCommsProtId {
    YODEL_UBX_MON_COMMS_PROTID_UBX    = 0,
    YODEL_UBX_MON_COMMS_PROTID_NMEA   = 1,
    YODEL_UBX_MON_COMMS_PROTID_RTCM2  = 2,
    YODEL_UBX_MON_COMMS_PROTID_RTCM3  = 5,
    YODEL_UBX_MON_COMMS_PROTID_NONE   = 256,
};

/**
 * UBX-MON-COMMS constants.
 */
enum YodelUbxMonCommsConstants {
    YODEL_UBX_MON_COMMS_Class	= 0x0a,
    YODEL_UBX_MON_COMMS_Id		= 0x36,
    YODEL_UBX_MON_COMMS_Length	= sizeof(((yodel_ubx_mon_comms_t *)0)->prefix), /* Minimum. */
};

/**
 * Process a possible UBX-MON-COMMS message.
 * @param mp points to a UBX-MON-COMMS structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return the number of ports processed, <0 otherwise.
 */
extern int yodel_ubx_mon_comms(yodel_ubx_mon_comms_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-NAV-ATT MESSAGES
 ******************************************************************************/

enum YodelUbxNavAttConstants {
    YODEL_UBX_NAV_ATT_Class				= 0x01,
    YODEL_UBX_NAV_ATT_Id				= 0x05,
    YODEL_UBX_NAV_ATT_Length			= 32,
};

/**
 * UBX-NAV-ATT (0x01, 0x05) [32] carries vehicle attitude.
 * Ublox 8 R19, p. 317.
 */
typedef struct YodelUbxNavAtt {
    uint32_t iTOW;          /* GPS Time Of Week. */
    uint8_t version;        /* Message version. */
    uint8_t reserved1[3];   /* Reserved. */
    int32_t roll;           /* Vehicle roll (1E-5 deg). */
    int32_t pitch;          /* Vehicle pitch (1E-5 deg). */
    int32_t heading;        /* Vehicle heading (1E-5 deg). */
    uint32_t accRoll;       /* Vehicle roll accuracy (1E-5 deg). */
    uint32_t accPitch;      /* Vehicle pitch accuracy (1E-5 deg). */
    uint32_t accHeading;    /* Vehicle heading accuracy (1E-5 deg). */
} yodel_ubx_nav_att_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_NAV_ATT_INITIALIZER
 * Initialize a YodelUbxNavAtt structure.
 */
#define YODEL_UBX_NAV_ATT_INITIALIZER \
    { 0, }

/**
 * Process a possible UBX-NAV-ATT message.
 * @param mp points to a UBX-NAV-ATT structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_att(yodel_ubx_nav_att_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-NAV-ODO MESSAGES
 ******************************************************************************/

enum YodelUbxNavOdoConstants {
    YODEL_UBX_NAV_ODO_Class				= 0x01,
    YODEL_UBX_NAV_ODO_Id				= 0x09,
    YODEL_UBX_NAV_ODO_Length			= 20,
};

/**
 * UBX-NAV-ODO (0x01, 0x09) [20] carries odometer solution.
 * Ublox 8 R19, p. 327.
 */
typedef struct YodelUbxNavOdo {
    uint8_t version;        /* Version. */
    uint8_t reserved1[3];   /* Reserved. */
    uint32_t iTOW;          /* GPS Time Of Week. */
    uint32_t distance;      /* Ground distance since last reset (meters). */
    uint32_t totalDistance; /* Total cumulative ground distance (meters). */
    uint32_t distanceStd;   /* Ground distance accuracy @ 1-sigma (meters). */
} yodel_ubx_nav_odo_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_NAV_ODO_INITIALIZER
 * Initialize a YodelUbxNavOdo structure.
 */
#define YODEL_UBX_NAV_ODO_INITIALIZER \
    { 0, }

/**
 * Process a possible UBX-NAV-ODO message.
 * @param mp points to a UBX-NAV-ODO structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_odo(yodel_ubx_nav_odo_t * mp, const void * bp, ssize_t length);

/*******************************************************************************
 * PROCESSING UBX-NAV-PVT MESSAGES
 ******************************************************************************/

enum YodelUbxNavPvtConstants {
    YODEL_UBX_NAV_PVT_Class				= 0x01,
    YODEL_UBX_NAV_PVT_Id				= 0x07,
    YODEL_UBX_NAV_PVT_Length			= 92,
};

/**
 * UBX-NAV-PVT (0x01, 0x07) [92] carries Position Velocity Time solution.
 * Ublox 8 R19, p. 332.
 */
typedef struct YodelUbxNavPvt {
    uint32_t iTOW;          /* GPS Time Of Week. */
    uint16_t year;          /* Year (UTC). */
    uint8_t month;          /* Month [1..12] (UTC). */
    uint8_t day;            /* Day of month [1..31] (UTC). */
    uint8_t hour;           /* Hour of day [0..23] (UTC). */
    uint8_t minute;         /* Minute of hour [0..59] (UTC). */
    uint8_t sec;            /* Second of minute [0..60] (UTC). */
    uint8_t valid;          /* Validity flags. */
    uint32_t tAcc;          /* Time accuracy estimate (UTC). */
    int32_t nano;           /* Fraction of a second [-1E9..+1E9] (UTC). */
    uint8_t fixType;        /* GNSSfix Type. */
    uint8_t flags;          /* Fix status flags. */
    uint8_t flags2;         /* Additional flags. */
    uint8_t numSV;          /* Number of satellites in solution. */
    int32_t lon;            /* Longitude (1e-7 deg). */
    int32_t lat;            /* Latitude (1e-7 deg). */
    int32_t height;         /* Height above ellipsoid (mm). */
    int32_t hMSL;           /* Height about MSL (mm). */
    uint32_t hAcc;          /* Horizontal accuracy estimate (mm). */
    uint32_t vAcc;          /* Vertical accuracy estimate (mm). */
    int32_t velN;           /* NED north velocity (mm/s). */
    int32_t velE;           /* NED east velocity (mm/s). */
    int32_t velD;           /* NED down velocity (mm/s). */
    int32_t gSpeed;         /* Ground speed 2-D (mm/s). */
    int32_t headMot;        /* Heading of motion 2-D (1E-5 deg). */
    uint32_t sAcc;          /* Speed accuracy estimate (mm/s). */
    uint32_t headAcc;       /* Heading accurach estimate (1E-5 deg). */
    uint16_t pDOP;          /* Poition Dilution Of Precision (0.01). */
    uint8_t flags3;         /* Additional flags (again). */
    uint8_t reserved1[5];   /* Reserved. */
    int32_t headVeh;        /* Heading of vehicle 2-D (1E-5 deg). */
    int16_t magDec;         /* Magnetic declination (1E-2 deg). */
    uint16_t magAcc;        /* Magnetic declination accuracy (1E-2 deg). */
} yodel_ubx_nav_pvt_t __attribute__((aligned(4)));

/**
 * @def YODEL_UBX_NAV_PVT_INITIALIZER
 * Initialize a YodelUbxNavPvt structure.
 */
#define YODEL_UBX_NAV_PVT_INITIALIZER \
    { 0, }

/**
 * UBX-NAV-PVT valid values.
 */
enum YodelUbxNavPvtValid {
    YODEL_UBX_NAV_PVT_valid_validMsg	    = 0x08,
    YODEL_UBX_NAV_PVT_valid_fullyResolved	= 0x04,
    YODEL_UBX_NAV_PVT_valid_validTime	    = 0x02,
    YODEL_UBX_NAV_PVT_valid_validDate	    = 0x01,
};

/**
 * UBX=NAV-PVT fixType values.
 */
enum YodelUbxNavPvtFixTypes {
    YODEL_UBX_NAV_PVT_fixType_noFix             = 0,
    YODEL_UBX_NAV_PVT_fixType_deadReckoningOnly = 1,
    YODEL_UBX_NAV_PVT_fixType_2D                = 2,
    YODEL_UBX_NAV_PVT_fixType_3D                = 3,
    YODEL_UBX_NAV_PVT_fixType_combined          = 4,
    YODEL_UBX_NAV_PVT_fixType_timeOnly          = 5,
};

/**
 * UBX-NAV-PVT flags values.
 */
enum YodelUbxNavPvtFlags {
    YODEL_UBX_NAV_PVT_flags_carrSoln	    = 0xc0,
    YODEL_UBX_NAV_PVT_flags_headVehValid	= 0x20,
    YODEL_UBX_NAV_PVT_flags_psmState	    = 0x1c,
    YODEL_UBX_NAV_PVT_flags_diffSoln	    = 0x02,
    YODEL_UBX_NAV_PVT_flags_gnssFixOK	    = 0x01,
};

/**
 * UBX-NAV-PVT flags2 values.
 */
enum YodelUbxNavPvtFlags2 {
    YODEL_UBX_NAV_PVT_flags2_confirmedTime  = 0x80,
    YODEL_UBX_NAV_PVT_flags2_confirmedDate	= 0x40,
    YODEL_UBX_NAV_PVT_flags2_confirmedAvai	= 0x20,
};

/**
 * UBX-NAV-PVT flags3 values.
 */
enum YodelUbxNavPvtFlags3 {
    YODEL_UBX_NAV_PVT_flags3_invalidLlh = 0x01,
};

/**
 * Process a possible UBX-NAV-PVT message.
 * @param mp points to a UBX-NAV-PVT structure in which to save the payload.
 * @param bp points to a buffer with a UBX header and payload.
 * @param length is the length of the header, payload, and checksum in bytes.
 * @return 0 if the message was valid, <0 otherwise.
 */
extern int yodel_ubx_nav_pvt(yodel_ubx_nav_pvt_t * mp, const void * bp, ssize_t length);

/******************************************************************************
 * ENDIAN CONVERSION
 ******************************************************************************/

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>

/**
 * @def COM_DIAG_YODEL_LETOH
 * Convert in-place variable @a _FIELD_ from Little Endian byte order to Host
 * byte order. The field width, 16, 32, or 64 bits, in inferred automatically.
 * The field must be appropriately aligned.
 */
#define COM_DIAG_YODEL_LETOH(_FIELD_) \
    do { \
        switch (sizeof(_FIELD_)) { \
        case sizeof(uint16_t): \
            _FIELD_ = le16toh(_FIELD_); \
            break; \
        case sizeof(uint32_t): \
            _FIELD_ = le32toh(_FIELD_); \
            break; \
        case sizeof(uint64_t): \
            _FIELD_ = le64toh(_FIELD_); \
            break; \
        default: \
            break; \
        } \
    } while (0)

/**
 * @def COM_DIAG_YODEL_HTOLE
 * Convert in-place variable @a _FIELD_ from Host byte order to Little Endian
 * byte order. The field width, 16, 32, or 64 bits, in inferred automatically.
 * The field must be appropriately aligned.
 */
#define COM_DIAG_YODEL_HTOLE(_FIELD_) \
    do { \
        switch (sizeof(_FIELD_)) { \
        case sizeof(uint16_t): \
            _FIELD_ = htole16(_FIELD_); \
            break; \
        case sizeof(uint32_t): \
            _FIELD_ = htole32(_FIELD_); \
            break; \
        case sizeof(uint64_t): \
            _FIELD_ = htole64(_FIELD_); \
            break; \
        default: \
            break; \
        } \
    } while (0)

#endif
