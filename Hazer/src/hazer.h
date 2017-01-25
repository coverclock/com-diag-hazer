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
 * Lengths
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 */
static size_t HAZER_NMEA_LENGTH_MINIMUM = 11;

/*******************************************************************************
 * Characters
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_CR             = '\r';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_LF             = '\n';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_START          = '$';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_CHECKSUM       = '*';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_DELIMITER      = ',';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_ENCAPSULATE    = '!';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_TAG            = '\\';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_HEXADECIMAL    = '^';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_MINIMUM        = ' ';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_MAXIMUM        = '}';

/**
 * NMEA 0183 4.10, 6.1.1, Table 3
 */
static const int HAZER_NMEA_SENTENCE_RESERVERD      = '~';

/*******************************************************************************
 * Talkers
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
static const char HAZER_NMEA_TALKER_GPS[]   = "GP";

/*******************************************************************************
 * Messages
 ******************************************************************************/

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_GGA[]  = "GGA";

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_GLL[]  = "GLL";

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_GSA[]  = "GSA";

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_GSV[]  = "GSV";

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_RMC[]  = "RMC";

/**
 * GP-2106, 2.1, Table 2-1
 */
static const char HAZER_NMEA_MESSAGE_VTG[]  = "VTG";

#endif
