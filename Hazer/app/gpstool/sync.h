/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_SYNC_
#define _H_COM_DIAG_HAZER_GPSTOOL_SYNC_

/**
 * @file
 * @copyright Copyright 2021-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Sync API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/datagram.h"

/**
 * This is the size of the sync buffer used to hold incoming data which
 * is not part of a known frame of any supported protocol (NMEA, UBX,
 * etc.).
 */
static const size_t SYNC_SIZE = sizeof(datagram_payload_t);

/**
 * Add a character to the sync troubleshooting buffer when the input
 * stream is out of syncronization.
 * @param ch is a character from the input stream.
 */
extern void sync_out(int ch);

/**
 * Process the sync troubleshooting buffer when the input stream comes
 * back into synchronization.
 * @param length is the length of the data that brought the stream into sync.
 */
extern void sync_in(size_t length);

/**
 * Handle any final processing of the sync troubleshooting buffer at end of job.
 */
extern void sync_end(void);

#endif
