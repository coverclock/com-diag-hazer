/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_LOG_
#define _H_COM_DIAG_HAZER_GPSTOOL_LOG_

/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Print API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "com/diag/diminuto/diminuto_types.h"
#include "types.h"

/**
 * Log a fault reported with the NMEA GBS sentence.
 * @param tp points to the fault structure.
 */
extern void log_fault(const hazer_fault_t * tp);

/**
 * Log an errno error message using data in a buffer minus the CR and LF
 * end matter.
 * @param file is the file name (typically __FILE__).
 * @param line is the line number (typically __LINE__).
 * @param buffer points to the buffer.
 * @param length is the buffer data length in bytes.
 */
extern void log_error_f(const char * file, int line, const void * buffer, ssize_t length);

/**
 * @def log_error
 * Calls log_error_f with __FILE__ and __LINE__.
 */
#define log_error(_BUFFER_, _LENGTH_) log_error_f(__FILE__, __LINE__, _BUFFER_, _LENGTH_)

#endif
