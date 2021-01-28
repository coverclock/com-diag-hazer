/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the rtktool RTK router.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * osmtool is a point-to-multipoint forwarder that receives UDP datagrams from
 * a source and forwards them over a connected TCP stream to a sink. The
 * source is typically a mobile unit sending GPS/GNSS updates, and the
 * sink is a fixed unit using an OpenStreetMaps tile server to create a
 * moving map display. Both the source and the sink connect to the forwarder,
 * so the only configuration necessary to the forwarder is its UDP and TCP
 * port numbers. Note that the same port number can be (and typically is)
 * used for both the UDP source and the TCP sink side. osmtool was developed
 * in support of the Tesoro project, which has its own source code repository.
 *
 * USAGE
 *
 * osmtool [ -? ] [ -d ] [ -v ] [ -b BYTES ] [ -M ] [ -V ] [ -u :PORT ] [ -t :PORT ]
 *
 * EXAMPLES
 *
 * osmtool -u :22020 -t :22020
 */

#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

/*******************************************************************************
 * GLOBALS
 ******************************************************************************/

/**
 * This is our program name as provided by the run-time system.
 */
static const char * Program = (const char *)0;

/*******************************************************************************
 * MAIN
 ******************************************************************************/

/**
 * Run the main program.
 * @param argc is the number of tokens on the command line argument list.
 * @param argv contains the tokens on the command line argument list.
 * @return the exit value of the main program.
 */
int main(int argc, char * argv[])
{
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int daemon = 0;
    int udpsock = -1;
    int tcpsock = -1;
    int error = 0;
    int ready = 0;
    int rc = 0;
    int fd = -1;
    char * udprendezvous = (char *)0;
    char * tcprendezvous = (char *)0;
    char * buffer = (char *)0;
    char * here = (char *)0;
    diminuto_ipc_endpoint_t udpendpoint = { 0, };
    diminuto_ipc_endpoint_t tcpendpoint = { 0, };
    diminuto_ipv6_t address = { 0, };
    diminuto_port_t port = 0;
    diminuto_ipv6_buffer_t ipv6 = { 0, };
    diminuto_ticks_t frequency = 0;
    ssize_t total = 512;
    ssize_t size = 0;
    ssize_t length = 0;
    diminuto_mux_t mux = { 0 };
    static const char OPTIONS[] = "MVb:dt:u:v?";
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

    /***************************************************************************
     * PREINITIALIZATION
     **************************************************************************/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;

    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);

    diminuto_log_setmask();

    /***************************************************************************
     * OPTIONS
     **************************************************************************/

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case 'M':
            daemon = !0;
            break;
        case 'V':
            DIMINUTO_LOG_INFORMATION("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'b':
            total = strtol(optarg, &here, 0);
            if ((here == (char *)0) || (*here != '\0') || (total <= 0)) { diminuto_perror(optarg); error = !0; }
            break;
        case 'd':
            debug = !0;
            break;
        case 't':
            tcprendezvous = optarg;
            rc = diminuto_ipc_endpoint(tcprendezvous, &tcpendpoint);
            if ((rc < 0) || (tcpendpoint.tcp == 0)) { diminuto_perror(optarg); error = !0; }
            break;
        case 'u':
            udprendezvous = optarg;
            rc = diminuto_ipc_endpoint(udprendezvous, &udpendpoint);
            if ((rc < 0) || (udpendpoint.udp == 0)) { diminuto_perror(optarg); error = !0; }
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -v ] [ -M ] [ -V ] [ -b BYTES ] [ -t :PORT ] [ -u :PORT ]\n", Program);
            fprintf(stderr, "       -M          Run in the background as a daeMon.\n");
            fprintf(stderr, "       -V          Log Version in the form of release, vintage, and revision.\n");
            fprintf(stderr, "       -b BYTES    Allocate a buffer of size BYTES.\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
            fprintf(stderr, "       -t :PORT    Use PORT as the TCP source port.\n");
            fprintf(stderr, "       -u :PORT    Use PORT as the UDP sink port.\n");
            fprintf(stderr, "       -v          Display Verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    if (error) {
        return 1;
    }

    /***************************************************************************
     * INITIALIZATION
     **************************************************************************/

    DIMINUTO_LOG_INFORMATION("Begin");

    if (daemon) {
        rc = diminuto_daemon(Program);
        DIMINUTO_LOG_NOTICE("Daemon %s %d %d %d %d", Program, rc, (int)getpid(), (int)getppid(), (int)getsid(getpid()));
        diminuto_assert(rc == 0);
    }

    rc = diminuto_terminator_install(0);
    diminuto_assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    diminuto_assert(rc >= 0);

    diminuto_mux_init(&mux);

    udpsock = diminuto_ipc6_datagram_peer(udpendpoint.udp);
    diminuto_assert(udpsock >= 0);
    DIMINUTO_LOG_NOTICE("Sink (%d) \"%s\" [%s]:%d", udpsock, udprendezvous, diminuto_ipc6_address2string(udpendpoint.ipv6, ipv6, sizeof(ipv6)), udpendpoint.udp);

    rc = diminuto_mux_register_read(&mux, udpsock);
    diminuto_assert(rc >= 0);

    tcpsock = diminuto_ipc6_stream_provider(tcpendpoint.tcp);
    diminuto_assert(tcpsock >= 0);
    DIMINUTO_LOG_NOTICE("Source (%d) \"%s\" [%s]:%d", tcpsock, tcprendezvous, diminuto_ipc6_address2string(tcpendpoint.ipv6, ipv6, sizeof(ipv6)), tcpendpoint.udp);

    rc = diminuto_mux_register_accept(&mux, tcpsock);
    diminuto_assert(rc >= 0);

    frequency = diminuto_frequency();
    diminuto_assert(frequency > 0);

    DIMINUTO_LOG_NOTICE("Buffer %zd\n", total);
    buffer = (char *)malloc(total);
    diminuto_assert(buffer != (char *)0);

    /***************************************************************************
     * WORK
     **************************************************************************/

    DIMINUTO_LOG_INFORMATION("Start");

    while (!0) {

        /*
         * Check our signal handlers.
         */

        if (diminuto_terminator_check()) {
            DIMINUTO_LOG_NOTICE("SIGTERM");
            break;
        }

        if (diminuto_interrupter_check()) {
            DIMINUTO_LOG_NOTICE("SIGINT");
            break;
        }

        /*
         * Wait until a socket needs to be serviced... or we time out.
         */

        if ((ready = diminuto_mux_wait(&mux, frequency)) == 0) {
            continue;
        } else if (ready > 0) {
            if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
                /* Fall through. */
            } else if ((fd = diminuto_mux_ready_accept(&mux)) >= 0) {
                /* Fall through. */
            } else if ((fd = diminuto_mux_ready_write(&mux)) >= 0) {
                continue;
            } else {
                continue;
            }
        } else if (errno == EINTR) {
            continue;
        } else {
            diminuto_assert(0);
        }

        /*
         * Service the socket.
         */

        if (fd == udpsock) {

            size = diminuto_ipc6_datagram_receive_generic(udpsock, buffer, total, &address, &port, 0);
            DIMINUTO_LOG_DEBUG("Receive %d %zd [%s]:%d\n", udpsock, size, diminuto_ipc6_address2string(address, ipv6, sizeof(ipv6)), port);
            if (size <= 0) {
                continue;
            }

            for (fd = mux.write.min; fd <= mux.write.max; ++fd) {
                if (FD_ISSET(fd, &mux.write.active)) {
                    size = diminuto_ipc6_stream_write(fd, buffer, size);
                    DIMINUTO_LOG_DEBUG("Write %d %zd\n", fd, size);
                    if (size <= 0) {
                        DIMINUTO_LOG_INFORMATION("Close %d", fd);
                        rc = diminuto_mux_close(&mux, fd);
                        diminuto_assert(rc >= 0);
                    }
                }
            }

        } else if (fd == tcpsock) {

            fd = diminuto_ipc6_stream_accept_generic(tcpsock, &address, &port);
            if (fd >= 0) {
                DIMINUTO_LOG_INFORMATION("Accept %d [%s]:%d\n", fd, size, diminuto_ipc6_address2string(address, ipv6, sizeof(ipv6)), port);
                rc = diminuto_mux_register_write(&mux, fd);
                diminuto_assert(rc >= 0);
            }

        } else {

            DIMINUTO_LOG_WARNING("Unexpected %d\n", fd);

        }

    }

    /***************************************************************************
     * FINALIZATION
     **************************************************************************/

    DIMINUTO_LOG_INFORMATION("Stop");

    (void)diminuto_mux_close(&mux, udpsock);

    (void)diminuto_mux_close(&mux, tcpsock);

    for (fd = mux.write.min; fd <= mux.write.max; ++fd) {
        if (FD_ISSET(fd, &mux.write.active)) {
            (void)diminuto_mux_close(&mux, fd);
        }
    }

    diminuto_mux_fini(&mux);

    DIMINUTO_LOG_INFORMATION("Exit");

    return 0;
}
