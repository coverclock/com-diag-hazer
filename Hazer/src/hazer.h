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
 */

#include <stddef.h>

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

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

/*******************************************************************************
 * Constants
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
static const char HAZER_NMEA_TALKER_GPS[]   = "GP";

/**
 * GP-2106, 2.1, Table 2-1, Global positioning system fixed data
 */
static const char HAZER_NMEA_MESSAGE_GGA[]  = "GGA";

/**
 * GP-2106, 2.1, Table 2-1, Geographic position latitude / longitude
 */
static const char HAZER_NMEA_MESSAGE_GLL[]  = "GLL";

/**
 * GP-2106, 2.1, Table 2-1, GNSS DOP and active satellites
 */
static const char HAZER_NMEA_MESSAGE_GSA[]  = "GSA";

/**
 * GP-2106, 2.1, Table 2-1, GNSS satellites in view
 */
static const char HAZER_NMEA_MESSAGE_GSV[]  = "GSV";

/**
 * GP-2106, 2.1, Table 2-1, Recommended minimum specific GNSS data
 */
static const char HAZER_NMEA_MESSAGE_RMC[]  = "RMC";

/**
 * GP-2106, 2.1, Table 2-1, Course over ground and ground speed
 */
static const char HAZER_NMEA_MESSAGE_VTG[]  = "VTG";

#endif
