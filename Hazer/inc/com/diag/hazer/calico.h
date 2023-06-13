/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_CALICO_
#define _H_COM_DIAG_HAZER_CALICO_

/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Support for proprietary messaging as used by come Garmin devices.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * THIS IS A WORK IN PROGRESS.
 * The Calico module provides support for the Garmin CPO binary output format
 * that is produced by some devices manufactured by Garmin International, Inc.
 * I have a Garmin GPS-18x PC device with an RS-232 DB9 interface, which
 * produces either binary CPO output or NMEA and Garmin proprietary NMEA-like
 * sentences (but not both at the same time). I have no idea what CPO stands
 * for, but it's used in the Garmin docs without, as far as I can tell,
 * explanation. Also, this code does NOT work with the Garmin GPS-18x USB
 * device, whose binary output so far mystifies me. The baud rate of the
 * GPS-18x PC is also a bit of a mystery: it seems to run at 4800 baud for the
 * NMEA output, but 9600 baud for the CPO output. Finally, the layout of the
 * fields in the structures in which the binary CPO output is emitted sucks;
 * with just a little moving things around it could be vastly improved.
 *
 * REFERENCES
 *
 * Garmin, "Garmin Device Interface Specification", 001-00063-00 Rev. G,
 * Garmin International, Inc., 2020-04-14
 *
 * Garmin, "Garmin Proprietary NMEA 0183 Sentences TECHNICAL SPECIFICATIONS",
 * 190-00684-00 Rev. C, Garmin International, Inc., 2008-12
 *
 * Garmin, "GPS 18x TECHNICAL SPECIFICATIONS", 190-00879-08 Rev. D, Garmin
 * International, Inc., 2011-10
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "com/diag/hazer/hazer.h"

/******************************************************************************
 * DEBUGGING
 ******************************************************************************/

/**
 * Sets the debug file pointer. If the pointer is non-null, debugging
 * information is emitted to it. The prior debug file pointer is returned.
 * @param now is the new file pointer used for debugging, or NULL.
 * @return the prior debug file pointer (which may be NULL).
 */
extern FILE * calico_debug(FILE * now);

/*******************************************************************************
 * STARTING UP AND SHUTTING DOWN
 ******************************************************************************/

/**
 * Perform any necessary initialization.
 * @return 0 for success, <0 otherwise.
 */
extern int calico_initialize(void);

/**
 * Perform any necessary finalization.
 * @return 0 for success, <0 otherwise.
 */
extern int calico_finalize(void);

/*******************************************************************************
 * COLLECTING A GARMIN PACKET
 ******************************************************************************/

/**
 * Calico CPO constants.
 */
enum CalicoCpoConstants {
    CALICO_CPO_FRONT    = 1,    /* DLE */
    CALICO_CPO_HEAD     = 2,    /* ID, SIZE */
    CALICO_CPO_HEADER   = CALICO_CPO_FRONT + CALICO_CPO_HEAD,
    CALICO_CPO_DATA     = 255,
    CALICO_CPO_TAIL     = 0,
    CALICO_CPO_BACK     = 3,    /* CS, DLE, ETX */
    CALICO_CPO_TRAILER  = CALICO_CPO_BACK,
    CALICO_CPO_SHORTEST = CALICO_CPO_HEADER + CALICO_CPO_TRAILER,
    CALICO_CPO_LONGEST  = CALICO_CPO_HEADER + CALICO_CPO_DATA + CALICO_CPO_TRAILER,
    CALICO_CPO_SUMMED   = CALICO_CPO_HEAD + CALICO_CPO_TAIL,
    CALICO_CPO_UNSUMMED = CALICO_CPO_FRONT + CALICO_CPO_BACK,
};

/**
 * This buffer is large enough to contain the largest CPO packet, plus a
 * trailing NUL, and then some. The NUL at the end is useless in the CPO binary
 * protocol, but is useful in some edge cases in which the data format has not
 * yet been determined (e.g. incoming UDP datagrams).
 */
typedef uint8_t (calico_buffer_t)[CALICO_CPO_LONGEST + 1];

/**
 * @def CALICO_BUFFER_INITIALIZER
 * Defines a static initializer for a CalicoBuffer.
 */
#define CALICO_BUFFER_INITIALIZER  \
    { '\0', }

/**
 * Calico CPO offsets.
 */
enum CalicoCpoOffsets {
    CALICO_CPO_SYNC         = 0,
    CALICO_CPO_ID           = 1,
    CALICO_CPO_SIZE         = 2,
    CALICO_CPO_PAYLOAD      = 3,
};

/**
 * This is the structure of the start matter of every CPO packet.
 */
typedef struct CalicoCpoHeader {
    uint8_t sync;
    uint8_t id;
    uint8_t size;
    uint8_t payload[0];
} calico_cpo_header_t;

/**
 * THis is the structure of the end matter of every CPO packet.
 */
typedef struct CalicoCpoTrailer {
    uint8_t sync;
    uint8_t end;
} calico_cpo_trailer_t;

/**
 * CPO state machine states. The only states the application needs
 * to take action on is END (complete CPO packet in buffer). The
 * rest are transitory states. If the machine transitions from a non-START
 * state to the START state, that means the framing of the current packet
 * failed; that might be of interest to the application.
 */
typedef enum CalicoState {
    CALICO_STATE_STOP           = 'X',
    CALICO_STATE_START          = 'S',
    CALICO_STATE_ID             = 'I',
    CALICO_STATE_SIZE           = 'Z',
    CALICO_STATE_SIZE_DLE       = 'z',
    CALICO_STATE_PAYLOAD        = 'P',
    CALICO_STATE_PAYLOAD_DLE    = 'p',
    CALICO_STATE_CS             = 'C',
    CALICO_STATE_CS_DLE         = 'c',
    CALICO_STATE_DLE            = 'D',
    CALICO_STATE_ETX            = 'T',
    CALICO_STATE_END            = 'E',
} calico_state_t;

/**
 * CPO state machine stimuli.
 */
enum CalicoStimulus {
    CALICO_STIMULUS_DLE     = (uint8_t)'\x10',
    CALICO_STIMULUS_ETX     = (uint8_t)'\x03',
};

/**
 * CPO state machine actions.
 */
typedef enum CalicoAction {
    CALICO_ACTION_SKIP       = 'X',
    CALICO_ACTION_SAVE       = 'S',
    CALICO_ACTION_TERMINATE  = 'T',
} calico_action_t;

/**
 * Calico CPO parser state machine context (which needs no initial value).
 */
typedef struct CalicoContext {
    uint8_t * bp;       /* Current buffer pointer. */
    size_t sz;          /* Remaining buffer size in bytes. */
    size_t tot;         /* Total size once packet is complete. */
    uint8_t ln;         /* Payload length in bytes. */
    uint8_t cc;         /* Running checksum counter. */
    uint8_t cs;         /* Running checksum value. */
    uint8_t error;      /* Checksum error indication. */
} calico_context_t;

/**
 * @def CALICO_CONTEXT_INITIALIZER
 * Defines a static intializer for Calico Context structure.
 */
#define CALICO_CONTEXT_INITIALIZER \
    { \
        (uint8_t *)0, \
    }

/**
 * Process a single character of stimulus for the state machine that is
 * assembling a single CPO packet in the caller provided buffer. State
 * is maintained in a character pointer and a size variable, pointers to
 * which are passed to the function, which will be initialized by the function
 * itself. The function returns the new state, which must be used in the
 * subsequent call. The initial state should be the START state. Of interest
 * to the application are the EOF and END states. The EOF state indicates
 * that the input stream has ended, detected by virtue of the stimulus character
 * being equal to the standard I/O EOF. The END state indicates that a complete
 * NMEA sentence resides in the buffer. The pointer state variable points
 * past the end of the NUL-terminated sentence, the size state variable
 * constrains the size of the sentence including the terminating NUL;
 * @param state is the prior state of the machine.
 * @param ch is the next character from the CPO packet stream.
 * @param buffer points to the beginning of the output buffer.
 * @param size is the size of the output buffer in bytes.
 * @param pp points to the context structure (which needs no initialization).
 * @return the next state of the machine.
 */
extern calico_state_t calico_machine(calico_state_t state, uint8_t ch, void * buffer, size_t size, calico_context_t * pp);

/**
 * Return the total size of the complete CPO message as computed by the parser.
 * @param pp points to the context structure.
 * @return the final size.
 */
static inline size_t calico_size(const calico_context_t * pp)
{
    return pp->tot;
}

/*******************************************************************************
 * VALIDATING A CPO PACKET
 ******************************************************************************/

/**
 * Update a running CPO checksum with the latest input character. The CPO
 * checksum is across bytes 1 (zero based) to byte N-4 i.e. ID through the
 * last payload byte.
 * @param ch is the input character.
 * @param ccp points to the running checksum counter.
 * @param csp points to the running checksum value.
 */
static inline void calico_checksum(uint8_t ch, uint8_t * ccp, uint8_t * csp)
{
    *ccp += ch;
    *csp = (uint8_t)(-((int8_t)(*ccp)));
}

/**
 * Compute the checksum used by CPO for the specified buffer. The buffer
 * points to the beginning of the CPO packet, not to the subset that is
 * checksummed, and the sentence must contain a valid length field. A pointer
 * is returned pointing just past the checksummed portion; this is where the
 * checksum will be stored in a correctly formed packet.
 * @param buffer points to the beginning of the buffer.
 * @param size is the size of the buffer in bytes.
 * @param ccp points to where the running checksum counter will be stored.
 * @param csp points to where the running checksum value will be stored.
 * @return a pointer just past the end of the checksummed portion, or NULL if an error occurred.
 */
extern const void * calico_checksum_buffer(const void * buffer, size_t size, uint8_t * ccp, uint8_t * csp);

/**
 * Return the length of the completed packet in bytes.
 * @param buffer points to buffer containing the completed packet.
 * @param size is the size of the buffer containing the packet.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t calico_length(const void * buffer, size_t size);

/**
 * Validate the contents of an buffer as a valid CPO packet.
 * @param buffer points to the buffer. This combines
 * the calico_length() and calico_checksum_buffer() functions along with the
 * checksum comparison.
 * @param size is the number of bytes in the buffer.
 * @return the length of the packet in bytes or <0 if an error occurred.
 */
extern ssize_t calico_validate(const void * buffer, size_t size);

/******************************************************************************
 * PROCESSING HELPERS
 ******************************************************************************/

/**
 * Return true if the CPO ID and length match the specified values.
 * @param bp points to the buffer.
 * @param length is the length of the buffer in bytes.
 * @param id is the desired identifier.
 * @param size is the desired length.
 * @return true if the id and size match.
 */
static inline int calico_is_cpo_id_length(const void * bp, ssize_t length, uint8_t id, ssize_t size) {
    const uint8_t * cp = (const uint8_t *)bp;

    return ((length > CALICO_CPO_SIZE) && (cp[CALICO_CPO_ID] == id) && (cp[CALICO_CPO_SIZE] == size));
}

/******************************************************************************
 * PROCESSING CPO SATELLITE DATA RECORD (SDR)
 ******************************************************************************/

/**
 * CPO SDR constants.
 */
enum CalicoCpoSatelliteDataRecordConstants {
    CALICO_CPO_SDR_Id       = 'r',
    CALICO_CPO_SDR_Length   = 84,
    CALICO_CPO_SDR_Count    = 12,
};

/**
 * Description of the CPO SDR.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 26.
 */
typedef struct CalicoCpoSatData {
    uint8_t     svid;       /* 1..32 for GPS, 33..64 for WAAS. */
    uint16_t    snr;
    uint8_t     elev;       /* Degrees. */
    uint16_t    azmth;      /* Degrees. */
    uint8_t     status;
} calico_cpo_sat_data_t;

/**
 * @def CALICO_CPO_SATELLITE_DATA_RECORD_INITIALIZER
 * Defines a static initializer for a CPO SDR.
 */
#define CALICO_CPO_SAT_DATA_INITIALIZER = \
    { \
        0, \
    }

/**
 * Defines the meaning of the CPO SDR Status bit mask.
 */
enum CalicoCpoSatelliteDataRecordStatus {
    CALICO_CPO_SDR_STATUS_Ephemeris    = (1<<0),
    CALICO_CPO_SDR_STATUS_Correction   = (1<<1),
    CALICO_CPO_SDR_STATUS_Solution     = (1<<2),
};

/**
 * Description of the CPO SDR packet.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 26.
 * The alignment design of this sucks.
 */
typedef struct CalicoCpoSatelliteDataPacket {
    uint8_t     svid;       /* 1..32 for GPS, 33..64 for WAAS. */
    uint16_t    snr;
    uint8_t     elev;       /* Degrees. */
    uint16_t    azmth;      /* Degrees. */
    uint8_t     status;
} __attribute__((packed)) calico_cpo_sat_data_packet_t;

/**
 * The full eighty-four byte CPO SDR contains twelve
 * instances of the structure.
 */
typedef struct CalicoCpoSatelliteDataArrayPacket {
    calico_cpo_sat_data_packet_t   sat[CALICO_CPO_SDR_Count];
} __attribute__((packed)) calico_cpo_sat_data_array_packet_t;

extern int calico_cpo_satellite_data_record(hazer_view_t * gvp, hazer_view_t * wvp, hazer_active_t * gap, hazer_active_t * wap, const void * bp, ssize_t length);

/******************************************************************************
 * PROCESSING CPO POSITION VELOCITY TIME (PVT) RECORD
 ******************************************************************************/

/**
 * CPO PVT constants.
 */
enum CalicoCpoPositionRecordConstants {
    CALICO_CPO_PVT_Id       = '3',
    CALICO_CPO_PVT_Length   = 64,
};

/**
 * Description of the CPO PVT.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 27.
 * The actual integer types were inferred from the record length.
 */
typedef struct CalicoCpoPvtData {
    float   alt;        /* Meters above ellipsoid. */
    float   epe;        /* Meters position error. */
    float   eph;        /* Meters horizontal error. */
    float   epv;        /* Meters vertical error. */
    int16_t fix;        /* Fix type. */
    double  gps_tow;    /* Seconds GPS Time Of Week. */
    double  lat;        /* Radians latitude. */
    double  lon;        /* Radians longitude. */
    float   lon_vel;    /* Meters/second longitude velocity. */
    float   lat_vel;    /* Meters/second latitude velocity. */
    float   alt_vel;    /* Meters/second altitude velocity. */
    float   msl_hght;   /* Meters height above mean sea level. */
    int16_t leap_sec;   /* UTC leap seconds. */
    int32_t grmn_days;  /* Days since 1989-12-31. */
} calico_cpo_pvt_data_t;

/**
 * @def CALICO_CPO_POSITION_RECORD_INITIALIZER
 * Defines a static initializer for a CPO SDR.
 */
#define CALICO_CPO_PVT_DATA_INITIALIZER \
    { \
        0., \
    }

/**
 * Defines the meaning of the CPO PVT Fix enumeration.
 */
enum CalicoCpoPositionRecordFix {
    CALICO_CPO_PVT_FIX_None             = 0,
    CALICO_CPO_PVT_FIX_StillNone        = 1,
    CALICO_CPO_PVT_FIX_2D               = 2,
    CALICO_CPO_PVT_FIX_3D               = 3,
    CALICO_CPO_PVT_FIX_2DDifferential   = 4,
    CALICO_CPO_PVT_FIX_3DDifferential   = 5,
};

/**
 * Description of the CPO PVT packet.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 27.
 * The actual integer types were inferred from the record length.
 */
typedef struct CalicoCpoPvtDataPacket {
    float   alt;        /* Meters above ellipsoid. */
    float   epe;        /* Meters position error. */
    float   eph;        /* Meters horizontal error. */
    float   epv;        /* Meters vertical error. */
    int16_t fix;        /* Fix type. */
    double  gps_tow;    /* Seconds GPS Time Of Week. */
    double  lat;        /* Radians latitude. */
    double  lon;        /* Radians longitude. */
    float   lon_vel;    /* Meters/second longitude velocity. */
    float   lat_vel;    /* Meters/second latitude velocity. */
    float   alt_vel;    /* Meters/second altitude velocity. */
    float   msl_hght;   /* Meters height above mean sea level. */
    int16_t leap_sec;   /* UTC leap seconds. */
    int32_t grmn_days;  /* Days since 1989-12-31. */
} __attribute__((packed)) calico_cpo_pvt_data_packet_t;

extern int calico_cpo_position_record(hazer_position_t * gpp, const void * bp, ssize_t length);

/******************************************************************************
 * ENDIAN CONVERSION
 ******************************************************************************/

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <string.h>

/**
 * @def COM_DIAG_CALICO_LETOH
 * Convert field @a _FIELD_ from Little Endian byte order to Host
 * byte order. The field width 8, 16, 32, or 64 bits, is inferred automatically.
 * Little endian (not network byte order) is used because that is empirically
 * what the Garmin GPS-18x emits.
 */
#define COM_DIAG_CALICO_LETOH(_DESTINATION_, _SOURCE_) \
    do { \
        switch (sizeof(_SOURCE_)) { \
        case sizeof(uint8_t): \
            _DESTINATION_ = _SOURCE_; \
            break; \
        case sizeof(uint16_t): \
            { \
                uint16_t _temporary_; \
                memcpy(&_temporary_, &(_SOURCE_), sizeof(_temporary_)); \
                _temporary_ = le16toh(_temporary_); \
                memcpy(&(_DESTINATION_), &_temporary_, sizeof(_DESTINATION_)); \
            } \
            break; \
        case sizeof(uint32_t): \
            { \
                uint32_t _temporary_; \
                memcpy(&_temporary_, &(_SOURCE_), sizeof(_temporary_)); \
                _temporary_ = le32toh(_temporary_); \
                memcpy(&(_DESTINATION_), &_temporary_, sizeof(_DESTINATION_)); \
            } \
            break; \
        case sizeof(uint64_t): \
            { \
                uint64_t _temporary_; \
                memcpy(&_temporary_, &(_SOURCE_), sizeof(_temporary_)); \
                _temporary_ = le64toh(_temporary_); \
                memcpy(&(_DESTINATION_), &_temporary_, sizeof(_DESTINATION_)); \
            } \
            break; \
        default: \
            break; \
        } \
    } while (0)

/******************************************************************************
 * NUMERICAL CONVERSIONS
 ******************************************************************************/

#include <math.h>

/**
 * Garmin uses floating point radians, NMEA uses unsigned integer degrees,
 * minutes, and decimal fraction of minutes, with a direction indicator,
 * and Hazer uses signed integer billionths of a minute.
 * @param radians is a latitude or longitude in radians.
 * @return a latitude or longitude in nanominutes.
 */
static inline int64_t calico_format_radians2nanominutes(double radians) {
    int64_t result = 0;

    result = (180.0 * radians * 1000000000.0) / (M_PI * 60.0);

    return result;
}

/*
 * Useful commands:
 * date -u --date='January 1, 1970' +'%s' # POSIX epoch offset in seconds.
 * date -u --date='January 6, 1980' +'%s' # GPS epoch offset in seconds.
 * date -u --date='January 1, 1990' +'%s' # Garmin epoch offset in seconds.
 */

/**
 * Convert the GPS Time Of Week to Hazer nanoseconds since the POSIX epoch.
 * @param tow is the number of GPS ticks since 1980-01-06.
 * @param leaps is the number of additional leap seconds to UTC.
 * @param days is the number of days since 1990-01-01.
 * @return nanoseconds since 1970-01-01.
 */
static inline uint64_t calico_format_tow2nanoseconds(double tow, int16_t leaps, int32_t days) {
    uint64_t result = 0;
    uint64_t nanoseconds = 0;

    nanoseconds = 315964800;
    nanoseconds *= 1000000000;
    result = nanoseconds;

    tow *= 1.5;
    tow *= 1000000000.0;
    result += tow;

    nanoseconds = leaps;
    nanoseconds *= 1000000000;
    result += nanoseconds;

    return result;
}

#endif
