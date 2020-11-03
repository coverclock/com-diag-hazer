/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_DATAGRAM_
#define _H_COM_DIAG_HAZER_GPSTOOL_DATAGRAM_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Datagram API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "types.h"

/**
 * Log connection information.
 * @param label is a label prepended to the output.
 * @param option is the command line endpoint option argument.
 * @param fd is the socket.
 * @param protocol is selects which address to show.
 * @param ipv6p points to an IPv6 address.
 * @param ipv4p points to an IPv4 address.
 * @param port is a port number.
 */
extern void show_connection(const char * label, const char * option, int fd, protocol_t protocol, const diminuto_ipv6_t * ipv6p, const diminuto_ipv4_t * ipv4p, diminuto_port_t port);

/**
 * Send an datagram to a remote IPv4 or IPv6 host and UDP port.
 * @param fd is an open socket.
 * @param protocol indicates either IPv4 or IPv6.
 * @param ipv4p points to an IPv4 address (if IPv4).
 * @param ipv6p points to an IPv6 address (if IPv6).
 * @param port is an IP UDP port.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 * @return the size of the sent datagram in bytes or <0 if an error occurred.
 */
extern ssize_t send_datagram(int fd, protocol_t protocol, const diminuto_ipv4_t * ipv4p, const diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size);

/**
 * Receive a datagram from a UDP port. The datagram will be NUL terminated.
 * The provided buffer must be sized one more byte than the received datagram.
 * @param fd is an open socket.
 * @param buffer points to the buffer.
 * @param size is the size of the buffer in bytes.
 * @return the size of the received datagram in bytes or <0 if an error occurred.
 */
extern ssize_t receive_datagram(int fd, void * buffer, size_t size);

#endif
