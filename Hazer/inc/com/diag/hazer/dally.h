/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_DALLY_
#define _H_COM_DIAG_HAZER_DALLY_

/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Support for proprietary messaging used by WitMotion WT901 IMUs.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * Dally is a set of functions to process the output of the WITMOTION
 * WT901BLECL 5.0 Inertial Measurement Unit (IMU) and other similar
 * devices that use the same WT901BLE chip or other WT901 IMUs. The
 * WT901 is not a GNSS receiver, so its output is not processed by gpstool.
 * This feature is used by wit901tool.
 *
 * REFERENCES
 *
 * WitMotion, "Bluetooth AHRS IMU sensor | WT901BLE", v20-0528, WitMotion
 * Shenzhen Co., Ltd.
 *
 * WitMotion, "Quick Guide Bluetooth 5.0 Inclinometer Sensor", WT901BLECL,
 * WitMotion Shenzhen Co., Ltd.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/******************************************************************************
 * DEBUGGING
 ******************************************************************************/

/**
 * Set a FILE pointer to which debugging information may be written. The
 * debug FILE pointer defaults to NULL, which prevents debugging info from
 * being generated.
 * @param now is the new FILE pointer or NULL to disable.
 * @return the old FILE pointer.
 */
extern FILE * dally_debug(FILE * now);

/******************************************************************************
 * TYPES
 ******************************************************************************/

/**
 * The WT901 data byte is an unsigned octet.
 */
typedef uint8_t dally_byte_t;

/**
 * The WT901 word is a 16-bit signed integer.
 */
typedef int16_t dally_word_t;

/**
 * The WT901 value is a single precision floating point.
 */
typedef float dally_value_t;

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/**
 * These constants define the sizes of various WT901 message payloads in
 * terms of WT901 bytes and words.
 */
enum DallyPayloads {
    DALLY_PAYLOAD_BYTES                     = 20,
    DALLY_PAYLOAD_WORDS                     = (DALLY_PAYLOAD_BYTES / sizeof(dally_word_t)),
    DALLY_PAYLOAD_DATA_BYTES                = (DALLY_PAYLOAD_BYTES - 2),
    DALLY_PAYLOAD_DATA_WORDS                = (DALLY_PAYLOAD_DATA_BYTES / sizeof(dally_word_t)),
    DALLY_PAYLOAD_REGISTER_BYTES            = (DALLY_PAYLOAD_DATA_BYTES - 2),
    DALLY_PAYLOAD_REGISTER_WORDS            = (DALLY_PAYLOAD_DATA_WORDS - 1),
};

/**
 * Each WT901 message starts with this value.
 */
enum DallyHeadings {
    DALLY_HEADING                           = 0x55U, /* 'U' */
};

/**
 * Each WT901 message is further classified with one of these flags.
 */
enum DallyFlags {
    DALLY_FLAG_DATA                         = 0x61U, /* 'a' */
    DALLY_FLAG_REGISTER                     = 0x71U, /* 'q' */
};

/**
 * The WT901 has a set of registers identified by these values.
 */
enum DallyRegisters {
    DALLY_REGISTER_YEARMONTH                = 0x30U, /* '0' */
    DALLY_REGISTER_DATEHOUR                 = 0x31U, /* '1' */
    DALLY_REGISTER_MINUTESECOND             = 0x32U, /* '2' */
    DALLY_REGISTER_MILLISECOND              = 0x33U, /* '3' */
    DALLY_REGISTER_MAGNETICFIELD            = 0x3aU, /* ':' */
    DALLY_REGISTER_TEMPERATURE              = 0x40U, /* '@' */
    DALLY_REGISTER_QUATERNION               = 0x51U, /* 'Q' */
};

/******************************************************************************
 * I/O STRUCTURES
 ******************************************************************************/

/**
 * This type defines an array of WT901 bytes that can hold a WT901 message
 */
typedef dally_byte_t (dally_bytes_t)[DALLY_PAYLOAD_BYTES];

/**
 * This type defines an array of WT901 words that can hold a WT901 message.
 */
typedef dally_word_t (dally_words_t)[DALLY_PAYLOAD_WORDS];

/**
 * This type defines a structure containing a WT901 Data message.
 */
typedef struct DallyData {
    dally_byte_t header;
    dally_byte_t flag;
    dally_word_t payload[DALLY_PAYLOAD_DATA_WORDS];
} dally_data_t;

/**
 * This type defines a structure containing a WT901 Register message.
 */
typedef struct DallyRegister {
    dally_byte_t header;
    dally_byte_t flag;
    dally_word_t reg;
    dally_word_t payload[DALLY_PAYLOAD_REGISTER_WORDS];
} dally_register_t;

/**
 * This type defines a union big enough to contain any of the
 * WT901 messages we may legitimately receive.
 */
typedef union DallyPacket {
    dally_words_t w; /* Alignment. */
    dally_bytes_t b;
    dally_data_t d;
    dally_register_t r;
} dally_packet_t;

/******************************************************************************
 * STATE MACHINE
 ******************************************************************************/

/**
 * These are the states in which the Dally state machine, which interprets
 * incoming WT901 data, may be in.
 */
typedef enum DallyStates {
    DALLY_STATE_START                       = '\0',  /**< Waiting for init. */
    DALLY_STATE_HEADING                     = 'S',   /**< Waiting for heading. */
    DALLY_STATE_FLAG                        = 'F',   /**< Waiting for flag. */
    DALLY_STATE_REGISTER_LOW                = 'l',   /**< Waiting for reg low. */
    DALLY_STATE_REGISTER_HIGH               = 'h',   /**< Waiting for reg high. */
    DALLY_STATE_DATA_LOW                    = 'L',   /**< Waiting for data low. */
    DALLY_STATE_DATA_HIGH                   = 'H',   /**< Waiting for data high. */
    DALLY_STATE_FINAL                       = '.',   /**< Waiting for fini. */
    DALLY_STATE_ERROR                       = '?',   /**< Error. */
    DALLY_STATE_EOF                         = '!',   /**< End of file */
} dally_state_t;

/**
 * This structure contains the current context of the Dally state machine.
 */
typedef struct DallyContext {
    dally_packet_t * packetp;
    dally_word_t * wordp;
    size_t count;
    dally_word_t word;
    dally_state_t state;
} dally_context_t;

/**
 * Reset the Dally state machine context to receive a new message. This
 * does not alter the packet pointer in the context.
 * @param cp points to the state machine context.
 * @return a pointer to the state machine context.
 */
static inline dally_context_t * dally_reset(dally_context_t * cp) {
    cp->wordp = (dally_word_t *)0;
    cp->count = 0;
    cp->word = 0;
    cp->state = DALLY_STATE_HEADING;
    return cp;
}

/**
 * Initialize the Dally state machine context including its pointer to
 * a packet structure.
 * @param cp points to the state machine context.
 * @param pp points to the packet.
 * @return a pointer to the state machine context.
 */
static inline dally_context_t * dally_init(dally_context_t * cp, dally_packet_t * pp) {
    cp->packetp = pp;
    return dally_reset(cp);
}

/**
 * Finalize the Dally state machine context, releaseing any dynamically
 * acquired resources. The context will have to be re-initialized if it
 * is to be used again.
 * @param cp points to the state machine context.
 * @return NULL for success, or a pointer to the context for failure.
 */
static inline dally_context_t * dally_fini(dally_context_t * cp) {
    cp->packetp = (dally_packet_t *)0;
    cp->wordp = (dally_word_t *)0;
    cp->count = 0;
    cp->word = 0;
    cp->state = DALLY_STATE_START;
    return (dally_context_t *)0;
}

/**
 * Implement the Dally state machine.
 * @param cp points to the state machine context.
 * @param ch is the next input charaacter (could be stdio EOF).
 * @return the new state.
 */
extern dally_state_t dally_machine(dally_context_t * cp, int ch);

/******************************************************************************
 * DATA STRUCTURES
 ******************************************************************************/

/**
 * This structure contains the processed data from an Acceleration message.
 */
typedef struct DallyAcceleration {
    dally_value_t ax;
    dally_value_t ay;
    dally_value_t az;
    dally_value_t wx;
    dally_value_t wy;
    dally_value_t wz;
    dally_value_t roll;
    dally_value_t pitch;
    dally_value_t yaw;
} dally_acceleration_t;

/**
 * This structure contains the processed data from a Magnetic Field message.
 */
typedef struct DallyMagneticfield {
    dally_value_t hx;
    dally_value_t hy;
    dally_value_t hz;
} dally_magneticfield_t;

/**
 * This structure contains the processed data from a Quaternion message.
 */
typedef struct DallyQuaternion {
    dally_value_t q0;
    dally_value_t q1;
    dally_value_t q2;
    dally_value_t q3;
} dally_quaternion_t;

/**
 * This structure contains the processed data from a Temperature message.
 */
typedef struct DallyTemperature {
    dally_value_t t;
} dally_temperature_t;

/******************************************************************************
 * CONVERSIONS
 ******************************************************************************/

/**
 * Convert a word into a value.
 * @param word is a WT901 word.
 * @return a WT901 value.
 */
static inline dally_value_t dally_word2value(dally_word_t word) {
    dally_value_t value = word;
    return value;
}

/**
 * Convert a value into an acceleration in units of G or 9.8m/s^2.
 * @param value is the original value.
 * @return an acceleration value.
 */
static inline dally_value_t dally_value2acceleration(dally_value_t value) {
    return ((value / 32768.0) * 16.0);
}

/**
 * Convert a value into an angular velocity in units of degrees/second.
 * @param value is the original value.
 * @return an angular velocity value.
 */
static inline dally_value_t dally_value2angularvelocity(dally_value_t value) {
    return ((value / 32768.0) * 2000.0);
}

/**
 * Convert a value into an angle in units of degrees.
 * @param value is the original value.
 * @return the angle value.
 */
static inline dally_value_t dally_value2angle(dally_value_t value) {
    return ((value / 32768.0) * 180.0);
}

/**
 * Convert a value into a magnetic field value in units of milli-G.
 * @param value is the original value.
 * @return the magnetic field value.
 */
static inline dally_value_t dally_value2magneticfield(dally_value_t value) {
    return value;
}

/**
 * Convert a value into a quaternion.
 * @param value is the original value.
 * @return the quaternion value.
 */
static inline dally_value_t dally_value2quaternion(dally_value_t value) {
    return (value / 32768.0);
}

/**
 * Convert a value into a temperature in degress Centigrade.
 * @param value is the original value.
 * @return the temperature value.
 */
static inline dally_value_t dally_value2temperature(dally_value_t value) {
    return (value / 100.0);
}

#endif
