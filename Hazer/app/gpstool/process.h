/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_PROCESS_
#define _H_COM_DIAG_HAZER_GPSTOOL_PROCESS_

/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Process API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/yodel.h"

/**
 * Process a UBX-CFG-VALGET message from a buffer that has been converted
 * to host byte order in place.
 * @param buffer points to the buffer.
 * @param length is the number of payload bytes in the buffer.
 */
extern void process_ubx_cfg_valget(const void * buffer, ssize_t length);

/**
 * Process a UBX-MON-COMMS message from a buffer that has been converted
 * to host byte order in place.
 * @param buffer points to the buffer.
 * @param length is the number of payload bytes in the buffer.
 */
extern void process_ubx_mon_comms(const void * buffer, ssize_t length);

/**
 * Process a UBX-MON-VER message from a buffer that has been converted
 * to host byte order in place.
 * @param buffer points to the buffer.
 * @param length is the number of payload bytes in the buffer.
 */
extern void process_ubx_mon_ver(const void * buffer, ssize_t length);

#endif
