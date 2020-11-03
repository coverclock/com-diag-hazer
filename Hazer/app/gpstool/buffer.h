/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_BUFFER_
#define _H_COM_DIAG_HAZER_GPSTOOL_BUFFER_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Buffer API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>

/**
 * Write a buffer to the specified stream.
 * @param fp points to the output stream.
 * @param buffer points to the buffer.
 * @param size is the number of bytes to write.
 */
extern void write_buffer(FILE * fp, const void * buffer, size_t size);

/**
 * Print an NMEA sentence or UBX message to a stream, expanding non-printable
 * characters into escape sequences.
 * @param fp points to the FILE stream.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 * @param limit is the maximum number of characters to display.
 */
extern void print_buffer(FILE * fp, const void * buffer, size_t size, size_t limit);

/**
 * Print an NMEA sentence or UBX message to a stream, expanding all
 * characters into hexadecimal escape sequences.
 * @param fp points to the FILE stream.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 */
extern void dump_buffer(FILE * fp, const void * buffer, size_t size);

#endif
