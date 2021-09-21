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
extern pthread_mutex_t Mutex;

/**
 * This is the Hazer time base frequency (typically one gigahertz).
 */
extern diminuto_sticks_t Frequency;

/**
 * This is the current system clock time. Unlike monotonic clock
 * time, this can change dynamicall via administrative action, by the
 * Network Time Protocol (NTP), or even by the insertion or deletion
 * of leap seconds.
 */
extern diminuto_sticks_t Clock;

/**
 * This is the initial monotonic clock time.
 */
extern diminuto_sticks_t Epoch;

/**
 * This is the monotonic clock time of the first fix.
 */
extern diminuto_sticks_t Fix;

/**
 * This is the current monotonic time.
 */
extern diminuto_sticks_t Now;

/**
 * This is the monotonic clock time of the last event.
 */
extern diminuto_sticks_t Event;

#endif
