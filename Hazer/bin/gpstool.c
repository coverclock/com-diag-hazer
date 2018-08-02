/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 *
 * ABSTRACT
 *
 * gpstool is the Swiss Army knife of Hazer. It can read NMEA sentences and UBX
 * packets from a GPS device or as datagrams from an IP UDP port, log the
 * data on standard error, write the data to a file, interpret the more
 * common NMEA sentences and display the results in a pretty way on standard
 * output using ANSI control sequences, and forward the data to an IP UDP port
 * where perhaps it will be received by another gpstool. It has been used, for
 * example, to integrate a GPS device with a USB interface with the Google Earth
 * web application to create a moving map display, and to implement remote
 * tracking of a moving vehicle.
 *
 * EXAMPLES
 *
 * gpstool -?
 *
 * gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -v
 *
 * gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E
 *
 * gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -L nmea.txt
 *
 * gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -6 -A ::1 -P 5555
 *
 * gpstool -6 -P 5555 -E
 *
 * gpstool -d -v
 */

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_coherentsection.h"
#include "com/diag/diminuto/diminuto_criticalsection.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_list.h"

static const size_t LIMIT = 80;
static const size_t UNLIMITED = ~0;

typedef enum Role { NONE = 0, PRODUCER = 1, CONSUMER = 2 } role_t;

typedef enum Protocol { UNUSED = 0, IPV4 = 4, IPV6 = 6, } protocol_t;

typedef enum Format { UNKNOWN = 0, NMEA = 1, UBX = 2 } format_t;

static int emit_sentence(FILE * fp, const char * string, size_t size)
{
    int rc = -1;
    uint8_t cs = 0;
    char msn = '\0';
    char lsn = '\0';

    do {

        (void)hazer_checksum(string, size, &cs);
        if (hazer_checksum2characters(cs, &msn, &lsn) < 0) { break; }

        if (fprintf(fp, "%s%c%c%c\r\n", string, HAZER_STIMULUS_CHECKSUM, msn, lsn) < 0) { break; }
        if (fflush(fp) == EOF) { break; }

        rc = 0;

    } while (0);

    return rc;
}

static int emit_packet(FILE * fp, const void * packet, size_t size)
{
    int rc = -1;
    const void * bp = (const void *)0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    size_t length = 0;

    do {

        bp = yodel_checksum(packet, size, &ck_a, &ck_b);
        if (bp == (void *)0) { break; }
        length = (const char *)bp - (const char *)packet;

        if (fwrite(packet, length, 1, fp) < 1) { break; }
        if (fwrite(&ck_a, sizeof(ck_a), 1, fp) < 1) { break; }
        if (fwrite(&ck_b, sizeof(ck_b), 1, fp) < 1) { break; }
        if (fflush(fp) == EOF) { break; }

        rc = 0;

    } while (0);

    return rc;
}

static void send_sentence(int sock, protocol_t protocol, diminuto_ipv4_t * ipv4p, diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size)
{
    int rc = 0;

    switch (protocol) {
    case IPV4:
        rc = diminuto_ipc4_datagram_send(sock, buffer, size, *ipv4p, port);
        if (rc < 0) { diminuto_perror("diminuto_ipc4_datagram_send"); }
        break;
    case IPV6:
        rc = diminuto_ipc6_datagram_send(sock, buffer, size, *ipv6p, port);
        if (rc < 0) { diminuto_perror("diminuto_ipc6_datagram_send"); }
        break;
    default:
        assert(0);
        break;
    }

}

static void print_sentence(FILE * fp, const void * buffer, size_t size, size_t limit)
{
    const char * bb = (const char *)0;
    size_t current = 0;
    int end = 0;

    for (bb = buffer; size > 0; --size) {
        diminuto_phex_emit(fp, *(bb++), ~(size_t)0, 0, 0, 0, &current, &end, 0);
        if (current > limit) { break; }
    }
    fputc('\n', fp);

    fflush(fp);
}

static void print_active(FILE * fp, const char * name, const hazer_solution_t * sp)
{
    static const unsigned int SATELLITES = sizeof(sp->id) / sizeof(sp->id[0]);
    int satellite = 0;
    int limit = 0;

    limit = sp->active;
    if (limit > SATELLITES) { limit = SATELLITES; }

    fprintf(fp, "%s {", name);
    for (satellite = 0; satellite < limit; ++satellite) {
        if (sp->id[satellite] != 0) {
            fprintf(fp, " %3u", sp->id[satellite]);
        }
    }
    fprintf(fp, " } [%02u] pdop %4.2lf hdop %4.2lf vdop %4.2lf\n", sp->active, sp->pdop, sp->hdop, sp->vdop);
}

static void print_view(FILE *fp, const char * name, const hazer_constellation_t cp[])
{
    static const int LIMIT = sizeof(cp[0].sat) / sizeof(cp[0].sat[0]);
    int channel = 0;
    int constellation = 0;
    int satellite = 0;
    int limit = 0;

    for (constellation = 0; constellation < HAZER_TALKER_TOTAL; ++constellation) {
        limit = cp[constellation].channels;
        if (limit > cp[constellation].view) { limit = cp[constellation].view; }
        if (limit > LIMIT) { limit = LIMIT; }
        for (satellite = 0; satellite < limit; ++satellite) {
            if (cp[constellation].sat[satellite].id != 0) {
                fprintf(fp, "%s [%02d] sat %3u elv %2u azm %3u snr %2udBHz con %s\n", name, ++channel, cp[constellation].sat[satellite].id, cp[constellation].sat[satellite].elv_degrees, cp[constellation].sat[satellite].azm_degrees, cp[constellation].sat[satellite].snr_dbhz, HAZER_TALKER_NAME[constellation]);
            }
        }
    }
}

static void print_position(FILE * fp, const char * name, const hazer_position_t * pp, int pps)
{
    uint64_t nanoseconds = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;
    int hundredths = 0;
    int direction = 0;
    const char * compass = (const char *)0;

    if (pp->dmy_nanoseconds == 0) { return; }

    fputs(name, fp);

    hazer_format_nanoseconds2timestamp(pp->tot_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);
    assert((1 <= month) && (month <= 12));
    assert((1 <= day) && (day <= 31));
    assert((0 <= hour) && (hour <= 23));
    assert((0 <= minute) && (minute <= 59));
    assert((0 <= second) && (second <= 59));
    assert((0 <= nanoseconds) && (nanoseconds < 1000000000ULL));
    fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);

    hazer_format_nanodegrees2position(pp->lat_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
    assert((0 <= degrees) && (degrees <= 90));
    assert((0 <= minutes) && (minutes <= 59));
    assert((0 <= seconds) && (seconds <= 59));
    assert((0 <= hundredths) && (hundredths <= 99));
    fprintf(fp, " %2d*%02d'%02d.%02d\"%c", degrees, minutes, seconds, hundredths, direction < 0 ? 'S' : 'N');

    hazer_format_nanodegrees2position(pp->lon_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
    assert((0 <= degrees) && (degrees <= 180));
    assert((0 <= minutes) && (minutes <= 59));
    assert((0 <= seconds) && (seconds <= 59));
    assert((0 <= hundredths) && (hundredths <= 99));
    fprintf(fp, ",%3d*%02d'%02d.%02d\"%c", degrees, minutes, seconds, hundredths, direction < 0 ? 'W' : 'E');

    fprintf(fp, " %8.2lf'", pp->alt_millimeters * 3.2808 / 1000.0);

    assert((0LL <= pp->cog_nanodegrees) && (pp->cog_nanodegrees <= 360000000000LL));

    compass = hazer_format_nanodegrees2compass8(pp->cog_nanodegrees);
    assert(compass != (const char *)0);
    assert(strlen(compass) <= 4);
    fprintf(fp, " %-2s", compass);

    fprintf(fp, " %8.3lfmph", pp->sog_microknots * 1.150779 / 1000000.0);

    fprintf(fp, " PPS %c", pps ? '1' : '0');

    fputc('\n', fp);
}

static void print_datum(FILE * fp, const char * name, const hazer_position_t * pp)
{
    double decimal = 0.0;

    fputs(name, fp);

    decimal = pp->lat_nanodegrees;
    decimal /= 1000000000.0;
    fprintf(fp, " %9.6lf", decimal);

    decimal = pp->lon_nanodegrees;
    decimal /= 1000000000.0;
    fprintf(fp, ",%10.6lf", decimal);

    decimal = pp->alt_millimeters;
    decimal /= 1000.0;
    fprintf(fp, " %9.3lfm", decimal);

    decimal = pp->cog_nanodegrees;
    decimal /= 1000000000.0;
    fprintf(fp, " %7.3lf*", decimal);

    decimal = pp->sog_microknots;
    decimal /= 1000000.0;
    fprintf(fp, " %8.3lfknots", decimal);

    fprintf(fp, " [%02u]", pp->sat_used);

    fprintf(fp, " %d %d %d %d %d", pp->lat_digits, pp->lon_digits, pp->alt_digits, pp->cog_digits, pp->sog_digits);

    fputc('\n', fp);
}

struct Context {
	int done;
	diminuto_mux_t * muxp;
	FILE * ppsfp;
	FILE * strobefp;
	int * oneppsp;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void * dcdpoller(void * argp)
{
	void * xc = (void *)1;
	struct Context * ctxp = (struct Context *)0;
	int done = 0;
	int rc = -1;
	int nowpps = 0;
	int waspps = 0;

	ctxp = (struct Context *)argp;

	while (!0) {
		DIMINUTO_COHERENT_SECTION_BEGIN;
			done = ctxp->done;
		DIMINUTO_COHERENT_SECTION_END;
		if (done) {
			xc = (void *)0;
			break;
		}
		rc = diminuto_serial_wait(fileno(ctxp->ppsfp));
		if (rc < 0) { break; }
		rc = diminuto_serial_status(fileno(ctxp->ppsfp));
		if (rc < 0) { break; }
		nowpps = !!rc;
		if (nowpps == waspps) {
			/* Do nothing. */
		} else if (nowpps) {
			if (ctxp->strobefp != (FILE *)0) {
				rc = diminuto_pin_set(ctxp->strobefp);
				if (rc < 0) { break; }
			}
			DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
				*(ctxp->oneppsp) = !0;
			DIMINUTO_CRITICAL_SECTION_END;
		} else {
			if (ctxp->strobefp != (FILE *)0) {
				rc = diminuto_pin_clear(ctxp->strobefp);
				if (rc < 0) { break; }
			}
		}
		waspps = nowpps;
	}

	return xc;
}

static void * gpiopoller(void * argp)
{
	void * xc = (void *)1;
	struct Context * ctxp = (struct Context *)0;
	int done = 0;
	int rc = -1;
	int nowpps = 0;
	int waspps = 0;

	ctxp = (struct Context *)argp;

	while (!0) {
		DIMINUTO_COHERENT_SECTION_BEGIN;
			done = ctxp->done;
		DIMINUTO_COHERENT_SECTION_END;
		if (done) {
			xc = (void *)0;
			break;
		}
		rc = diminuto_mux_wait(ctxp->muxp, -1);
		if (rc < 0) { break; }
		if (rc == 0) { continue; }
		rc = diminuto_mux_ready_interrupt(ctxp->muxp);
		assert(rc == fileno(ctxp->ppsfp));
		rc = diminuto_pin_get(ctxp->ppsfp);
		if (rc < 0) { break; }
		nowpps = !!rc;
		if (nowpps == waspps) {
			/* Do nothing. */
		} else if (nowpps) {
			if (ctxp->strobefp != (FILE *)0) {
				rc = diminuto_pin_set(ctxp->strobefp);
				if (rc < 0) { break; }
			}
			DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
				*(ctxp->oneppsp) = !0;
			DIMINUTO_CRITICAL_SECTION_END;
		} else {
			if (ctxp->strobefp != (FILE *)0) {
				rc = diminuto_pin_clear(ctxp->strobefp);
				if (rc < 0) { break; }
			}
		}
		waspps = nowpps;
	}

	return xc;
}

int main(int argc, char * argv[])
{
    const char * program = (const char *)0;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
    hazer_state_t nmea_state = HAZER_STATE_EOF;
    hazer_buffer_t nmea_buffer = { 0 };
    char * nmea_bb = (char *)0;
    size_t nmea_ss = 0;
    uint8_t nmea_cs = 0;
    uint8_t nmea_ck = 0;
    yodel_state_t ubx_state = YODEL_STATE_EOF;
    yodel_buffer_t ubx_buffer = { 0 };
    char * ubx_bb = (char *)0;
    size_t ubx_ss = 0;
    size_t ubx_ll = 0;
    uint8_t ubx_ck_a = 0;
    uint8_t ubx_ck_b = 0;
    unsigned char * buffer = (unsigned char *)0;
	unsigned char * bp = (char *)0;
    hazer_buffer_t datagram = { 0 };
    hazer_vector_t vector = { 0 };
    hazer_position_t position = { 0 };
    hazer_solution_t solution = { 0 };
    hazer_constellation_t constellation[HAZER_SYSTEM_TOTAL] = { {  0 } };
    FILE * infp = stdin;
    FILE * outfp = stdout;
    FILE * errfp = stderr;
    FILE * devfp = stdout;
    FILE * logfp = (FILE *)0;
    FILE * strobefp = (FILE *)0;
    FILE * ppsfp = (FILE *)0;
    int devfd = -1;
    int ppsfd = -1;
    int sock = -1;
    int rc = 0;
    int ch = EOF;
    ssize_t size = 0;
    ssize_t length = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    ssize_t count = 0;
    char ** vv = (char **)0;
    char msn = '\0';
    char lsn = '\0';
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    int opt = -1;
    const char * device = (const char *)0;
    const char * strobe = (const char *)0;
    const char * pps = (const char *)0;
    const char * path = (const char *)0;
    int bitspersecond = 9600;
    int databits = 8;
    int paritybit = 0;
    int stopbits = 1;
    int modemcontrol = 0;
    int rtscts = 0;
    int xonxoff = 0;
    int readonly = !0;
    int carrierdetect = 0;
    int strobepin = -1;
    int ppspin = -1;
    role_t role = NONE;
    protocol_t protocol = IPV4;
    format_t format = UNKNOWN;
    const char * host = (const char *)0;
    const char * service = (const char *)0;
    diminuto_ipv4_t ipv4 = 0;
    diminuto_ipv6_t ipv6 = { 0 };
    diminuto_port_t port = 0;
    int output = 0;
    diminuto_list_t * node = (diminuto_list_t *)0;
    diminuto_list_t head = DIMINUTO_LIST_NULLINIT(&head);
    int onepps = 0;
    int tmppps = 0;
    uint64_t nanoseconds = 0;
    diminuto_mux_t mux = { 0 };
    struct Context ctx = { 0 };
    void * result = (void *)0;
    pthread_t thread;
    int pthreadrc = -1;
    FILE * fp = (FILE *)0;
    static const char OPTIONS[] = "124678A:D:EI:L:OP:RW:Vb:cdehlmnop:rsv?";
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    rc = diminuto_interrupter_install(0);
    assert(rc >= 0);

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case '1':
            stopbits = 1;
            break;
        case '2':
            stopbits = 2;
            break;
        case '4':
            protocol = IPV4;
            break;
        case '6':
            protocol = IPV6;
            break;
        case '7':
            databits = 7;
            break;
        case '8':
            databits = 8;
            break;
        case 'A':
            host = optarg;
            break;
        case 'D':
            device = optarg;
            break;
        case 'E':
            report = !0;
            escape = !0;
            break;
        case 'I':
            pps = optarg;
            break;
        case 'L':
        	path = optarg;
        	break;
        case 'O':
            output = !0;
            break;
        case 'P':
            service = optarg;
            break;
        case 'R':
            report = !0;
            break;
        case 'W':
            readonly = 0;
            node = (diminuto_list_t *)malloc(sizeof(diminuto_list_t));
            diminuto_list_datainit(node, optarg);
            diminuto_list_enqueue(&head, node);
            break;
        case 'V':
        	fprintf(outfp, "com-diag-hazer %s %s %s %s\n", program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
        	break;
        case 'b':
            bitspersecond = strtoul(optarg, (char **)0, 0);
            break;
        case 'c':
            modemcontrol = !0;
            carrierdetect = !0;
            break;
        case 'd':
            debug = !0;
            break;
        case 'e':
            paritybit = 2;
            break;
        case 'h':
            rtscts = !0;
            break;
        case 'l':
            modemcontrol = 0;
            break;
        case 'm':
            modemcontrol = !0;
            break;
        case 'n':
            paritybit = 0;
            break;
        case 'o':
            paritybit = 1;
            break;
        case 'p':
            strobe = optarg;
            break;
        case 'r':
        	fp = outfp;
        	outfp = errfp;
        	errfp = fp;
            break;
        case 's':
            xonxoff = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(errfp, "usage: %s [ -d ] [ -v ] [ -V ] [ -D DEVICE ] [ -b BPS ] [ -7 | -8 ]  [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] [ -I PIN ] [ -c ] [ -p PIN ] [ -W NMEA ] [ -R | -E ] [ -A ADDRESS ] [ -P PORT ] [ -O ] [ -L FILE ]\n", program);
            fprintf(errfp, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(errfp, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(errfp, "       -4          Use IPv4 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -6          Use IPv6 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(errfp, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(errfp, "       -A ADDRESS  Send sentences to ADDRESS.\n");
            fprintf(errfp, "       -D DEVICE   Use DEVICE.\n");
            fprintf(errfp, "       -E          Like -R but use ANSI escape sequences.\n");
            fprintf(errfp, "       -I PIN      Take 1PPS from GPIO input PIN (requires -D).\n");
            fprintf(errfp, "       -L FILE     Log sentences to FILE.\n");
            fprintf(errfp, "       -O          Output sentences to DEVICE.\n");
            fprintf(errfp, "       -P PORT     Send to or receive from PORT.\n");
            fprintf(errfp, "       -R          Print a report on standard output.\n");
            fprintf(errfp, "       -W NMEA     Collapse escapes, generate and append suffix, and write to DEVICE.\n");
            fprintf(errfp, "       -V          Print release, vintage, and revision on standard output.\n");
            fprintf(errfp, "       -b BPS      Use BPS bits per second for DEVICE.\n");
            fprintf(errfp, "       -c          Wait for DCD to be asserted (requires -D and implies -m).\n");
            fprintf(errfp, "       -d          Display debug output on standard error.\n");
            fprintf(errfp, "       -e          Use even parity for DEVICE.\n");
            fprintf(errfp, "       -l          Use local control for DEVICE.\n");
            fprintf(errfp, "       -m          Use modem control for DEVICE.\n");
            fprintf(errfp, "       -o          Use odd parity for DEVICE.\n");
            fprintf(errfp, "       -p PIN      Assert GPIO output PIN with 1PPS (requires -D and -I or -c).\n");
            fprintf(errfp, "       -n          Use no parity for DEVICE.\n");
            fprintf(errfp, "       -h          Use RTS/CTS for DEVICE.\n");
            fprintf(errfp, "       -r          Reverse use of standard output and standard error.\n");
            fprintf(errfp, "       -s          Use XON/XOFF for DEVICE.\n");
            fprintf(errfp, "       -v          Display verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    /**
     ** INITIALIZATION
     **/

    if (device != (const char *)0) {

        devfd = open(device, readonly ? O_RDONLY : O_RDWR);
        if (devfd < 0) { diminuto_perror(device); }
        assert(devfd >= 0);

        rc = diminuto_serial_set(devfd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
        assert(rc == 0);

        rc = diminuto_serial_raw(devfd);
        assert(rc == 0);

        devfp = fdopen(devfd, readonly ? "r" : "a+");
        if (devfp == (FILE *)0) { diminuto_perror(device); }
        assert(devfp != (FILE *)0);
        infp = devfp;

    }

    if (path == (const char *)0) {
    	/* Do nothing. */
    } else if (strcmp(path, "-") == 0) {
    	logfp = stdout;
    } else {
    	logfp = fopen(path, "ab");
    	if (logfp == (FILE *)0) { diminuto_perror(path); }
    	assert(logfp != (FILE *)0);
    }

    if (service == (const char *)0) {

        role = NONE;

    } else if (host == (const char *)0) {

        switch (protocol) {
        case IPV4:
            port = diminuto_ipc4_port(service, "udp");
            if (port == 0) { diminuto_perror(service); break; }
            sock = diminuto_ipc4_datagram_peer(port);
            if (sock < 0) { diminuto_perror(service); break; }
            break;
        case IPV6:
            port = diminuto_ipc6_port(service, "udp");
            if (port == 0) { diminuto_perror(service); break; }
            sock = diminuto_ipc6_datagram_peer(port);
            if (sock < 0) { diminuto_perror(service); break; }
            break;
        default:
            break;
        }
        assert(port > 0);
        assert(sock >= 0);

        role = CONSUMER;

    } else {

        switch (protocol) {
        case IPV4:
            ipv4 = diminuto_ipc4_address(host);
            if (diminuto_ipc4_is_unspecified(&ipv4)) { diminuto_perror(host); break; }
            port = diminuto_ipc4_port(service, "udp");
            if (port == 0) { diminuto_perror(service); break; }
            sock = diminuto_ipc4_datagram_peer(0);
            if (sock < 0) { diminuto_perror(service); break; }
            break;
        case IPV6:
            ipv6 = diminuto_ipc6_address(host);
            if (diminuto_ipc6_is_unspecified(&ipv6)) { diminuto_perror(host); break; }
            port = diminuto_ipc6_port(service, "udp");
            if (port == 0) { diminuto_perror(service); break; }
            sock = diminuto_ipc6_datagram_peer(0);
            if (sock < 0) { diminuto_perror(service); break; }
            break;
        default:
            break;
        }
        assert(port > 0);
        assert(sock >= 0);

        rc = diminuto_ipc_set_nonblocking(sock, !0);
        if (rc < 0) { diminuto_perror(service); }
        assert(rc >= 0);

        role = PRODUCER;

    }

    if (strobe != (const char *)0) {
        strobepin = strtol(strobe, (char **)0, 0);
        if (strobepin >= 0) {
            strobefp = diminuto_pin_output(strobepin);
            if (strobefp != (FILE *)0) {
                diminuto_pin_clear(strobefp);
            }
        }
    }

    /*
	 * Handle 1PPS from General Purpose Input/Output (GPIO) pin
	 * by polling until it has changed. The GPIO output of the
	 * USB-Port-GPS doesn't appear to correlate with its serial
	 * output in any way, nor is polling it when we do character
	 * I/O sufficient. So it's interrogated in a separate thread.
	 */

    do {
        if (pps == (const char *)0) {
            break;
        }
        ppspin = strtol(pps, (char **)0, 0);
        if (ppspin < 0) {
            break;
        }
        rc = diminuto_pin_export(ppspin);
        assert(rc >= 0);
        rc = diminuto_pin_direction(ppspin, 0);
        assert(rc >= 0);
        rc = diminuto_pin_active(ppspin, !0);
        assert(rc >= 0);
        rc = diminuto_pin_edge(ppspin, DIMINUTO_PIN_EDGE_BOTH);
        assert(rc >= 0);
        ppsfp = diminuto_pin_open(ppspin);
        assert (ppsfp != (FILE *)0);
        rc = diminuto_pin_get(ppsfp);
        assert(rc >= 0);
        ppsfd = fileno(ppsfp);
        diminuto_mux_init(&mux);
        rc = diminuto_mux_register_interrupt(&mux, ppsfd);
        assert(rc >= 0);
        ctx.done = 0;
        ctx.muxp = &mux;
        ctx.ppsfp = ppsfp;
        ctx.strobefp = strobefp;
        ctx.oneppsp = &onepps;
        pthreadrc = pthread_create(&thread, 0, gpiopoller, &ctx);
        if (pthreadrc != 0) {
        	errno = pthreadrc;
        	diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);
    } while (0);

    /*
     * Handle 1PPS from Data Carrier Detect (DCD) serial line by
     * blocking until it is asserted. The GR-701W asserts DCD just
     * before it unloads a block of sentences. The leading edge of DCD
     * indicates 1PPS. We interrogate DCD in a separate thread to decouple
     * it from our serial input.
     */

    do {
		if (devfp == (FILE *)0) {
			break;
		}
		if (!modemcontrol) {
			break;
		}
		if (!carrierdetect) {
			break;
		}
        ctx.done = 0;
        ctx.ppsfp = devfp;
        ctx.strobefp = strobefp;
        ctx.oneppsp = &onepps;
        pthreadrc = pthread_create(&thread, 0, dcdpoller, &ctx);
        if (pthreadrc != 0) {
        	errno = pthreadrc;
        	diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);
    } while (0);

    if (debug) {
        hazer_debug(errfp);
        yodel_debug(errfp);
    }

    /**
     ** WORK LOOP
     **/

    rc = hazer_initialize();
    assert(rc == 0);

    rc = yodel_initialize();
    assert(rc == 0);

    if (escape) { fputs("\033[1;1H\033[0J", outfp); }

    while (!diminuto_interrupter_check()) {

    	buffer = (void *)0;
        nmea_state = HAZER_STATE_START;
        ubx_state = YODEL_STATE_START;

        /**
         ** INPUT
         **
         ** The input could be an NMEA sentence, a binary UBX packet, or
         ** something we don't know about. It could be coming from standard
         ** input, from a GPS device with a serial byte stream, or from a UDP
         ** IPv4 or IPv6 datagram stream.
         **/

        if (role != CONSUMER) {

        	/*
        	 * If we have any initialization strings to send, do so one at a
        	 * time, if we have a device and its idle. This prevents any
             * incoming data from backing up too much. (I should convert
             * all of this code to a multiplexing scheme using Mux.) Because
             * this queue of writes is checked everytime we reiterate in the
             * work loop, so later code can enqueue new commands to be written
             * to the device. Because this is a doubly-linked list, queued
             * commands can be removed from the queue before they are processed.
             * And the list header can be prepended onto a command string as
             * part of a dynamically allocated structure, and this code will
             * free it.
        	 */

        	if (devfd < 0) {
        		/* Do nothing. */
        	} else if (diminuto_serial_available(devfd) > 0) {
        		/* Do nothing. */
        	} else if (diminuto_list_isempty(&head)) {
        		/* Do nothing. */
        	} else {
        		node = diminuto_list_dequeue(&head);
        		assert(node != (diminuto_list_t *)0);
        		buffer = (unsigned char *)diminuto_list_data(node);
        		assert(buffer != (unsigned char *)0);
            	length = strlen(buffer) + 1;
                size = diminuto_escape_collapse(buffer, buffer, length);
                rc = (size < length) ? emit_packet(devfp, buffer, size - 1) : emit_sentence(devfp, buffer, size - 1);
                if (rc < 0) { fprintf(errfp, "%s: ERR \"%s\"\n", program, buffer); }
                if (verbose) { print_sentence(errfp, buffer, size - 1, UNLIMITED); }
                if (escape) { fputs("\033[2;1H\033[0J", outfp); }
                if (report) { print_sentence(outfp, buffer, size - 1, LIMIT); }
                free(node);
        	}

        	/*
        	 * The NMEA and UBX parsers can be thought of as a single
        	 * non-deterministic finite state machine: an automaton that
        	 * can be in more than one state at a time. The two state
        	 * machines must use different state variables and even
        	 * different buffers, since it is possible both could be active
        	 * at the same time until one of them determines that it has
        	 * collected a correct sentence or packet. The datagram
        	 * code below use the UBX buffer, since it may ultimately
        	 * receive either NMEA or UBX data from the far end.
        	 */

            while (!0) {

            	ch = fgetc(infp);

                nmea_state = hazer_machine(nmea_state, ch, nmea_buffer, sizeof(nmea_buffer), &nmea_bb, &nmea_ss);

                ubx_state = yodel_machine(ubx_state, ch, ubx_buffer, sizeof(ubx_buffer), &ubx_bb, &ubx_ss, &ubx_ll);

                if (nmea_state == HAZER_STATE_END) {
                	break;
                } else if  (nmea_state == HAZER_STATE_EOF) {
                	fprintf(errfp, "%s: EOF\n", program);
                    break;
                } else {
                    /* Do nothing. */
                }

                if (ubx_state == YODEL_STATE_END) {
                	break;
                } else if  (ubx_state == YODEL_STATE_EOF) {
                	fprintf(errfp, "%s: EOF\n", program);
                	break;
                } else {
                    /* Do nothing. */
                }

            }

            if (nmea_state == HAZER_STATE_EOF) {
            	break;
            } else if (ubx_state == YODEL_STATE_EOF) {
            	break;
            } else if (nmea_state == HAZER_STATE_END) {
            	buffer = nmea_buffer;
            	size = nmea_ss;
            } else if (ubx_state == YODEL_STATE_END) {
            	buffer = ubx_buffer;
            	size = ubx_ss;
            } else {
            	assert(0);
            }

        } else if (protocol == IPV4) {

            size = diminuto_ipc4_datagram_receive(sock, ubx_buffer, sizeof(ubx_buffer) - 1);
            if (size <= 0) { break; }
            ubx_buffer[size++] = '\0';
            buffer = ubx_buffer;

        } else if (protocol == IPV6) {

            size = diminuto_ipc6_datagram_receive(sock, ubx_buffer, sizeof(ubx_buffer) - 1);
            if (size <= 0) { break; }
            ubx_buffer[size++] = '\0';
            buffer = ubx_buffer;

        } else {

            assert(0);

        }

        if (verbose) { print_sentence(errfp, buffer, size - 1, UNLIMITED); }

        /**
         ** VALIDATE
         **
         ** We know how to validate an NMEA sentence and a UBX packet. We sanity
         ** check the data format in either case, and compute the appropriate
         ** checksum and verify it. The state machines know what the format of
         ** the data is when we got it directly from the device, but in the case
         ** of IP datagrams, we haven't figured that out yet.
         **/

        if ((length = hazer_length(buffer, size)) > 0) {

        	bp = (unsigned char *)hazer_checksum(buffer, size, &nmea_cs);
        	assert(bp != (unsigned char *)0);

            rc = hazer_characters2checksum(bp[1], bp[2], &nmea_ck);
            assert(rc >= 0);

            if (nmea_ck != nmea_cs) {
                fprintf(errfp, "%s: BAD 0x%02x 0x%02x\n", program, nmea_cs, nmea_ck);
                continue;
            }

            format = NMEA;

        } else if ((length = yodel_length(buffer, size)) > 0) {

        	bp = (unsigned char *)yodel_checksum(buffer, size, &ubx_ck_a, &ubx_ck_b);
        	assert(bp != (unsigned char *)0);

        	if ((ubx_ck_a != bp[0]) || (ubx_ck_b != bp[1])) {
                fprintf(errfp, "%s: BAD 0x%02x%02x 0x%02x%02x\n", program, ubx_ck_a, ubx_ck_b, bp[0], bp[1]);
                continue;
        	}

        	format = UBX;

        } else {

            fprintf(errfp, "%s: ERR %zd\n", program, length);
        	continue;

        }

        if (escape) { fputs("\033[1;1H\033[0K", outfp); }
        if (report) { print_sentence(outfp, buffer, length, LIMIT); }

        /**
         ** FORWARD AND LOG
         **
         ** We forward and log anything we recognize: currently NMEA sentences
         ** or UBX packets. Note that we don't forward the terminating NUL
         ** (using length, instead of size) that terminate all datagrams of any
         ** format (whether that's useful or not).
         **/

        if (role == PRODUCER) { send_sentence(sock, protocol, &ipv4, &ipv6, port, buffer, length); }
        if (logfp != (FILE *)0) { fwrite(buffer, length, 1, logfp); }

        /**
         ** PROCESS
         **
         ** (Currently) we only process NMEA sentences.
         **/

        if (format == NMEA) {

			count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]),  buffer, size);
			assert(count >= 0);
			assert(vector[count - 1] == (char *)0);
			assert(count <= (sizeof(vector) / sizeof(vector[0])));

			/*
			 * This next part is mostly done just to functionally test the API
			 * by demonstrating reversability by regenerating the original
			 * sentence.
			 */

			size = hazer_serialize(datagram, sizeof(datagram), vector, count);
			assert(size <= (sizeof(datagram) - 4));
			assert(datagram[size - 1] == '\0');
			assert(datagram[size - 2] == '*');
			bp = (unsigned char *)hazer_checksum(datagram, size, &nmea_cs);
			hazer_checksum2characters(nmea_cs, &msn, &lsn);
			assert(bp[0] == '*');
			*(++bp) = msn;
			*(++bp) = lsn;
			*(++bp) = '\r';
			*(++bp) = '\n';
			*(++bp) = '\0';
			size += 4;
			assert(size <= sizeof(datagram));
			assert(strncmp(datagram, buffer, size));

			if (count < 1) {
				/* Do nothing. */
			} else if ((talker = hazer_parse_talker(vector[0])) >= HAZER_TALKER_TOTAL) {
				/* Do nothing. */
			} else if ((system = hazer_parse_system(talker)) >= HAZER_SYSTEM_TOTAL) {
				/* Do nothing. */
			} else if (hazer_parse_gga(&position, vector, count) == 0) {
				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;
				if (escape) { fputs("\033[3;1H\033[0K", outfp); }
				if (report) { print_position(outfp, "MAP",  &position, tmppps); }
				if (escape) { fputs("\033[4;1H\033[0K", outfp); }
				if (report) { print_datum(outfp, "GGA",  &position); }
			} else if (hazer_parse_rmc(&position, vector, count) == 0) {
				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;
				if (escape) { fputs("\033[3;1H\033[0K", outfp); }
				if (report) { print_position(outfp, "MAP", &position, tmppps); }
				if (escape) { fputs("\033[4;1H\033[0K", outfp); }
				if (report) { print_datum(outfp, "RMC",  &position); }
			} else if (hazer_parse_gsa(&solution, vector, count) == 0) {
				if (escape) { fputs("\033[5;1H\033[0K", outfp); }
				if (report) { print_active(outfp, "GSA", &solution); }
			} else if (hazer_parse_gsv(&constellation[system], vector, count) == 0) {
				if (escape) { fputs("\033[6;1H\033[0J", outfp); }
				if (report) { print_view(outfp, "GSV", constellation); }
			} else {
				/* Do nothing. */
			}

			if (report) { fflush(outfp); }

			assert(position.tot_nanoseconds >= nanoseconds);
			nanoseconds = position.tot_nanoseconds;

			if (!output) {
				/* Do nothing. */
			} else if (position.dmy_nanoseconds == 0) {
				/* Do nothing (confuses Google Earth). */
			} else {
				fputs(datagram, devfp);
				fflush(devfp);
			}

        } else if (format == UBX) {

        	if (verbose) { diminuto_dump(errfp, buffer, length); }

        } else {

        	/* Do nothing. */

        }

    }

    /**
     ** FINIALIZATION
     **/

    fprintf(errfp, "%s: END\n", program);

    rc = yodel_finalize();
    assert(rc >= 0);

    rc = hazer_finalize();
    assert(rc >= 0);

    if (pthreadrc == 0) {
    	DIMINUTO_COHERENT_SECTION_BEGIN;
    		ctx.done = !0;
    	DIMINUTO_COHERENT_SECTION_END;
    	pthreadrc = pthread_join(thread, &result);
    	if (pthreadrc != 0) {
    		errno = pthreadrc;
    		diminuto_perror("pthread_join");
    	}
    }

    if (ppsfp != (FILE *)0) {
        rc = diminuto_mux_unregister_interrupt(&mux, ppsfd);
        diminuto_mux_fini(&mux);
        ppsfp = diminuto_pin_unused(ppsfp, ppspin);
    }

    if (strobefp != (FILE *)0) {
    	strobefp = diminuto_pin_unused(strobefp, strobepin);
    }

    if (sock >= 0) {
        diminuto_ipc_close(sock);
    }

    if (logfp == (FILE *)0) {
    	/* Do nothing. */
    } else if (logfp == stdout) {
    	/* Do nothing. */
    } else {
    	fclose(logfp);
    }

    (void)fclose(infp);

    (void)fclose(outfp);

    (void)fclose(errfp);

    return 0;
}
