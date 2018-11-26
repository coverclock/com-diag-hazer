/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_YODEL_
#define _H_COM_DIAG_YODEL_

/**
 * @file
 *
 * Copyright 2018 Digital Aggregates Corporation, Colorado, USA<BR>
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
	YODEL_UBX_LONGEST	= 512,	/* No clue what this should be. */
};

/**
 * This buffer is large enough to contain the largest UBX packet, plus a
 * trailing NUL, and then some. The NUL at the end is useless in the UBX binary
 * protocol, but is useful in some edge cases in which the data format has not
 * yet been determined (e.g. incoming UDP datagrams).
 */
typedef unsigned char (yodel_buffer_t)[YODEL_UBX_LONGEST + 1];

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
} yodel_ubx_header_t __attribute__ ((__aligned__(2)));

/**
 * Yodel state machine states. The only states the application needs
 * to take action on are START (to initialize the state), EOF (end of file
 * on the input stream), and END (complete NMEA sentence in buffer). The
 * rest are transitory states. If the machine transitions from a non_START
 * state to the START state, that means the framing of the current sentence
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
 * Yodel state machine stimuli.
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
 * @param ch is the next character from the NMEA sentence stream.
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
} yodel_ubx_mon_hw_t;

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

/**
 * Structure combining both a UBX-MON-HW payload and its expiry time in ticks.
 */
typedef struct YodelHardware {
	yodel_ubx_mon_hw_t payload;	/* Payload from UBX-MON-HW message. */
    uint8_t ticks;				/* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_hardware_t;

/*******************************************************************************
 * PROCESSING UBX-NAV_STATUS MESSAGES
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
} yodel_ubx_nav_status_t;

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

/**
 * Structure combining both a UBX-NAV-STATUS payload and its expiry time in ticks.
 */
typedef struct YodelStatus {
	yodel_ubx_nav_status_t payload;	/* Payload from UBX-NAV-STATUS message. */
    uint8_t ticks;					/* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_status_t;

#endif
