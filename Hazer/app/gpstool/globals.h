/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_GLOBALS_
#define _H_COM_DIAG_HAZER_GPSTOOL_GLOABLS_

/**
 * @file
 * @copyright Copyright 2019-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Globals.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <sys/types.h>
#include <pthread.h>
#include "com/diag/diminuto/diminuto_types.h"

/**
 * This is our program name as provided by the run-time system.
 */
extern const char * Program;

/**
 * This is our host name as provided by the run-time system.
 */
extern char Hostname[9];

/**
 * This is our process identifier.
 */
extern pid_t Process;

/**
 * This is the path name to the serial device we are reading from
 * or writing to.
 */
extern const char * Device;

/**
 * This is our POSIX thread mutual exclusion semaphore.
 */
extern pthread_mutex_t mutex;

/**
 * THis is the initial system (not GPS) time.
 */
extern diminuto_sticks_t Epoch;

/**
 * THis is the current system (not GPS) time.
 */
extern diminuto_sticks_t Now;

#endif
