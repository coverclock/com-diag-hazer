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
#include <stddef.h>
#include <stdint.h>

/**
 * NMEA 0183 4.10, 5.3.3.1, Table 1
 * NMEA 0183 4.10, 5.3
 */
enum HazerNmeaConstant {
    HAZER_NMEA_CONSTANT_SHORTEST    = sizeof("$ccccc*hh\r\n") - 1,
    HAZER_NMEA_CONSTANT_LONGEST     = 82,
    HAZER_NMEA_CONSTANT_TALKER      = sizeof("GP") - 1,
    HAZER_NMEA_CONSTANT_MESSAGE     = sizeof("GGA") - 1,
};

/**
 * Hazer NMEA state machine states. The only states the application needs
 * to take action on are START (to initialize the state), EOF (end of file
 * on the input stream), and END (complete NMEA sentence in buffer). The
 * rest are transitory states. If the machine transitions from a non_START
 * state to the START state, that means the framing of the current sentence
 * failed; that might be of interest to the application.
 */
typedef enum HazerState {
    HAZER_STATE_EOF         = -1,
    HAZER_STATE_START       = 0,
    HAZER_STATE_TALKER_1,
    HAZER_STATE_TALKER_2,
    HAZER_STATE_MESSAGE_1,
    HAZER_STATE_MESSAGE_2,
    HAZER_STATE_MESSAGE_3,
    HAZER_STATE_DELIMITER,
    HAZER_STATE_CHECKSUM,
    HAZER_STATE_CHECKSUM_1,
    HAZER_STATE_CHECKSUM_2,
    HAZER_STATE_CR,
    HAZER_STATE_LF,
    HAZER_STATE_END,
} hazer_state_t;

/**
 * NMEA 0183 4.10, 5.3, p. 11
 */
typedef char (hazer_buffer_t)[HAZER_NMEA_CONSTANT_LONGEST + 1]; /* plus NUL */

typedef char * (hazer_vector_t)[HAZER_NMEA_CONSTANT_LONGEST - HAZER_NMEA_CONSTANT_SHORTEST + 1]; /* plus NULL */

/*
 * As a long time embedded developer, I really dislike using floating
 * point below. A double precision variable is needed since a four byte
 * IEEE float as about five significant digits of precision.
 */

/**
 * NMEA 0183 4.10, GGA, Global Positioning System Fix Data, p. 86-87
 */
typedef struct HazerNmeaGga {
    uint64_t    gga_ticks;              /* Diminuto ticks */
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
 * NMEA 0183 4.10, GGL, Geographic Position Latitude/Longitude, p. 87
 */
typedef struct HazerNmeaGll {
    uint64_t    ggl_ticks;              /* Diminuto ticks */
    char        ggl_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "GLL" */
    char        ggl_status;             /* Status */
    char        ggl_mode;               /* Mode indicator */
    uint8_t     ggl_unused[2];
    double      ggl_latitude;           /* Latitude (N+, S-)  */
    double      ggl_longitude;          /* Longitude (E+, W-) */
    double      ggl_utc;                /* UTC of position fix */
} hazer_nmea_gll_t;

/**
 * NMEA 0183 4.10, GSA, GNSS DOP and Active Satellites, p. 94-95
 */
typedef struct HazerNmeaGsa {
    uint64_t    gsa_ticks;              /* Diminuto ticks */
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
    uint64_t    gsv_ticks;              /* Diminuto ticks */
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
    uint64_t    rmc_ticks;              /* Diminuto ticks */
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

/**
 * NMEA 0183 4.10, VTG, Course Over Ground and Ground Speed, p. 127-128
 */
typedef struct HazerNmeaVtg {
    uint64_t    vtg_ticks;              /* Diminuto ticks */
    char        vtg_name[HAZER_NMEA_CONSTANT_MESSAGE + 1]; /* "VTG" */
    char        vtg_mode;               /* Mode indicator */
    uint8_t     vtg_unused[3];
    double      vtg_course_true;        /* Course over ground degrees true */
    double      vtg_course_magnetic;    /* Course over ground degrees mag. */
    double      vtg_speed_knots;        /* Speed over ground knots */
    double      vtg_speed_kph;          /* Speed over ground kilometers/hour */
} hazer_nmea_vtg_t;

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * hazer_debug(FILE *now);

/**
 * Process a single character of stimulus for the state machine that is
 * assembling a single NMEA sentence in the caller provided buffer. State
 * is maintained in a character pointer and a size variable, pointers to
 * which are passed to the function, which will be initialized by the function
 * itself. The function returns the new state, which must be used in the
 * subsequent call. The initial state should be the START state. Of interest
 * to the application are the EOF and END staes. The EOF state indicates
 * that the input stream has ended. The END state indicates that a complete
 * NMEA sentence resides in the buffer; the pointer state variable points
 * past the end of the NUL-terminated sentence, and the size state variable
 * contrains the size of the sentence including the terminating NUL.
 * @param state is the prior state of the machine.
 * @param ch is the character stimulus.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param bp points to a character pointer state variable.
 * @param sp points to a size state variable.
 * @return the next state of the machine.
 */
extern hazer_state_t hazer_nmea_machine(hazer_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp);

/**
 * Read a single NMEA sentence into the caller provided buffer. The size of
 * the resulting sentence in bytes, including its terminating NUL, is returned.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @return the size of the sentence in bytes, 0 for EOF, -1 for error.
 */
extern ssize_t hazer_nmea_read(FILE *fp, void * buffer, size_t size);

/**
 * Check the contents of a single NMEA sentence. Some basic sanity checks
 * are done in addition to computing and verifying the checksum.
 * @param buffer points to the beginning of the sentence buffer.
 * @param size is the size of the sentence in bytes.
 * @return the location where the check failed or size of no error occurred.
 */
extern ssize_t hazer_nmea_check(const void * buffer, size_t size);

/**
 * Tokenize an NMEA sentence by splitting it into substrings whose pointers
 * are placed in an argument vector. This is a descructive operation on the
 * sentence since delimiers are replaced with NULs to terminate the substrings.
 * A null pointer terminates the argument vector.
 * @param vector is an argument vector in which the substring pointers are stored.
 * @param count is the size of the argument vector in array positions.
 * @param buffer points to the beginning of the sentence buffer.
 * @param size is the size of the sentence in bytes.
 * @return the number of arguments including the final null pointer.
 */
ssize_t hazer_nmea_tokenize(char * vector[], size_t count, void * buffer, size_t size);

#endif
