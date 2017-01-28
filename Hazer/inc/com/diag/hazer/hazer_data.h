/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_DATA_
#define _H_COM_DIAG_HAZER_DATA_

/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://www.diag.com/navigation/downloads/Hazer.html<BR>
 */

#include <stdint.h>
#include "com/diag/hazer/hazer.h"

/**
 * NMEA 0183 4.10, GGA, Global Positioning System Fix Data, p. 86-87
 */
typedef struct HazerNmeaGga {
    char        gga_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "GGA" */
    uint16_t    gga_station;            /* Differential reference station */
    uint8_t     gga_satellites;         /* Number of satellites in use */
    char        gga_quality;            /* GPS quality indicator */
    double      gga_utc;                /* UTC of position fix */
    double      gga_latitude;           /* Latitude (N+, S-) hhmmss.ss */
    double      gga_longitude;          /* Longitude (E+, W-) hhmmss.ss */
    double      gga_altitude;           /* Altitude mean sea level meters */
    double      gga_hdop;               /* Horizontal dilution of precision */
    double      gga_geoidal;            /* Geoidal separation meters */
    double      gga_age;                /* Age of differential GPS data */
} hazer_nmea_gga_t;

/**
 * NMEA 0183 4.10, GSA, GNSS DOP and Active Satellites, p. 94-95
 */
typedef struct HazerNmeaGsa {
    char        gsa_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "GSA" */
    uint8_t     gsa_system;             /* GNSS system ID */
    char        gsa_mode;               /* Mode: manual or automatic  */
    char        gsa_dimensionality;     /* Mode: 2D or 3D or not-available */
    uint8_t     gsa_unused[1];
    double      gsa_pdop;               /* position dilution of precision */
    double      gsa_hdop;               /* horizontal dilution of precision */
    double      gsa_vdop;               /* vertical dilution of precision */
    uint8_t     gsa_satellites[12];     /* ID numbers of satellites */
} hazer_nmea_gsa_t;

/**
 * NMEA 0183 4.10, GSV, GNSS Satellites In View, p. 96-97
 */
typedef struct HazerNmeaGsv {
    char        gsv_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "GSV" */
    uint8_t     gsv_sentences;          /* Total number of sentences */
    uint8_t     gsv_sentence;           /* Sentence number */
    uint8_t     gsv_satellites;         /* Total satellites in view */
    uint8_t     gsv_unused[1];
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
    char        rmc_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "RMC" */
    char        rmc_status;             /* Status */
    char        rmc_mode;               /* Mode indicator */
    char        rmc_navigational;       /* Navigational status */
    uint8_t     rmc_unused[1];
    double      rmc_utc;                /* UTC of position fix */
    double      rmc_latitude;           /* Latitude (N+, S-) */
    double      rmc_longitude;          /* Longitude (E+, W-) */
    double      rmc_speed;              /* Speed over ground knots */
    double      rmc_course;             /* Course over ground degrees true */
    double      rmc_date;               /* Date ddmmyy */
    double      rmc_variation;          /* Magnetic variation degrees */
} hazer_nmea_rmc_t;

typedef struct HazerNmeaBase {
    char        base_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; 
} hazer_nmea_base_t;

typedef union HazerNmea {
    hazer_nmea_base_t   base;
    hazer_nmea_gga_t    gga;
    hazer_nmea_gsa_t    gsa;
    hazer_nmea_gsv_t    gsv;
    hazer_nmea_rmc_t    rmc;
} hazer_nmea_t;

#endif
