/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Datagram module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/datagram.h"

/**
 * Check to see if this datagram came out of order.
 * @param expectedp points to the expected sequence number.
 * @param header points to the datagram header.
 * @param length is the total number of received bytes including the header.
 * @param outoforderp points to the Out Of Order counter.
 * @param missingp points to the Missing counter.
 * @return the size of the actual payload of the buffer or <0 if out of order.
 */
ssize_t datagram_validate(datagram_sequence_t * expectedp, datagram_header_t * header, ssize_t length, unsigned int * outoforderp, unsigned int * missingp)
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
void datagram_stamp(datagram_header_t * buffer, datagram_sequence_t * expectedp)
{
    buffer->sequence = htonl(*expectedp);
    *expectedp += 1;
}
