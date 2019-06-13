/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 *
 * ABSTRACT
 *
 * rtktool is a point-to-multipoint router that distributes RTK updates to
 * mobile rovers via datagrams containing RTCM messages received from a
 * stationary base station running in survey mode. The datagrams are sent to
 * the port identified as the source of periodic keepalives sent from each
 * rover to the router.
 */

#undef NDEBUG
#include <assert.h>
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
#include <locale.h>
#include "./common.h"
#include "./rtktool.h"
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_containerof.h"

/*******************************************************************************
 * GLOBALS
 ******************************************************************************/

/**
 * This is our program name as provided by the run-time system.
 */
static const char * Program = (const char *)0;

/**
 * This is our host name as provided by the run-time system.
 */
static char Hostname[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' };

static const client_t CLIENT = CLIENT_INITIALIZER;

/*******************************************************************************
 * HELPERS
 ******************************************************************************/

static int comparator(diminuto_tree_t * tap, diminuto_tree_t * tbp)
{
	int rc = 0;
	client_t * cap = (client_t *)0;
	client_t * cbp = (client_t *)0;

	cap = diminuto_containerof(client_t, node, tap);
	cbp = diminuto_containerof(client_t, node, tbp);

	if ((rc = diminuto_ipc6_compare(&(cap->address), &(cbp->address))) != 0) {
		/* Do nothing. */
	} else if ((rc = (cap->port < cbp->port) ? -1 : (cap->port > cbp->port) ? 1 : 0) != 0) {
		/* Do nothing. */
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
    long timeout = 30;
    int sock = -1;
    int error = 0;
    char * end = (char *)0;
    int rc = 0;
    char * locale = (char *)0;
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
    client_t * base = (client_t *)0;
    role_t role = ROLE;
    const char * label = (const char *)0;
    diminuto_mux_t mux = { 0 };
    int ready = 0;
    int fd = -1;
    diminuto_sticks_t frequency = 0;
    long now = 0;
    long was = 0;
    unsigned int outoforder = 0;
    unsigned int missing = 0;
    static const char OPTIONS[] = "Vdp:t:v?";
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

    (void)gethostname(Hostname, sizeof(Hostname));
    Hostname[sizeof(Hostname) - 1] = '\0';

    locale = setlocale(LC_ALL, "");

    /***************************************************************************
     * OPTIONS
     **************************************************************************/

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case 'V':
            fprintf(stderr, "%s: version com-diag-hazer %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'd':
            debug = !0;
            break;
        case 'p':
            rendezvous = optarg;
            rc = diminuto_ipc_endpoint(rendezvous, &endpoint);
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
        	break;
        case 't':
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -d ] [ -v ] [ -V ] [ -p PORT ] [ -t SECONDS ]\n", Program);
            fprintf(stderr, "       -V          Print release, Vintage, and revision on standard output.\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
            fprintf(stderr, "       -p PORT     Use PORT as the RTCM source and sink port.\n");
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

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    assert(rc >= 0);

    (void)diminuto_time_timezone(diminuto_time_clock());

    diminuto_mux_init(&mux);

    sock = diminuto_ipc6_datagram_peer(endpoint.udp);
    assert(sock >= 0);
    DIMINUTO_LOG_INFORMATION("Connection (%d) \"%s\" [%s]:%d", sock, rendezvous, diminuto_ipc6_address2string(endpoint.ipv6, ipv6, sizeof(ipv6)), endpoint.udp);


    rc = diminuto_mux_register_read(&mux, sock);
    assert(rc >= 0);

    frequency = diminuto_frequency();
    assert(frequency > 0);

    now = was = diminuto_time_elapsed() / frequency;
    assert(now >= 0);

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
			DIMINUTO_LOG_NOTICE("SIGHUP OutOfOrder=%u Missing=%u", outoforder, missing);
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
			assert(0);
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
			 * If we don't have a new node handy, make a new one and zero
			 * it out.
			 */

			if (this == (client_t *)0) {
			    this = (client_t *)calloc(1, sizeof(client_t));
			    assert(this != (client_t *)0);
			    temp = diminuto_tree_nullinit(&(this->node));
			    assert(temp != (diminuto_tree_t *)0);
			}

			/*
			 * Receive the pending datagram.
			 */

			if ((total = diminuto_ipc6_datagram_receive_generic(sock, &buffer, sizeof(buffer), &(this->address), &(this->port), 0)) < sizeof(buffer.header)) {
				DIMINUTO_LOG_ERROR("Datagram Length [%zd] [%s]:%d", total, diminuto_ipc6_address2string(this->address, ipv6, sizeof(ipv6)), this->port);
				continue;
			}

            if (verbose) {
				fprintf(stderr, "[%s]:%d [%zd]\n", diminuto_ipc6_address2string(this->address, ipv6, sizeof(ipv6)), this->port, total);
            	diminuto_dump(stderr, &buffer, total);
            }

			/*
			 * See if we know about this client.
			 */

			node = diminuto_tree_search(root, &(this->node), &comparator, &rc);

			/*
			 * If we don't, add it to the database.
			 */

			if (node == (diminuto_tree_t *)0) {
				assert(root == (diminuto_tree_t *)0);
				node = diminuto_tree_insert_root(&(this->node), &root);
				assert(node != (diminuto_tree_t *)0);
				this = (client_t *)0;
			} else if (rc < 0) {
				assert(root != (diminuto_tree_t *)0);
				node = diminuto_tree_insert_right(&(this->node), node);
				assert(node != (diminuto_tree_t *)0);
				this = (client_t *)0;
			} else if (rc > 0) {
				assert(root != (diminuto_tree_t *)0);
				node = diminuto_tree_insert_left(&(this->node), node);
				assert(node != (diminuto_tree_t *)0);
				this = (client_t *)0;
			} else {
				/* Do nothing. */
			}

			that = diminuto_containerof(client_t, node, node);

			/*
			 * Validate the datagram.
			 */

			if ((size = validate_datagram(&(that->sequence), &(buffer.header), total, &outoforder, &missing)) < 0) {
				DIMINUTO_LOG_NOTICE("Datagram Order {%lu} {%lu} [%s]:%d", (unsigned long)(that->sequence), (unsigned long)ntohl(buffer.header.sequence), diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
				continue;
			} else if ((length = tumbleweed_validate(buffer.payload.rtcm, size)) < TUMBLEWEED_RTCM_SHORTEST) {
				DIMINUTO_LOG_WARNING("Datagram Data [%zd] 0x%02x [%s]:%d", length, buffer.payload.data[0], diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
				continue;
			} else {
				/* Do nothing. */
			}

			/*
			 * Determine this client's role.
			 */

			if (length > TUMBLEWEED_RTCM_SHORTEST) {
				role = BASE;
				label = "base";
			} else {
				role = ROVER;
				label = "rover";
			}

			/*
			 * If this is a new client, save its role.
			 */

			if (this == (client_t *)0) {
				that->role = role;
				DIMINUTO_LOG_NOTICE("Client New %s [%s]:%d", label, diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
			}

			/*
			 * If this client's role has changed, we reject it. If it's in fact
			 * legitimate (somehow), its existing entry will eventually time
			 * out, be removed, and can be registered anew on reception of its
			 * subsequent datagram.
			 */

			if (role != that->role) {
				DIMINUTO_LOG_WARNING("Client Role %s [%s]:%d", label, diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
				continue;
			}

			/*
			 * If this is a base, but we already have a base, we reject it.
			 * Again, the existing base will time out if it is no longer
			 * sending, we'll remove it, and the new one can be reregistered.
			 */

			if (role != BASE) {
				/* Do nothing. */
			} else if (base == (client_t *)0) {
				base = that;
				DIMINUTO_LOG_NOTICE("Client Set %s [%s]:%d", label, diminuto_ipc6_address2string(base->address, ipv6, sizeof(ipv6)), base->port);
			} else if (base != that) {
				DIMINUTO_LOG_WARNING("Client Bad %s [%s]:%d", label, diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
				continue;
			} else {
				/* Do nothing. */
			}

			/*
			 * Timestamp the client now that we know that the client ant its
			 * datagram is valid. If we haven't heard from a client within the
			 * timeout period, we'll remove it. As a useful side effect, if a
			 * client gets restarted such that its sequence numbers are
			 * unexpected, or if a client changes roles from base to rover or
			 * vice versa, we will eventually remove it and reregister its
			 * connection as a new one.
			 */

			that->then = now;

			/*
			 * If this was a base, forward the datagram to all rovers. Note
			 * that if it is truly a new base, its sequence numbers will
			 * likely be behind that of the old base, and all of the rovers
			 * will need to be restarted manually. But it is also possible
			 * that the base is the same and some darn NATting firewall just
			 * changed its address (I've seen this happen), in which case the
			 * sequence numbers are fine.
			 */

			if (role == BASE) {

				node = diminuto_tree_first(&root);
				assert(node != (diminuto_tree_t *)0);

				last = diminuto_tree_last(&root);
				assert(last != (diminuto_tree_t *)0);

				while (!0) {
					that = diminuto_containerof(client_t, node, node);
					if (that->role == ROVER) {
						result = diminuto_ipc6_datagram_send(sock, &buffer, total, that->address, that->port);
						DIMINUTO_LOG_DEBUG("Datagram Sent [%s]:%d %zd", diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port, result);
					}
					if (node == last) { break; }
					node = diminuto_tree_next(node);
				}

			}

		}

		/*
		 * Once a second or so, step through all of the clients in the database
		 * and see if any of them have timed out. We time out both rovers and
		 * bases (so we need to check if its a base).
		 */

		if (diminuto_tree_isempty(&root)) {
			/* Do nothing. */
		} else if ((now - was) <= 0) {
			/* Do nothing. */
		} else {

			node = diminuto_tree_first(&root);
			assert(node != (diminuto_tree_t *)0);

			last = diminuto_tree_last(&root);
			assert(last != (diminuto_tree_t *)0);

			while (!0) {
				that = diminuto_containerof(client_t, node, node);
				next = diminuto_tree_next(node);
				if ((now - that->then) > timeout) {
					DIMINUTO_LOG_NOTICE("Client Old %s [%s]:%d", (that->role == BASE) ? "base" : "rover", diminuto_ipc6_address2string(that->address, ipv6, sizeof(ipv6)), that->port);
					node = diminuto_tree_remove(&(that->node));
					assert(node != (diminuto_tree_t *)0);
					if (that == base) { base = (client_t *)0; }
					free(that);
				}
				if (node == last) { break; }
				assert(next != (diminuto_tree_t *)0);
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
    assert(rc >= 0);


	if (!diminuto_tree_isempty(&root)) {

		node = diminuto_tree_first(&root);
		assert(node != (diminuto_tree_t *)0);

		last = diminuto_tree_last(&root);
		assert(last != (diminuto_tree_t *)0);

		while (!0) {
			that = diminuto_containerof(client_t, node, node);
			next = diminuto_tree_next(node);
			node = diminuto_tree_remove(&(that->node));
			assert(node != (diminuto_tree_t *)0);
			free(that);
			if (node == last) { break; }
			assert(next != (diminuto_tree_t *)0);
			node = next;
		}

	}

	DIMINUTO_LOG_INFORMATION("Exit");

    return 0;
}
