/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes and displays the checksum of an NMEA, UBX, or RTCM message.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Forwards a fixed subset of the CSV output as a datagram to a UDP endpoint.
 * Developed for use with Tesoro, the OpenStreetMaps tile server project.
 *
 * USAGE
 * 
 * csv2udp HOST:PORT
 *
 * INPUT
 *
 * "neon", 11508, 4, 0, 12, 1599156753.732339499, 1599156755.000000000, 39.7943205, -105.1533455, 0., 1708.600, 1687.100, 0., 0.003000, 184.530000000, 0.71916, 0.58270, 184.53446, 0.50630, 0.53894, 0.69224, 0, 0.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "com/diag/diminuto/diminuto_ipc.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_countof.h"

int main(int argc, char * argv[])
{
    int xc = 1;
    int sock = -1;
    int rc = -1;
    int ii = -1;
    diminuto_ipc_endpoint_t endpoint = { 0, };
    char input[512] = { '\0', };
    char * token[23] = { 0, };
    char * pointer = (char *)0;
    char output[256] = { '\0', };
    size_t length = 0;
    ssize_t size = 0;

    do {

        diminuto_log_setmask();

        if (argc < 2) {
            errno = EINVAL;
            diminuto_perror(argv[0]);
            break;
        }

        if (diminuto_ipc_endpoint(argv[1], &endpoint) < 0) {
            errno = EINVAL;
            diminuto_perror(argv[1]);
            break;
        }

        if (endpoint.udp == 0) {
            errno = EINVAL;
            diminuto_perror(argv[1]);
            break;
        }

        if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
            sock = diminuto_ipc4_datagram_peer(0);
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
            sock = diminuto_ipc6_datagram_peer(0);
        } else {
            break;
        }

        if (sock < 0) {
            break;
        }

        for (;;) {

            if (fgets(input, sizeof(input), stdin) == (char *)0) {
                xc = 0;
                break;
            }
            input[sizeof(input) - 1] = '\0';

            if (strncmp(input, "NAM, ", sizeof("NAM, ")  - 1) == 0) {
                continue;
            }

            for (ii = 0; ii < diminuto_countof(token); ++ii) {
                if (ii == 0) {
                    token[ii] = strtok_r(input, ", ", &pointer);
                } else if (ii == (diminuto_countof(token) - 1)) {
                    token[ii] = strtok_r(pointer, "\n", &pointer);
                } else {
                    token[ii] = strtok_r(pointer, ", ", &pointer);
                }
                if (token[ii] == (char *)0) {
                    break;
                }
                DIMINUTO_LOG_DEBUG("token[%d]=\"%s\"\n", ii, token[ii]);
            }

            if (ii != diminuto_countof(token)) {
                errno = EIO;
                diminuto_perror("strtok_r");
                continue;
            }

            snprintf(output, sizeof(output), "%s %s %s %s", token[6], token[7], token[8], token[10]);
            output[sizeof(output) - 1] = '\0';
            length = strnlen(output, sizeof(output));

            DIMINUTO_LOG_DEBUG("output=\"%s\"\n", output);

            if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
                size = diminuto_ipc4_datagram_send(sock, output, length, endpoint.ipv4, endpoint.udp);
            } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
                size = diminuto_ipc6_datagram_send(sock, output, length, endpoint.ipv6, endpoint.udp);
            } else {
                break;
            }

            if (size <= 0) {
                break;
            }

        }

    } while (0);

    return xc;
}
