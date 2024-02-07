/* vi: set ts    =4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_DALLY_
#define _H_COM_DIAG_HAZER_DALLY_

/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Support for proprietary messaging as used by some WITMOTION devices.
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
 * SYMBOLICS
 ******************************************************************************/

enum DallyStates {
    DALLY_STATE_ERROR                       = '?',
    DALLY_STATE_START                       = 'S',
    DALLY_STATE_HEADING                     = 'H',
    DALLY_STATE_FLAG_DATA                   = 'P',
    DALLY_STATE_FLAG_REGISTER               = 'R',
    DALLY_STATE_DATA                        = 'D',
    DALLY_STATE_REGISTER_MAGNETICFIELD      = 'M',
    DALLY_STATE_REGISTER_QUATERNION         = 'Q',
    DALLY_STATE_REGISTER_TEMPERATURE        = 'T',
};

enum DallyPayload {
    DALLY_PAYLOAD_TOTAL                     = 20,
    DALLY_PAYLOAD_DATA                      = (DALLY_PAYLOAD_TOTAL - 2),
    DALLY_PAYLOAD_REGISTER                  = (DALLY_PAYLOAD_DATA - 2),
    DALLY_PAYLOAD_REGISTER_PAIRS            = (DALLY_PAYLOAD_REGISTER / 2),
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
 * TYPES
 ******************************************************************************/

typedef int16_t dally_word_t;

typedef double dally_datum_t;

typedef uint8_t (dally_packet_t)[DALLY_PAYLOAD_TOTAL];

typedef struct DallyData {
    uint8_t header;
    uint8_t flag;
    int8_t  axL;
    int8_t  axH;
    int8_t  ayL;
    int8_t  ayH;
    int8_t  azL;
    int8_t  azH;
    int8_t  wxL;
    int8_t  wxH;
    int8_t  wyL;
    int8_t  wyH;
    int8_t  wzL;
    int8_t  wzH;
    int8_t  rollL;
    int8_t  rollH;
    int8_t  pitchL;
    int8_t  pitchH;
    int8_t  yawL;
    int8_t  yawH;
} dally_data_t;

typedef struct DallyPair {
    int8_t dataL;
    int8_t dataH;
} dally_pair_t;

typedef struct DallyRegister {
    uint8_t header;
    uint8_t sign;
    uint8_t regL;
    uint8_t regH;
    dally_pair_t data[DALLY_PAYLOAD_REGISTER_PAIRS];
} dally_register_t;

/******************************************************************************
 * CONVERSIONS
 ******************************************************************************/

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif
