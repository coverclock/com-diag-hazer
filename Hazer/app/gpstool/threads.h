/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_THREADS_
#define _H_COM_DIAG_HAZER_GPSTOOL_THREADS_

/**
 * @file
 *
 * Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

/**
 * Implement a thread that polls for the data carrier detect (DCD) state for
 * 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
extern void * dcdpoller(void * argp);

/**
 * Implement a thread that polls for the general purpose input/output (GPIO)
 * state for 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
extern void * gpiopoller(void * argp);

#endif
