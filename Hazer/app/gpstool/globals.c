/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Globals.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "globals.h"

const char * Program = (const char *)0;

char Hostname[9] = { '\0' };

pid_t Process = 0;

const char * Device = "stdin";

pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;

diminuto_sticks_t Frequency = -1;

diminuto_sticks_t Clock = -1;

diminuto_sticks_t Epoch = -1;

diminuto_sticks_t Now = -1;

diminuto_sticks_t Fix = -1;

diminuto_sticks_t First = -1;

diminuto_sticks_t Event = -1;
