/* vi: set ts=4 expandtab shiftwidth=4: */
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
 * CONSTANTS
 ******************************************************************************/

enum DallyWt901OutputConstants {
    DALLY_WIT901_HEADING                = (uint8_t)0x55,
    DALLY_WIT901_FLAG_MAGNETICFIELD     = (uint8_t)0x71,
    DALLY_WIT901_OUTPUT_MAGNETICFIELD   = (uint8_t)0x3a,
    DALLY_WIT901_OUTPUT_QUATERNION      = (uint8_t)0x51,
    DALLY_WIT901_OUTPUT_TEMPERATURE     = (uint8_t)0x40,
    DALLY_WIT901_EXTRA_3                = (uint8_t)0x00,
};

enum DallyWit901CommandConstants {
    DALLY_WIT901_COMMAND_0              = (uint8_t)0xff,
    DALLY_WIT901_COMMAND_1              = (uint8_t)0xaa,
};

/******************************************************************************
 * TYPES
 ******************************************************************************/

typedef int16_t dally_raw;

typedef float dally_datum;

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif
