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
 * mobile rovers via RTCM messages received from a base station.
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
#include "com/diag/diminuto/diminuto_ipc4.h"
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
    diminuto_port_t rendezvous = 2101;
    diminuto_ipv6_t address = { 0, };
    diminuto_port_t port = 0;
    diminuto_ipv6_buffer_t ipv6 = { 0, };
    datagram_buffer_t datagram = DATAGRAM_BUFFER_INITIALIZER;
    ssize_t length = 0;
    diminuto_tree_t * root = DIMINUTO_TREE_EMPTY;
    diminuto_tree_t node = DIMINUTO_TREE_NULLINIT;
    diminuto_mux_t mux = { 0 };
    int ready = 0;
    int fd = -1;
    diminuto_sticks_t frequency = 0;
    static const char OPTIONS[] = "P:Vdt:v?";
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
        case 'P':
        	rendezvous = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (rendezvous < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
        	break;
        case 'V':
            fprintf(stderr, "%s: version com-diag-hazer %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'd':
            debug = !0;
            break;
        case 't':
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -d ] [ -v ] [ -V ] [ -P PORT ]"
                           "\n", Program);
            fprintf(stderr, "       -P PORT     Use PORT as the RTCM source and sink port.\n");
            fprintf(stderr, "       -V          Print release, Vintage, and revision on standard output.\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
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

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    assert(rc >= 0);

    (void)diminuto_time_timezone(diminuto_time_clock());

    diminuto_mux_init(&mux);

    sock = diminuto_ipc6_datagram_peer(port);
    assert(sock >= 0);

    rc = diminuto_mux_register_interrupt(&mux, sock);
    assert(rc >= 0);

    frequency = diminuto_frequency();
    assert(frequency > 0);

    /***************************************************************************
     * WORK
     **************************************************************************/

    while (!0) {

		if (diminuto_terminator_check()) {
			break;
		}

		if (diminuto_interrupter_check()) {
			break;
		}

		if (diminuto_hangup_check()) {
			/* Do nothing. */
		}

		if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
			/* Fall through. */
		} else if ((ready = diminuto_mux_wait(&mux, frequency)) < 0) {
			assert(0); /* Failed! */
		} else if (ready == 0) {
			fd = -1; /* Timed out. */
		} else if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
			/* Fall through. */
		} else {
			assert(0); /* Should be impossible! */
		}

		if (fd != sock) {

			/* TODO */

		}
#if 0
		if ((total = diminuto_ipc6_datagram_receive_generic(sock, &datagram, sizeof(datagram - 1), &address, &port, 0)) <= 0) {

			DIMINUTO_LOG_WARNING("Surveyor socket (%d) [%zd]\n", sock, total);

		} else if ((size = validate_datagram(&surveyor_sequence, &surveyor_buffer, surveyor_total)) < 0) {

			DIMINUTO_LOG_NOTICE("Surveyor order (%d) [%zd] {%lu} {%lu}\n", surveyor_fd, surveyor_total, (unsigned long)surveyor_sequence, (unsigned long)ntohl(surveyor_buffer.sequence));

		} else if ((surveyor_length = tumbleweed_validate(surveyor_buffer.payload.rtcm, surveyor_size)) < TUMBLEWEED_RTCM_SHORTEST) {

			DIMINUTO_LOG_WARNING("Surveyor data (%d) [%zd] [%zd] [%zd] 0x%02x\n", surveyor_fd, surveyor_total, surveyor_size, surveyor_length, surveyor_buffer.payload.data[0]);

		} else if (surveyor_length == TUMBLEWEED_RTCM_SHORTEST) {

            DIMINUTO_LOG_DEBUG("Surveyor keepalive received");

		if ((length = diminuto_ipc6_datagram_receive_generic(sock, &datagram, sizeof(datagram - 1), &address, &port, 0)) <= 0) {
			/* Do nothing. */
			/* TODO */

		} else {

		}

    	//DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\" [%s]:%d", label, fd, option, diminuto_ipc6_address2string(*ipv6p, ipv6, sizeof(ipv6)), port);

    	//(void)diminuto_ipc6_datagram_send(fd, buffer, size, *ipv6p, port);
#endif

    }

    /***************************************************************************
     * FINALIZATION
     **************************************************************************/

    return 0;
}
