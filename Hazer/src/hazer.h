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

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
enum HazerNmeaCharacter {
    HAZER_NMEA_CHARACTER_MINIMUM        = ' ',
    HAZER_NMEA_CHARACTER_START          = '$',
    HAZER_NMEA_CHARACTER_ENCAPSULATION  = '!',
    HAZER_NMEA_CHARACTER_GNSS           = 'G',
    HAZER_NMEA_CHARACTER_DELIMITER      = ',',
    HAZER_NMEA_CHARACTER_TAG            = '\\',
    HAZER_NMEA_CHARACTER_HEXADECIMAL    = '^',
    HAZER_NMEA_CHARACTER_CHECKSUM       = '*',
    HAZER_NMEA_CHARACTER_DECMIN         = '0',
    HAZER_NMEA_CHARACTER_DECMAX         = '9',
    HAZER_NMEA_CHARACTER_HEXMIN         = 'A',
    HAZER_NMEA_CHARACTER_HEXMAX         = 'F',
    HAZER_NMEA_CHARACTER_CR             = '\r',
    HAZER_NMEA_CHARACTER_LF             = '\n',
    HAZER_NMEA_CHARACTER_MAXIMUM        = '}',
    HAZER_NMEA_CHARACTER_RESERVERED     = '~',
};

typedef enum HazerAction {
    HAZER_ACTION_SKIP           = 0,
    HAZER_ACTION_SAVE,
    HAZER_ACTION_SAVESPECIAL,
    HAZER_ACTION_TERMINATE,
} hazer_action_t;

#endif
