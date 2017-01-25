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

/*
 * NMEA 3.0, 6.1.1, Table 3
 */

static const int HAZER_NMEA_SENTENCE_CR             = '\r';
static const int HAZER_NMEA_SENTENCE_LF             = '\n';
static const int HAZER_NMEA_SENTENCE_START          = '$';
static const int HAZER_NMEA_SENTENCE_CHECKSUM       = '*';
static const int HAZER_NMEA_SENTENCE_DELIMITER      = ',';
static const int HAZER_NMEA_SENTENCE_ENCAPSULATE    = '!';
static const int HAZER_NMEA_SENTENCE_TAG            = '\\';
static const int HAZER_NMEA_SENTENCE_HEXADECIMAL    = '^';
static const int HAZER_NMEA_SENTENCE_MINIMUM        = ' ';
static const int HAZER_NMEA_SENTENCE_MAXIMUM        = '}';
/*               HAZER_NMEA_SENTENCE_RESERVED       = '~'; */

/*
 * NMEA 3.0, 6.1.4, Table 6
 */

static const char HAZER_NMEA_TALKER_GPS[]   = "GP";

/*
 * GP-2106, 2.1, Table 2-1
 *
 * GGA - global positioning system fixed data
 * GLL - geographic position latitute / longitude
 * GSA - GNSS DOP and active satellites
 * GSV - GNSS satellites in view
 * RMC - recommended minimum specific GNSS data
 * VTG - course over ground and ground speed
 */

static const char HAZER_NMEA_MESSAGE_GGA[]  = "GGA";
static const char HAZER_NMEA_MESSAGE_GLL[]  = "GLL";
static const char HAZER_NMEA_MESSAGE_GSA[]  = "GSA";
static const char HAZER_NMEA_MESSAGE_GSV[]  = "GSV";
static const char HAZER_NMEA_MESSAGE_RMC[]  = "RMC";
static const char HAZER_NMEA_MESSAGE_VTG[]  = "VTG";

#endif
