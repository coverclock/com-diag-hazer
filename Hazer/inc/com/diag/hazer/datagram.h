/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_DATAGRAM_
#define _H_COM_DIAG_HAZER_DATAGRAM_

/**
 * @file
 * @copyright Copyright 2019-2022 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Common facilities for using datagrams.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * The Datagrams module provides some common facilities for dealing
 * with datagrams used for transmitting NMEA or RTK data between
 * instances of gpstool.
 *
 * This file has some definitions common to both gpstool and rtktool that
 * are pertinent to handling datagrams.
 *
 * It turns out to be remarkably difficult to solve the sequence number
 * wrap (roll over) problem for the general case. This code borrows from
 * other applications without implementing the full blown Protection Against
 * Wrapped Sequence numbers (PAWS) algorithm used by TCP as described in
 * RFC1323.
 */

#include <stdint.h>
#include <stddef.h>
#include <arpa/inet.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/calico.h"

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
 * This is the union of all of the possible protocol buffers. It is used
 * solely to compute the datagram buffer length below.
 */
union DatagramAny {
    hazer_buffer_t n;
    yodel_buffer_t u;
    tumbleweed_buffer_t r;
    calico_buffer_t d;
};

/**
 * This is mostly just so the initializer zeros everything.
 */
enum {
    DATAGRAM_SIZE = sizeof(union DatagramAny),
};

/**
 * This buffer is large enough to the largest UDP datagram we are willing to
 * support, plus a trailing NUL. It's not big enough to hold any datagram
 * (that would be in the neighborhood of 65508 bytes). But it will for sure
 * hold a NMEA, UBX, RTCM, or CPO payload. It includes a leading sequence number
 * field that is transmitted over wire or air in network byte order. The
 * sequence number is uint32_t aligned, which Yodel/UBX cares about.
 */
typedef struct DatagramBuffer {
    datagram_header_t header;
    union {
        uint8_t data[DATAGRAM_SIZE + 1];
        hazer_buffer_t nmea;
        yodel_buffer_t ubx;
        tumbleweed_buffer_t rtcm;
        calico_buffer_t cpo;
    } payload;
} datagram_buffer_t;

/**
 * @def DATAGRAM_BUFFER_INITIALIZER
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
ssize_t datagram_validate(datagram_sequence_t * expectedp, datagram_header_t * header, ssize_t length, unsigned int * outoforderp, unsigned int * missingp);

/**
 * Generate a sequence number and store it in the sequence field of the
 * datagram and update the expected sequence number.
 * @param buffer points to the datagram buffer.
 * @param expectedp points to the expected sequence number.
 */
void datagram_stamp(datagram_header_t * buffer, datagram_sequence_t * expectedp);

#endif
