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
 *
 * Electronic Doberman, "Modern GPS Teardown - GlobalSat BU-353S4 SiRF Star
 * IV USB GPS", https://www.youtube.com/watch?v=8xn8FspJDnY
 */

#include <stdio.h>

/*******************************************************************************
 * Types
 ******************************************************************************/

/**
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef char (hazer_buffer_t)[82 + 1]; /* plus NUL */

typedef char * (hazer_vector_t)[82 - 11 + 1];

/**
 * NMEA 0183 4.10, GGA, Global Positioning System Fix Data, p. 86-87
 */
typedef struct HazerNmeaGga {
    char        gga_name[3 + 1];        /* "GGA" */
    float       gga_utc;                /* UTC of position fix */
    float       gga_latitude;           /* Latitude (N+, S-) hhmmss.ss */
    float       gga_longitude;          /* Longitude (E+, W-) hhmmss.ss */
    float       gga_altitude;           /* Altitude mean sea level meters */
    float       gga_hdop;               /* Horizontal dilution of precision */
    float       gga_geoidal;            /* Geoidal separation meters */
    float       gga_age;                /* Age of differential GPS data */
    uint16_t    gga_station;            /* Differential reference station */
    uint8_t     gga_satellites;         /* Number of satellites in use */
    char        gga_quality;            /* GPS quality indicator */
} hazer_nmea_gga_t;

/**
 * NMEA 0183 4.10, GGL, Geographic Position Latitude/Longitude, p. 87
 */
typedef struct HazerNmeaGll {
    char        ggl_name[3 + 1];        /* "GLL" */
    float       ggl_latitude;           /* Latitude (N+, S-)  */
    float       ggl_longitude;          /* Longitude (E+, W-) */
    float       ggl_utc;                /* UTC of position fix */
    char        ggl_status;             /* Status */
    char        ggl_mode;               /* Mode indicator */
} hazer_nmea_gll_t;

/**
 * NMEA 0183 4.10, GSA, GNSS DOP and Active Satellites, p. 94-95
 */
typedef struct HazerNmeaGsa {
    char        gsa_name[3 + 1];        /* "GSA" */
    float       gsa_pdop;               /* position dilution of precision */
    float       gsa_hdop;               /* horizontal dilution of precision */
    float       gsa_vdop;               /* vertical dilution of precision */
    uint8_t     gsa_satellites[12];     /* ID numbers of satellites */
    uint8_t     gsa_system;             /* GNSS system ID */
    char        gsa_mode;               /* Mode: manual or automatic  */
    char        gsa_dimensionality;     /* Mode: 2D or 3D or not-available */
} hazer_nmea_gsa_t;

/**
 * NMEA 0183 4.10, GSV, GNSS Satellites In View, p. 96-97
 */
typedef struct HazerNmeaGsv {
    char        gsv_name[3 + 1];        /* "GSV" */
    uint8_t     gsv_sentences;          /* Total number of sentences */
    uint8_t     gsv_sentence;           /* Sentence number */
    uint8_t     gsv_satellites;         /* Total satellites in view */
    struct {
        uint8_t gsv_satellite;          /* Satellite ID number */
        uint8_t gsv_elevation;          /* Elevation degrees */
        uint8_t gsv_azimuth;            /* Azimuth degrees true */
        uint8_t gsv_snr;                /* Signal/noise ratio */
    } gsv_sv[4];
} hazer_nmea_gsv_t;

/**
 * NMEA 0183 4.10, RMC, Recommended Minimum Specific GNSS Data, p. 113-114
 */
typedef struct HazerNmeaRmc {
    char        rmc_name[3 + 1];        /* "RMC" */
    float       rmc_utc;                /* UTC of position fix */
    float       rmc_latitude;           /* Latitude (N+, S-) */
    float       rmc_longitude;          /* Longitude (E+, W-) */
    float       rmc_speed;              /* Speed over ground knots */
    float       rmc_course;             /* Course over ground degrees true */
    float       rmc_date;               /* Date ddmmyy */
    float       rmc_variation;          /* Magnetic variation degrees */
    char        rmc_status;             /* Status */
    char        rmc_mode;               /* Mode indicator */
    char        rmc_navigational;       /* Navigational status */
} hazer_nmea_rmc_t;

/**
 * NMEA 0183 4.10, VTG, Course Over Ground and Ground Speed, p. 127-128
 */
typedef struct HazerNmeaVtg {
    char        vtg_name[3 + 1];        /* "VTG" */
    float       vtg_course_true;        /* Course over ground degrees true */
    float       vtg_course_magnetic;    /* Course over ground degrees magnetic */
    float       vtg_speed_knots;        /* Speed over ground knots */
    float       vtg_speed_kph;          /* Speed over ground kilometers/hour */
    char        vtg_mode;               /* Mode indicator */
} hazer_nmea_vtg_t;

/*******************************************************************************
 * Functions
 ******************************************************************************/

extern FILE * hazer_debug(FILE *now);

extern ssize_t hazer_sentence_read(FILE *fp, void * buffer, size_t size);

extern ssize_t hazer_sentence_check(const void * buffer, size_t size);

#endif
