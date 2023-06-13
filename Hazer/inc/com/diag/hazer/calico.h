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
 * The Calico module provides support for the Garmin Device Interface
 * Specification binary packet format that is produced by some
 * devices manufactured by Garmin International, Inc. I wrote this code
 * because I had a Garmin 18x dongle with a USB interface, which only
 * produces DIS, not NMEA or the Garmin proprietary NMEA-like sentences.
 * N.B. I have no idea what CPO stands for, but it's used in the Garmin
 * docs without, as far as I can tell, explanation.
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
 * Initialize a CalicoBuffer type.
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
 * This is the structure of the header on every CPO packet.
 */
typedef struct CalicoCpoHeader {
    uint8_t sync;
    uint8_t id;
    uint8_t size;
    uint8_t payload[0];
} calico_cpo_header_t;

/**
 * @def CALICO_CPO_HEADER_INITIALIZER
 * Initialize a CalicoCpoHeader structure.
 */
#define CALICO_CPO_HEADER_INITIALIZER \
    { 0, }

/**
 * These are the two header identifier values that we understand.
 */
enum CalicoCpoHeaderId {
    CALICO_CPO_HEADER_ID_SDR    = 'r',
    CALICO_CPO_HEADER_ID_PVT    = '3',
};

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
 * Defines an intializer for the context structure.
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
 * PROCESSING CPO SATELLITE DATA RECORD (SDR)
 ******************************************************************************/

/**
 * CPO SDR constants.
 */
enum CalicoCpoSatelliteDataRecordConstants {
    CALICO_CPO_SDR_LENGTH   = 84,
    CALICO_CPO_SDR_COUNT    = 12,
};

/**
 * Description of the CPO SDR.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 26.
 * The alignment design of this sucks.
 */
typedef struct CalicoCpoSatelliteDataRecord {
    uint8_t     svid;       /* 1..32 for GPS, 33..64 for WAAS. */
    uint16_t    snr;
    uint8_t     elev;       /* Degrees. */
    uint16_t    azmth;      /* Degrees. */
    uint8_t     status;
} __attribute__((packed)) calico_cpo_sat_data;

/**
 * Defines the meaning of the CPO SDR Status field.
 */
enum CalicoCpoSatelliteDataRecordStatus {
    CALICO_CPO_SDR_STATUS_Ephemeris    = (1<<0),
    CALICO_CPO_SDR_STATUS_Correction   = (1<<1),
    CALICO_CPO_SDR_STATUS_Solution     = (1<<2),
};

/**
 * The full eighty-four byte CPO SDR contains twelve
 * instances of the structure.
 */
typedef struct CalicoCpoSatelliteDataArray {
    calico_cpo_sat_data     sat[CALICO_CPO_SDR_COUNT];
} __attribute__((packed)) calico_cpo_sat_data_array;

/******************************************************************************
 * PROCESSING CPO POSITION RECORD (PVT)
 ******************************************************************************/

/**
 * CPO PVT constants.
 */
enum CalicoCpoPositionRecordConstants {
    CALICO_CPO_PVT_LENGTH   = 64,
};

/**
 * Description of the CPO PVT.
 * Reference: GPS 18x Tech Specs, Rev. D, Appendix B, p. 27.
 * The actual integer types were inferred from the record length.
 */
typedef struct CalicoCpoPositionRecord {
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
    float   msl_hght;   /* Meters height above sea level. */
    int16_t leap_sec;   /* UTC leap seconds. */
    int32_t grmn_days;  /* Days since 1989-12-31. */
} __attribute__((packed)) calico_cpo_pvt_data;

#endif
