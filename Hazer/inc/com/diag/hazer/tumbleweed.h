/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_TUMBLEWEED_
#define _H_COM_DIAG_TUMBLEWEED_

/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 * Hazer is a simple C-based parser of the National Marine Electronics
 * Association (NMEA) strings produced by typical consumer GPS devices.
 * Tumbleweed is a C-based parser for the RTCM binary message format that is
 * produced by devices supporting Differential GNSS (DGNSS) using Real-Time
 * Kinematics (RTK).
 *
 * REFERENCES
 *
 * RTCM 10403.3, "Differential GNSS (Global Navigation Satellite Systems)
 * Services - Version 3", 141-2016-SC104-STD, 2016-10-07
 *
 * RTCM 10410.1, "Networked Transport of RTCM via Internet Protocol (NTRIP) -
 * Version 2.0", 111-2009-SC104-STD + 139-2011-SC104-STD, 2011-06-28
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
extern FILE * tumbleweed_debug(FILE *now);

/*******************************************************************************
 * STARTING UP AND SHUTTING DOWN
 ******************************************************************************/

/**
 * Perform any necessary initialization.
 * @return 0 for success, <0 otherwise.
 */
extern int tumbleweed_initialize(void);

/**
 * Perform any necessary finalization.
 * @return 0 for success, <0 otherwise.
 */
extern int tumbleweed_finalize(void);

/*******************************************************************************
 * COLLECTING A RTCM PACKET
 ******************************************************************************/

/**
 * Tumbleweed RTCM constants.
 * RTCM 10403.3 p. 263-265
 */
enum TumbleweedRtcmConstants {
	TUMBLEWEED_RTCM_SHORTEST	= 6,	/* Preamble[8b] + Zeros[6b] + Length[10b] + CRC[24b]. */
	TUMBLEWEED_RTCM_UNSUMMED	= 3,	/* CRC24Q[0], CRC24Q[1], CRC24Q[2]. */
	TUMBLEWEED_RTCM_SUMMED		= 3,	/* Preamble[8b] + Zeros[6b] + Length[10b]. */
	TUMBLEWEED_RTCM_CRC			= 3,	/* CRC24Q[0], CRC24Q[1], CRC24Q[2]. */
	TUMBLEWEED_RTCM_LENGTH		= 2,	/* Length[10b]. */
	TUMBLEWEED_RTCM_NUMBER		= 2,	/* Number[12b]. */
	TUMBLEWEED_RTCM_LONGEST		= 1029,	/* Shortest + Length=0x03FF. */
};

/**
 * This buffer is large enough to contain the largest RTCM messsage, plus a
 * trailing NUL. The NUL at the end is useless in the RTCM binary
 * protocol, but is useful in some edge cases in which the data format has not
 * yet been determined (e.g. incoming UDP datagrams).
 */
typedef unsigned char (tumbleweed_buffer_t)[TUMBLEWEED_RTCM_LONGEST + 1];

/**
 * @define TUMBLEWEED_BUFFER_INITIALIZER
 * Initialize a TumbleweedBuffer type.
 */
#define TUMBLEWEED_BUFFER_INITIALIZER  { '\0', }

/**
 * Tumbleweed RTCM offsets.
 * RTCM 10403.3 p. 263-265
 */
enum TumbleweedRtcmOffsets {
    TUMBLEWEED_RTCM_PREAMBLE	= 0,	/* always 0b11010011 = 0xd3. */
	TUMBLEWEED_RTCM_LENGTH_MSB	= 1,	/* Zeros[6b], Length[10b] MSB. */
	TUMBLEWEED_RTCM_LENGTH_LSB	= 2,	/* Zeros[6b], Length[10b] LSB. */
	TUMBLEWEED_RTCM_NUMBER_MSB	= 3,	/* Number[12b] MSB. */
	TUMBLEWEED_RTCM_NUMBER_LSB	= 4,	/* Number[12b] LSB. */
};

/**
 * Tumbleweed RTCM Masks.
 * RTCM 10403.3 p. 263-265
 */
enum TumbleweedRtcmMasks {
	TUMBLEWEED_RTCM_MASK_RESERVED	= 0xfc00,	/* Zeros[6b]. */
	TUMBLEWEED_RTCM_MASK_LENGTH		= 0x03ff,	/* Length[10b]. */
	TUMBLEWEED_RTCM_MASK_NUMBER		= 0xfff0,	/* Number[12b]. */
};

/**
 * Tumbleweed RTCM Shifts.
 * RTCM 10403.3 p. 263-265
 */
enum TumbleweedRtcmShifts {
	TUMBLEWEED_RTCM_SHIFT_RESERVED	= 10,
	TUMBLEWEED_RTCM_SHIFT_LENGTH	= 0,
	TUMBLEWEED_RTCM_SHIFT_NUMBER	= 4,
};

/**
 * RTCM state machine states. The only states the application needs
 * to take action on are START (to initialize the state), EOF (end of file
 * on the input stream), and END (complete RTCM message in buffer). The
 * rest are transitory states. If the machine transitions from a non-START
 * state to the START state, that means the framing of the current message
 * failed; that might be of interest to the application.
 */
typedef enum TumbleweedState {
    TUMBLEWEED_STATE_EOF					= 0,
    TUMBLEWEED_STATE_START,
    TUMBLEWEED_STATE_LENGTH_1,
    TUMBLEWEED_STATE_LENGTH_2,
    TUMBLEWEED_STATE_PAYLOAD,
    TUMBLEWEED_STATE_CRC_1,
    TUMBLEWEED_STATE_CRC_2,
	TUMBLEWEED_STATE_CRC_3,
    TUMBLEWEED_STATE_END,
} tumbleweed_state_t;

/**
 * RTCM state machine stimuli.
 */
enum TumbleweedStimulus {
    TUMBLEWEED_STIMULUS_PREAMBLE		= 0xd3,
	TUMBLEWEED_STIMULUS_RESERVED		= 0x00,
};

/**
 * UBX state machine actions.
 */
typedef enum TumbleweedAction {
    TUMBLEWEED_ACTION_SKIP               = 0,
    TUMBLEWEED_ACTION_SAVE,
    TUMBLEWEED_ACTION_TERMINATE,
} tumbleweed_action_t;

/**
 * Process a single character of stimulus for the state machine that is
 * assembling a single RTCM message in the caller provided buffer. State
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
 * @param ch is the next character from the RTCM message stream.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param bp points to a character pointer state variable of no initial value.
 * @param sp points to a size state variable of no initial value.
 * @param lp points to the length state variable of no initial value.
 * @return the next state of the machine.
 */
extern tumbleweed_state_t tumbleweed_machine(tumbleweed_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp, size_t * lp);

/*******************************************************************************
 * VALIDATING AN RTCM MESSAGE
 ******************************************************************************/

/**
 * This is the cyclic redundancy check table generated from the CRC-24Q
 * polynomial. It is exposed for unit testing.
 *
 * p(X) = X^23 + X^17 + X^13 + X^12 + X^11 + X^9 + X^8 + X^7 + X^5 + X^3 + 1
 *
 * RTCM 10403.3, p. 264
 */
extern const uint32_t TUMBLEWEED_CRC24Q[256];

/**
 * Compute the CRC-24Q used by RTCM for the specified buffer. The
 * buffer points to the beginning of the UBX packet, not to the subset that
 * is CRCed, and the sentence must contain a valid length field. A pointer
 * is returned pointing just past the CRCed portion; this is where the
 * CRC-24Q will be stored in a correctly formed packet.
 * @param buffer points to the beginning of the buffer.
 * @param size is the size of the buffer in bytes.
 * @param crc_1p points to where the CRC-24Q[0] value will be stored.
 * @param crc_2p points to where the CRC-24Q[1] value will be stored.
 * @param crc_3p points to where the CRC-24Q[2] value will be stored.
 * @return a pointer just past the end of the checksummed portion, or NULL if an error occurred.
 */
extern const void * tumbleweed_crc24q(const void * buffer, size_t size, uint8_t * crc_1p, uint8_t * crc_2p, uint8_t * crc_3p);

/**
 * Return the length of the completed message in bytes.
 * @param buffer points to buffer containing the completed packet.
 * @param size is the size of the buffer containing the packet.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t tumbleweed_length(const void * buffer, size_t size);

/**
 * Return the RTCM message number from the completed message. It will be in the
 * range 0..4095. Not all values are currently defined to be valid messages.
 * 0..100 are defined to be experimental messages. 4001..4095 are defined to
 * be proprietary messages. Production messages are in the range 1001..1230
 * with some omissions.
 * @param buffer points to buffer containing the completed packet.
 * @param size is the size of the buffer containing the packet.
 * @return the message number or <0 if an error occurred.
 */
extern int tumbleweed_message(const void * buffer, size_t size);

/*******************************************************************************
 * KEEPALIVE RTCM MESSAGE
 ******************************************************************************/

extern const uint8_t TUMBLEWEED_KEEPALIVE[6];

/******************************************************************************
 * ENDIAN CONVERSION
 ******************************************************************************/

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>

/**
 * @def COM_DIAG_TUMBLEWEED_BETOH
 * Convert in-place variable @a _FIELD_ from Big Endian byte order to Host
 * byte order. The field width, 16, 32, or 64 bits, in inferred automatically.
 * The field must be appropriately aligned.
 */
#define COM_DIAG_TUMBLEWEED_BETOH(_FIELD_) \
    do { \
        switch (sizeof(_FIELD_)) { \
        case sizeof(uint16_t): \
            _FIELD_ = be16toh(_FIELD_); \
            break; \
        case sizeof(uint32_t): \
            _FIELD_ = be32toh(_FIELD_); \
            break; \
        case sizeof(uint64_t): \
            _FIELD_ = be64toh(_FIELD_); \
            break; \
        default: \
            break; \
        } \
    } while (0)

#endif
