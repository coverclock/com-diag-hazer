/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2022 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the gpstool Datagram API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_ipc.h"
#include "com/diag/hazer/datagram.h"
#include "types.h"
#include "datagram.h"

protocol_t choose_protocol(const diminuto_ipc_endpoint_t * ep, protocol_t preference)
{
    protocol_t protocol = PROTOCOL;

    if (!diminuto_ipc6_is_unspecified(&(ep->ipv6))) {
        if (diminuto_ipc4_is_unspecified(&(ep->ipv4))) {
            protocol = IPV6;    /* IPV6 available but not IPV4. */
        } else if (preference == IPV4) {
            protocol = IPV4;    /* IPV6 and IPV4 available. */
        } else if (preference == IPV6) {
            protocol = IPV6;    /* IPV6 and IPV4 available. */
        } else {
            protocol = IPV6;    /* No preference. */
        }
    } else if (!diminuto_ipc4_is_unspecified(&(ep->ipv4))) {
        protocol = IPV4;        /* IPV4 available but not IPV6. */
    } else {
        protocol = PROTOCOL;    /* Neither available, probably a consumer. */
    }

    return protocol;
}

void show_connection(const char * label, const char * option, int fd, protocol_t protocol, const diminuto_ipv6_t * ipv6p, const diminuto_ipv4_t * ipv4p, diminuto_port_t port)
{
    diminuto_ipv4_buffer_t ipv4;
    diminuto_ipv6_buffer_t ipv6;

    switch (protocol) {

    case IPV6:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\" [%s]:%d", label, fd, option, diminuto_ipc6_address2string(*ipv6p, ipv6, sizeof(ipv6)), port);
        break;

    case IPV4:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\" %s:%d", label, fd, option, diminuto_ipc4_address2string(*ipv4p, ipv4, sizeof(ipv4)), port);
        break;

    case PROTOCOL:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\"", label, fd, option);
        break;

    }

}

ssize_t send_datagram(int fd, protocol_t protocol, const diminuto_ipv4_t * ipv4p, const diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size)
{
    ssize_t length = 0;

    if (size <= 0) {
        /* Do nothing. */
    } else if (protocol == IPV4) {
        length = diminuto_ipc4_datagram_send(fd, buffer, size, *ipv4p, port);
    } else if (protocol == IPV6) {
        length = diminuto_ipc6_datagram_send(fd, buffer, size, *ipv6p, port);
    } else {
        /* Do nothing. */
    }

    return length;
}

ssize_t receive_datagram(int fd, void * buffer, size_t size) {
    ssize_t length = 0;
    diminuto_ipv6_t address = { 0, };
    diminuto_port_t port = 0;

    if (size <= 1) {
        /* Do nothing. */
    } else if ((length = diminuto_ipc6_datagram_receive_generic(fd, buffer, size - 1, &address, &port, 0)) <= 0) {
        /* Do nothing. */
    } else if (length >= size) {
        /* Should be impossible. */
    } else {
        ((uint8_t *)buffer)[length++] = '\0';
    }

    return length;
}
