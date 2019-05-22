/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_YODEL_
#define _H_COM_DIAG_YODEL_

/**
 * @file
 *
 * Copyright 2018-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 * Hazer is a simple C-based parser of the National Marine Electronics
 * Association (NMEA) strings produced by typical consumer GPS devices.
 * Yodel is a C-based parser for the UBX binary packet format that is optionally
 * produced by GPS devices manufactured by ublox AG. Yodel can be used in
 * parallel with Hazer for devices that produce both output formats in the same
 * data stream. (ublox is based in Switzerland; yodeling was a style of singing
 * introduced to Western U.S. culture by cowboys who immigrated from there.)
 *
 * REFERENCES
 *
 * "u-blox 7 Receiver Description Including Protocol Specification V14",
 * GPS.G7-SW-12001-B, ublox AG, 2013
 *
 * "u-blox 8 / u-blox M8 Receiver Description Including Protocol Specification",
 * UBX-13003221-R15, ublox AG, 2018-03-06
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
extern FILE * yodel_debug(FILE *now);

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
typedef unsigned char (yodel_buffer_t)[YODEL_UBX_LONGEST + 1];

/**
 * @define YODEL_BUFFER_INITIALIZER
 * Initialize a YodelBuffer type.
 */
#define YODEL_BUFFER_INITIALIZER  { '\0', }

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
    uint8_t class;
    uint8_t	id;
    uint16_t length;	/* little endian */
    uint8_t payload[0];
} yodel_ubx_header_t __attribute__((__aligned__(2)));

/**
 * @define YODEL_UBX_HEADER_INITIALIZER
 * Initialize a YodelUbxHeader structure.
 */
#define YODEL_UBX_HEADER_INITIALIZER \
    { \
	    0, 0, \
		0, \
		0, \
		0 \
    }

/**
 * UBX state machine states. The only states the application needs
 * to take action on are START (to initialize the state), EOF (end of file
 * on the input stream), and END (complete UBX packet in buffer). The
 * rest are transitory states. If the machine transitions from a non-START
 * state to the START state, that means the framing of the current packet
 * failed; that might be of interest to the application.
 */
typedef enum YodelState {
    YODEL_STATE_EOF					= 0,
    YODEL_STATE_START,
    YODEL_STATE_SYNC_2,
    YODEL_STATE_CLASS,
    YODEL_STATE_ID,
    YODEL_STATE_LENGTH_1,
    YODEL_STATE_LENGTH_2,
    YODEL_STATE_PAYLOAD,
    YODEL_STATE_CK_A,
    YODEL_STATE_CK_B,
    YODEL_STATE_END,
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
    YODEL_ACTION_SKIP               = 0,
    YODEL_ACTION_SAVE,
    YODEL_ACTION_TERMINATE,
} yodel_action_t;

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
 * @param bp points to a character pointer state variable of no initial value.
 * @param sp points to a size state variable of no initial value.
 * @param lp points to the length state variable of no initial value.
 * @return the next state of the machine.
 */
extern yodel_state_t yodel_machine(yodel_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp, size_t * lp);

/*******************************************************************************
 * VALIDATING A UBX PACKET
 ******************************************************************************/

/**
 * Compute the Fletcher checksum used by UBX for the specified buffer. The
 * buffer points to the beginning of the UBX packet, not to the subset that
 * is checksummed, and the sentence must contain a valid length field. A pointer
 * is returned pointing just past the checksummed portion; this is where the
 * checksum will be stored in a correctly formed packet.
 * @param buffer points to the beginning of the buffer.
 * @param size is the size of the buffer in bytes.
 * @param ck_ap points to where the ck_a value will be stored.
 * @param ck_bp points to where the ck_b value will be stored.
 * @return a pointer just past the end of the checksummed portion, or NULL if an error occurred.
 */
extern const void * yodel_checksum(const void * buffer, size_t size, uint8_t * ck_ap, uint8_t * ck_bp);

/**
 * Return the length of the completed packet in bytes.
 * @param buffer points to buffer containing the completed packet.
 * @param size is the size of the buffer containing the packet.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t yodel_length(const void * buffer, size_t size);

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
 * @define YODEL_SYSTEM_NAME_INITIALIZER
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
 * @define YODEL_UBX_NAV_HPPOSLLH_INITIALIZER
 * Initialize a YodelUbxNavHpposllh structure.
 */
#define YODEL_UBX_NAV_HPPOSLLH_INITIALIZER \
    { \
        0, \
        { 0, }, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
        0, \
    }

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
 * @define YODEL_UBX_MON_HW_INITIALIZER
 * Initialize a YodelUbxMonHw structure.
 */
#define YODEL_UBX_MON_HW_INITIALIZER \
    { \
	    0, 0, 0, 0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		{ 0, }, \
		0, \
		{ 0, }, \
		0, \
		0, 0 \
    }

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
 * @define YODEL_UBX_NAV_STATUS_INITIALIZER
 * Initialize a YodelUbxNavStatus structure.
 */
#define YODEL_UBX_NAV_STATUS_INITIALIZER \
    { \
	    0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0 \
    }

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
enum YodelUbxNavStatusFLags2SpoolDetState {
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
 * @define YODEL_UBX_ACK_INITIALIZER
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
 * @define YODEL_UBX_CFG_VALGET_INITIALIZER
 * Initialize the fixed portion of a YodelUbxCfgValget structure.
 */
#define YODEL_UBX_CFG_VALGET_INITIALIZER \
    { \
		0, \
		0, \
		{ 0, }, \
    }

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
	int32_t meanAcc;
	int32_t obs;
	int8_t valid;
	int8_t active;
	int8_t reserved3[2];
} yodel_ubx_nav_svin_t __attribute__((aligned(4)));

/**
 * @define YODEL_UBX_RXM_RTCM_INITIALIZER
 * Initialize a YodelUbxAck structure.
 */
#define YODEL_UBX_NAV_SVIN_INITIALIZER \
    { \
		0, \
		{ 0, }, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		0, \
		{ 0, }, \
		0, \
		0, \
		0, \
		0, \
		{ 0, }, \
    }

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
 * @define YODEL_UBX_RXM_RTCM_INITIALIZER
 * Initialize a YodelUbxAck structure.
 */
#define YODEL_UBX_RXM_RTCM_INITIALIZER \
    { \
		0, \
		0, \
		0, \
		0, \
		0, \
    }

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

/******************************************************************************
 * ENDIAN CONVERSION
 ******************************************************************************/

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>

/**
 * @def COM_DIAG_YODEL_LETOH
 * Convert in-place variable @a _FIELD_ from Little Endian byte order TO Host
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

#endif
