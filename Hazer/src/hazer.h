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
 * @def HAZER_NMEA_SENTENCE_START
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 */
#define HAZER_NMEA_SENTENCE_START "$"

/**
 * @def HAZER_NMEA_GPS_TALKER
 * NMEA 0183 4.10, 6.1.4, Table 6
 */
#define HAZER_NMEA_GPS_TALKER "GP"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GGA
 * SiRF NMEA, Table 1-2, Time, position, and fix type data
 */
#define HAZER_NMEA_GPS_MESSAGE_GGA "GGA"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GLL
 * SiRF NMEA, Table 1-2, Latitude, longitude, UTC time of position fix and status
 */
#define HAZER_NMEA_GPS_MESSAGE_GLL "GLL"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GSA
 * SiRF NMEA, Table 1-2, GPS receiver operating mode, satellites used, DOP values
 */
#define HAZER_NMEA_GPS_MESSAGE_GSA "GSA"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_GSV
 * SiRF NMEA, Table 1-2, Number of satellites in view, satellites used, DOP values
 */
#define HAZER_NMEA_GPS_MESSAGE_GSV "GSV"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_MSS
 * SiRF NMEA, Table 1-2, SNR, signal strength, frequency, bit rate from beacon
 */
#define HAZER_NMEA_GPS_MESSAGE_MSS "MSS"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_RMC
 * SiRF NMEA, Table 1-2, Time, date position, course, and speed data
 */
#define HAZER_NMEA_GPS_MESSAGE_RMC "RMC"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_VTG
 * SiRF NMEA, Table 1-2, Course and speed relative to ground
 */
#define HAZER_NMEA_GPS_MESSAGE_VTG "VTG"

/**
 * @def HAZER_NMEA_GPS_MESSAGE_ZDA
 * SiRF NMEA, Table 1-2, PPS timing message
 */
#define HAZER_NMEA_GPS_MESSAGE_ZDA "ZDA"

#endif
