/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_RTKTOOL_
#define _H_COM_DIAG_HAZER_RTKTOOL_

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

#include "com/diag/diminuto/diminuto_tree.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_time.h"

typedef enum Role { ROLE = '?', BASE = 'B', ROVER = 'R', } role_t;

typedef struct Client {
	diminuto_tree_t node;
    long then;
    datagram_sequence_t sequence;
	role_t role;
    diminuto_ipv6_t address;
    diminuto_port_t port;
} client_t;

#define CLIENT_INITIALIZER { DIMINUTO_TREE_NULLINIT, 0, 0, ROLE, { 0, }, 0, }

#endif
