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
    DALLY_STATE_WAITING                     = 'W',
    DALLY_STATE_SYNCED                      = 'S',
    DALLY_STATE_HEADING                     = 'H',
    DALLY_STATE_FLAG_PERIODIC               = 'P',
    DALLY_STATE_FLAG_REQUESTED              = 'R',
    DALLY_STATE_REQUESTED_MAGNETICFIELD     = 'M',
    DALLY_STATE_REQUESTED_QUATERNION        = 'Q',
    DALLY_STATE_REQUESTED_TEMPERATURE       = 'T',
};

enum DallyLengths {
    DALLY_LENGTH_PERIODIC                   = 20,
    DALLY_LENGTH_REQUESTED_MAGNETICFIELD    = 20,
    DALLY_LENGTH_REQUESTED_QUATERNION       = 13,
    DALLY_LENGTH_REQUESTED_TEMPERATURE      = 6,
};

enum DallyHeadingConstants {
    DALLY_HEADING                           = 0x55U,
};

enum DallyFlagConstants {
    DALLY_FLAG_PERIODIC                     = 0x61U,
    DALLY_FLAG_REQUESTED                    = 0x71U,
};

enum DallyRequestConstants {
    DALLY_REQUESTED_MAGNETICFIELD           = 0x3aU,
    DALLY_REQUESTED_QUATERNION              = 0x51U,
    DALLY_REQUESTED_TEMPERATURE             = 0x40U,
};

enum DallyExtraConstants {
    DALLY_EXTRA                             = 0x00U,
};

enum DallyCommandConstants {
    DALLY_COMMAND_0                         = 0xffU,
    DALLY_COMMAND_1                         = 0xaaU,
};

/******************************************************************************
 * TYPES
 ******************************************************************************/

typedef int16_t dally_raw;

typedef float dally_datum;

/******************************************************************************
 * CONVERSIONS
 ******************************************************************************/

/******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

#endif
