/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_GLOBALS_
#define _H_COM_DIAG_HAZER_GPSTOOL_GLOABLS_

/**
 * @file
 *
 * Copyright 2019-2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
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
 * THis is the current system (not GPS) time.
 */
extern diminuto_sticks_t Now;

#endif
