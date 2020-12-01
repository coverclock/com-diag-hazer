/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_RTKTOOL_TYPES_
#define _H_COM_DIAG_HAZER_RTKTOOL_TYPES_

/**
 * @file
 * @copyright Copyright 2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief These are the type definitions for the rtktool RTK router.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_tree.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/hazer/datagram.h"

/**
 * Clients of the router can be in one of two classes: a stationary base
 * station sending RTK updates, or a mobile (maybe) rover sending keepalives
 * and receiving RTK updates.
 */
typedef enum Class {
    CLASS   = '?',
    BASE    = 'B',
    ROVER   = 'R',
} class_t;

/**
 * This structure describes the state we have to maintain about clients.
 */
typedef struct Client {
    diminuto_tree_t node;
    long last;
    datagram_sequence_t sequence;
    class_t classification;
    diminuto_ipv6_t address;
    diminuto_port_t port;
} client_t;

/**
 * @define CLIENT_INITIALIZER
 * This is how we can statically initialize the client structure.
 */
#define CLIENT_INITIALIZER { DIMINUTO_TREE_NULLINIT, 0, 0, CLASSIFICATION, { 0, }, 0, }

#endif
