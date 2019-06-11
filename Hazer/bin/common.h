/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_COMMON_
#define _H_COM_DIAG_HAZER_COMMON_

/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 */

#include <arpa/inet.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_types.h"
#include "com/diag/diminuto/diminuto_log.h"

/*******************************************************************************
 * DATAGRAM BUFFER
 ******************************************************************************/

/**
 * This type contains the datagram sequence number. It is the same size
 * as the sequence numbers used in TCP headers.
 */
typedef uint32_t datagram_sequence_t;

/**
 * All UDP datagrams that this application sends or receives starts with
 * a header containing a thirty-two bit sequence number.
 */
typedef struct DatagramHeader {
	datagram_sequence_t sequence;
	uint8_t data[0];
} datagram_header_t;

/**
 * This is mostly just so the initializer zeros everything.
 */
enum {
    DATAGRAM_SIZE = (
    	(HAZER_NMEA_LONGEST > YODEL_UBX_LONGEST)
            ? ((HAZER_NMEA_LONGEST > TUMBLEWEED_RTCM_LONGEST)
                ? HAZER_NMEA_LONGEST
                : TUMBLEWEED_RTCM_LONGEST)
            : ((YODEL_UBX_LONGEST > TUMBLEWEED_RTCM_LONGEST)
                ? YODEL_UBX_LONGEST
                : TUMBLEWEED_RTCM_LONGEST)
    ),
};

/**
 * This buffer is large enough to the largest UDP datagram we are willing to
 * support, plus a trailing NUL. It's not big enough to hold any datagram
 * (that would be in the neighborhood of 65508 bytes). But it will for sure
 * hold a NMEA, UBX, or RTCM payload. It includes a leading sequence number
 * field that is transmitted over wire or air in network byte order.
 */
typedef struct DatagramBuffer {
	datagram_header_t header;
	union {
		uint8_t data[DATAGRAM_SIZE + 1];
		hazer_buffer_t nmea;
		yodel_buffer_t ubx;
		tumbleweed_buffer_t rtcm;
	} payload;
} datagram_buffer_t;

/**
 * @define DATAGRAM_BUFFER_INITIALIZER
 * Initialize a DatagramBuffer type.
 */
#define DATAGRAM_BUFFER_INITIALIZER  { { 0, }, { { '\0', } }, }

/**
 * Check to see if this datagram came out of order (it's okay if there are gaps
 * in the sequence, we expect that).
 * @param sequencep points to the expected sequence counter.
 * @param buffer points to the datagram buffer.
 * @param length is the number of bytes in the datagram buffer.
 * @return the size of the actual payload of the buffer or <0 if out of order.
 */
static inline ssize_t validate_datagram(datagram_sequence_t * sequencep, datagram_header_t * buffer, ssize_t length)
{
	ssize_t result = -1;
	datagram_sequence_t sequence = 0;

	if (length < sizeof(datagram_header_t)) {
		/* Do nothing. */
	} else if ((sequence = ntohl(buffer->sequence)) < *sequencep) {
		/* Do nothing. */
	} else {
		result = length - sizeof(datagram_header_t);
		*sequencep = sequence + 1;
	}

	return result;
}

/**
 * Generate a sequence number and store it in the sequence field of the
 * datagram and update the expected sequence counter.
 * @param buffer points to the datagram buffer.
 * @param sequencep points to the expected sequence counter.
 */
static inline void stamp_datagram(datagram_header_t * buffer, datagram_sequence_t * sequencep)
{
	buffer->sequence = htonl(*sequencep);
	*sequencep += 1;
}

#endif
