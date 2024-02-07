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
 *
 * REFERENCES
 *
 * WitMotion, "Bluetooth AHRS IMU sensor | WT901BLE", v20-0528, WitMotion
 * Shenzhen Co., Ltd.
 *
 * WitMotion, "Quick Guide Bluetooth 5.0 Inclinometer Sensor", WT901BLECL,
 * WitMotion Shenzhen Co., Ltd.
 *
 * THIS IS A WORK IN PROGRESS.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/******************************************************************************
 * DEBUGGING
 ******************************************************************************/

extern FILE * dally_debug(FILE * now);

/******************************************************************************
 * FUNDAMENTAL TYPES
 ******************************************************************************/

typedef uint8_t dally_byte_t;

typedef int16_t dally_word_t;

typedef double dally_datum_t;

/******************************************************************************
 * SYMBOLICS
 ******************************************************************************/

typedef enum DallyStates {
    DALLY_STATE_START                       = '\0',  /* Waiting for init. */
    DALLY_STATE_HEADING                     = 'S',   /* Waiting for heading. */
    DALLY_STATE_FLAG                        = 'F',   /* Waiting for flag. */
    DALLY_STATE_REGISTER_LOW                = 'l',   /* Waiting for reg low. */
    DALLY_STATE_REGISTER_HIGH               = 'h',   /* Waiting for reg high. */
    DALLY_STATE_DATA_LOW                    = 'L',   /* Waiting for data low. */
    DALLY_STATE_DATA_HIGH                   = 'H',   /* Waiting for data high. */
    DALLY_STATE_FINAL                       = '.',   /* Waiting for fini. */
    DALLY_STATE_ERROR                       = '?',   /* Error. */
} dally_state_t;

enum DallyPayloads {
    DALLY_PAYLOAD_BYTES                     = 20,
    DALLY_PAYLOAD_WORDS                     = (DALLY_PAYLOAD_BYTES / sizeof(dally_word_t)),
    DALLY_PAYLOAD_DATA_BYTES                = (DALLY_PAYLOAD_BYTES - 2),
    DALLY_PAYLOAD_DATA_WORDS                = (DALLY_PAYLOAD_DATA_BYTES / sizeof(dally_word_t)),
    DALLY_PAYLOAD_REGISTER_BYTES            = (DALLY_PAYLOAD_DATA_BYTES - 2),
    DALLY_PAYLOAD_REGISTER_WORDS            = (DALLY_PAYLOAD_DATA_WORDS - 1),
};

enum DallyHeadings {
    DALLY_HEADING                           = 0x55U, /* 'U' */
};

enum DallyFlags {
    DALLY_FLAG_DATA                         = 0x61U, /* 'a' */
    DALLY_FLAG_REGISTER                     = 0x71U, /* 'q' */
};

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
 * DERIVED TYPES
 ******************************************************************************/

typedef dally_word_t (dally_words_t)[DALLY_PAYLOAD_WORDS];

typedef dally_byte_t (dally_bytes_t)[DALLY_PAYLOAD_BYTES];

typedef struct DallyPrefix {
    dally_byte_t header;
    dally_byte_t flag;
} dally_prefix_t;

typedef struct DallyData {
    dally_prefix_t prefix;
    dally_word_t ax;
    dally_word_t ay;
    dally_word_t az;
    dally_word_t wx;
    dally_word_t wy;
    dally_word_t wz;
    dally_word_t roll;
    dally_word_t pitch;
    dally_word_t yaw;
} dally_data_t;

typedef struct DallyRegister {
    dally_prefix_t prefix;
    dally_word_t reg;
    dally_word_t data[DALLY_PAYLOAD_REGISTER_WORDS];
} dally_register_t;

typedef union DallyPacket {
    dally_words_t w;
    dally_bytes_t b;
    dally_prefix_t p;
    dally_data_t d;
    dally_register_t r;
} dally_packet_t;

typedef struct DallyContext {
    dally_packet_t * packetp;
    dally_word_t * wordp;
    size_t count;
    dally_word_t word;
    dally_state_t state;
} dally_context_t;

/******************************************************************************
 * CONVERSIONS
 ******************************************************************************/

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static inline dally_context_t * dally_init(dally_context_t * cp, dally_packet_t * pp) {
    cp->packetp = pp;
    cp->wordp = (dally_word_t *)0;
    cp->count = 0;
    cp->word = 0;
    cp->state = DALLY_STATE_HEADING;
    return cp;
}

static inline dally_context_t * dally_fini(dally_context_t * cp) {
    cp->state = DALLY_STATE_START;
    return (dally_context_t *)0;
}

extern dally_state_t dally_machine(dally_context_t * mp, int ch);

#endif
