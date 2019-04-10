/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA<BR>
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
 *	gpstool -?
 *
 *	gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -v
 *
 *	gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E
 *
 *	gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -L nmea.txt
 *
 *	gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -6 -A ::1 -P 5555
 *
 *	pstool -6 -P 5555 -E
 *
 *	gpstool -d -v
 *
 *	gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -E -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0'
 *
 *	gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -F -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0'
 *
 * You can log the standard error output to the system log using the Diminuto
 * log command.
 *
 *	gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -E 2> >(log -S)
 */

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
#include "gpstool.h"
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include "com/diag/diminuto/diminuto_fd.h"
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
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_containerof.h"

/*
 * ENUMERATIONS
 */

typedef enum Role { ROLE = 0, PRODUCER = 1, CONSUMER = 2 } role_t;

typedef enum Protocol { PROTOCOL = 0, IPV4 = 4, IPV6 = 6, } protocol_t;

typedef enum Format { FORMAT = 0, NMEA = 1, UBX = 2 } format_t;

typedef enum Status { STATUS = '#', UNKNOWN = '?', NONE = '-', WARNING = '+', CRITICAL = '!', INVALID = '*' } status_t;

typedef enum Marker { MARKER = '#', INACTIVE = ' ', ACTIVE = '<', PHANTOM = '?', UNTRACKED = '!' } marker_t;

/*
 * CONSTANTS
 */

static const size_t LIMIT = 80 - (sizeof("OUT ") - 1) - (sizeof("[123] ") - 1) - (sizeof("\r\n") - 1) - 1;

static const size_t UNLIMITED = ~(size_t)0;

#if !0
static const wchar_t DEGREE = 0x00B0;
#else
static const wchar_t DEGREE = 0x002A;
#endif

/*
 * GLOBALS
 */

static const char * Program = (const char *)0;

static char Hostname[9] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0' };

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
 * Emit a UBX message to the specified stream after adding the ending matter
 * consisting of the two Fletcher checksum bytes.
 * @param fp points to the FILE stream.
 * @param packet points to the packet minus the ending matter.
 * @param size is the size of the UBX packet.
 * @return 0 for success, <0 if an error occurred.
 */
static int emit_message(FILE * fp, const void * packet, size_t size)
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
 * Forward an NMEA sentence or a UBX message to a remote IPv4 or IPv6 host and
 * UDP port.
 * @param sock is an open socket.
 * @param protocol indicates either IPv4 or IPv6.
 * @param ipv4p points to an IPv4 address (if IPv4).
 * @param ipv6p points to an IPv6 address (if IPv6).
 * @param port is an IP UDP port.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 */
static void send_buffer(int sock, protocol_t protocol, diminuto_ipv4_t * ipv4p, diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size)
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
 * Print an NMEA sentence or UBX message to a stream, expanding non-printable
 * characters into escape sequences.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 * @param limit is the maximum number of characters to display.
 */
static void print_buffer(FILE * fp, const void * buffer, size_t size, size_t limit)
{
    const char * bb = (const char *)0;
    size_t current = 0;
    int end = 0;

    for (bb = buffer; size > 0; --size) {
        diminuto_phex_emit(fp, *(bb++), UNLIMITED, 0, 0, 0, &current, &end, 0);
        if (current >= limit) { break; }
    }
    fputc('\n', fp);
}

/**
 * Print all of the active satellites used for the most recent fix.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param aa points to the array of active satellites.
 */
static void print_actives(FILE * fp, FILE * ep, const hazer_active_t aa[])
{
    static const unsigned int IDENTIFIERS = countof(aa[0].id);
    unsigned int system = 0;
    unsigned int satellite = 0;
    unsigned int count = 0;
    unsigned int total = 0;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {
        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }
        total += aa[system].active;
    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }

        fprintf(fp, "%s {", "ACT [1] ");

        count = 0;
        for (satellite = 0; satellite < (IDENTIFIERS / 2); ++satellite) {
            if ((satellite < aa[system].active) && (aa[system].id[satellite] != 0)) {
                fprintf(fp, " %5u", aa[system].id[satellite]);
                count += 1;
            } else {
                fputs("      ", fp);
            }
        }

        fprintf(fp, " } [%2u] [%2u] [%2u]", count, aa[system].active, total);

        fprintf(fp, "%7s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

        if (aa[system].active <= (IDENTIFIERS / 2)) { continue; }

        fprintf(fp, "%s {", "ACT [2] ");

        count = 0;
        for (satellite = (IDENTIFIERS / 2); satellite < IDENTIFIERS; ++satellite) {
            if ((satellite < aa[system].active) && (aa[system].id[satellite] != 0)) {
                fprintf(fp, " %5u", aa[system].id[satellite]);
                count += 1;
            } else {
                fputs("      ", fp);
            }
        }

        fprintf(fp, " } [%2u] [%2u] [%2u]", count, aa[system].active, total);

        fprintf(fp, "%7s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }

        fprintf(fp, "%s", "DOP");

        fprintf(fp, " %6.2lfpdop %6.2lfhdop %6.2lfvdop", (double)aa[system].pdop / 100.0, (double)aa[system].hdop / 100.0, (double)aa[system].vdop / 100.0);

        fprintf(fp, "%34s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

}

/**
 * Print all of the satellites currently being viewed by the receiver.
 * @param fp points to the FILE stream.
 * @param va points to the array of all satellite being viewed.
 * @param aa points to the array of active satellites.
 */
static void print_views(FILE *fp, FILE * ep, const hazer_view_t va[], const hazer_active_t aa[])
{
    static const unsigned int SATELLITES = countof(va[0].sat);
    static const unsigned int IDENTIFIERS = countof(aa[0].id);
    unsigned int system = 0;
    unsigned int channel = 0;
    unsigned int satellite = 0;
    unsigned int active = 0;
    unsigned int limit = 0;
    unsigned int sequence = 0;
    marker_t ranged = MARKER;
    marker_t phantom = MARKER;
    marker_t untracked = MARKER;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (va[system].ticks == 0) { continue; }
        if (va[system].pending > 0) { continue; }

        limit = va[system].channels;
        if (limit > va[system].view) { limit = va[system].view; }
        if (limit > SATELLITES) { limit = SATELLITES; }

        for (satellite = 0; satellite < limit; ++satellite) {

        	if (va[system].sat[satellite].id == 0) { continue; }

			ranged = INACTIVE;
			if (aa[system].active > 0) {
				for (active = 0; active < IDENTIFIERS; ++active) {

					if (active >= aa[system].active) { break; }
					if (aa[system].id[active] == 0) { break; }
					if (aa[system].id[active] == va[system].sat[satellite].id) { ranged = ACTIVE; }

				}
			}

			phantom = va[system].sat[satellite].phantom ? PHANTOM : INACTIVE;
			untracked = va[system].sat[satellite].untracked ? UNTRACKED : INACTIVE;

            sequence = satellite / HAZER_GNSS_VIEWS;

			fputs("SAT", fp);

			fprintf(fp, " [%3u] %5uid %3d%lcelv %4d%lcazm %4ddBHz %2dsig %c %c %c", ++channel, va[system].sat[satellite].id, va[system].sat[satellite].elv_degrees, DEGREE, va[system].sat[satellite].azm_degrees, DEGREE, va[system].sat[satellite].snr_dbhz, va[system].signal[sequence], ranged, phantom, untracked);

			fprintf(fp, "%15s", "");

			fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

			fputc('\n', fp);

        }

#if 0
        /*
         * I have gotten GSV sentences from the U-blox ZED-F9P chip
         * in which I believe the count in the "satellites in view"
         * field is one more than the total number of satellites 
         * reported in the aggregate GSV sentences. I upgraded the
         * FW to 1.11 and still get this message _thousands_ of
         * times, _always_ on the GLONASS constellation.
         */
        if (va[system].pending > 0) {
            /* Do nothing. */
        } else if  (va[system].channels == va[system].view) {
            /* Do nothing. */
        } else {
            fprintf(ep, "ERR %s: VIEW! \"%s\" %u %u %u\n", Program, HAZER_SYSTEM_NAME[system], va[system].pending, va[system].channels, va[system].view);
        }
#endif

    }

}

/**
 * Print the local (Juliet) time (and the release string).
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param timetofirstfix is the number of ticks until thr first fix.
 */
static void print_local(FILE * fp, FILE * ep, diminuto_sticks_t timetofirstfix)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;
    diminuto_sticks_t milliseconds = 0;
    diminuto_sticks_t now = 0;
    diminuto_sticks_t offset = 0;
    diminuto_ticks_t fraction = 0;
    char zone = '\0';
    int rc = 0;

    fputs("LOC", fp);

    now = diminuto_time_clock();
    assert(now >= 0);
    rc = diminuto_time_juliet(now, &year, &month, &day, &hour, &minute, &second, &fraction);
    assert(rc == 0);
    assert((1 <= month) && (month <= 12));
    assert((1 <= day) && (day <= 31));
    assert((0 <= hour) && (hour <= 23));
    assert((0 <= minute) && (minute <= 59));
    assert((0 <= second) && (second <= 59));

    /*
     * I arbitrarily decided to render the fractional part in milliseconds.
     */

    milliseconds = diminuto_frequency_ticks2units(fraction, 1000LL);
    assert((0 <= milliseconds) && (milliseconds < 1000LL));
    fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02d.%03lu", year, month, day, hour, minute, second, (long unsigned int)milliseconds);

    /*
     * There are time zones whose offset are not in whole hours. That's why
     * ISO 8601 permits HH:MM as a format. Go figure. Why do we call this
     * every time rather than cache the offset ourselves? Because it is
     * conceivable that it might be manually changed by a misguided systems
     * administrator while the application is running, and the underlying
     * tzset(3) might actually notice that the zone information in the file
     * system has been altered. Not only will this change the local time,
     * but it will add some latency and jitter to the GPS display. (Yet
     * another reason to admin your embedded system to UTC.)
     */

    offset = diminuto_time_timezone(now);
    zone = diminuto_time_zonename(offset);

    offset = diminuto_frequency_ticks2wholeseconds(offset);
    hour = offset / 3600;
    minute = (offset % 3600) / 60;
    if (minute < 0) { minute = -minute; }
    fprintf(fp, "%+2.2d:%02d", hour, minute);

    /*
     * The abomination that is Daylight Saving Time has an offset that
     * depends upon the current date and time. We express this separately,
     * in a mild extension of ISO 8601, so that we don't confuse the DST
     * offset (which changes seasonally) with the time zone offset (which is,
     * typically, fixed).
     */

    offset = diminuto_time_daylightsaving(now);
    offset = diminuto_frequency_ticks2wholeseconds(offset);
    hour = offset / 3600;
    fprintf(fp, "%+2.2d%c", hour, zone);

    /*
     * This is where we calculate time to first fix.
     */

    if (timetofirstfix >= 0) {

        rc = diminuto_time_duration(timetofirstfix, &day, &hour, &minute, &second, &fraction);
        assert(rc >= 0);
        assert(day >= 0);
        assert((0 <= hour) && (hour <= 23));
        assert((0 <= minute) && (minute <= 59));
        assert((0 <= second) && (second <= 59));

        milliseconds = diminuto_frequency_ticks2units(fraction, 1000LL);
        assert((0 <= milliseconds) && (milliseconds < 1000LL));

        fprintf(fp, " %10d/%02d:%02d:%02d.%03lu", day, hour, minute, second, (long unsigned int)milliseconds);

    } else {

        fprintf(fp, " %10s/%2s:%2s:%2s.%3s", "*", "**", "**", "**", "***");

    }

    fprintf(fp, " %-8.8s", COM_DIAG_HAZER_RELEASE);

    fprintf(fp, " %-8.8s", Hostname);

    fputc('\n', fp);
}

/**
 * Print the hardware monitor details.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param hp points to the hardware monitor details.
 */
static void print_hardware(FILE * fp, FILE * ep, const yodel_hardware_t * hp)
{
    /*
     * Indicate detection of broadband or continuous wave (cw) jamming.
     * Relies on support from later versions of Ublox 8 firmware, and must be
     * explicitly enabled by sending appropriate messages to the Ublox device.
     */

    if (hp->ticks > 0) {
        uint8_t value;
        char jamming;
        static char jamming_prior = STATUS;
        static char jamming_history = STATUS;
        static uint8_t jamInd_maximum = 0;

        value = (hp->payload.flags >> YODEL_UBX_MON_HW_flags_jammingState_SHIFT) & YODEL_UBX_MON_HW_flags_jammingState_MASK;
        switch (value) {
        case YODEL_UBX_MON_HW_flags_jammingState_unknown:
            jamming = UNKNOWN;
            if (jamming_history == STATUS) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_none:
            jamming = NONE;
            if ((jamming_history == STATUS) || (jamming_history == UNKNOWN)) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_warning:
            jamming = WARNING;
            if (jamming_history != CRITICAL) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_critical:
            jamming = CRITICAL;
            jamming_history = jamming;
            break;
        default:
            jamming = INVALID;
            if ((jamming_history == STATUS) || (jamming_history == UNKNOWN)) { jamming_history = jamming; }
            break;
        }

        if (jamming != jamming_prior) {
        	diminuto_log_syslog(DIMINUTO_LOG_PRIORITY_NOTICE, "%s: ubx jamming %u indicator %u\n", Program, value, hp->payload.jamInd);
            jamming_prior = jamming;
        }

        if (hp->payload.jamInd > jamInd_maximum) { jamInd_maximum = hp->payload.jamInd; }

        fputs("MON", fp);

        fprintf(fp, " %cjamming  %chistory %3uindicator %3umaximum", jamming, jamming_history, hp->payload.jamInd, jamInd_maximum);

        fprintf(fp, "%24s", ""); /* This is actually important. */

        fprintf(fp, " %-8s", ""); /* This is actually important. */

        fputc('\n', fp);
    }
}

/**
 * Print the navigation status details.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param sp points to the navigation status details.
 */
static void print_status(FILE * fp, FILE * ep, const yodel_status_t * sp)
{
    static uint32_t msss_prior = 0;
    static uint16_t msss_epoch = 0;

    /*
     * Indicate detection of spoofing by comparing solutions from multiple
     * GNSSes if (and only if) available. Relies on support from later versions
     * of Ublox 8 firmware, and must be explicitly enabled by sending
     * appropriate messages to the UBlox device.
     */

    if (sp->ticks > 0) {
        static char spoofing_prior = STATUS;
        static char spoofing_history = STATUS;
        uint8_t value;
        char spoofing;

        value = (sp->payload.flags2 >> YODEL_UBX_NAV_STATUS_flags2_spoofDetState_SHIFT) & YODEL_UBX_NAV_STATUS_flags2_spoofDetState_MASK;

        switch (value) {
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_unknown:
            spoofing = UNKNOWN;
            if (spoofing_history == STATUS) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_none:
            spoofing = NONE;
            if ((spoofing_history == STATUS) || (spoofing_history == UNKNOWN)) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_one:
            spoofing = WARNING;
            if (spoofing_history != CRITICAL) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_many:
            spoofing = CRITICAL;
            spoofing_history = spoofing;
            break;
        default:
            spoofing = INVALID;
            if ((spoofing_history == STATUS) || (spoofing_history == UNKNOWN)) { spoofing_history = spoofing; }
            break;
        }

        if (spoofing != spoofing_prior) {
        	diminuto_log_syslog(DIMINUTO_LOG_PRIORITY_NOTICE, "%s: ubx spoofing %u\n", Program, value);
                spoofing_prior = spoofing;
        }

        if (sp->payload.msss < msss_prior) {
            msss_epoch += 1;
        }

        fputs("STA", fp);

        fprintf(fp, " %cspoofing %chistory %10ums %10ums %5uepoch", spoofing, spoofing_history, sp->payload.ttff, sp->payload.msss, msss_epoch);

        fprintf(fp, "%12s", ""); /* This is actually important. */

        fprintf(fp, " %-8s", ""); /* This is actually important. */

        fputc('\n', fp);
    }

    msss_prior = sp->payload.msss;
}

/**
 * Print all of the navigation position fixes.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param pa points to an array of positions.
 * @param pps is the current value of the 1PPS strobe.
 * @param dmyokay is true if the DMY field has been set.
 * @param totokay is true if time is monotonically increasing.
 */
static void print_positions(FILE * fp, FILE * ep, const hazer_position_t pa[], int pps, int dmyokay, int totokay)
{
    unsigned int system = 0;
    double decimal = 0.0;
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
    char zone = '\0';

    zone = diminuto_time_zonename(0);

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }
        if (pa[system].dmy_nanoseconds == 0) { continue; }

        fputs("TIM", fp);

        hazer_format_nanoseconds2timestamp(pa[system].tot_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);
        assert((1 <= month) && (month <= 12));
        assert((1 <= day) && (day <= 31));
        assert((0 <= hour) && (hour <= 23));
        assert((0 <= minute) && (minute <= 59));
        assert((0 <= second) && (second <= 59));
        assert((0 <= nanoseconds) && (nanoseconds < 1000000000ULL));
        fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02d.000-00:00+00%c", year, month, day, hour, minute, second, zone);

        fprintf(fp, " %cpps", pps ? '1' : '0');

        fprintf(fp, "%28s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("POS", fp);

        hazer_format_nanodegrees2position(pa[system].lat_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
        assert((0 <= degrees) && (degrees <= 90));
        assert((0 <= minutes) && (minutes <= 59));
        assert((0 <= seconds) && (seconds <= 59));
        assert((0 <= hundredths) && (hundredths <= 99));
        fprintf(fp, " %2d%lc%02d'%02d.%02d\"%c,", degrees, DEGREE, minutes, seconds, hundredths, direction < 0 ? 'S' : 'N');

        hazer_format_nanodegrees2position(pa[system].lon_nanodegrees, &degrees, &minutes, &seconds, &hundredths, &direction);
        assert((0 <= degrees) && (degrees <= 180));
        assert((0 <= minutes) && (minutes <= 59));
        assert((0 <= seconds) && (seconds <= 59));
        assert((0 <= hundredths) && (hundredths <= 99));
        fprintf(fp, " %3d%lc%02d'%02d.%02d\"%c", degrees, DEGREE, minutes, seconds, hundredths, direction < 0 ? 'W' : 'E');

        fputc(' ', fp);

        decimal = pa[system].lat_nanodegrees;
        decimal /= 1000000000.0;
        fprintf(fp, " %10.6lf,", decimal);

        decimal = pa[system].lon_nanodegrees;
        decimal /= 1000000000.0;
        fprintf(fp, " %11.6lf", decimal);

        fprintf(fp, "%12s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("ALT", fp);

        fprintf(fp, " %10.2lf'", pa[system].alt_millimeters * 3.2808 / 1000.0);

        decimal = pa[system].alt_millimeters;
        decimal /= 1000.0;
        fprintf(fp, " %10.3lfm", decimal);

        fprintf(fp, "%43s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("COG", fp);

        assert((0LL <= pa[system].cog_nanodegrees) && (pa[system].cog_nanodegrees <= 360000000000LL));

        compass = hazer_format_nanodegrees2compass8(pa[system].cog_nanodegrees);
        assert(compass != (const char *)0);
        assert(strlen(compass) <= 4);
        fprintf(fp, " %-2s", compass);

        decimal = pa[system].cog_nanodegrees;
        decimal /= 1000000000.0;
        fprintf(fp, " %7.3lf%lcT", decimal, DEGREE);

        decimal = pa[system].mag_nanodegrees;
        decimal /= 1000000000.0;
        fprintf(fp, " %7.3lf%lcM", decimal, DEGREE);

        fprintf(fp, "%44s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("SOG", fp);

        fprintf(fp, " %10.3lfmph", pa[system].sog_microknots * 1.150779 / 1000000.0);

        decimal = pa[system].sog_microknots;
        decimal /= 1000000.0;
        fprintf(fp, " %10.3lfknots", decimal);

        decimal = pa[system].sog_millimeters;
        decimal /= 1000000.0;
        fprintf(fp, " %10.3lfkph", decimal);

        fprintf(fp, "%23s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }

        fputs("INT", fp);

        fprintf(fp, " %s", pa[system].label);

        fprintf(fp, " [%2u]", pa[system].sat_used);

        fprintf(fp, " %ddmy", dmyokay);

        fprintf(fp, " %dinc", totokay);

        fprintf(fp, " ( %2d %2d %2d %2d %2d %2d %2d )", pa[system].lat_digits, pa[system].lon_digits, pa[system].alt_digits, pa[system].cog_digits, pa[system].mag_digits, pa[system].sog_digits, pa[system].smm_digits);

        fprintf(fp, "%23s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

}

/**
 * Print information about the base and the rover that communicate via RTCM.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param bp points to the base structure.
 * @param rp points to the rover structure.
 */
static void print_corrections(FILE * fp, FILE * ep, const yodel_base_t * bp, const yodel_rover_t * rp)
{
	 if (bp->ticks != 0) {

        fputs("BAS", fp);

        fprintf(fp, " %dactive %dvalid %10usec %10uobs %10dum", !!bp->payload.active, !!bp->payload.valid, bp->payload.dur, bp->payload.obs, bp->payload.meanAcc * 100);

        fprintf(fp, "%11s", "");

        fprintf(fp, " %-8s", "RTCM");

        fputc('\n', fp);

	}

	 if (rp->ticks != 0) {

        fputs("ROV", fp);

        fprintf(fp, " %5u: %5u (%5u)", rp->payload.refStation, rp->payload.msgType, rp->payload.subType);

        fprintf(fp, "%46s", "");

        fprintf(fp, " %-7s", "RTCM");

        fputc('\n', fp);

	 }
}

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
                ctxp->onepps = !0;
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
                    pollerp->onepps = !0;
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
 * Common function to count down the expiration fields in the database.
 * @param ep points to the expiration field to count down.
 * @param elapsed is the number of ticks to count down.
 */
static inline void countdown(expiry_t * ep, diminuto_ticks_t elapsed)
{
	if (*ep == 0) {
		/* Do nothing. */
	} else if (*ep <= elapsed) {
		*ep = 0;
	} else {
		*ep -= elapsed;
	}
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
	const char * source = (const char *)0;
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
    int slow = 0;
    int expire = 0;
    role_t role = ROLE;
    protocol_t protocol = IPV4;
    unsigned long timeout = HAZER_GNSS_TICKS;
    struct Command * command = (struct Command *)0;
    diminuto_list_t * node = (diminuto_list_t *)0;
    diminuto_list_t head = DIMINUTO_LIST_NULLINIT(&head);
    int unknown = 0;
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
    /*
     * NMEA parser state variables.
     */
    hazer_state_t nmea_state = HAZER_STATE_EOF;
    hazer_buffer_t nmea_buffer = HAZER_BUFFER_INITIALIZER;
    char * nmea_bb = (char *)0;
    size_t nmea_ss = 0;
    uint8_t nmea_cs = 0;
    uint8_t nmea_ck = 0;
    /*
     * UBX parser state variables.
     */
    yodel_state_t ubx_state = YODEL_STATE_EOF;
    yodel_buffer_t ubx_buffer = HAZER_BUFFER_INITIALIZER;
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
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;
    hazer_buffer_t synthesized = HAZER_BUFFER_INITIALIZER;
    hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
    format_t format = FORMAT;
    /*
     * NMEA state databases.
     */
    hazer_position_t position[HAZER_SYSTEM_TOTAL] = {
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    	HAZER_POSITION_INITIALIZER,
    };
    hazer_active_t active[HAZER_SYSTEM_TOTAL] = {
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
    	HAZER_ACTIVE_INITIALIZER,
	};
    hazer_view_t view[HAZER_SYSTEM_TOTAL] = {
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    	HAZER_VIEW_INITIALIZER,
    };
    /*
     * UBX state databases.
     */
    yodel_hardware_t hardware = YODEL_HARDWARE_INITIALIZER;
    yodel_status_t status = YODEL_STATUS_INITIALIZER;
    yodel_base_t base = YODEL_BASE_INITIALIZER;
    yodel_rover_t rover = YODEL_ROVER_INITIALIZER;
    yodel_ubx_ack_t acknak = YODEL_UBX_ACK_INITIALIZER;
    int acknakpending = 0;
    /*
     * Real time related variables.
     */
    diminuto_sticks_t frequency = 0;
    diminuto_sticks_t was = 0;
    diminuto_sticks_t now = 0;
    diminuto_ticks_t elapsed = 0;
    /*
     * Monotonic time related variables.
     */
    diminuto_sticks_t epoch = 0;
    diminuto_sticks_t fix = -1;
    diminuto_sticks_t timetofirstfix = -1;
    /*
     * Miscellaneous working variables.
     */
    int rc = 0;
    int ch = EOF;
    ssize_t size = 0;
    ssize_t size1 = 0; /* size - 1 */
    ssize_t length = 0;
    size_t current = 0;
    ssize_t check = 0;
    ssize_t count = 0;
    char msn = '\0';
    char lsn = '\0';
    int output = 0;
    FILE * fp = (FILE *)0;
    int refresh = !0;
    int index = -1;
    char * end = (char *)0;
    hazer_active_t cache = HAZER_ACTIVE_INITIALIZER;
    int dmyokay = 0;
    int totokay = 0;
    size_t limitation = 0;
    char * locale = (char *)0;
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
    static const char OPTIONS[] = "124678A:CD:EFI:L:OP:RS:U:VW:Xb:cdehlmnop:rst:uv?";

    /**
     ** PREINITIALIZATION
     **/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;

    (void)gethostname(Hostname, sizeof(Hostname));
    Hostname[sizeof(Hostname) - 1] = '\0';

    locale = setlocale(LC_ALL, "");

    diminuto_log_setmask();

    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);

    epoch = diminuto_time_elapsed();

    /*
     * Parse the command line.
     */

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
            slow = 0;
            break;
        case 'F':
            report = !0;
            escape = !0;
            slow = !0;
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
        case 'S':
            source = optarg;
            break;
        case 'U':
            readonly = 0;
            command = (struct Command *)malloc(sizeof(struct Command));
            command->acknak = !0;
            node = &(command->link);
            diminuto_list_datainit(node, optarg);
            diminuto_list_enqueue(&head, node);
            break;
        case 'V':
            fprintf(outfp, "com-diag-hazer %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'W':
            readonly = 0;
            command = (struct Command *)malloc(sizeof(struct Command));
            command->acknak = 0;
            node = &(command->link);
            diminuto_list_datainit(node, optarg);
            diminuto_list_enqueue(&head, node);
            break;
        case 'X':
            expire = !0;
            break;
        case 'b':
            bitspersecond = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (bitspersecond == 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
            }
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
            timeout = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout > HAZER_GNSS_TICKS)) {
                errno = EINVAL;
                diminuto_perror(optarg);
            }
            break;
        case 'u':
        	unknown = !0;
        	break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(errfp, "usage: %s [ -d ] [ -u ] [ -v ] [ -V ] [ -X ] [ -M PRN ] [ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] | -S SOURCE ] [ -I PIN ] [ -c ] [ -p PIN ] [ -W STRING ... ] [ -U STRING ... ] [ -R | -E | -F ] [ -A ADDRESS ] [ -P PORT ] [ -O ] [ -L FILE ] [ -t SECONDS ] [ -C ]\n", Program);
            fprintf(errfp, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(errfp, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(errfp, "       -4          Use IPv4 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -6          Use IPv6 for ADDRESS, PORT.\n");
            fprintf(errfp, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(errfp, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(errfp, "       -A ADDRESS  Send sentences to ADDRESS.\n");
            fprintf(errfp, "       -C          Ignore bad checksums.\n");
            fprintf(errfp, "       -D DEVICE   Use DEVICE for input or output.\n");
            fprintf(errfp, "       -E          Like -R but use ANSI escape sequences.\n");
            fprintf(errfp, "       -F          Like -E but refresh at 1Hz.\n");
            fprintf(errfp, "       -I PIN      Take 1PPS from GPIO input PIN (requires -D).\n");
            fprintf(errfp, "       -L FILE     Log sentences to FILE.\n");
            fprintf(errfp, "       -O          Output sentences to DEVICE.\n");
            fprintf(errfp, "       -P PORT     Send to or receive from PORT.\n");
            fprintf(errfp, "       -R          Print a report on standard output.\n");
            fprintf(errfp, "       -S SOURCE   Use SOURCE for input.\n");
            fprintf(errfp, "       -U STRING   Collapse STRING, append checksum, write to DEVICE, expect ACK.\n");
            fprintf(errfp, "       -U ''       Exit when this empty STRING is processed.\n");
            fprintf(errfp, "       -V          Print release, vintage, and revision on standard output.\n");
            fprintf(errfp, "       -W STRING   Collapse STRING, append checksum, write to DEVICE.\n");
            fprintf(errfp, "       -W ''       Exit when this empty STRING is processed.\n");
            fprintf(errfp, "       -X          Enable message expiration test mode.\n");
            fprintf(errfp, "       -b BPS      Use BPS bits per second for DEVICE.\n");
            fprintf(errfp, "       -c          Take 1PPS from DCD (requires -D and implies -m).\n");
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
            fprintf(errfp, "       -u          Note unknown NMEA or UBX on standard error.\n");
            fprintf(errfp, "       -v          Display verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    /**
     ** INITIALIZATION
     **/

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

        role = ROLE;

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
        poller.ppsfp = ppsfp;
        poller.strobefp = strobefp;
        poller.onepps = 0;
        poller.done = 0;
        pthreadrc = pthread_create(&thread, 0, gpiopoller, &poller);
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

    /*
     * Initialize our time zone. The underlying tzset(3) call is relatively
     * expensive (it accesses the file system). But at least some
     * implementations memoize (a.k.a. cache) the information gleaned from
     * the file system and from the environment. So we'll call it here to
     * do that so when print_local() calls it, it doesn't introduce a bunch
     * of latency while we're processing the NMEA stream. IMPORTANT TIP: if
     * your Hazer application is in a system that routinely crosses (perhaps
     * many) time zones - as at least four of the products I've worked on do -
     * consider setting the time zone of your system to UTC. If nothing else,
     * your field support people may thank you.
     */

    (void)diminuto_time_timezone(diminuto_time_clock());

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

    /*
     * Are we using a GPS receiver with a serial port instead of a IP datagram
     * or standard input? If this is the case, it turns out to be a good idea
     * to open the serial port(ish) device as close to where we first read from
     * it as practical. This prevents us from losing sentences that the device
     * generates when - apparently - it detects the open from the far end
     * (I'm looking at *you* U-blox 8).
     *
     * N.B. For USB GPS devices, it takes a moment or three for the device to
     * enumerate and show up in the file system. If you, for example, plug in
     * the GPS device and start gpstool too quickly, the open(2) will fail, the
     * assert(3) will fire, and the application will dump core. I do this
     * routinely, alas. Maybe in the future I'll add a check, a delay, and a
     * retry.
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
     * If we are using some other source of input (e.g. a file, a FIFO, etc.),
     * open it here.
     */

    if (source != (const char *)0) {

        infp = fopen(source, "r");
        if (infp == (FILE *)0) { diminuto_perror(source); }
        assert(infp != (FILE *)0);

    }

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
        poller.ppsfp = devfp;
        poller.strobefp = strobefp;
        poller.onepps = 0;
        poller.done = 0;
        pthreadrc = pthread_create(&thread, 0, dcdpoller, &poller);
        if (pthreadrc != 0) {
            errno = pthreadrc;
            diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);
    } while (0);

    /*
     * How much of each packet do we display? Depends on whether we're doing
     * cursor control or not.
     */

    limitation = escape ? LIMIT : UNLIMITED;

    /*
     * Initialize screen.
     */

    if (escape) {
    	fputs("\033[1;1H\033[0J", outfp);
        if (report) {
        	fprintf(outfp, "INP [%3d]\n", 0);
        	fprintf(outfp, "OUT [%3d]\n", 0);
            print_local(outfp, errfp, timetofirstfix);
        	fflush(outfp);
        }
    }

    /*
     * Start the clock.
     */

    frequency = diminuto_frequency();
    now = diminuto_time_elapsed() / frequency;

    /**
     ** WORK LOOP
     **
     ** We keep working until the far end goes away (file EOF or socket close),
     ** or until we are interrupted by a SIGINT or terminated by a SIGTERM.
     **/

     while (!0) {

        if (diminuto_interrupter_check()) {
        	break;
        }

        if (diminuto_terminator_check()) {
        	break;
        }

        /**
         ** INPUT
         **
         ** The input could be an NMEA sentence, a binary UBX packet, or
         ** something we don't know about. It could be coming from standard
         ** input, from a GPS device with a serial byte stream, or from a UDP
         ** IPv4 or IPv6 datagram stream.
         **/

        buffer = (void *)0;

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
            } else if (acknakpending > 0) {
            	/* Do nothing. */
            } else if (diminuto_list_isempty(&head)) {
            	/* Do nothing. */
            } else {
                node = diminuto_list_dequeue(&head);
                assert(node != (diminuto_list_t *)0);
                command = diminuto_containerof(struct Command, link, node);
                buffer = diminuto_list_data(node);
                assert(buffer != (unsigned char *)0);
                if (buffer[0] == '\0') {
                    fprintf(errfp, "END %s: ZERO.\n", Program);
	                free(node);
                    break;
                }
				length = strlen(buffer) + 1;
				size = diminuto_escape_collapse(buffer, buffer, length);
				size1 = size - 1;
				rc = (size < length) ? emit_message(devfp, buffer, size1) : emit_sentence(devfp, buffer, size1);
				if (rc < 0) {
					fprintf(errfp, "ERR %s: FAILED!\n", Program);
					print_buffer(errfp, buffer, size1, UNLIMITED);
				} else {
	                if (command->acknak) { acknakpending += 1; }
					if (verbose) { print_buffer(errfp, buffer, size1, UNLIMITED); }
					if (escape) { fputs("\033[2;1H\033[0K", outfp); }
					if (report) { fprintf(outfp, "OUT [%3zd] ", size1); print_buffer(outfp, buffer, size1, limitation); fflush(outfp); }
				}
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

            nmea_state = HAZER_STATE_START;
            ubx_state = YODEL_STATE_START;

            while (!0) {

                ch = fgetc(infp);

                nmea_state = hazer_machine(nmea_state, ch, nmea_buffer, sizeof(nmea_buffer), &nmea_bb, &nmea_ss);

                ubx_state = yodel_machine(ubx_state, ch, ubx_buffer, sizeof(ubx_buffer), &ubx_bb, &ubx_ss, &ubx_ll);

                if (nmea_state == HAZER_STATE_END) {
                    break;
                } else if  (nmea_state == HAZER_STATE_EOF) {
                    fprintf(errfp, "EOF %s: NMEA.\n", Program);
                    break;
                } else {
                    /* Do nothing. */
                }

                if (ubx_state == YODEL_STATE_END) {
                    break;
                } else if  (ubx_state == YODEL_STATE_EOF) {
                    fprintf(errfp, "EOF %s: UBX.\n", Program);
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

        size1 = size - 1;

        /**
         ** VALIDATE
         **
         ** We know how to validate an NMEA sentence and a UBX message. We
         ** sanity check the data format in either case, and compute the
         ** appropriate checksum and verify it. The state machines know what
         ** the format of the data is when we got it directly from the device,
         ** but in the case of IP datagrams, we haven't figured that out yet.
         **/

        if ((length = hazer_length(buffer, size)) > 0) {

            bp = (unsigned char *)hazer_checksum(buffer, size, &nmea_cs);
            assert(bp != (unsigned char *)0);

            rc = hazer_characters2checksum(bp[1], bp[2], &nmea_ck);
            assert(rc >= 0);

            if (nmea_ck != nmea_cs) {
                fprintf(errfp, "ERR %s: CHECKSUM! 0x%02x 0x%02x\n", Program, nmea_cs, nmea_ck);
                print_buffer(errfp, buffer, size1, UNLIMITED);
                if (!ignorechecksums) { continue; }
            }

            format = NMEA;

        } else if ((length = yodel_length(buffer, size)) > 0) {

            bp = (unsigned char *)yodel_checksum(buffer, size, &ubx_ck_a, &ubx_ck_b);
            assert(bp != (unsigned char *)0);

            if ((ubx_ck_a != bp[0]) || (ubx_ck_b != bp[1])) {
                fprintf(errfp, "ERR %s: CHECKSUM! 0x%02x%02x 0x%02x%02x\n", Program, ubx_ck_a, ubx_ck_b, bp[0], bp[1]);
                print_buffer(errfp, buffer, size1, UNLIMITED);
                if (!ignorechecksums) { continue; }
            }

            format = UBX;

        } else {

            fprintf(errfp, "ERR %s: FORMAT! %zd\n", Program, length);
            print_buffer(errfp, buffer, size1, UNLIMITED);

            format = FORMAT;

            continue;

        }

        if (verbose) { print_buffer(errfp, buffer, size1, UNLIMITED); }
        if (escape) { fputs("\033[1;1H\033[0K", outfp); }
        if (report) { fprintf(outfp, "INP [%3zd] ", length); print_buffer(outfp, buffer, length, limitation); fflush(outfp); }

        /**
         ** FORWARD AND LOG
         **
         ** We forward and log anything we recognize: currently NMEA sentences
         ** or UBX packets. Note that we don't forward the terminating NUL
         ** (using length, instead of size) that terminate all input of any
         ** format (whether that's useful or not).
         **/

        if (role == PRODUCER) { send_buffer(sock, protocol, &ipv4, &ipv6, port, buffer, length); }
        if (logfp != (FILE *)0) { fwrite(buffer, length, 1, logfp); }

        /*
         * EXPIRATION
         *
         * See how many seconds have elapsed since the last time we received
         * a valid message from any system we recognize. (Might be zero.)
         * Subtract that number from all the lifetimes of all the systems we
         * care about to figure out if there's a system from which we've
         * stopped hearing. This implements an expiration for each entry in our
         * database, because NMEA isn't kind enough to remind us that we
         * haven't heard from a system lately (and UBX isn't kind enough to
         * remind us when a device has stopped transmitting entirely); hence
         * data can get stale and needs to be aged out. (We subtract one to
         * eliminate what is almost certainly a partial second.)
         */

        was = now;
        now = diminuto_time_elapsed() / frequency;
        elapsed = (now > was) ? now - was : 0;

        if (elapsed > 0) {

        	for (index = 0; index < HAZER_SYSTEM_TOTAL; ++index) {
                countdown(&position[index].ticks, elapsed);
                countdown(&active[index].ticks, elapsed);
                countdown(&view[index].ticks, elapsed);
            }

        	countdown(&hardware.ticks, elapsed);
            countdown(&status.ticks, elapsed);
            countdown(&base.ticks, elapsed);
            countdown(&rover.ticks, elapsed);

        }

        /**
         ** PROCESS
         **/

        if (format == NMEA) {

            /*
             * We tokenize the NMEA sentence so we can parse it later. Then
             * we regenerate the sentence, and verify it, mostly to test the
             * underlying API. We can use the regenerated sentence for output,
             * since the original was mutated by the tokenization.
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
             * As a special case, if we receive an update on active satellites
             * or satellites in view from something we don't recognize, then
             * we have a new GNSS that isn't supported. That's worth noting.
             */

            if (count < 1) {
                continue;
            } else if ((talker = hazer_parse_talker(vector[0])) >= HAZER_TALKER_TOTAL) {
                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    fprintf(errfp, "UNK %s: TALKER? \"%c%c\"\n", Program, vector[0][1], vector[0][2]);
                    print_buffer(errfp, buffer, size - 1, UNLIMITED);
                }
                continue;
            } else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {
                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    fprintf(errfp, "UNK %s: SYSTEM? \"%c%c\"\n", Program, vector[0][1], vector[0][2]);
                    print_buffer(errfp, buffer, size - 1, UNLIMITED);
                }
                continue;
            } else {
                /* Do nothing. */
            }

            /*
             * Parse the sentences we care about and update our state to
             * reflect the new data. As we go along we do some reality checks
             * to decide if this sentence is valid in the sense at we want
             * to output it to an application like Google Earth Pro, that
             * gets confused is time runs backwards (which can happen if
             * we got this sentence via a UDP datagram).
             */

            if (hazer_parse_gga(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                fix = diminuto_time_elapsed();
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_rmc(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                fix = diminuto_time_elapsed();
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_gll(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                fix = diminuto_time_elapsed();
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_vtg(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_gsa(&cache, vector, count) == 0) {

                /*
                 * This is a special case for the Ublox 8 used in devices like
                 * the GN-803G. It emits multiple GSA sentences all under the
                 * GN (GNSS) talker, but the satellites are either GPS or
                 * GLONASS *plus* WAAS. We'd like to classify them as either
                 * GPS or GLONASS. Sadly, later NMEA standards actually have
                 * a field in the GSA sentence that contains a GNSS System ID,
                 * but I have yet to see a device that supports it. However,
                 * the GSA parser function has untested code to extract this ID
                 * if it exists, and the map function below will use it. Also
                 * note that apparently the DOP values are computed across all
                 * the satellites in whatever constellations were used for a
                 * navigation solution; this means the DOP values for GPS
                 * and GLONASS will be identical in the Ublox 8.
                 */

                if (system == HAZER_SYSTEM_GNSS) {
                    candidate = hazer_map_active_to_system(&cache);
                    if (candidate < HAZER_SYSTEM_TOTAL) {
                        system = candidate;
                    }
                }

                active[system] = cache;
                active[system].ticks = timeout;
                refresh = !0;

            } else if ((rc = hazer_parse_gsv(&view[system], vector, count)) >= 0) {

                /*
                 * I choose not to signal for a refresh unless we have
                 * processed the last GSV sentence of a tuple for a
                 * particular constellation. But I do set the timer
                 * in case the remaining GSV sentences in the tuple
                 * never arrive.
                 */

                view[system].ticks = timeout;
                if (rc == 0) { refresh = !0; }

            } else if (hazer_parse_txt(vector, count) == 0) {

                const char * bb = vector[4];
                size_t current = 0;
                int end = 0;

                fprintf(errfp, "TXT %s: TXT [%2d][%2d][%2d] \"", Program, atoi(vector[1]), atoi(vector[2]), atoi(vector[3]));

                while ((*bb != HAZER_STIMULUS_NUL) && (*bb != HAZER_STIMULUS_CHECKSUM)) {
                    diminuto_phex_emit(errfp, *(bb++), UNLIMITED, 0, 0, 0, &current, &end, 0);
                }

                fputs("\".\n", errfp);

            } else if (unknown) {

                fprintf(errfp, "ERR %s: NMEA \"%s\"\n", Program, vector[0]);

            } else {

            	/* Do nothing. */

            }

        } else if (format == UBX) {

            if (verbose) { diminuto_dump(errfp, buffer, length); }

            if (yodel_ubx_mon_hw(&(hardware.payload), ubx_buffer, length) == 0) {

                hardware.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_nav_status(&(status.payload), ubx_buffer, length) == 0) {

                status.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_ack(&acknak, ubx_buffer, length) == 0) {

            	refresh = !0;

                fprintf(errfp, "UBX %s: %s 0x%02x 0x%02x (%d)\n", Program, acknak.state ? "ACK" : "NAK", acknak.clsID, acknak.msgID, acknakpending);

            	if (acknakpending > 0) { acknakpending -= 1; }

            } else if (yodel_ubx_cfg_valget(ubx_buffer, length) == 0) {

            	/*
            	 * All of the validity checking and byte swapping is done in
            	 * yodel_ubx_cfg_valget(). The parse function doesn't accept
            	 * the message unless it checks out. This is also why the
            	 * buffer is passed as non-const; the variable length payload
            	 * is byteswapped in-place.
            	 */

            	yodel_ubx_cfg_valget_t * pp = (yodel_ubx_cfg_valget_t *)&(ubx_buffer[YODEL_UBX_PAYLOAD]);
            	const char * bb = (const char *)0;
            	const char * ee = &ubx_buffer[length - YODEL_UBX_CHECKSUM];
            	const char * layer = (const char *)0;
            	int ii = 0;
            	yodel_ubx_cfg_valget_key_t kk = 0;
            	size_t ss = 0;
            	size_t ll = 0;
            	uint8_t vv1 = 0;
            	uint16_t vv16 = 0;
            	uint32_t vv32 = 0;
            	uint64_t vv64 = 0;

            	refresh = !0;

            	switch (pp->layer) {
            	case YODEL_UBX_CFG_VALGET_Layer_RAM:
            		layer = "RAM";
            		break;
            	case YODEL_UBX_CFG_VALGET_Layer_BBR:
            		layer = "BBR";
            		break;
            	case YODEL_UBX_CFG_VALGET_Layer_NVM:
            		layer = "NVM";
            		break;
            	case YODEL_UBX_CFG_VALGET_Layer_ROM:
            		layer = "ROM";
            		break;
            	default:
            		layer = "INV";
            		break;
            	}

            	for (bb = &(pp->cfgData[0]); bb < ee; bb += ll) {

					memcpy(&kk, bb, sizeof(kk));

					ss = (kk >> YODEL_UBX_CFG_VALGET_Key_Size_SHIFT) & YODEL_UBX_CFG_VALGET_Key_Size_MASK;

					switch (ss) {
					case YODEL_UBX_CFG_VALGET_Size_BIT:
					case YODEL_UBX_CFG_VALGET_Size_ONE:
						ll = 1;
						break;
					case YODEL_UBX_CFG_VALGET_Size_TWO:
						ll = 2;
						break;
					case YODEL_UBX_CFG_VALGET_Size_FOUR:
						ll = 4;
						break;
					case YODEL_UBX_CFG_VALGET_Size_EIGHT:
						ll = 8;
						break;
					}

					if (ll == 0) { break; }

					bb += sizeof(kk);

					switch (ss) {
					case YODEL_UBX_CFG_VALGET_Size_BIT:
						memcpy(&vv1, bb, sizeof(vv1));
						fprintf(errfp, "UBX %s: CFG VALGET v%d %s [%d] 0x%08x 0x%01x\n", Program, pp->version, layer, ii, kk, vv1);
						break;
					case YODEL_UBX_CFG_VALGET_Size_ONE:
						memcpy(&vv1, bb, sizeof(vv1));
						fprintf(errfp, "UBX %s: CFG VALGET v%d %s [%d] 0x%08x 0x%02x\n", Program, pp->version, layer, ii, kk, vv1);
						break;
					case YODEL_UBX_CFG_VALGET_Size_TWO:
						memcpy(&vv16, bb, sizeof(vv16));
						fprintf(errfp, "UBX %s: CFG VALGET v%d %s [%d] 0x%08x 0x%04x\n", Program, pp->version, layer, ii, kk, vv16);
						break;
					case YODEL_UBX_CFG_VALGET_Size_FOUR:
						memcpy(&vv32, bb, sizeof(vv32));
						fprintf(errfp, "UBX %s: CFG VALGET v%d %s [%d] 0x%08x 0x%08x\n", Program, pp->version,layer, ii, kk, vv32);
						break;
					case YODEL_UBX_CFG_VALGET_Size_EIGHT:
						memcpy(&vv64, bb, sizeof(vv64));
						fprintf(errfp, "UBX %s: CFG VALGET v%d %s [%d] 0x%08x 0x%016llx\n", Program, pp->version, layer, ii, kk, (unsigned long long)vv64);
						break;
					}

					++ii;

				}

            } else if (yodel_ubx_mon_ver(ubx_buffer, length) == 0) {

            	const char * bb = &ubx_buffer[YODEL_UBX_PAYLOAD];
            	const char * ee = &ubx_buffer[length - YODEL_UBX_CHECKSUM];

            	refresh = !0;

            	do {

                	if (bb >= ee) { break; }
            		fprintf(errfp, "UBX %s: MON VER SW \"%s\"\n", Program, bb);
            		bb += YODEL_UBX_MON_VER_swVersion_LENGTH;

            		if (bb >= ee) { break; }
            		fprintf(errfp, "UBX %s: MON VER HW \"%s\"\n", Program, bb);
            		bb += YODEL_UBX_MON_VER_hwVersion_LENGTH;

            		while (bb < ee) {
            			fprintf(errfp, "UBX %s: MON VER EX \"%s\"\n", Program, bb);
            			bb += YODEL_UBX_MON_VER_extension_LENGTH;
            		}

            	} while (0);

            } else if (yodel_ubx_nav_svin(&base.payload, ubx_buffer, length) == 0) {

                base.ticks = timeout;
            	refresh = !0;

            } else if (yodel_ubx_rxm_rtcm(&rover.payload, ubx_buffer, length) == 0) {

                rover.ticks = timeout;
            	refresh = !0;

            } else if (unknown) {

                fprintf(errfp, "ERR %s: UBX <0x%02x 0x%02x 0x%02x 0x%02x>\n", Program, ubx_buffer[0], ubx_buffer[1], ubx_buffer[2], ubx_buffer[3]);

            } else {

            	/* Do nothing. */

            }

        } else {

            /* Do nothing. */

        }

        /*
         * We only output NMEA sentences to a device, and even then
         * we output the regenerated sentence, not the original one.
         * Note that this can only be done if we got the original
         * sentence over the IP UDP port or from standard input, because
         * that's the only circumstances in which we interpret DEVICE this
         * way. Finally, time must monotonically increase (UDP can reorder
         * packets), and we have to have gotten an RMC sentence to set the
         * date before we forward fixes; doing anything else confuses
         * Google Earth, and perhaps other applications.
         */

        if (!output) {
            /* Do nothing. */
        } else if (format != NMEA) {
            /* Do nothing. */
        } else if (!dmyokay) {
            /* Do nothing. */
        } else if (!totokay) {
            /* Do nothing. */
        } else if (fputs(synthesized, devfp) == EOF) {
            fprintf(errfp, "ERR %s: OUT!\n", Program);
            break;
        } else if (fflush(devfp) == EOF) {
            fprintf(errfp, "ERR %s: OUT!\n", Program);
            break;
        } else {
            /* Do nothing. */
        }

        /*
         * This code is just for testing the expiration feature.
         * It turns out to be remarkably difficult to block the most recent
         * GPS receivers, e.g. the UBlox 8. Multiple RF-shielded bags will not
         * block the GPS frequencies. Makes me wish I still had access to those
         * gigantic walk-in Faraday cages that several of my clients have.
         */

        if (!expire) {
        	/* Do nothing. */
        } else if (!refresh) {
        	/* Do nothing. */
        } else {
            static int crowbar = 1000;
            if (crowbar <= 0) {
                for (index = 0; index < HAZER_SYSTEM_TOTAL; ++index) {
                    position[index].ticks = 0;
                }
            }
            if (crowbar <= 100) {
                for (index = 0; index < HAZER_SYSTEM_TOTAL; ++index) {
                    active[index].ticks = 0;
                 }
            }
            if (crowbar <= 200) {
                for (index = 0; index < HAZER_SYSTEM_TOTAL; ++index) {
                    view[index].ticks = 0;
                }
            }
            if (crowbar <= 300) {
                hardware.ticks = 0;
            }
            if (crowbar <= 400) {
                status.ticks = 0;
            }
            if (crowbar > 0) {
                crowbar -= 1;
            }
        }

        /*
         * Calculate our time to first fix.
         */

        if (fix < 0) {
            /* Do nothing. */
        } else if (timetofirstfix >= 0) {
            /* Do nnothing. */
        } else {
            timetofirstfix = fix - epoch;
        }

        /*
         * Refresh our display.
         */

        if (!refresh) {
            /* Do nothing: nothing changed. */
        } else if ((devfd >= 0) && (diminuto_serial_available(devfd) > 0)) {
            /* Do nothing: we still have real-time input waiting. */
        } else if (slow && (was == now)) {
            /* Do nothing: slow display cannot handle real-time refresh rate. */
        } else {
            if (escape) { fputs("\033[3;1H", outfp); }
            if (report) {
                DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
                    onepps = poller.onepps;
                    poller.onepps = 0;
                DIMINUTO_CRITICAL_SECTION_END;
                print_hardware(outfp, errfp, &hardware);
                print_status(outfp, errfp, &status);
                print_local(outfp, errfp, timetofirstfix);
                print_positions(outfp, errfp, position, onepps, dmyokay, totokay);
                print_corrections(outfp, errfp, &base, &rover);
                print_actives(outfp, errfp, active);
                print_views(outfp, errfp, view, active);
            }
            if (escape) { fputs("\033[0J", outfp); }
            if (report) { fflush(outfp); }
            refresh = 0;
        }

    }

    /**
     ** FINIALIZATION
     **/

    fprintf(errfp, "END %s: END.\n", Program);

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

    rc = fclose(infp);
    assert(rc != EOF);

    rc = fclose(outfp);
    assert(rc != EOF);

    rc = fclose(errfp);
    assert(rc != EOF);

    return 0;
}
