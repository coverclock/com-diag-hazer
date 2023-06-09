/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_TYPES_
#define _H_COM_DIAG_HAZER_GPSTOOL_TYPES_

/**
 * @file
 * @copyright Copyright 2019-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This defines the gpstool Types.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <pthread.h>
#include "com/diag/diminuto/diminuto_types.h"
#include "com/diag/diminuto/diminuto_list.h"
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/datagram.h"

/*******************************************************************************
 * INTEGERS
 ******************************************************************************/

/**
 * Monotonic elapsed seconds.
 */
typedef int64_t seconds_t;

/*******************************************************************************
 * ENUMERATIONS
 ******************************************************************************/

/**
 * Are we producing datagrams, consuming datagrams, or neither?
 */
typedef enum Role {
    ROLE        = '?',
    PRODUCER    = 'P',
    CONSUMER    = 'C',
} role_t;

/**
 * Are we inputting serial data, outputting serial data, or neither?
 */
typedef enum Direction {
    DIRECTION   = '?',
    INPUT       = 'I',
    OUTPUT      = 'O',
} direction_t;

/**
 * Are we using IPv4, IPv6, or not using IP at all?
 */
typedef enum Protocol {
    PROTOCOL    = '?',
    IPV4        = '4',
    IPV6        = '6',
} protocol_t;

/**
 * Are we processing an NMEA sentence, a UBX packet, an RTCM message, a
 * DIS packet, or none of the above?
 */
typedef enum Format {
    FORMAT  = 0,
    NMEA    = (1<<0),
    UBX     = (1<<1),
    RTCM    = (1<<2),
    DIS     = (1<<3),
    ANY     = NMEA | UBX | RTCM | DIS,
} format_t;

/**
 * Are we receiving RTCM updates from the device (in which case we are a fixed
 * base station in survey mode) or from the network (in which case we are a
 * mobile rover)?
 */
typedef enum Source {
    SOURCE  = '?',
    DEVICE  = 'D',
    NETWORK = 'N',
} source_t;

/**
 * What is our jamming status?
 */
typedef enum Status {
    STATUS      = '#',
    UNKNOWN     = '?',
    NONE        = '-',
    WARNING     = '+',
    CRITICAL    = '!',
    INVALID     = '*',
} status_t;

/**
 * How have we classified a satellite track?
 */
typedef enum Marker {
    MARKER      = '#',
    INACTIVE    = ' ',
    ACTIVE      = '<',
    PHANTOM     = '?',
    UNTRACKED   = '!',
    UNUSED      = '-',
} marker_t;

/**
 * What update did we receive?
 */
typedef enum Update {
    UPDATE          = '.',
    RTCM_TYPE_1005  = 'B',
    RTCM_TYPE_1074  = 'N',
    RTCM_TYPE_1084  = 'R',
    RTCM_TYPE_1094  = 'E',
    RTCM_TYPE_1124  = 'C',
    RTCM_TYPE_1230  = 'r',
    RTCM_TYPE_9999  = '?',
} update_t;

/**
 *  What kind of data is being emitted?
 */
typedef enum Emission {
    EMISSION        = '?',
    OPT_A           = 'A',
    OPT_W           = 'W',
    OPT_U           = 'U',
    OPT_Z           = 'Z',
} emission_t;

/*******************************************************************************
 * HIGH PRECISION SOLUTION
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV-HPPOSLLH payload and its expiry time in
 * ticks.
 */
typedef struct YodelSolution {
    yodel_ubx_nav_hpposllh_t payload;   /* Payload from UBX-NAV-HPPOSLLH message. */
    hazer_expiry_t ticks;               /* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_solution_t;

/**
 * @def YODEL_SOLUTION_INITIALIZER
 * Initialize a YodelSolution structure.
 */
#define YODEL_SOLUTION_INITIALIZER \
    { \
        YODEL_UBX_NAV_HPPOSLLH_INITIALIZER, \
        0, \
        { 0, } \
    }

/*******************************************************************************
 * HARDWARE MONITOR
 ******************************************************************************/

/**
 * Structure combining both a UBX-MON-HW payload and its expiry time in ticks.
 */
typedef struct YodelHardware {
    yodel_ubx_mon_hw_t payload;     /* Payload from UBX-MON-HW message. */
    hazer_expiry_t ticks;           /* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_hardware_t;

/**
 * @def YODEL_HARDWARE_INITIALIZER
 * Initialize a YodelHardware structure.
 */
#define YODEL_HARDWARE_INITIALIZER \
    { \
        YODEL_UBX_MON_HW_INITIALIZER, \
        0, \
        { 0, } \
    }

/*******************************************************************************
 * NAVIGATION STATUS
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV-STATUS payload and its expiry time in ticks.
 */
typedef struct YodelStatus {
    yodel_ubx_nav_status_t payload; /* Payload from UBX-NAV-STATUS message. */
    hazer_expiry_t ticks;           /* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_status_t;

/**
 * @def YODEL_STATUS_INITIALIZER
 * Initialize a YodelStatus structure.
 */
#define YODEL_STATUS_INITIALIZER \
    { \
        YODEL_UBX_NAV_STATUS_INITIALIZER, \
        0, \
        { 0, } \
    }

/*******************************************************************************
 * BASE STATUS
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV-SVIN payload and its expiry time in ticks.
 */
typedef struct YodelBase {
    yodel_ubx_nav_svin_t payload;   /* Payload from UBX-NAV-SVIN message. */
    hazer_expiry_t ticks;           /* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_base_t;

/**
 * @def YODEL_BASE_INITIALIZER
 * Initialize a YodelBase structure.
 */
#define YODEL_BASE_INITIALIZER \
    { \
        YODEL_UBX_NAV_SVIN_INITIALIZER, \
        0, \
        { 0, }, \
    }

/*******************************************************************************
 * ROVER STATUS
 ******************************************************************************/

/**
 * Structure combining both a UBX-RXM-RTCM payload and its expiry time in ticks.
 */
typedef struct YodelRover {
    yodel_ubx_rxm_rtcm_t payload;   /* Payload from UBX-RXM-RTCM message. */
    hazer_expiry_t ticks;           /* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_rover_t;

/**
 * @def YODEL_ROVER_INITIALIZER
 * Initialize a YodelRover structure.
 */
#define YODEL_ROVER_INITIALIZER \
    { \
        YODEL_UBX_RXM_RTCM_INITIALIZER, \
        0, \
        { 0, }, \
    }

/*******************************************************************************
 * VEHICLE ATTITUDE
 ******************************************************************************/

/**
 * Structure combining both a UBX-RXM-RTCM payload and its expiry time in ticks.
 */
typedef struct YodelAttitude {
    yodel_ubx_nav_att_t payload;    /* Payload from UBX-NAV_ATT message. */
    hazer_expiry_t ticks;           /* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_attitude_t;

/**
 * @def YODEL_ATTITUDE_INITIALIZER
 * Initialize a YodelAttitude structure.
 */
#define YODEL_ATTITUDE_INITIALIZER \
    { \
        YODEL_UBX_NAV_ATT_INITIALIZER, \
        0, \
        { 0, }, \
    }

/*******************************************************************************
 * VEHICLE ODOMETER
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV_ODO payload and its expiry time in ticks.
 */
typedef struct YodelOdometer {
    yodel_ubx_nav_odo_t payload;	/* Payload from UBX-NAV_ODO message. */
    hazer_expiry_t ticks;			/* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_odometer_t;

/**
 * @def YODEL_ODOMETER_INITIALIZER
 * Initialize a YodelOdometer structure.
 */
#define YODEL_ODOMETER_INITIALIZER \
    { \
        YODEL_UBX_NAV_ODO_INITIALIZER, \
        0, \
        { 0, }, \
    }

/*******************************************************************************
 * POSITION, VELOCITY, TIME SOLUTION
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV-PVT payload and its expiry time in ticks.
 */
typedef struct YodelPosVelTim {
    yodel_ubx_nav_pvt_t payload;	/* Payload from UBX-NAV-PVT message. */
    hazer_expiry_t ticks;			/* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_posveltim_t;

/**
 * @def YODEL_POSVELTIM_INITIALIZER
 * Initialize a YodelPosVelTim structure.
 */
#define YODEL_POSVELTIM_INITIALIZER \
    { \
        YODEL_UBX_NAV_PVT_INITIALIZER, \
        0, \
        { 0, }, \
    }

/*******************************************************************************
 * RTCM MESSAGE
 ******************************************************************************/

/**
 * Structure combining both a RTCM message number and its expiry time in ticks.
 */
typedef struct TumbleweedMessage {
    ssize_t length;
    int number;				/* Message number e.g. 1005. */
    source_t source;
    hazer_expiry_t ticks;	/* Lifetime in application-defined ticks. */
} tumbleweed_message_t;

/**
 * @def TUMBLEWEED_MESSAGE_INITIALIZER
 * Initialize a TumbleweedMessage structure.
 */
#define TUMBLEWEED_MESSAGE_INITIALIZER \
    { \
        0, \
        0, \
        SOURCE, \
        0, \
    }

/**
 * Union makes it a little simpler to track RTCM messages.
 */
typedef union TumbleweedUpdates {
    uint8_t bytes[sizeof(uint64_t)];
    uint64_t word;
} tumbleweed_updates_t;

/**
 * @def TUMBLEWEED_UPDATES_INITIALIZER
 * Initialize a TumbleweedUpdates union.
 */
#define TUMBLEWEED_UPDATES_INITIALIZER \
    { \
        { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', } \
    }

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * The Poller structure is used by periodic DCD or GPIO poller threads to
 * communicate with the main program about the assertion of the 1Hz 1PPS
 * signal from certain GPS receivers which are so-equipped. The volatile
 * declaration is used to suggest to the compiler that it doesn't optimize
 * use of these variables out since they can be altered by other threads.
 */
typedef struct Poller {
    FILE * ppsfp;
    FILE * strobefp;
    volatile int onepps;
    volatile int done;
} poller_t;

/**
 * The Command structure contains a linked list node whose data pointer
 * points to the command we want to send, and the emission field indicates
 * whether this command expects an UBX CFG ACK or a NAK from the device.
 */
typedef struct Command {
    diminuto_list_t link;
    emission_t emission;
} command_t;

#endif
