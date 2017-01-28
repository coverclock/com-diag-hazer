/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_PRIVATE_
#define _H_COM_DIAG_HAZER_PRIVATE_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://www.diag.com/navigation/downloads/Hazer.html<BR>
 * These are the private definitions for the hazer API.
 */

#include <stdint.h>

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
typedef enum HazerStimulus {
    HAZER_STIMULUS_MINIMUM        = ' ',
    HAZER_STIMULUS_START          = '$',
    HAZER_STIMULUS_ENCAPSULATION  = '!',
    HAZER_STIMULUS_GNSS           = 'G',
    HAZER_STIMULUS_DELIMITER      = ',',
    HAZER_STIMULUS_TAG            = '\\',
    HAZER_STIMULUS_HEXADECIMAL    = '^',
    HAZER_STIMULUS_CHECKSUM       = '*',
    HAZER_STIMULUS_DECMIN         = '0',
    HAZER_STIMULUS_DECMAX         = '9',
    HAZER_STIMULUS_HEXMIN         = 'A',
    HAZER_STIMULUS_HEXMAX         = 'F',
    HAZER_STIMULUS_CR             = '\r',
    HAZER_STIMULUS_LF             = '\n',
    HAZER_STIMULUS_MAXIMUM        = '}',
    HAZER_STIMULUS_RESERVERED     = '~',
} hazer_stimulus_t;

/**
 * State machine actions
 */
typedef enum HazerAction {
    HAZER_ACTION_SKIP           = 0,
    HAZER_ACTION_SAVE,
    HAZER_ACTION_SAVESPECIAL,
    HAZER_ACTION_TERMINATE,
} hazer_action_t;

extern uint32_t hazer_fraction(char * string, uint32_t * numeratorp);

#endif
