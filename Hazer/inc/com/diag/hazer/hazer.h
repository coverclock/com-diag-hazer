/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_
#define _H_COM_DIAG_HAZER_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://www.diag.com/navigation/downloads/Hazer.html<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 * Hazer is a simple C-based parser of the National Marine Electronics
 * Association (NMEA) strings produced by the USGlobalSat BU-353S4 Global
 * Positioning System (GPS) device, a tiny little external GPS receiver that
 * emits NMEA strings over its built-in USB-to-serial adaptor. The BU-353S4
 * is based on the SiRF Star IV chipset. If you want to futz around with
 * satellite geolocation, the BU-353S4 is a cheap and easy way to do it.
 *
 * REFERENCES
 *
 * "BU-353S4 GPS Receiver Data Sheet", BU353S4-DS08212013B, USGLobalSat Inc.,
 * 2013
 *
 * "GP-2106 SiRF Star IV GPS module with antenna", version 0.2, ADH Technology
 * Co. Ltd., 2010-12-08
 *
 * "SiRF Binary Protocol Reference Manual", revision 2.4, 1040-0041, SiRF
 * Technology, Inc., 2008-11
 *
 * "NMEA 0183 Standard for Interfacing Marine Electronic Devices", version 4.10,
 * NMEA 0183, National Marine Electronics Association, 2012-06
 */

#include <stdio.h>

/*******************************************************************************
 * Types
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef char (hazer_buffer_t)[82 + 1]; /* plus NUL */

/**
 * NMEA 0183 4.10, GGA, Global Positioning System Fix Data, p. 86-87
 */
typedef struct HazerNmeaGga {
    char        gga_name[3 + 1];
    float       gga_utc;
    float       gga_latitude;
    float       gga_longitude;
    float       gga_altitude;
    float       gga_hdop;
    float       gga_geoidal;
    float       gga_geoidal_units;
    float       gga_age;
    uint16_t    gga_station;
    uint8_t     gga_satellites;
    char        gga_quality;
    char        gga_altitude_units;
} hazer_nmea_gga_t;

/**
 * NMEA 0183 4.10, GGL, Geographic Position Latitude/Longitude, p. 87
 */
typedef struct HazerNmeaGll {
    char        ggl_name[3 + 1];
    float       ggl_latitude;
    float       ggl_longitude;
    float       ggl_utc;
    char        ggl_status;
    char        ggl_mode;
} hazer_nmea_gll_t;

/**
 * NMEA 0183 4.10, GSA, GNSS DOP and Active Satellites, p. 94-95
 */
typedef struct HazerNmeaGsa {
    char        gsa_name[3 + 1];
    float       gsa_pdop;
    float       gsa_hdop;
    float       gsa_vdop;
    uint8_t     gsa_satellites[12];
    uint8_t     gsa_system;
    char        gsa_mode;
    char        gsa_dimensionality;
} hazer_nmea_gsa_t;

/**
 * NMEA 0183 4.10, GSV, GNSS Satellites In View, p. 96-97
 */
typedef struct HazerNmeaGsv {
    char        gsv_name[3 + 1];
    uint8_t     gsv_sentences;
    uint8_t     gsv_sentence;
    uint8_t     gsv_satellites;
    struct {
        uint8_t gsv_satellite;
        uint8_t gsv_elevation;
        uint8_t gsv_azimuth;
        uint8_t gsv_snr;
    } gsv_sv[4];
} hazer_nmea_gsv_t;

/**
 * NMEA 0183 4.10, RMC, Recommended Minimum Specific GNSS Data, p. 113-114
 */
typedef struct HazerNmeaRmc {
    char        rmc_name[3 + 1];
    float       rmc_utc;
    float       rmc_latitude;
    float       rmc_longitude;
    float       rmc_speed;
    float       rmc_course;
    float       rmc_date;
    float       rmc_variation;  
    char        rmc_status;
    char        rmc_mode;
    char        rmc_navigational;
} hazer_nmea_rmc_t;

/**
 * NMEA 0183 4.10, VTG, Course Over Ground and Ground Speed, p. 127-128
 */
typedef struct HazerNmeaVtg {
    char        vtg_name[3 + 1];
    float       vtg_course_true;
    float       vtg_course_magnetic;
    float       vtg_speed_knots;
    float       vtg_speed_kph;
    char        vtg_mode;
} hazer_nmea_vtg_t;

/*******************************************************************************
 * Functions
 ******************************************************************************/

extern FILE * hazer_debug(FILE *now);

extern ssize_t hazer_sentence_read(FILE *fp, void * buffer, size_t size);

extern ssize_t hazer_sentence_check(const void * buffer, size_t size);

#endif
