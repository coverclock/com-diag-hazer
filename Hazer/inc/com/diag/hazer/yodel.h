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
 * UBlox, p.73
 */
enum YodelUbx {
	YODEL_UBX_SYNC_1		= 0,	/* Always 0xb5. */
	YODEL_UBX_SYNC_2		= 1,	/* Always 0x62. */
	YODEL_UBX_CLASS			= 2,
	YODEL_UBX_ID			= 3,
	YODEL_UBX_LENGTH_LSB	= 4,	/* 16-bit, little endian (LSB). */
	YODEL_UBX_LENGTH_MSB 	= 5,	/* 16-bit, little endian (MSB). */
	/* ... */						/* Payload[LENGTH]. */
	YODEL_UBX_CK_A			= 6,	/* Only if no LENGTH == 0. */
	YODEL_UBX_CK_B			= 7,	/* Only if no LENGTH == 0. */
	YODEL_UBX_UNSUMMED		= 2,	/* SYNC1[1], SYNC2[1] */
	YODEL_UBX_SUMMED		= 4,	/* CLASS[1], ID[1], LENGTH[2] ... */
	YODEL_UBX_SHORTEST		= 8,	/* UNSUMMED[2], SUMMED[4], CK_A[1], CK_B[1] */
	YODEL_UBX_LONGEST		= 512,	/* No clue what this should be. */
};

/**
 * UBX state machine states. The only states the application needs
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
 * This buffer is large enough to contain the largest UBX packet,
 * plus a trailing NUL (and then some), aligned so that we can lay
 * a UBX structure on top of it.
 */
typedef unsigned char (yodel_buffer_t)[YODEL_UBX_LONGEST + 1]  __attribute__ ((aligned (8))); /* plus NUL */

/**
 * This is the structure of the header on every UBX packet.
 */
typedef struct YodelHeader {
	uint8_t yodel_sync_1;		/* 0xb5 */
	uint8_t yodel_sync_2;		/* 0x62 */
	uint8_t yodel_class;
	uint8_t yodel_id;
	uint16_t yodel_length;		/* Little endian. */
	uint8_t yodel_payload[0];
} yodel_header_t;

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

#endif
