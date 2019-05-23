/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_
#define _H_COM_DIAG_HAZER_GPSTOOL_

/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * This file is part of the Digital Aggregates Corporation Hazer package.
 */

#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_list.h"

/*******************************************************************************
 * DATAGRAM BUFFER
 ******************************************************************************/

/**
 * Datagram Constants.
 */
enum DatagramConstants {
	DATAGRAM_LONGEST = ((HAZER_NMEA_LONGEST > YODEL_UBX_LONGEST)
						? ((HAZER_NMEA_LONGEST > TUMBLEWEED_RTCM_LONGEST) ? HAZER_NMEA_LONGEST : TUMBLEWEED_RTCM_LONGEST)
						: ((YODEL_UBX_LONGEST > TUMBLEWEED_RTCM_LONGEST) ? YODEL_UBX_LONGEST : TUMBLEWEED_RTCM_LONGEST)),
};

/**
 * This buffer is large enough to the largest UDP datagram we are willing to
 * support, plus a trailing NUL. It's not big enough to hold any datagram
 * (that would be in the neighborhood of 65508 bytes). But it will for sure
 * hold a NMEA, UBX, or RTCM payload.
 */
typedef unsigned char (datagram_buffer_t)[DATAGRAM_LONGEST + 1];

/**
 * @define DATAGRAM_BUFFER_INITIALIZER
 * Initialize a DatagramBuffer type.
 */
#define DATAGRAM_BUFFER_INITIALIZER  { '\0', }

/*******************************************************************************
 * HIGH PRECISION SOLUTION
 ******************************************************************************/

/**
 * Structure combining both a UBX-NAV-HPPOSLLH payload and its expiry time in
 * ticks.
 */
typedef struct YodelSolution {
	yodel_ubx_nav_hpposllh_t payload;   /* Payload from UBX-NAV-HPPOSLLH message. */
    expiry_t ticks;                     /* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_solution_t;

/**
 * @define YODEL_SOLUTION_INITIALIZER
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
    yodel_ubx_mon_hw_t payload;	/* Payload from UBX-MON-HW message. */
    expiry_t ticks;				/* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_hardware_t;

/**
 * @define YODEL_HARDWARE_INITIALIZER
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
    yodel_ubx_nav_status_t payload;	/* Payload from UBX-NAV-STATUS message. */
    expiry_t ticks;					/* Lifetime in application-defined ticks. */
    uint8_t unused[3];
} yodel_status_t;

/**
 * @define YODEL_STATUS_INITIALIZER
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
    yodel_ubx_nav_svin_t payload;	/* Payload from UBX-NAV-SVIN message. */
    expiry_t ticks;					/* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_base_t;

/**
 * @define YODEL_BASE_INITIALIZER
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
    yodel_ubx_rxm_rtcm_t payload;	/* Payload from UBX-RXM-RTCM message. */
    expiry_t ticks;					/* Lifetime in application-defined ticks. */
    uint8_t unused[7];
} yodel_rover_t;

/**
 * @define YODEL_ROVER_INITIALIZER
 * Initialize a YodelRover structure.
 */
#define YODEL_ROVER_INITIALIZER \
    { \
	    YODEL_UBX_RXM_RTCM_INITIALIZER, \
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
	int number;			/* Message number e.g. 1005. */
	ssize_t length;
	ssize_t minimum;
	ssize_t maximum;
	expiry_t ticks;		/* Lifetime in application-defined ticks. */
} tumbleweed_message_t;

/**
 * @define TUMBLEWEED_MESSAGE_INTIALIZER
 * Initialize a TumbleweedMessage structure.
 */
#define TUMBLEWEED_MESSAGE_INITIALIZER \
    { \
        0, \
		0, \
		TUMBLEWEED_RTCM_LONGEST, \
		0, \
		0, \
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
struct Poller {
    FILE * ppsfp;
    FILE * strobefp;
    volatile int onepps;
    volatile int done;
};

/**
 * The Command structure contains a linked list node whose data pointer
 * points to the command we want to send, and the acknak field indicates
 * whether this command expects an UBX CFG ACK or a NAK from the device.
 */
struct Command {
	diminuto_list_t link;
	int acknak;
};

/*******************************************************************************
 * ENUMERATIONS
 ******************************************************************************/

typedef enum Role { ROLE = 0, PRODUCER = 1, CONSUMER = 2, } role_t;

typedef enum Direction { DIRECTION = 0, INPUT = (1<<0), OUTPUT = (1<<1) } direction_t;

typedef enum Protocol { PROTOCOL = 0, IPV4 = 4, IPV6 = 6, } protocol_t;

typedef enum Format { FORMAT = 0, NMEA = (1<<0), UBX = (1<<1), RTCM = (1<<2), } format_t;

typedef enum Status { STATUS = '#', UNKNOWN = '?', NONE = '-', WARNING = '+', CRITICAL = '!', INVALID = '*', } status_t;

typedef enum Marker { MARKER = '#', INACTIVE = ' ', ACTIVE = '<', PHANTOM = '?', UNTRACKED = '!', } marker_t;

#endif
