/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_NMEA_GPS_GPS_
#define _H_COM_DIAG_HAZER_NMEA_GPS_GPS_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://www.diag.com/navigation/downloads/Hazer.html<BR>
 */

/**
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
static const char HAZER_NMEA_GPS_TALKER[]   = "GP";

/**
 * GP-2106, 2.1, Table 2-1, Global positioning system fixed data
 */
static const char HAZER_NMEA_GPS_MESSAGE_GGA[]  = "GGA";

/**
 * GP-2106, 2.1, Table 2-1, Geographic position latitude / longitude
 */
static const char HAZER_NMEA_GPS_MESSAGE_GLL[]  = "GLL";

/**
 * GP-2106, 2.1, Table 2-1, GNSS DOP and active satellites
 */
static const char HAZER_NMEA_GPS_MESSAGE_GSA[]  = "GSA";

/**
 * GP-2106, 2.1, Table 2-1, GNSS satellites in view
 */
static const char HAZER_NMEA_GPS_MESSAGE_GSV[]  = "GSV";

/**
 * GP-2106, 2.1, Table 2-1, Recommended minimum specific GNSS data
 */
static const char HAZER_NMEA_GPS_MESSAGE_RMC[]  = "RMC";

/**
 * GP-2106, 2.1, Table 2-1, Course over ground and ground speed
 */
static const char HAZER_NMEA_GPS_MESSAGE_VTG[]  = "VTG";

#endif
