/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_EMIT_
#define _H_COM_DIAG_HAZER_GPSTOOL_EMIT_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Emit API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "types.h"

/**
 * Emit an NMEA configuration sentence to the specified stream after adding the
 * end matter consisting of the checksum delimiter, the two checksum
 * characters, a carriage return, and a line feed.
 * @param fp points to the FILE stream.
 * @param sentence points to the sentence minus the ending matter but including the terminating nul.
 * @param size is the size of the NMEA sentence in bytes including the terminating nul.
 * @return total length emitted for success, <0 for error.
 */
extern ssize_t emit_sentence(FILE * fp, char * sentence, size_t size);

/**
 * Emit a UBX configuration packet to the specified stream after adding the
 * end matter consisting of the two Fletcher checksum bytes.
 * @param fp points to the FILE stream.
 * @param packet points to the packet minus the ending matter but including the terminating nul.
 * @param size is the size of the UBX packet in bytes including the terminating nul.
 * @return total length emitted for success, <0 for error.
 */
extern ssize_t emit_packet(FILE * fp, void * packet, size_t size);

/**
 * Emit configuration data to the specified stream exactly as is.
 * @param fp points to the FILE stream.
 * @param data points to the data including the terminating nul.
 * @param size is the size of the datum in bytes including the terminating nul.
 * @return total length emitted for success, <0 for error.
 */
extern ssize_t emit_data(FILE * fp, void * data, size_t size);

/**
 * Save the current PVT solution to the trace file in CSV format.
 * @param fp points to the FILE stream.
 * @param pa is the positions (NMEA) array.
 * @param sp points to the solution (UBX HPPOSLLH) structure.
 * @param ap points to the attitude (UBX UBXNAVATT) structure.
 * @param pp points to the PVT (UBX UBXPOSVELTIM) structure
 * @param bp points the DGNSS base (UBX UBXNAVSVIN) structure.
 */
extern void emit_trace(FILE * fp, const hazer_position_t pa[], const yodel_solution_t * sp, const yodel_attitude_t * ap, const yodel_posveltim_t * pp, const yodel_base_t * bp);

/**
 * If the caller has passed a valid file name, and the solution is not active
 * yet valid, emit the appropriate UBX messages minus checksums for feeding
 * this solution into this programming running in fixed mode.
 * @param arp points to a Antenna Reference Point file name.
 * @param bp points to a base structure.
 * @param sp points to a solution structure.
 * @return true if the solution was emitted, false otherwise.
 */
extern int emit_solution(const char * arp, const yodel_base_t * bp, const yodel_solution_t * sp);

#endif
