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
 * osmtool is a multipoint-to-multipoint forwarder that receives UDP datagrams
 * from sources and forwards them over connected TCP streams to sinks. A
 * source is typically a mobile unit sending GPS/GNSS updates, and a
 * sink is a fixed computer using an OpenStreetMaps (OWM) tile server to create
 * a moving map display. Both sources and sinks connect to the forwarder,
 * so the only configuration necessary to the forwarder is its UDP and TCP
 * port numbers. Note that the same port number can be (and typically is)
 * used for both the UDP source and the TCP sink side. osmtool was developed
 * in support of the Tesoro project, which has its own source code repository.
 * Although osmtool is agnostic as to how many sources and sinks are conneected
 * to it, best results are achieved when there is one of each.
 *
 * USAGE
 *
 * osmtool [ -? ] [ -m ] [ -B BYTES ] [ -F FILE ] [ -T :PORT ] [ -V ] [ -U :PORT ]
 *
 * EXAMPLES
 *
 * export COM_DIAG_DIMINUTO_LOG_MASK=0xff
 * osmtool -U :22020 -T :22020 -F Observation.txt  &
 * csvmeter < ./dat/yodel/20200903/vehicle.csv | csv2dgm -j -U localhost:22020 &
 * socat TCP:localhost:22020 -
 */

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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
    int daemon = 0;
    int udpsock = -1;
    int tcpsock = -1;
    int error = 0;
    int ready = 0;
    int rc = 0;
    int fd = -1;
    FILE * fp = (FILE *)0;
    char * udprendezvous = (char *)0;
    char * tcprendezvous = (char *)0;
    char * filename = (char *)0;
    char * temp = (char *)0;
    char * buffer = (char *)0;
    char * here = (char *)0;
    diminuto_ipc_endpoint_t udpendpoint = { 0, };
    diminuto_ipc_endpoint_t tcpendpoint = { 0, };
    diminuto_ipv6_t address6 = { 0, };
    diminuto_port_t port = 0;
    diminuto_ipv6_buffer_t buffer6 = { 0, };
    diminuto_ticks_t frequency = 0;
    ssize_t total = 512;
    ssize_t size = 0;
    size_t length = 0;
    diminuto_mux_t mux = { 0 };
    static const char OPTIONS[] = "B:F:T:U:Vm?";
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
        case 'm':
            daemon = !0;
            break;
        case 'B':
            total = strtol(optarg, &here, 0);
            if ((here == (char *)0) || (*here != '\0') || (total <= 0)) { diminuto_perror(optarg); error = !0; }
            break;
        case 'F':
            filename = optarg;
            if (strcmp(filename, "-") == 0) {
                fp = stdout;
            } else if ((fp = diminuto_observation_create(filename, &temp)) == (FILE *)0) {
                errno = EINVAL;
                diminuto_perror(filename);
                error = !0;
            } else {
                /* Do nothing. */
            }
            break;
        case 'T':
            tcprendezvous = optarg;
            if (
                    (diminuto_ipc_endpoint(tcprendezvous, &tcpendpoint) != 0) ||
                    ((tcpendpoint.type != DIMINUTO_IPC_TYPE_IPV4) &&
                        (tcpendpoint.type != DIMINUTO_IPC_TYPE_IPV6)) ||
                    (!((diminuto_ipc4_is_unspecified(&tcpendpoint.ipv4) &&
                        diminuto_ipc6_is_unspecified(&tcpendpoint.ipv6)))) ||
                    (tcpendpoint.tcp == 0)
            ) {
                errno = EINVAL;
                diminuto_perror(tcprendezvous);
                error = !0;
            }
            break;
        case 'U':
            udprendezvous = optarg;
            if (
                    (diminuto_ipc_endpoint(udprendezvous, &udpendpoint) != 0) ||
                    ((udpendpoint.type != DIMINUTO_IPC_TYPE_IPV4) &&
                        (udpendpoint.type != DIMINUTO_IPC_TYPE_IPV6)) ||
                    (!((diminuto_ipc4_is_unspecified(&udpendpoint.ipv4) &&
                        diminuto_ipc6_is_unspecified(&udpendpoint.ipv6)))) ||
                    (udpendpoint.udp == 0)
            ) {
                errno = EINVAL;
                diminuto_perror(udprendezvous);
                error = !0;
            }
            break;
        case 'V':
            DIMINUTO_LOG_INFORMATION("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case '?':
        default:
            fprintf(stderr, "usage: %s [ -? ] [ -m ] [ -V ] [ -B BYTES ] [ -T :PORT ] [ -U :PORT ] [ -F FILE ]\n", Program);
            fprintf(stderr, "       -m          Run in the background as a daeMon.\n");
            fprintf(stderr, "       -B BYTES    Allocate a buffer of size BYTES.\n");
            fprintf(stderr, "       -F FILE     Save latest datagram in FILE.\n");
            fprintf(stderr, "       -T :PORT    Use PORT as the TCP source port.\n");
            fprintf(stderr, "       -U :PORT    Use PORT as the UDP sink port.\n");
            fprintf(stderr, "       -V          Log Version in the form of release, vintage, and revision.\n");
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
        DIMINUTO_LOG_INFORMATION("Daemon %s %d %d %d %d", Program, rc, (int)getpid(), (int)getppid(), (int)getsid(getpid()));
        diminuto_assert(rc == 0);
    }

    rc = diminuto_terminator_install(0);
    diminuto_assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    diminuto_assert(rc >= 0);

    rc = diminuto_pipe_install(0);
    diminuto_assert(rc >= 0);

    diminuto_mux_init(&mux);

    if (fp != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Observation (%d) \"%s\"", fileno(fp), filename);
    }

    if (udprendezvous != (char *)0) {
        udpsock = diminuto_ipc6_datagram_peer(udpendpoint.udp);
        diminuto_assert(udpsock >= 0);
        DIMINUTO_LOG_INFORMATION("Source (%d) \"%s\" [%s]:%d", udpsock, udprendezvous, diminuto_ipc6_address2string(udpendpoint.ipv6, buffer6, sizeof(buffer6)), udpendpoint.udp);
        rc = diminuto_mux_register_read(&mux, udpsock);
        diminuto_assert(rc >= 0);
    }

    if (tcprendezvous != (char *)0) {
        tcpsock = diminuto_ipc6_stream_provider(tcpendpoint.tcp);
        diminuto_assert(tcpsock >= 0);
        DIMINUTO_LOG_INFORMATION("Sink (%d) \"%s\" [%s]:%d", tcpsock, tcprendezvous, diminuto_ipc6_address2string(tcpendpoint.ipv6, buffer6, sizeof(buffer6)), tcpendpoint.udp);
        rc = diminuto_mux_register_accept(&mux, tcpsock);
        diminuto_assert(rc >= 0);
    }

    frequency = diminuto_frequency();
    DIMINUTO_LOG_INFORMATION("Frequency %llu\n", (unsigned long long)frequency);
    diminuto_assert(frequency > 0);

    DIMINUTO_LOG_INFORMATION("Buffer %zd\n", total);
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

        if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
            /* Fall out. */
        } else if ((fd = diminuto_mux_ready_accept(&mux)) >= 0) {
            /* Fall out. */
        } else if (diminuto_mux_ready_write(&mux) >= 0) {
            while (diminuto_mux_ready_write(&mux) >= 0) { }
            continue;
        } else if ((ready = diminuto_mux_wait(&mux, frequency)) == 0) {
            continue;
        } else if (ready > 0) {
            if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
                /* Fall out. */
            } else if ((fd = diminuto_mux_ready_accept(&mux)) >= 0) {
                /* Fall out. */
            } else if (diminuto_mux_ready_write(&mux) >= 0) {
                while (diminuto_mux_ready_write(&mux) >= 0) { }
                continue;
            } else {
                DIMINUTO_LOG_WARNING("Unexpected %d\n", ready);
                continue;
            }
        } else if (errno == EINTR) {
            continue;
        } else {
            diminuto_assert(0);
        }

        /*
         * Service the socket. Note that if the UDP or TCP sockets aren't
         * open, the checks below will never be true.
         */

        if (fd == udpsock) {

            size = diminuto_ipc6_datagram_receive_generic(udpsock, buffer, total, &address6, &port, 0);
            DIMINUTO_LOG_DEBUG("Receive %d %zd [%s]:%d\n", udpsock, size, diminuto_ipc6_address2string(address6, buffer6, sizeof(buffer6)), port);

            if (size <= 0) {
                continue;
            }

            for (fd = mux.write.min; fd <= mux.write.max; ++fd) {
                if (FD_ISSET(fd, &mux.write.active)) {
                    size = diminuto_ipc6_stream_write(fd, buffer, size);
                    if (diminuto_pipe_check()) {
                        DIMINUTO_LOG_INFORMATION("SIGPIPE");
                        size = 0;
                    }
                    DIMINUTO_LOG_DEBUG("Write %d %zd\n", fd, size);
                    if (size <= 0) {
                        DIMINUTO_LOG_INFORMATION("Close %d", fd);
                        rc = diminuto_mux_close(&mux, fd);
                        diminuto_assert(rc >= 0);
                    }
                }
            }

            if (fp == (FILE *)0) {
                /* Do nothing. */
            } else if ((length = fwrite(buffer, size, 1, fp)) == 1) {
                /* Do nothing. */
            } else {
                errno = EIO;
                diminuto_perror(feof(fp) ? "EOF" : ferror(fp) ? "ERROR" : "UNEXPECTED");
                fclose(fp);
                fp = (FILE *)0;
            }

            if (fp == (FILE *)0) {
                /* Do nothing. */
            } else if (fp == stdout) {
                /* Do nothing. */
            } else if ((fp = diminuto_observation_commit(fp, &temp)) != (FILE *)0) {
                fclose(fp);
                fp = (FILE *)0;
            } else if ((fp = diminuto_observation_create(filename, &temp)) == (FILE *)0) {
                /* Do nothing. */
            } else {
                /* Do nothing. */
            }

        } else if (fd == tcpsock) {

            fd = diminuto_ipc6_stream_accept_generic(tcpsock, &address6, &port);
            if (fd >= 0) {
                DIMINUTO_LOG_INFORMATION("Accept %d [%s]:%d\n", fd, diminuto_ipc6_address2string(address6, buffer6, sizeof(buffer6)), port);
                rc = diminuto_mux_register_write(&mux, fd);
                diminuto_assert(rc >= 0);
            }

        } else {

            DIMINUTO_LOG_WARNING("Invalid %d\n", fd);

        }

    }

    /***************************************************************************
     * FINALIZATION
     **************************************************************************/

    DIMINUTO_LOG_INFORMATION("Stop");

    if (udpsock >= 0) {
        (void)diminuto_mux_close(&mux, udpsock);
    }

    if (tcpsock >= 0) {
        (void)diminuto_mux_close(&mux, tcpsock);
    }

    for (fd = mux.write.min; fd <= mux.write.max; ++fd) {
        if (FD_ISSET(fd, &mux.write.active)) {
            (void)diminuto_mux_close(&mux, fd);
        }
    }

    diminuto_mux_fini(&mux);

    if (fp == (FILE *)0) {
        /* Do nothing. */
    } else if (fp == stdout) {
        /* Do nothing. */
    } else {
        fp = diminuto_observation_discard(fp, &temp);
    }

    DIMINUTO_LOG_INFORMATION("Exit");

    return 0;
}
