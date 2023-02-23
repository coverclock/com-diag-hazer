/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes and displays the checksum of an NMEA, UBX, or RTCM message.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Sends an empty RTK packet with a Hazer datagram header to a remote
 * UDP port. If a response datagram is received, the Hazer header is
 * stripped off and the remainder of the datagram (which may be binary and
 * unprintable) is emitted to standard output. In normal usage, this
 * response will be an RTK correction from a differential GNSS base
 * station. This basically simulates what a Hazer Tumbleweed differential
 * GNSS rover does without the need for a relatively expensive DGNSS device.
 * Developed for use with a Hazer Tumbleweed differential GNSS base station
 * and a Codex Stagecoach OpenSSL tunnel.
 *
 * The command line flags match what gpstool uses for the same parameters.
 * The 'Y' is supposed to remind you of the word "surveyor".
 *
 * USAGE
 *
 * rtk2dgm [ -Y HOST:PORT ] [ -y SECONDS ]
 * 
 * EXAMPLE
 *
 * rtk2dgm -Y eljefe:tumbleweed -y 1
 *
 * REFERENCES
 *
 * https://github.com/coverclock/com-diag-codex
 */

/*******************************************************************************
 * DEPENDENCIES
 ******************************************************************************/

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_ipc.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>

/*
 * CONSTANTS
 */

enum Constants {
    DATAGRAM = 65527 /* max(datagram)=(2^16-1)-8 */
};

/*
 * TYPES
 */

typedef union Buffer {
    diminuto_ipv6_buffer_t buffer6;
    diminuto_ipv4_buffer_t buffer4;
    uint8_t buffer[0];
} buffer_t;

typedef uint32_t sequence_t;

typedef struct Request {
    sequence_t header;
    uint8_t payload[6];
} request_t;

typedef struct Response {
    sequence_t header;
    uint8_t payload[DATAGRAM - sizeof(sequence_t)];
} response_t;

/*
 * MAIN
 */

int main(int argc, char * argv[])
{
    extern char * optarg;
    int xc = 1;
    const char * program = (const char *)0;
    int opt = -1;
    bool error = false;
    char * end = (char *)0;
    const char * endpointname = (const char *)0;
    long seconds = 0;
    diminuto_sticks_t period = 25000000000LL;
    diminuto_sticks_t timeout = 12500000000LL;;
    diminuto_ticks_t then = 0;
    diminuto_ticks_t now = 0;
    request_t request = { 0, { 0xd3, 0x00, 0x00, 0x47, 0xea, 0x4b } };
    response_t response = { 0, { 0, } };
    sequence_t sending = 0;
    sequence_t received = 0;
    sequence_t expected = 0;
    diminuto_ipc_endpoint_t endpoint = { 0, };
    diminuto_mux_t mux = { 0, };
    diminuto_mux_t * muxp = (diminuto_mux_t *)0;
    diminuto_ipv4_t ipv4 = 0;
    diminuto_ipv6_t ipv6 = { 0, };
    diminuto_port_t port = 0;
    buffer_t source = { { '\0', } };
    buffer_t sink = { { '\0', } };
    int sock = -1;
    int rc = 0;
    int nfds = 0;
    int fd = -1;
    ssize_t bytes = -1;
    size_t size = 0;
    bool first = true;

    do {

        diminuto_log_setmask();

        /*
         * PARSE
         */

        program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

        while ((opt = getopt(argc, argv, "?Y:y:")) >= 0) {
            switch (opt) {
            case 'Y':
                if (diminuto_ipc_endpoint(optarg, &endpoint) != 0) {
                    diminuto_perror(optarg);
                    error = true;
                    break;
                } else if (
                        ((endpoint.type != DIMINUTO_IPC_TYPE_IPV4) &&
                            (endpoint.type != DIMINUTO_IPC_TYPE_IPV6)) ||
                        ((diminuto_ipc4_is_unspecified(&endpoint.ipv4) &&
                            diminuto_ipc6_is_unspecified(&endpoint.ipv6))) ||
                        (endpoint.udp == 0)) {
                    errno = EINVAL;
                    diminuto_perror(optarg);
                    error = true;
                    break;
                } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
                    endpointname = optarg;
                    sock = diminuto_ipc4_datagram_peer(0);
                    diminuto_assert(sock >= 0);
                    (void)diminuto_ipc4_address2string(endpoint.ipv4, &sink, sizeof(sink));
                } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
                    endpointname = optarg;
                    sock = diminuto_ipc6_datagram_peer(0);
                    diminuto_assert(sock >= 0);
                    (void)diminuto_ipc6_address2string(endpoint.ipv6, &sink, sizeof(sink));
                } else {
                    errno = EINVAL;
                    diminuto_perror(optarg);
                    break;
                }
                break;
            case 'y':
                seconds = strtol(optarg, &end, 0);
                if ((end == (char *)0) || (*end != '\0') || (seconds < 0)) {
                    errno = EINVAL;
                    diminuto_perror(optarg);
                    error = true;
                } else {
                    period = diminuto_frequency_units2ticks(seconds, 1 /* Hz */);
                    timeout = period / 2;
                }
                break;
            default:
                fprintf(stderr, "usage: %s [ -? ] [ -Y HOST:PORT ] [ -y SECONDS ]\n", program);
                error = true;
                break;
            }
        }

        if (error) {
            /* Do nothing. */
        } else if (endpointname != (const char *)0) {
            /* Do nothing. */
        } else {
            errno = EINVAL;
            diminuto_perror("-Y HOST:PORT");
            error = true;
        }

        if (error) {
            break;
        }

        /*
         * INITIALIZE
         */

        muxp = diminuto_mux_init(&mux);
        diminuto_assert(muxp != (diminuto_mux_t *)0);

        rc = diminuto_mux_register_read(&mux, sock);
        diminuto_assert(rc >= 0);

        rc = diminuto_terminator_install(0);
        diminuto_assert(rc >= 0);

        then = diminuto_time_elapsed() - period;

        /*
         * WORK LOOP
         */

        for (;;) {

            /*
             * WAIT
             */
 
            nfds = diminuto_mux_wait(&mux, timeout);
            diminuto_assert((nfds >= 0) || ((nfds < 0) && (errno == EINTR)));

            /*
             * CHECK
             */

            if (diminuto_terminator_check()) {
                xc = 0;
                break;
            }

            /*
             * RECEIVE
             */

            if (nfds <= 0) {
                /* Do nothing. */
            } else if ((fd = diminuto_mux_ready_read(&mux)) != sock) {
                diminuto_assert(false);
            } else {
                switch (endpoint.type) {
                case DIMINUTO_IPC_TYPE_IPV4:
                    bytes = diminuto_ipc4_datagram_receive_generic(sock, &response, sizeof(response), &ipv4, &port, 0);
                    diminuto_assert(bytes > 0);
                    (void)diminuto_ipc4_address2string(ipv4, &source, sizeof(source));
                    rc = diminuto_ipc4_compare(&ipv4, &endpoint.ipv4);
                    break;
                case DIMINUTO_IPC_TYPE_IPV6:
                    bytes = diminuto_ipc6_datagram_receive_generic(sock, &response, sizeof(response), &ipv6, &port, 0);
                    diminuto_assert(bytes > 0);
                    (void)diminuto_ipc6_address2string(ipv6, &source, sizeof(source));
                    rc = diminuto_ipc6_compare(&ipv6, &endpoint.ipv6);
                    break;
                default:
                    diminuto_assert(false);
                    break;
                }
                error = false;
                if (rc != 0) {
                    fprintf(stderr, "%s: address!\n", program);
                    error = true;
                }
                if (port != endpoint.udp) {
                    fprintf(stderr, "%s: port!\n", program);
                    error = true;
                }
                if (bytes <= sizeof(sequence_t)) {
                    fprintf(stderr, "%s: size!\n", program);
                    error = true;
                }
                if (!error) {
                    received = be32toh(response.header);
                    if (first) {
                        first = false;
                    } else if (received == expected) {
                        /* Do nothing. */
                    } else {
                        fprintf(stderr, "%s: sequence! (%u!=%u)\n", program, received, expected);
                    }
                    expected = received + 1;
                    size = fwrite(&response.payload, bytes - sizeof(sequence_t), 1, stdout);
                    diminuto_assert(size == 1);
                }
            }

            /*
             * TRANSMIT
             */

            now = diminuto_time_elapsed();
            if ((now - then) >= period) {
                request.header = htobe32(sending);
                switch (endpoint.type) {
                case DIMINUTO_IPC_TYPE_IPV4:
                    bytes = diminuto_ipc4_datagram_send(sock, &request, sizeof(request), endpoint.ipv4, endpoint.udp);
                    diminuto_assert(bytes == sizeof(request));
                    break;
                case DIMINUTO_IPC_TYPE_IPV6:
                    bytes = diminuto_ipc6_datagram_send(sock, &request, sizeof(request), endpoint.ipv6, endpoint.udp);
                    diminuto_assert(bytes == sizeof(request));
                    break;
                default:
                    diminuto_assert(false);
                    break;
                } 
                sending += 1;
                then = now;
            }

        }

    } while (false);

    /*
     * FINALIZE
     */

    if (sock >= 0) {
        sock = diminuto_ipc_close(sock);
        diminuto_assert(sock < 0);
    }

    exit(xc);
}
