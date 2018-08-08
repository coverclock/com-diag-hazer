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
 * output using ANSI escape sequences, and forward the data to an IP UDP port
 * where perhaps it will be received by another gpstool. It has been used, for
 * example, to integrate a GPS device with a USB interface with the Google Earth
 * web application to create a moving map display, and to implement remote
 * tracking of a moving vehicle by forwarding GPS output in UDP datagrams
 * using an IPv6 connection over an LTE modem.
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
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_list.h"
#include "com/diag/diminuto/diminuto_alarm.h"
#include "com/diag/diminuto/diminuto_timer.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_countof.h"

typedef enum Role { NONE = 0, PRODUCER = 1, CONSUMER = 2 } role_t;

typedef enum Protocol { UNUSED = 0, IPV4 = 4, IPV6 = 6, } protocol_t;

typedef enum Format { UNKNOWN = 0, NMEA = 1, UBX = 2 } format_t;

static const size_t LIMIT = 80;

static const size_t UNLIMITED = ~(size_t)0;

static const char LABEL[] = "FIX";

/**
 * Emit an NMEA sentences to the specified stream after adding the ending
 * matter consisting of the checksum delimiter, the two checksum characters,
 * a carriage return, and a line feed.
 * @param fp points to the FILE stream.
 * @param string points to the NUL-terminated sentence minus the ending matter.
 * @param size is the size of the NMEA sentence.
 * @return 0 for success, <0 if an error occurred.
 */
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

/**
 * Emit a UBX packet to the specified stream after adding the ending matter
 * consisting of the two Fletcher checksum bytes.
 * @param fp points to the FILE stream.
 * @param packet points to the packet minus the ending matter.
 * @param size is the size of the UBX packet.
 * @return 0 for success, <0 if an error occurred.
 */
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

/**
 * Forward an NMEA sentence or a UBX packet to a remote IPv4 or IPv6 host and
 * UDP port.
 * @param sock is an open socket.
 * @param protocol indicates either IPv4 or IPv6.
 * @param ipv4p points to an IPv4 address (if IPv4).
 * @param ipv6p points to an IPv6 address (if IPv6).
 * @param port is an IP UDP port.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 */
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

/**
 * Print an NMEA sentence or UBX packet to a stream, expanding non-printable
 * characters into escape sequences.
 * @param fp points to the FILE stream.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 * @param limit is the maximum number of characters to display.
 */
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

#if 0
/**
 * Select the best global navigation satellite system from among those
 * currently active by finding the one with the lowest dilution of precision.
 * @param aa points to the array of active GNSSes.
 * @param la points to the array of lifetimes for each GNSS.
 * @return an index to the best GNSS or HAZER_SYSTEM_TOTAL is an error occurred.
 */
static hazer_system_t select_active(const hazer_active_t aa[], const unsigned la[])
{
	hazer_system_t system = HAZER_SYSTEM_TOTAL;
	int index = 0;
	int candidate = 0;

	for (index = 0; index < HAZER_SYSTEM_TOTAL; ++index) {
		/*
		 * We want GNSS to be the last possible choice.
		 */
		candidate = (index + 1) % HAZER_SYSTEM_TOTAL;
		if (la[candidate] == 0) {
			continue;
		} else if (aa[candidate].active == 0) {
			continue;
		} else if (system == HAZER_SYSTEM_TOTAL) {
			system = (hazer_system_t)candidate;
		} else if (aa[candidate].pdop > aa[system].pdop) {
			continue;
		} else if (aa[candidate].pdop < aa[system].pdop) {
			system = (hazer_system_t)candidate;
		} else if (aa[candidate].hdop > aa[system].hdop) {
			continue;
 		} else if (aa[candidate].hdop < aa[system].hdop) {
 			system = (hazer_system_t)candidate;
 		} else if (aa[candidate].vdop > aa[system].vdop) {
 			continue;
 		} else if (aa[candidate].vdop < aa[system].vdop) {
 			system = (hazer_system_t)candidate;
 		} else {
 			/* Do nothing. */
 		}
	}

	return system;
}
#endif

/**
 * Print all of the active global navigation satellite systems.
 * @param fp points to the FILE stream.
 * @param name is the NMEA sentence performing this print.
 * @param aa points to the array of active GNSSes.
 * @param la points to an array of lifetimes.
 */
static void print_actives(FILE * fp, const char * name, const hazer_active_t aa[], const unsigned la[])
{
    static const unsigned int IDENTIFIERS = countof(aa[0].id);
    int system = 0;
    int satellite = 0;
    int limit = 0;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

    	if (la[system] == 0) { continue; }

        limit = aa[system].active;
        if (limit == 0) { continue; }

        fprintf(fp, "%s {", name);

        for (satellite = 0; satellite < IDENTIFIERS; ++satellite) {
            if ((satellite < limit) && (aa[system].id[satellite] != 0)) {
                fprintf(fp, " %3u", aa[system].id[satellite]);
            } else {
               	fputs("    ", fp);
            }
        }

       fprintf(fp, " } [%02u] pdop %4.2lf hdop %4.2lf vdop %4.2lf", aa[system].active, aa[system].pdop, aa[system].hdop, aa[system].vdop);

       fprintf(fp, " sys %-8s", HAZER_SYSTEM_NAME[system]);

       fputc('\n', fp);

    }
}

/**
 * Print all of the satellites currently being viewed by the receiver.
 * @param fp points to the FILE stream.
 * @param name is the NMEA sentence performing this print.
 * @param va points to the array of all satellite being viewed.
 * @param la points to an array of lifetimes.
 */
static void print_views(FILE *fp, const char * name, const hazer_view_t va[], const unsigned la[])
{
    static const int SATELLITES = countof(va[0].sat);
    int system = 0;
    int channel = 0;
    int satellite = 0;
    int limit = 0;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

    	if (la[system] == 0) { continue; }

    	limit = va[system].channels;

    	if (limit > va[system].view) { limit = va[system].view; }
        if (limit > SATELLITES) { limit = SATELLITES; }

        for (satellite = 0; satellite < limit; ++satellite) {
            if (va[system].sat[satellite].id != 0) {

        		fputs(name, fp);

            	fprintf(fp, " [%02d] sat %3u elv %2u* azm %3u* snr %2udBHz", ++channel, va[system].sat[satellite].id, va[system].sat[satellite].elv_degrees, va[system].sat[satellite].azm_degrees, va[system].sat[satellite].snr_dbhz);

                fprintf(fp, " sys %-8s", HAZER_SYSTEM_NAME[system]);

                fputc('\n', fp);

            }

        }

    }
}

/**
 * Print all of the navigation position fixes.
 * @param fp points to the FILE stream.
 * @param name is NMEA sentence performing this print.
 * @param pa points to an array of positions.
 * @param la points to an array of lifetimes.
 * @param pps is the current value of the 1PPS strobe.
 */
static void print_positions(FILE * fp, const char * name, const hazer_position_t pa[], const unsigned la[], int pps)
{
    int system = 0;
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

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

    	if (la[system] == 0) { continue; }

    	if (pa[system].dmy_nanoseconds == 0) { continue; }

		fputs(name, fp);

		hazer_format_nanoseconds2timestamp(pa[system].tot_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);
		assert((1 <= month) && (month <= 12));
		assert((1 <= day) && (day <= 31));
		assert((0 <= hour) && (hour <= 23));
		assert((0 <= minute) && (minute <= 59));
		assert((0 <= second) && (second <= 59));
		assert((0 <= nanoseconds) && (nanoseconds < 1000000000ULL));
		fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);

		hazer_format_nanodegrees2position(pa[system].lat_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
		assert((0 <= degrees) && (degrees <= 90));
		assert((0 <= minutes) && (minutes <= 59));
		assert((0 <= seconds) && (seconds <= 59));
		assert((0 <= hundredths) && (hundredths <= 99));
		fprintf(fp, " %2d*%02d'%02d.%02d\"%c", degrees, minutes, seconds, hundredths, direction < 0 ? 'S' : 'N');

		hazer_format_nanodegrees2position(pa[system].lon_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
		assert((0 <= degrees) && (degrees <= 180));
		assert((0 <= minutes) && (minutes <= 59));
		assert((0 <= seconds) && (seconds <= 59));
		assert((0 <= hundredths) && (hundredths <= 99));
		fprintf(fp, ",%3d*%02d'%02d.%02d\"%c", degrees, minutes, seconds, hundredths, direction < 0 ? 'W' : 'E');

		fprintf(fp, " %8.2lf'", pa[system].alt_millimeters * 3.2808 / 1000.0);

		assert((0LL <= pa[system].cog_nanodegrees) && (pa[system].cog_nanodegrees <= 360000000000LL));

		compass = hazer_format_nanodegrees2compass8(pa[system].cog_nanodegrees);
		assert(compass != (const char *)0);
		assert(strlen(compass) <= 4);
		fprintf(fp, " %-2s", compass);

		fprintf(fp, " %8.3lfmph", pa[system].sog_microknots * 1.150779 / 1000000.0);

		fprintf(fp, " pps %c", pps ? '1' : '0');

		fprintf(fp, " sec %u", la[system]);

        fprintf(fp, " sys %-8s", HAZER_SYSTEM_NAME[system]);

		fputc('\n', fp);

    }

}

/**
 * Print all of the navigation solutions.
 * @param fp points to the FILE stream.
 * @param name is NMEA sentence performing this print.
 * @param pa points an array of navigation positions.
 * @param la points to an array of lifetimes.
 */
static void print_solutions(FILE * fp, const char * name, const hazer_position_t pa[], const unsigned la[])
{
	int system = 0;
    double decimal = 0.0;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

    	if (la[system] == 0) { continue; }

    	if (pa[system].dmy_nanoseconds == 0) { continue; }

		fputs(name, fp);

		decimal = pa[system].lat_nanodegrees;
		decimal /= 1000000000.0;
		fprintf(fp, " %9.6lf", decimal);

		decimal = pa[system].lon_nanodegrees;
		decimal /= 1000000000.0;
		fprintf(fp, ",%10.6lf", decimal);

		decimal = pa[system].alt_millimeters;
		decimal /= 1000.0;
		fprintf(fp, " %9.3lfm", decimal);

		decimal = pa[system].cog_nanodegrees;
		decimal /= 1000000000.0;
		fprintf(fp, " %7.3lf*T", decimal);

		decimal = pa[system].mag_nanodegrees;
		decimal /= 1000000000.0;
		fprintf(fp, " %7.3lf*M", decimal);

		decimal = pa[system].sog_microknots;
		decimal /= 1000000.0;
		fprintf(fp, " %8.3lfknots", decimal);

		decimal = pa[system].sog_millimeters;
		decimal /= 1000000.0;
		fprintf(fp, " %8.3lfkph", decimal);

		fprintf(fp, " [%02u]", pa[system].sat_used);

#if 0
		fprintf(fp, " ( %d %d %d %d %d %d %d )", pa[system].lat_digits, pa[system].lon_digits, pa[system].alt_digits, pa[system].cog_digits, pa[system].mag_digits, pa[system].sog_digits, pa[system].smm_digits);
#endif

		fprintf(fp, " sys %-8s", HAZER_SYSTEM_NAME[system]);

		fputc('\n', fp);

    }

}

struct Poller {
	FILE * ppsfp;
	FILE * strobefp;
	int * oneppsp;
	int done;
};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Implement a thread that polls for the data carrier detect (DCD) state for
 * 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
static void * dcdpoller(void * argp)
{
	void * xc = (void *)1;
	struct Poller * ctxp = (struct Poller *)0;
	int done = 0;
	int rc = -1;
	int nowpps = 0;
	int waspps = 0;

	ctxp = (struct Poller *)argp;

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

/**
 * Implement a thread that polls for the general purpose input/output (GPIO)
 * state for 1PPS.
 * @param argp points to the thread context.
 * @return the final value of the thread.
 */
static void * gpiopoller(void * argp)
{
	void * xc = (void *)1;
	struct Poller * pollerp = (struct Poller *)0;
    diminuto_mux_t mux = { 0 };
	int ppsfd = -1;
	int done = 0;
	int rc = -1;
    int fd = -1;
	int nowpps = 0;
	int waspps = 0;

	pollerp = (struct Poller *)argp;

    diminuto_mux_init(&mux);
    ppsfd = fileno(pollerp->ppsfp);
    rc = diminuto_mux_register_interrupt(&mux, ppsfd);
    assert(rc >= 0);

	while (!0) {
		DIMINUTO_COHERENT_SECTION_BEGIN;
			done = pollerp->done;
		DIMINUTO_COHERENT_SECTION_END;
		if (done) {
			xc = (void *)0;
			break;
		}
		rc = diminuto_mux_wait(&mux, -1);
		if (rc <= 0) { break; }
		while (!0) {
			fd = diminuto_mux_ready_interrupt(&mux);
			if (fd < 0) { break; }
			assert(fd == ppsfd);
			rc = diminuto_pin_get(pollerp->ppsfp);
			if (rc < 0) { break; }
			nowpps = !!rc;
			if (nowpps == waspps) {
				/* Do nothing. */
			} else if (nowpps) {
				if (pollerp->strobefp != (FILE *)0) {
					rc = diminuto_pin_set(pollerp->strobefp);
					if (rc < 0) { break; }
				}
				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					*(pollerp->oneppsp) = !0;
				DIMINUTO_CRITICAL_SECTION_END;
			} else {
				if (pollerp->strobefp != (FILE *)0) {
					rc = diminuto_pin_clear(pollerp->strobefp);
					if (rc < 0) { break; }
				}
			}
			waspps = nowpps;
		}
		if (rc < 0) { break; }
	}

    rc = diminuto_mux_unregister_interrupt(&mux, ppsfd);
    diminuto_mux_fini(&mux);

	return xc;
}

/**
 * Run the main program.
 * @param argc is the number of tokens on the command line argument list.
 * @param argv contains the tokens on the command line argument list.
 * @return the exit value of the main program.
 */
int main(int argc, char * argv[])
{
	/*
	 * Command line options and parameters with defaults.
	 */
    const char * program = (const char *)0;
    const char * device = (const char *)0;
    const char * strobe = (const char *)0;
    const char * pps = (const char *)0;
    const char * path = (const char *)0;
    const char * host = (const char *)0;
    const char * service = (const char *)0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
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
    int ignorechecksums = 0;
    role_t role = NONE;
    protocol_t protocol = IPV4;
    unsigned timeout = 60;
    diminuto_list_t * node = (diminuto_list_t *)0;
    diminuto_list_t head = DIMINUTO_LIST_NULLINIT(&head);
    /*
     * Source and sink I/O variables.
     */
    FILE * infp = stdin;
    FILE * outfp = stdout;
    FILE * errfp = stderr;
    FILE * devfp = stdout;
    FILE * logfp = (FILE *)0;
    FILE * strobefp = (FILE *)0;
    FILE * ppsfp = (FILE *)0;
    int devfd = -1;
    /*
     * Datagram socket variables.
     */
    diminuto_ipv4_t ipv4 = 0;
    diminuto_ipv6_t ipv6 = { 0 };
    diminuto_port_t port = 0;
    int sock = -1;
    /*
     * 1PPS poller thread variables.
     */
    struct Poller poller = { 0 };
    void * result = (void *)0;
    pthread_t thread;
    int pthreadrc = -1;
    int onepps = 0;
    int tmppps = 0;
    /*
     * Parser state variables.
     */
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
    /*
     * Processing variables.
     */
    unsigned char * buffer = (unsigned char *)0;
	unsigned char * bp = (char *)0;
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_buffer_t synthesized = { 0 };
    hazer_vector_t vector = { 0 };
    format_t format = UNKNOWN;
    /*
     * GNSS state databases.
     */
    hazer_position_t fix[HAZER_SYSTEM_TOTAL] = { { 0 } };
    hazer_active_t active[HAZER_SYSTEM_TOTAL] = { { 0 } };
    hazer_view_t view[HAZER_SYSTEM_TOTAL] = { { 0 } };
    unsigned lifetime[HAZER_SYSTEM_TOTAL] = { 0 };
    /*
     * Miscellaneous working variables.
     */
    int rc = 0;
    int ch = EOF;
    ssize_t size = 0;
    ssize_t length = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    ssize_t count = 0;
    char msn = '\0';
    char lsn = '\0';
    int output = 0;
    uint64_t nanoseconds = 0;
    FILE * fp = (FILE *)0;
    int elapsed = 0;
    int refresh = 0;
    /*
     * External symbols.
     */
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;
    /*
     * Command line options.
     */
    static const char OPTIONS[] = "124678A:CD:EI:L:OP:RW:Vb:cdehlmnop:rst:v?";

    /*
     * Parse the command line.
     */

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

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
        case 'C':
        	ignorechecksums = !0;
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
            readonly = 0;
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
        case 't':
            timeout = strtoul(optarg, (char **)0, 0);
        	break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(errfp, "usage: %s [ -d ] [ -v ] [ -V ] [ -D DEVICE ] [ -b BPS ] [ -7 | -8 ]  [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] [ -I PIN ] [ -c ] [ -p PIN ] [ -W NMEA ] [ -R | -E ] [ -A ADDRESS ] [ -P PORT ] [ -O ] [ -L FILE ] [ -t SECONDS ] [ -C ]\n", program);
            fprintf(errfp, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(errfp, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(errfp, "       -4          Use IPv4 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -6          Use IPv6 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(errfp, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(errfp, "       -A ADDRESS  Send sentences to ADDRESS.\n");
            fprintf(errfp, "       -C          Ignore bad checksums.\n");
            fprintf(errfp, "       -D DEVICE   Use DEVICE.\n");
            fprintf(errfp, "       -E          Like -R but use ANSI escape sequences.\n");
            fprintf(errfp, "       -I PIN      Take 1PPS from GPIO input PIN (requires -D).\n");
            fprintf(errfp, "       -L FILE     Log sentences to FILE.\n");
            fprintf(errfp, "       -O          Output sentences to DEVICE.\n");
            fprintf(errfp, "       -P PORT     Send to or receive from PORT.\n");
            fprintf(errfp, "       -R          Print a report on standard output.\n");
            fprintf(errfp, "       -W NMEA     Collapse escapes, append checksum, and write to DEVICE.\n");
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
            fprintf(errfp, "       -t SECONDS  Expire GNSS data after SECONDS seconds.\n");
            fprintf(errfp, "       -v          Display verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    /**
     ** INITIALIZATION
     **/

    /*
     * Are we using a GPS receiver with a serial port instead of a IP datagram
     * or standard input?
     */

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

    /*
     * Are we logging every valid sentence or packet to an output file?
     */

    if (path == (const char *)0) {
    	/* Do nothing. */
    } else if (strcmp(path, "-") == 0) {
    	logfp = stdout;
    } else {
    	logfp = fopen(path, "ab");
    	if (logfp == (FILE *)0) { diminuto_perror(path); }
    	assert(logfp != (FILE *)0);
    }

    /*
     * Are we consuming GPS data from an IP port, or producing GPS data on an
     * IP host and port?
     */

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

    /*
     * Are we strobing a GPIO pin with the one pulse per second (1PPS)
     * indication we receive via either another GPIO pin or Data Carrier
     * Detect (DCD) on the serial line?
     */

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
     * Are we monitoring 1PPS from a General Purpose Input/Output pin?
	 * A thread polls the pin until it has changed. The GPIO output of the
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
        poller.done = 0;
        poller.ppsfp = ppsfp;
        poller.strobefp = strobefp;
        poller.oneppsp = &onepps;
        pthreadrc = pthread_create(&thread, 0, gpiopoller, &poller);
        if (pthreadrc != 0) {
        	errno = pthreadrc;
        	diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);
    } while (0);

    /*
     * Are we monitoring 1PPS via Data Carrier Detect (DCD) on a serial line?
     * A thread blocks until it is asserted. The GR-701W asserts DCD just
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
        poller.done = 0;
        poller.ppsfp = devfp;
        poller.strobefp = strobefp;
        poller.oneppsp = &onepps;
        pthreadrc = pthread_create(&thread, 0, dcdpoller, &poller);
        if (pthreadrc != 0) {
        	errno = pthreadrc;
        	diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);
    } while (0);

    /*
     * Install our signal handlers.
     */

    rc = diminuto_interrupter_install(0);
    assert(rc >= 0);

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    rc = diminuto_alarm_install(!0);
    assert(rc >= 0);

    /*
     * Fire up our periodic timer so we can keep track of the age of every
     * GPS update we receive. GPS receivers aren't polite enough to inform
     * us when they've stopped getting updates from a particular global
     * navigation satellite system (GNSS); we must notice this by their absence.
     * And some receivers can receive updates from more than one GNSS by virtue
     * of having more than one radio frequency (RF) stage, so that they can
     * receive multiple frequencies simultaneously. Fresher updates are
     * preferred over stale ones.
     */

    rc = diminuto_timer_periodic(diminuto_frequency());
    assert(rc == 0);

    /*
     * Initialize the NMEA (Hazer) and UBX (Yodel) parsers. If you're into this
     * kind of thing, these parsers are effectively a single non-deterministic
     * finite state automata, an FSA that can be in more than one state at a
     * time, with both state machines racing to see who can recognize a valid
     * statement in their own grammar first.
     */

    rc = hazer_initialize();
    assert(rc == 0);

    rc = yodel_initialize();
    assert(rc == 0);

    if (debug) {
        hazer_debug(errfp);
        yodel_debug(errfp);
    }

    /**
     ** WORK LOOP
     **
     ** We keep working until the far end goes away (file EOF or socket close),
     ** or until we are interrupted by a SIGINT or terminated by a SIGTERM.
     **/

    if (escape) { fputs("\033[1;1H\033[0J", outfp); }

    while ((!diminuto_interrupter_check()) && (!diminuto_terminator_check())) {

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
             * free it. If an post-collapse string is empty, that signals
             * the application to exit. This allows gpstool to be used to
             * initialize a GPS device then exit, perhaps for some other
             * application (even another gpstool) to use the device. One such
             * rationale for this is to send a command to change the baud rate
             * of the GPS device.
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
            	if (buffer[0] == '\0') {
                   	fprintf(errfp, "%s: EXIT.\n", program);
            		break;
            	}
                rc = (size < length) ? emit_packet(devfp, buffer, size - 1) : emit_sentence(devfp, buffer, size - 1);
                if (rc < 0) {
                	fprintf(errfp, "%s: FAILED!\n", program);
                	print_sentence(errfp, buffer, size - 1, UNLIMITED);
                }
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
                	fprintf(errfp, "%s: EOF.\n", program);
                    break;
                } else {
                    /* Do nothing. */
                }

                if (ubx_state == YODEL_STATE_END) {
                	break;
                } else if  (ubx_state == YODEL_STATE_EOF) {
                	fprintf(errfp, "%s: EOF.\n", program);
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
                fprintf(errfp, "%s: CHECKSUM! 0x%02x 0x%02x\n", program, nmea_cs, nmea_ck);
                print_sentence(errfp, buffer, size - 1, UNLIMITED);
                if (!ignorechecksums) { continue; }
            }

            format = NMEA;

        } else if ((length = yodel_length(buffer, size)) > 0) {

        	bp = (unsigned char *)yodel_checksum(buffer, size, &ubx_ck_a, &ubx_ck_b);
        	assert(bp != (unsigned char *)0);

        	if ((ubx_ck_a != bp[0]) || (ubx_ck_b != bp[1])) {
                fprintf(errfp, "%s: CHECKSUM! 0x%02x%02x 0x%02x%02x\n", program, ubx_ck_a, ubx_ck_b, bp[0], bp[1]);
                print_sentence(errfp, buffer, size - 1, UNLIMITED);
                if (!ignorechecksums) { continue; }
        	}

        	format = UBX;

        } else {

            fprintf(errfp, "%s: FORMAT! %zd\n", program, length);
            print_sentence(errfp, buffer, size - 1, UNLIMITED);
        	continue;

        }

        if (verbose) { print_sentence(errfp, buffer, size - 1, UNLIMITED); }
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

        	/*
        	 * We tokenize the NMEA sentence so we can parse it later. Then
        	 * we regenerate the sentence, and verify it, mostly to test the
        	 * underlying API (although we may use the regenerated sentence
        	 * later, since the original was mutated by the tokenization).
        	 */

			count = hazer_tokenize(vector, countof(vector), buffer, size);
			assert(count >= 0);
			assert(vector[count - 1] == (char *)0);
			assert(count <= countof(vector));

			size = hazer_serialize(synthesized, sizeof(synthesized), vector, count);
			assert(size >= 3);
			assert(size <= (sizeof(synthesized) - 4));
			assert(synthesized[0] == HAZER_STIMULUS_START);
			assert(synthesized[size - 2] == HAZER_STIMULUS_CHECKSUM);
			assert(synthesized[size - 1] == HAZER_STIMULUS_NUL);

			bp = (unsigned char *)hazer_checksum(synthesized, size, &nmea_cs);
			hazer_checksum2characters(nmea_cs, &msn, &lsn);
			assert(bp[0] == HAZER_STIMULUS_CHECKSUM);

			*(++bp) = msn;
			*(++bp) = lsn;
			*(++bp) = HAZER_STIMULUS_CR;
			*(++bp) = HAZER_STIMULUS_LF;
			*(++bp) = HAZER_STIMULUS_NUL;

			size += 4;
			assert(size <= sizeof(synthesized));
			assert(strncmp(synthesized, buffer, size));

			/*
			 * Make sure it's a talker and a GNSS that we care about.
			 *
			 * As a special case, if we receive an update on active satellites
			 * or satellites in view from something we don't recognize, then
			 * we have a new GNSS that isn't supported. That's worth noting.
			 */

			if (count < 1) {
				continue;
			} else if ((talker = hazer_parse_talker(vector[0])) >= HAZER_TALKER_TOTAL) {
				if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
					fprintf(errfp, "%s: TALKER?\n", program);
	                print_sentence(errfp, buffer, size - 1, UNLIMITED);
				}
				continue;
			} else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {
				if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
					fprintf(errfp, "%s: SYSTEM?\n", program);
	                print_sentence(errfp, buffer, size - 1, UNLIMITED);
				}
				continue;
			} else {
				/* Do nothing. */
			}

	        /*
	         * See how many seconds have elapsed since the last time we received
	         * a valid message from any system we recognize. (Might be zero.)
	         * Subtract that number from all the lifetimes of all the systems we
	         * care about to figure out if there's a system from which we've
	         * stopped hearing. Finally, update the lifetime for the system
	         * from which we just got an update to give it a full ration of
	         * time.
	         */

	        elapsed = diminuto_alarm_check();

	        if (elapsed > 0) {
	        	int ii;
	        	for (ii = 0; ii < countof(lifetime); ++ii) {
	        		if (lifetime[ii] == 0) {
	        			/* Do nothing. */
	        		} else if (lifetime[ii] <= elapsed) {
	        			lifetime[ii] = 0;
	        		} else {
	        			lifetime[ii] -= elapsed;
	        		}
	        	}
	        }

			/*
			 * Parse the sentences we care about - GGA, RMC, GSA, and GSV
			 * currently - and update our state to reflect to new data.
			 */

			if (hazer_parse_gga(&fix[system], vector, count) == 0) {

				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;

		        lifetime[system] = timeout;

		        refresh = !0;

			} else if (hazer_parse_rmc(&fix[system], vector, count) == 0) {

				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;

		        lifetime[system] = timeout;

		        refresh = !0;


			} else if (hazer_parse_gll(&fix[system], vector, count) == 0) {

				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;

		        lifetime[system] = timeout;

		        refresh = !0;

			} else if (hazer_parse_vtg(&fix[system], vector, count) == 0) {

				DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
					tmppps = onepps;
					onepps = 0;
				DIMINUTO_CRITICAL_SECTION_END;

		        lifetime[system] = timeout;

		        refresh = !0;

			} else if (hazer_parse_gsa(&active[system], vector, count) == 0) {

		        refresh = !0;

			} else if (hazer_parse_gsv(&view[system], vector, count) == 0) {

		        lifetime[system] = timeout;

		        refresh = !0;

			} else {

		        refresh = 0;

			}

			/*
			 * If anything was updated, refresh our display.
			 */

			if (refresh) {
				if (escape) { fputs("\033[3;1H\033[0J", outfp); }
				if (report) {
					print_positions(outfp, LABEL,  fix, lifetime, tmppps);
					print_solutions(outfp, HAZER_NMEA_GPS_MESSAGE_GGA,  fix, lifetime);
					print_actives(outfp, HAZER_NMEA_GPS_MESSAGE_GSA, active, lifetime);
					print_views(outfp, HAZER_NMEA_GPS_MESSAGE_GSV, view, lifetime);
					fflush(outfp);
				}
			}

			/*
			 * We only output NMEA sentences to a device, and even then
			 * we output the regenerated sentence, not the original one.
			 * Note that this can only be done if we got the original
			 * sentence over the IP UDP port or from standard input, because
			 * that's the only circumstances in which we interpret DEVICE this
			 * way.
			 */

			if (!output) {
				/* Do nothing. */
			} else if (fix[system].dmy_nanoseconds == 0) {
				/* Do nothing: day-month-year zero until RMC received. */
            } else if (fix[system].tot_nanoseconds < nanoseconds) {
				/* Do nothing: time running backwards because UDP OOO delivery. */
			} else if (fputs(synthesized, devfp) == EOF) {
                fprintf(errfp, "%s: OUT!\n", program);
                break;
            } else if (fflush(devfp) == EOF) {
                fprintf(errfp, "%s: OUT!\n", program);
                break;
            } else {
                /* Do nothing. */
			}

			nanoseconds = fix[system].tot_nanoseconds;

        } else if (format == UBX) {

        	if (verbose) { diminuto_dump(errfp, buffer, length); }

        } else {

        	/* Do nothing. */

        }

    }

    /**
     ** FINIALIZATION
     **/

    fprintf(errfp, "%s: END.\n", program);

    rc = yodel_finalize();
    assert(rc >= 0);

    rc = hazer_finalize();
    assert(rc >= 0);

    if (pthreadrc == 0) {
    	DIMINUTO_COHERENT_SECTION_BEGIN;
    		poller.done = !0;
    	DIMINUTO_COHERENT_SECTION_END;
        pthreadrc = pthread_kill(thread, SIGINT);
    	if (pthreadrc != 0) {
    		errno = pthreadrc;
    		diminuto_perror("pthread_join");
    	}
    	pthreadrc = pthread_join(thread, &result);
    	if (pthreadrc != 0) {
    		errno = pthreadrc;
    		diminuto_perror("pthread_join");
    	}
    }

    if (ppsfp != (FILE *)0) {
        ppsfp = diminuto_pin_unused(ppsfp, ppspin);
        assert(ppsfp == (FILE *)0);
    }

    if (strobefp != (FILE *)0) {
    	strobefp = diminuto_pin_unused(strobefp, strobepin);
        assert(strobefp == (FILE *)0);
    }

    if (sock >= 0) {
        rc = diminuto_ipc_close(sock);
        assert(rc >= 0);
    }

    if (logfp == (FILE *)0) {
    	/* Do nothing. */
    } else if (logfp == stdout) {
    	/* Do nothing. */
    } else {
    	rc = fclose(logfp);
        assert(rc != EOF);
    }

    rc =fclose(infp);
    assert(rc != EOF);

    rc = fclose(outfp);
    assert(rc != EOF);

    rc = fclose(errfp);
    assert(rc != EOF);

    return 0;
}
