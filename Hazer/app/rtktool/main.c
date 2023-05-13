/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the rtktool RTK router.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * rtktool is a point-to-multipoint router that distributes RTK updates to
 * mobile rovers via datagrams containing RTCM messages received from a
 * stationary base station running in survey mode. The datagrams are sent to
 * the port identified as the source of periodic keepalives sent from each
 * rover to the router.
 *
 * USAGE
 *
 * rtktool [ -? ] [ -d ] [ -v ] [ -M ] [ -V ] [ -M ] [ -p :PORT ] [ -t SECONDS ]
 *
 * EXAMPLES
 *
 * rtktool -p :21010 -t 30
 */

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/hazer_version.h"
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
#include "types.h"

/*******************************************************************************
 * GLOBALS
 ******************************************************************************/

/**
 * This is our program name as provided by the run-time system.
 */
static const char * Program = (const char *)0;

/*******************************************************************************
 * HELPERS
 ******************************************************************************/

static int comparator(const diminuto_tree_t * tap, const diminuto_tree_t * tbp)
{
    int rc = 0;
    client_t * cap = (client_t *)0;
    client_t * cbp = (client_t *)0;

    cap = (client_t *)diminuto_tree_data(tap);
    cbp = (client_t *)diminuto_tree_data(tbp);

    if ((rc = diminuto_ipc6_compare(&(cap->address), &(cbp->address))) != 0) {
        /* Do nothing. */
    } else if (cap->port < cbp->port) {
        rc = -1;
    } else if (cap->port > cbp->port) {
        rc = 1;
    } else {
        /* Do nothing. */
    }

    return rc;
}

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
    long timeout = 30;
    int sock = -1;
    int error = 0;
    char * end = (char *)0;
    int rc = 0;
    int comparison = 0;
    const char * rendezvous = (const char *)0;
    diminuto_ipc_endpoint_t endpoint = { 0, };
    diminuto_ipv6_buffer_t ipv6 = { 0, };
    datagram_buffer_t buffer = DATAGRAM_BUFFER_INITIALIZER;
    ssize_t total = 0;
    ssize_t size = 0;
    ssize_t length = 0;
    ssize_t result = 0;
    diminuto_tree_t * root = DIMINUTO_TREE_EMPTY;
    diminuto_tree_t * node = DIMINUTO_TREE_NULL;
    diminuto_tree_t * last = DIMINUTO_TREE_NULL;
    diminuto_tree_t * next = DIMINUTO_TREE_NULL;
    diminuto_tree_t * temp = DIMINUTO_TREE_NULL;
    client_t * this = (client_t *)0;
    client_t * that = (client_t *)0;
    client_t * thou = (client_t *)0;
    client_t * thee = (client_t *)0;
    client_t * then = (client_t *)0;
    client_t * base = (client_t *)0;
    const char * label = (const char *)0;
    diminuto_mux_t mux = { 0 };
    int ready = 0;
    int fd = -1;
    diminuto_sticks_t frequency = 0;
    long now = 0;
    long was = 0;
    unsigned int outoforder = 0;
    unsigned int missing = 0;
    static const char OPTIONS[] = "MVdp:t:v?";
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
            DIMINUTO_LOG_INFORMATION("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE_VALUE, COM_DIAG_HAZER_VINTAGE_VALUE, COM_DIAG_HAZER_REVISION_VALUE);
            break;
        case 'd':
            debug = !0;
            break;
        case 'p':
            rendezvous = optarg;
            rc = diminuto_ipc_endpoint(rendezvous, &endpoint);
            if ((rc < 0) || (endpoint.udp == 0)) { diminuto_perror(optarg); error = !0; }
            break;
        case 't':
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -? ] [ -d ] [ -v ] [ -M ] [ -V ] [ -p :PORT ] [ -t SECONDS ]\n", Program);
            fprintf(stderr, "       -M          Run in the background as a daeMon.\n");
            fprintf(stderr, "       -V          Log Version in the form of release, vintage, and revision.\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
            fprintf(stderr, "       -p :PORT    Use PORT as the RTCM source and sink port.\n");
            fprintf(stderr, "       -t SECONDS  Set the client timeout to SECONDS seconds.\n");
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
        diminuto_contract(rc == 0);
    }

    rc = diminuto_terminator_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    diminuto_contract(rc >= 0);

    rc = diminuto_hangup_install(!0);
    diminuto_contract(rc >= 0);

    (void)diminuto_time_timezone();

    diminuto_mux_init(&mux);

    sock = diminuto_ipc6_datagram_peer(endpoint.udp);
    diminuto_contract(sock >= 0);
    DIMINUTO_LOG_INFORMATION("Router (%d) \"%s\" [%s]:%d", sock, rendezvous, diminuto_ipc6_address2string(endpoint.ipv6, ipv6, sizeof(ipv6)), endpoint.udp);


    rc = diminuto_mux_register_read(&mux, sock);
    diminuto_contract(rc >= 0);

    frequency = diminuto_frequency();
    diminuto_contract(frequency > 0);

    now = was = diminuto_time_elapsed() / frequency;
    diminuto_contract(now >= 0);

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

        if (diminuto_hangup_check()) {
            diminuto_log_mask ^= DIMINUTO_LOG_MASK_DEBUG;
        }

        /*
         * Wait until our socket needs to be serviced... or we time out.
         */

        if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
            /* Do nothing. */
        } else if ((ready = diminuto_mux_wait(&mux, frequency)) == 0) {
            fd = -1;
        } else if (ready > 0) {
            fd = diminuto_mux_ready_read(&mux);
        } else if (errno == EINTR) {
            continue;
        } else {
            diminuto_panic();
        }

        /*
         * Get a timestamp.
         */

        now = diminuto_time_elapsed() / frequency;

        /*
         * Service the socket.
         */

        if (fd == sock) {

            /*
             * If we don't have a new node handy, make a new one, and
             * initialize it.
             */

            if (this == (client_t *)0) {
                this = (client_t *)malloc(sizeof(client_t));
                diminuto_contract(this != (client_t *)0);
                temp = diminuto_tree_datainit(&(this->node), this);
                diminuto_contract(temp != (diminuto_tree_t *)0);
                this->last = 0;
                this->sequence = 0;
                this->classification = CLASS;
            }

            /*
             * Receive the pending datagram.
             */

            if ((total = diminuto_ipc6_datagram_receive_generic(sock, &buffer, sizeof(buffer), &(this->address), &(this->port), 0)) < sizeof(buffer.header)) {
                DIMINUTO_LOG_ERROR("Datagram Length [%s]:%d [%zd]", diminuto_ipc6_address2string(this->address, ipv6, sizeof(ipv6)), this->port, total);
                continue;
            }

            DIMINUTO_LOG_DEBUG("Datagram Received [%s]:%d [%zd]", diminuto_ipc6_address2string(this->address, ipv6, sizeof(ipv6)), this->port, total);

            if (verbose) {
                fprintf(stderr, "Datagram [%s]:%d [%zd]\n", diminuto_ipc6_address2string(this->address, ipv6, sizeof(ipv6)), this->port, total);
                diminuto_dump(stderr, &buffer, total);
            }

            /*
             * See if we know about this client.
             */

            node = diminuto_tree_search(root, &(this->node), &comparator, &comparison);

            if (node == (diminuto_tree_t *)0) {
                that = (client_t *)0;
                then = (client_t *)0;
                thou = this;
                this->sequence = 0; /* RESET */
            } else if (comparison != 0) {
                that = (client_t *)0;
                then = (client_t *)diminuto_tree_data(node);
                diminuto_contract(then != (client_t *)0);
                thou = this;
                this->sequence = 0; /* RESET */
            } else {
                that = (client_t *)diminuto_tree_data(node);
                diminuto_contract(that != (client_t *)0);
                then = (client_t *)0;
                thou = that;
            }

            /*
             * At this point in our story:
             *
             * this points to the client we allocated and which may become a
             * new entry in the database or may be later reused;
             *
             * that points to the matching client we found in the database or
             * NULL if we didn't find a matching client;
             *
             * then points to the nearest non-match in the tree that we will
             * use to insert the new client when we didn't find a match or NULL
             * if we didn't find a nearest match because the database was
             * empty;
             *
             * thou points to the client we're going to use to validate the
             * datagram we just received, and it will either be equal to either
             * this or that and will always be non-NULL;
             *
             * thee will temporarily point to clients as we traverse through
             * the database for one reason or another.
             */

            /*
             * Validate the datagram. This is more complicated than it looks.
             * I'd really like to add end-to-end encryption to this data
             * stream. But to do so, I either have to have this utility be a
             * man-in-the-middle, decrypting and reencrypting the stream, or
             * else distribute the datagram without validation. I don't like
             * either option.
             */

            if ((size = datagram_validate(&(thou->sequence), &(buffer.header), total, &outoforder, &missing)) < 0) {
                DIMINUTO_LOG_NOTICE("Datagram Order {%lu} {%lu} [%s]:%d", (unsigned long)(thou->sequence), (unsigned long)ntohl(buffer.header.sequence), diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                continue; /* REJECT */
            } else if ((length = tumbleweed_validate(buffer.payload.rtcm, size)) < TUMBLEWEED_RTCM_SHORTEST) {
                DIMINUTO_LOG_WARNING("Datagram Data [%zd] 0x%02x [%s]:%d", length, buffer.payload.data[0], diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                continue; /* REJECT */
            } else {
                /* Do nothing. */
            }

            /*
             * Determine this client's classification.
             */

            if (length > TUMBLEWEED_RTCM_SHORTEST) {
                this->classification = BASE;
                label = "base";
            } else {
                this->classification = ROVER;
                label = "rover";
            }

            /*
             * If this client's classification has changed, we reject it.
             * If it's in fact legitimate (somehow), its existing entry will
             * eventually time out, be removed, and can be registered anew on
             * reception of a subsequent datagram.
             */

            if (this->classification != thou->classification) {
                DIMINUTO_LOG_WARNING("Client Change %s [%s]:%d", label, diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                continue; /* REJECT */
            }

            /*
             * If this is a base, but we already have a base, we reject it.
             * Again, the existing base will time out if it is no longer
             * sending, we'll remove it, and the new one can be reregistered.
             * Note that we log a pretender base at DEBUG level since otherwise
             * it can flood the log.
             */

            if (thou->classification != BASE) {
                /* Do nothing. */
            } else if (base == (client_t *)0) {
                /* Do nothing. */
            } else if (base == thou) {
                /* Do nothing. */
            } else {
                DIMINUTO_LOG_DEBUG("Client Conflict %s [%s]:%d", label, diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                continue; /* REJECT */
            }

            /*
             * Cannot REJECT after this point.
             */

            /*
             * If this is a base, forward the datagram to all rovers. Note
             * that if it is truly a new base, its sequence numbers will
             * likely be behind that of the old base, and all of the rovers
             * will need to be restarted manually. But it is also possible
             * that the base is the same and some darn NATting firewall just
             * changed the client's address, in which case the sequence
             * numbers are fine. (Rover clients that are truly mobile may
             * see their IPv4 addresses change as they switch from cell site
             * to cell site. But it can happen to non-mobile rovers and even
             * stationary bases, because of a particular cell site becoming
             * overloaded and the network deciding to switch a client to a
             * different, perhaps slightly more distant, cell site.)
             */

            if (thou->classification != BASE) {
                /* Do nothing. */
            } else if (diminuto_tree_isempty(&root)) {
                /* Do nothing. */
            } else {

                node = diminuto_tree_first(&root);
                diminuto_contract(node != (diminuto_tree_t *)0);

                last = diminuto_tree_last(&root);
                diminuto_contract(last != (diminuto_tree_t *)0);

                while (!0) {
                    thee = (client_t *)diminuto_tree_data(node);
                    if (thee->classification == ROVER) {
                        result = diminuto_ipc6_datagram_send(sock, &buffer, total, thee->address, thee->port);
                        DIMINUTO_LOG_DEBUG("Datagram Sent [%s]:%d [%zd]", diminuto_ipc6_address2string(thee->address, ipv6, sizeof(ipv6)), thee->port, result);
                    }
                    if (node == last) { break; }
                    node = diminuto_tree_next(node);
                }

            }

            /*
             * If this a new client, add it to the database.
             */

            if (that == (client_t *)0) {
                DIMINUTO_LOG_NOTICE("Client New %s [%s]:%d ", label, diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                if (thou->classification == BASE) {
                    base = thou;
                    DIMINUTO_LOG_NOTICE("Client Set %s [%s]:%d", label, diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port);
                }
                if (debug) {
                    fprintf(stderr, "Client [%s]:%d [%zd] %p %p %p %d\n", diminuto_ipc6_address2string(thou->address, ipv6, sizeof(ipv6)), thou->port, total, root, node, then, comparison);
                    diminuto_dump(stderr, &(thou->address), sizeof(thou->address));
                    diminuto_dump(stderr, &(thou->port), sizeof(thou->port));
                }
            }

            if (that != (client_t *)0) {
                /* Do nothing. */
            } else if (diminuto_tree_isempty(&root)) {
                diminuto_contract(this != (client_t *)0);
                node = diminuto_tree_insert_root(&(this->node), &root);
                diminuto_contract(node != (diminuto_tree_t *)0);
                this = (client_t *)0; /* CONSUMED */
            } else if (comparison < 0) {
                diminuto_contract(this != (client_t *)0);
                diminuto_contract(then != (client_t *)0);
                node = diminuto_tree_insert_right(&(this->node), &(then->node));
                diminuto_contract(node != (diminuto_tree_t *)0);
                this = (client_t *)0; /* CONSUMED */
            } else if (comparison > 0) {
                diminuto_contract(this != (client_t *)0);
                diminuto_contract(then != (client_t *)0);
                node = diminuto_tree_insert_left(&(this->node), &(then->node));
                diminuto_contract(node != (diminuto_tree_t *)0);
                this = (client_t *)0; /* CONSUMED */
            } else {
                diminuto_panic();
            }

            if (debug) {
                node = diminuto_tree_audit(&root);
                diminuto_contract(node == (diminuto_tree_t *)0);
            }

            /*
             * Timestamp the client now that we know that the client and its
             * datagram are valid. If we haven't heard from a client within the
             * timeout period, we'll remove it. As a useful side effect, if a
             * client gets restarted such that its sequence numbers are
             * unexpected, or if a client changes classifications from base to
             * rover or vice versa, we will eventually remove it and reregister
             * its connection as a new one.
             */

            thou->last = now;

        }

        /*
         * Once a second or so, step through all of the clients in the database
         * and see if any of them have timed out. We time out both rovers and
         * bases (so we need to check if its a base).
         */

        if ((now - was) <= 0) {
            /* Do nothing. */
        } else  if (diminuto_tree_isempty(&root)) {
            /* Do nothing. */
        } else {

            node = diminuto_tree_first(&root);
            diminuto_contract(node != (diminuto_tree_t *)0);

            last = diminuto_tree_last(&root);
            diminuto_contract(last != (diminuto_tree_t *)0);

            while (!0) {
                thee = (client_t *)diminuto_tree_data(node);
                next = diminuto_tree_next(node);
                if ((now - thee->last) > timeout) {
                    DIMINUTO_LOG_NOTICE("Client Old %s [%s]:%d", (thee->classification == BASE) ? "base" : (thee->classification == ROVER) ? "rover" : "unknown", diminuto_ipc6_address2string(thee->address, ipv6, sizeof(ipv6)), thee->port);
                    node = diminuto_tree_remove(&(thee->node));
                    diminuto_contract(node != (diminuto_tree_t *)0);
                    if (thee == base) {
                        base = (client_t *)0;
                    }
                    free(thee);
                }
                if (node == last) { break; }
                diminuto_contract(next != (diminuto_tree_t *)0);
                node = next;
            }

            was = now;

        }

    }

    /***************************************************************************
     * FINALIZATION
     **************************************************************************/

    DIMINUTO_LOG_INFORMATION("Stop");

    DIMINUTO_LOG_INFORMATION("Counters OutOfOrder=%u Missing=%u", outoforder, missing);

    diminuto_mux_fini(&mux);

    rc = diminuto_ipc_close(sock);
    diminuto_contract(rc >= 0);


    if (!diminuto_tree_isempty(&root)) {

        node = diminuto_tree_first(&root);
        diminuto_contract(node != (diminuto_tree_t *)0);

        last = diminuto_tree_last(&root);
        diminuto_contract(last != (diminuto_tree_t *)0);

        while (!0) {
            thee = (client_t *)diminuto_tree_data(node);
            next = diminuto_tree_next(node);
            node = diminuto_tree_remove(&(thee->node));
            diminuto_contract(node != (diminuto_tree_t *)0);
            free(thee);
            if (node == last) { break; }
            diminuto_contract(next != (diminuto_tree_t *)0);
            node = next;
        }

    }

    DIMINUTO_LOG_INFORMATION("Exit");

    return 0;
}
