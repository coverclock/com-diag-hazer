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
 *
 * This file is not in the inc or src directory because Hazer has a rule
 * about not creating a dependency to Diminuto in the resulting archive or
 * shared object.
 *
 * It turns out to be remarkably difficult to solve the sequence number
 * wrap (roll over) problem for the general case. This code borrows from
 * other applications without implementing the full blown Protection Against
 * Wrapped Sequence numbers (PAWS) algorithm used by TCP as described in
 * RFC1323.
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
 * Check to see if this datagram came out of order.
 * @param expectedp points to the expected sequence number.
 * @param header points to the datagram header.
 * @param length is the total number of received bytes including the header.
 * @param outoforderp points to the Out Of Order counter.
 * @param missingp points to the Missing counter.
 * @return the size of the actual payload of the buffer or <0 if out of order.
 */
static ssize_t validate_datagram(datagram_sequence_t * expectedp, datagram_header_t * header, ssize_t length, unsigned int * outoforderp, unsigned int * missingp)
{
	ssize_t result = -1;
	datagram_sequence_t actual = 0;
	datagram_sequence_t gap = 0;
	static const datagram_sequence_t THRESHOLD = ((datagram_sequence_t)1) << ((sizeof(datagram_sequence_t) * 8) - 1);

	// (EXPECTED < ACTUAL) if (0 < (ACTUAL - EXPECTED) < THRESHOLD)

	actual = ntohl(header->sequence);
	if (actual == *expectedp) {
		*expectedp = *expectedp + 1;
		result = length - sizeof(datagram_header_t);
	} else {
		gap = actual - *expectedp;
		if (gap < THRESHOLD) {
			*missingp += gap;
			*expectedp = actual + 1;
			result = length - sizeof(datagram_header_t);
		} else {
			*outoforderp += 1;
		}
	}

	return result;
}

/**
 * Generate a sequence number and store it in the sequence field of the
 * datagram and update the expected sequence number.
 * @param buffer points to the datagram buffer.
 * @param expectedp points to the expected sequence number.
 */
static void stamp_datagram(datagram_header_t * buffer, datagram_sequence_t * expectedp)
{
	buffer->sequence = htonl(*expectedp);
	*expectedp += 1;
}

#endif
