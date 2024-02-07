/* vi: set ts    =4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_DALLY_
#define _H_COM_DIAG_HAZER_DALLY_

/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Support for proprietary messaging used WitMotion WT901 IMUs.
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
 * FUNDAMENTAL TYPES
 ******************************************************************************/

typedef uint8_t dally_byte_t;

typedef int16_t dally_word_t;

typedef double dally_datum_t;

/******************************************************************************
 * SYMBOLICS
 ******************************************************************************/

enum DallyStates {
    DALLY_STATE_HEADING                     = 'H',
    DALLY_STATE_FLAG                        = 'F',
    DALLY_STATE_REGISTER_LOW                = 'l',
    DALLY_STATE_REGISTER_HIGH               = 'h',
    DALLY_STATE_DATA_LOW                    = 'L',
    DALLY_STATE_DATA_HIGH                   = 'H',
};

enum DallyPayloads {
    DALLY_PAYLOAD_BYTES                     = 20,
    DALLY_PAYLOAD_WORDS                     = (DALLY_PAYLOAD_BYTES / sizeof(dally_word_t));
    DALLY_PAYLOAD_DATA_BYTES                = (DALLY_PAYLOAD_BYTES - 2),
    DALLY_PAYLOAD_DATA_WORDS                = (DALLY_PAYLOAD_DATA_BYTES / sizeof(dally_word_t)),
    DALLY_PAYLOAD_REGISTER_BYTES            = (DALLY_PAYLOAD_DATA_BYTES - 2),
    DALLY_PAYLOAD_REGISTER_WORDS            = (DALLY_PAYLOAD_REGISTER_BYTES / sizeof(dally_word_t)),
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

typedef struct DallyData {
    dally_byte_t header;
    dally_byte_t flag;
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
    dally_byte_t header;
    dally_byte_t sign;
    dally_word_t reg;
    dally_word_t data[DALLY_PAYLOAD_REGISTER_WORDS];
} dally_register_t;

typedef union DallyPacket {
    dally_words_t    w;
    dally_bytes_t    b;
    dally_data_t     d;
    dally_register_t r;
} dally_packet_t;

/******************************************************************************
 * CONVERSIONS
 ******************************************************************************/

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif
