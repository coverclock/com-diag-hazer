/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
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
 *  gpstool -?
 *
 *  gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -v
 *
 *  gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E
 *
 *  gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -L nmea.txt
 *
 *  gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -6 -A ::1 -P 5555
 *
 *  gpstool -6 -P 5555 -E
 *
 *  gpstool -d -v
 *
 *  gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -E -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0'
 *
 *  gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -F -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0'
 *
 * You can log the standard error output to the system log using the Diminuto
 * log command.
 *
 *  gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -E 2> >(log -S)
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
#include "./gpstool.h"
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
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_containerof.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_file.h"

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/**
 * If we're displaying in real-time using full screen control, we try to limit
 * our output lines to this many bytes.
 */
static const size_t LIMIT = 80 - (sizeof("OUT ") - 1) - (sizeof("[123] ") - 1) - (sizeof("\r\n") - 1) - 1;

/**
 * If we're just scrolling our output continuously, we don't limit the line
 * length.
 */
static const size_t UNLIMITED = ~(size_t)0;

/**
 * This is the Unicode for the degree symbol.
 */
static const wchar_t DEGREE = 0x00B0;

/**
 * This is the Unicode for the plus&minus symbol.
 */
static const wchar_t PLUSMINUS = 0x00B1;

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

/**
 * This is our POSIX thread mutual exclusion semaphore.
 */
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*******************************************************************************
 * HELPERS
 ******************************************************************************/

/**
 * Return the absolute value of a signed sixty-four bit integer.
 * @param datum is a signed sixty-four bit integer.
 * @return an unsigned sixty-four bit integer.
 */
static inline uint64_t abs64(int64_t datum)
{
    return (datum >= 0) ? datum : -datum;
}

static inline diminuto_sticks_t ticktock(diminuto_sticks_t frequency)
{
    return diminuto_time_elapsed() / frequency;
}

/**
 * Common function to count down the expiration fields in the database.
 * @param ep points to the expiration field to count down.
 * @param elapsed is the number of ticks to count down.
 */
static inline void countdown(expiry_t * ep, diminuto_sticks_t elapsed)
{
    if (*ep == 0) {
        /* Do nothing. */
    } else if (elapsed <= 0) {
        /* Do nothing. */
    } else if (*ep <= elapsed) {
        *ep = 0;
    } else {
        *ep -= elapsed;
    }
}

/*******************************************************************************
 * EMITTERS
 ******************************************************************************/

/**
 * Emit an NMEA configuration sentence to the specified stream after adding the
 * ending matter consisting of the checksum delimiter, the two checksum
 * characters, a carriage return, and a line feed.
 * @param fp points to the FILE stream.
 * @param string points to the NUL-terminated sentence minus the ending matter.
 * @param size is the size of the NMEA sentence in bytes.
 */
static void emit_sentence(FILE * fp, const char * string, size_t size)
{
    uint8_t msn = '\0';
    uint8_t lsn = '\0';

    if (hazer_checksum_buffer(string, size, &msn, &lsn) == (void *)0) {
        errno = EIO;
        diminuto_perror("emit_sentence: checksum");
    } else if (fprintf(fp, "%s%c%c%c\r\n", string, HAZER_STIMULUS_CHECKSUM, msn, lsn) < 0) {
        errno = EIO;
        diminuto_perror("emit_sentence: fprintf");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("emit_sentence: fflush");
   } else {
        /* Do nothing. */
   }
}

/**
 * Emit a UBX configuration packet to the specified stream after adding the
 * ending matter consisting of the two Fletcher checksum bytes.
 * @param fp points to the FILE stream.
 * @param packet points to the packet minus the ending matter.
 * @param size is the size of the UBX packet in bytes.
 */
static void emit_packet(FILE * fp, const void * packet, size_t size)
{
    const void * bp = (const void *)0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    size_t length = 0;

    if ((bp = yodel_checksum_buffer(packet, size, &ck_a, &ck_b)) == (void *)0) {
        errno = EIO;
        diminuto_perror("emit_packet: checksum");
    } else if (fwrite(packet, length = (const char *)bp - (const char *)packet, 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 1");
    } else if (fwrite(&ck_a, sizeof(ck_a), 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 2");
    } else if (fwrite(&ck_b, sizeof(ck_b), 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 3");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("emit_packet: fflush");
    } else {
        /* Do nothing. */
    }
}

/**
 * Write a buffer to the specified stream.
 * @param fp points to the output stream.
 * @param ep points to the error stream.
 * @param buffer points to the buffer.
 * @param size is the number of bytes to write.
 */
static void write_buffer(FILE * fp, const void * buffer, size_t size)
{
    if (fwrite(buffer, size, 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("write_buffer: fwrite");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("write_buffer: fflush");
    } else {
        /* Do nothing. */
    }
}

/**
 * Send an datagram to a remote IPv4 or IPv6 host and UDP port.
 * @param fd is an open socket.
 * @param protocol indicates either IPv4 or IPv6.
 * @param ipv4p points to an IPv4 address (if IPv4).
 * @param ipv6p points to an IPv6 address (if IPv6).
 * @param port is an IP UDP port.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 */
static void send_datagram(int fd, protocol_t protocol, const diminuto_ipv4_t * ipv4p, const diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size)
{
    if (size <= 0) {
        /* Do nothing. */
    } else if (protocol == IPV4) {
        (void)diminuto_ipc4_datagram_send(fd, buffer, size, *ipv4p, port);
    } else if (protocol == IPV6) {
        (void)diminuto_ipc6_datagram_send(fd, buffer, size, *ipv6p, port);
    } else {
        /* Do nothing. */
    }
}

/**
 * Receive a datagram from a UDP port. The datagram will be NUL terminated.
 * The provided buffer must be sized one more byte than the received datagram.
 * @param fd is an open socket.
 * @param buffer points to the buffer.
 * @param size is the size of the buffer in bytes.
 * @return the size of the received datagram plus the terminating NUL in bytes.
 */
static ssize_t receive_datagram(int fd, void * buffer, size_t size) {
    ssize_t length = 0;
    diminuto_ipv6_t address = { 0, };
	diminuto_port_t port = 0;

    if (size <= 1) {
        /* Do nothing. */
    } else if ((length = diminuto_ipc6_datagram_receive_generic(fd, buffer, size - 1, &address, &port, 0)) <= 0) {
        /* Do nothing. */
    } else if (length >= size) {
        /* Should be impossible. */
    } else {
        ((uint8_t *)buffer)[length++] = '\0';
    }

    return length;
}

/*******************************************************************************
 * REPORTERS
 ******************************************************************************/

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
        diminuto_phex_emit(fp, *(bb++), UNLIMITED, 0, !0, 0, &current, &end, 0);
        if (current >= limit) { break; }
    }
    fputc('\n', fp);
}

/**
 * Print all of the active satellites used for the most recent fix.
 * @param fp points to the FILE stream.
 * @param aa points to the array of active satellites.
 */
static void print_actives(FILE * fp, const hazer_active_t aa[])
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
static void print_views(FILE *fp, const hazer_view_t va[], const hazer_active_t aa[])
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
         * times, _always_ on the GLONASS constellation. I reported what
         * I believe is a bug to U-blox.
         */
        if (va[system].pending > 0) {
            /* Do nothing. */
        } else if  (va[system].channels == va[system].view) {
            /* Do nothing. */
        } else {
            DIINUTO_LOG_WARNING("VIEW \"%s\" %u %u %u\n", HAZER_SYSTEM_NAME[system], va[system].pending, va[system].channels, va[system].view);
        }
#endif

    }

}

/**
 * Print the local (Juliet) time (and the release string).
 * @param fp points to the FILE stream.
 * @param timetofirstfix is the number of ticks until thr first fix.
 */
static void print_local(FILE * fp, diminuto_sticks_t timetofirstfix)
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
 * @param hp points to the hardware monitor details.
 */
static void print_hardware(FILE * fp, const yodel_hardware_t * hp)
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
            DIMINUTO_LOG_NOTICE("UBX MON jamming %u indicator %u\n", value, hp->payload.jamInd);
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
 * @param sp points to the navigation status details.
 */
static void print_status(FILE * fp, const yodel_status_t * sp)
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
            DIMINUTO_LOG_NOTICE("UBX NAV spoofing %u\n", value);
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
 * @param pa points to an array of positions.
 * @param pps is the current value of the 1PPS strobe.
 * @param dmyokay is true if the DMY field has been set.
 * @param totokay is true if time is monotonically increasing.
 */
static void print_positions(FILE * fp, const hazer_position_t pa[], int pps, int dmyokay, int totokay)
{
    unsigned int system = 0;
    int64_t whole = 0;
    uint64_t fraction = 0;
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

        whole = pa[system].lat_nanodegrees / 1000000000LL;
        fraction = abs64(pa[system].lat_nanodegrees) % 1000000000LLU;
        fprintf(fp, " %4lld.%09llu,", (long long signed int)whole, (long long unsigned int)fraction);

        whole = pa[system].lon_nanodegrees / 1000000000LL;
        fraction = abs64(pa[system].lon_nanodegrees) % 1000000000LLU;
        fprintf(fp, " %4lld.%09llu", (long long signed int)whole, (long long unsigned int)fraction);

        fprintf(fp, "%5s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("ALT", fp);

        fprintf(fp, " %10.2lf'", pa[system].alt_millimeters * 3.2808 / 1000.0);

        whole = pa[system].alt_millimeters / 1000LL;
        fraction = abs(pa[system].alt_millimeters) % 1000LLU;
        fprintf(fp, " %6lld.%03llum", (long long signed int)whole, (long long unsigned int)fraction);

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

        whole = pa[system].cog_nanodegrees / 1000000000LL;
        fraction = abs64(pa[system].cog_nanodegrees) % 1000000000LLU;
        fprintf(fp, " %4lld.%09llu%lcT", (long long signed int)whole, (long long unsigned int)fraction, DEGREE);

        whole = pa[system].mag_nanodegrees / 1000000000LL;
        fraction = abs64(pa[system].mag_nanodegrees) % 1000000000LLU;
        fprintf(fp, " %4lld.%09llu%lcM", (long long signed int)whole, (long long unsigned int)fraction, DEGREE);

        fprintf(fp, "%30s", "");

        fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (pa[system].ticks == 0) { continue; }
        if (pa[system].utc_nanoseconds == 0) { continue; }

        fputs("SOG", fp);

        fprintf(fp, " %11.3lfmph", pa[system].sog_microknots * 1.150779 / 1000000.0);

        whole = pa[system].sog_microknots / 1000000LL;
        fraction = abs64(pa[system].sog_microknots) % 1000000ULL;
        fprintf(fp, " %7lld.%06lluknots", (long long signed int)whole, (long long unsigned int)fraction);

        whole = pa[system].sog_millimeters / 1000000LL;
        fraction = abs64(pa[system].sog_millimeters) % 1000000ULL;
        fprintf(fp, " %7lld.%06llukph", (long long signed int)whole, (long long unsigned int)fraction);

        fprintf(fp, "%14s", "");

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
 * @param bp points to the base structure.
 * @param rp points to the rover structure.
 * @param mp points to the kinematics structure.
 */
static void print_corrections(FILE * fp, const yodel_base_t * bp, const yodel_rover_t * rp, const tumbleweed_message_t * kp)
{
     if (bp->ticks != 0) {

        fputs("BAS", fp);
        fprintf(fp, " %dactive %dvalid %10usec %10uobs %12.4lfm", !!bp->payload.active, !!bp->payload.valid, bp->payload.dur, bp->payload.obs, (double)bp->payload.meanAcc / 10000.0);
        fprintf(fp, "%10s", "");
        fprintf(fp, " %-8s", "DGNSS");
        fputc('\n', fp);

    }

     if (rp->ticks != 0) {

        fputs("ROV", fp);
        fprintf(fp, " %5u: %5u (%5u)", rp->payload.refStation, rp->payload.msgType, rp->payload.subType);
        fprintf(fp, "%46s", "");
        fprintf(fp, " %-8s", "DGNSS");
        fputc('\n', fp);

     }

     if (kp->ticks != 0) {

         fputs("RTK", fp);
         fprintf(fp, " %4u [%4zu] [%4zu] [%4zu]", kp->number, kp->minimum, kp->length, kp->maximum);
         fprintf(fp, "%42s", "");
         fprintf(fp, "%-8s", "DGNSS");
         fputc('\n', fp);

     }

}

/**
 * Print information about the high-precision positioning solution that UBX
 * provides. I think this is the same result as NMEA but is expressed with
 * the maximum precision available in the underlying device and beyond which
 * NMEA can express.
 * @param fp points to the FILE stream.
 * @param sp points to the solutions structure.
 */
static void print_solution(FILE * fp, const yodel_solution_t * sp)
{
    int64_t value = 0;
    int64_t whole = 0;
    uint64_t fraction = 0;

    if (sp->ticks != 0) {

        fputs("HPP", fp);

        value = sp->payload.lat;
        value *= 100;
        value += sp->payload.latHp;
        whole = value / 1000000000LL;
        fraction = abs64(value) % 1000000000ULL;
        fprintf(fp, " %4lld.%09llu,", (long long signed int)whole, (long long unsigned int)fraction);

        value = sp->payload.lon;
        value *= 100;
        value += sp->payload.lonHp;
        whole = value / 1000000000LL;
        fraction = abs64(value) % 1000000000ULL;
        fprintf(fp, " %4lld.%09llu", (long long signed int)whole, (long long unsigned int)fraction);

        // fprintf(fp, " hAcc=%d", sp->payload.hAcc);

        value = sp->payload.hAcc;
        whole = value / 10000LL;
        fraction = abs64(value) % 10000ULL;
        fprintf(fp, " %lc%6lld.%04llum", PLUSMINUS, (long long signed int)whole, (long long unsigned int)fraction);

        fprintf(fp, "%22s", "");

        fprintf(fp, " %-8s", "GNSS");

        fputc('\n', fp);

        fputs("HPA", fp);

        value = sp->payload.hMSL;
        value *= 10;
        value += sp->payload.hMSLHp;
        whole = value / 10000LL;
        fraction = abs64(value) % 10000ULL;
        fprintf(fp, " %6lld.%04llum", (long long signed int)whole, (long long unsigned int)fraction);

        // fprintf(fp, " vACC=%d", sp->payload.vAcc);

        value = sp->payload.vAcc;
        whole = value / 10000LL;
        fraction = abs64(value) % 10000ULL;
        fprintf(fp, " %lc%6lld.%04llum", PLUSMINUS, (long long signed int)whole, (long long unsigned int)fraction);

        fprintf(fp, "%40s", "");

        fprintf(fp, " %-8s", "GNSS");

        fputc('\n', fp);

    }
}

/*******************************************************************************
 * THREADS
 ******************************************************************************/

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
    /*
     * Command line options and parameters with defaults.
     */
    const char * source = (const char *)0;
    const char * strobe = (const char *)0;
    const char * logging = (const char *)0;
    const char * headless = (const char *)0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
    int strobepin = -1;
    int ppspin = -1;
    int ignorechecksums = 0;
    int slow = 0;
    int expire = 0;
    int unknown = 0;
    long timeout = HAZER_GNSS_SECONDS;
    long keepalive = TUMBLEWEED_KEEPALIVE_SECONDS;
    /*
     * Configuration command variables.
     */
    struct Command * command = (struct Command *)0;
    diminuto_list_t * command_node = (diminuto_list_t *)0;
    diminuto_list_t command_list = DIMINUTO_LIST_NULLINIT(&command_list);
    uint8_t * command_payload = (uint8_t *)0;
    ssize_t command_size = 0;
    ssize_t command_length = 0;
    /*
     * FILE pointer variables.
     */
    FILE * in_fp = stdin;
    FILE * out_fp = stdout;
    FILE * dev_fp = stdout;
    FILE * log_fp = (FILE *)0;
    FILE * strobe_fp = (FILE *)0;
    FILE * pps_fp = (FILE *)0;
    /*
     * Serial device variables.
     */
    direction_t direction = INPUT;
    const char * device = (const char *)0;
    int bitspersecond = 9600;
    int databits = 8;
    int paritybit = 0;
    int stopbits = 1;
    int modemcontrol = 0;
    int rtscts = 0;
    int xonxoff = 0;
    int readonly = !0;
    int carrierdetect = 0;
    long device_mask = NMEA;
    /*
     * Datagram variables.
     */
    protocol_t datagram_protocol = PROTOCOL;
    datagram_buffer_t datagram_buffer = DATAGRAM_BUFFER_INITIALIZER;
    const char * datagram_option = (const char *)0;
    diminuto_ipc_endpoint_t datagram_endpoint = { 0, };
    ssize_t datagram_size = 0;
    ssize_t datagram_length = 0;
    long datagram_mask = NMEA;
    role_t role = ROLE;
    /*
     * Surveyor variables.
     */
    protocol_t surveyor_protocol = PROTOCOL;
    datagram_buffer_t surveyor_buffer = DATAGRAM_BUFFER_INITIALIZER;
    const char * surveyor_option = (const char *)0;
    diminuto_ipc_endpoint_t surveyor_endpoint = { 0, };
    ssize_t surveyor_size = 0;
    ssize_t surveyor_length = 0;
    uint8_t surveyor_crc1 = 0;
    uint8_t surveyor_crc2 = 0;
    uint8_t surveyor_crc3 = 0;
    /*
     * File Descriptor variables.
     */
    int in_fd = -1;
    int dev_fd = -1;
    int datagram_fd = -1;
    int surveyor_fd = -1;
    /*
     * 1PPS poller thread variables.
     */
    const char * pps = (const char *)0;
    struct Poller poller = { 0 };
    void * result = (void *)0;
    pthread_t thread;
    int pthreadrc = -1;
    int onepps = 0;
    /*
     * NMEA parser state variables.
     */
    hazer_state_t nmea_state = HAZER_STATE_STOP;
    hazer_buffer_t nmea_buffer = HAZER_BUFFER_INITIALIZER;
    hazer_context_t nmea_context = { 0, };
    /*
     * UBX parser state variables.
     */
    yodel_state_t ubx_state = YODEL_STATE_STOP;
    yodel_buffer_t ubx_buffer = YODEL_BUFFER_INITIALIZER;
    yodel_context_t ubx_context = { 0, };
    /*
     * RTCM parser state variables.
     */
    tumbleweed_state_t rtcm_state = TUMBLEWEED_STATE_STOP;
    tumbleweed_buffer_t rtcm_buffer = TUMBLEWEED_BUFFER_INITIALIZER;
	tumbleweed_context_t rtcm_context = { 0, };
    /*
     * NMEA processing variables.
     */
    hazer_buffer_t tokenized = HAZER_BUFFER_INITIALIZER;
    hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;
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
    yodel_solution_t solution = YODEL_SOLUTION_INITIALIZER;
    yodel_hardware_t hardware = YODEL_HARDWARE_INITIALIZER;
    yodel_status_t status = YODEL_STATUS_INITIALIZER;
    yodel_base_t base = YODEL_BASE_INITIALIZER;
    yodel_rover_t rover = YODEL_ROVER_INITIALIZER;
    yodel_ubx_ack_t acknak = YODEL_UBX_ACK_INITIALIZER;
    int acknakpending = 0;
    /*
     * RTCM state databases.
     */
    tumbleweed_message_t kinematics = TUMBLEWEED_MESSAGE_INITIALIZER;
    /*
     * Time keeping variables.
     */
    diminuto_sticks_t frequency = 0;
    diminuto_sticks_t expiration_was = 0;
    diminuto_sticks_t expiration_now = 0;
    diminuto_sticks_t display_was = 0;
    diminuto_sticks_t display_now = 0;
    diminuto_sticks_t keepalive_was = 0;
    diminuto_sticks_t keepalive_now = 0;
    diminuto_sticks_t elapsed = 0;
    diminuto_sticks_t epoch = 0;
    diminuto_sticks_t fix = -1;
    diminuto_sticks_t timetofirstfix = -1;
    /*
     * I/O buffer variables.
     */
    void * io_buffer = (void *)0;
    size_t io_size = BUFSIZ;
    ssize_t io_available = 0;
    size_t io_maximum = 0;
    /*
     * Source variables.
     */
    diminuto_mux_t mux;
    int ch = EOF;
    int ready = 0;
    int fd = -1;
    format_t format = FORMAT;
    FILE * fp = (FILE *)0;
    uint8_t * buffer = (uint8_t *)0;
    ssize_t size = 0;
    ssize_t length = 0;
    /*
     * Display variables.
     */
    char * temporary = (char *)0;
    size_t limitation = 0;
    /**
     * Control variables.
     */
    int eof = 0;		/** If true then the input stream hit end of file. */
    int sync = 0;		/** If true then the input stream is synchronized. */
    int frame = 0;		/** If true then the input stream is at frame start. */
    int refresh = !0;	/** If true then the display needs to be refreshed. */
    /*
     * Command line processing variables.
     */
    int error = 0;
    char * end = (char *)0;
    /*
     * Data processing variables.
     */
    ssize_t count = 0;
    hazer_active_t cache = HAZER_ACTIVE_INITIALIZER;
    int dmyokay = 0;
    int totokay = 0;
    /*
     * Miscellaneous variables.
     */
    int rc = 0;
    char * locale = (char *)0;
    diminuto_ipv4_buffer_t ipv4;
    diminuto_ipv6_buffer_t ipv6;
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
    static const char OPTIONS[] = "1278B:CD:EFG:H:I:KL:ORS:U:VW:XY:b:cdeg:hk:lmnop:st:uvy:?"; /* Unused: AJNPQTXZ afijqrwxz Pairs: Aa Jj Qq Zz */

    /**
     ** PREINITIALIZATION
     **/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;

    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);

    diminuto_log_setmask();

    DIMINUTO_LOG_INFORMATION("Start");

    (void)gethostname(Hostname, sizeof(Hostname));
    Hostname[sizeof(Hostname) - 1] = '\0';

    locale = setlocale(LC_ALL, "");

    diminuto_mux_init(&mux);

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case '1':
            stopbits = 1;
            break;
        case '2':
            stopbits = 2;
            break;
        case '7':
            databits = 7;
            break;
        case '8':
            databits = 8;
            break;
        case 'B':
            io_size = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (io_size < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
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
        case 'F':
            report = !0;
            slow = !0;
            break;
        case 'G':
            datagram_option = optarg;
            rc = diminuto_ipc_endpoint(optarg, &datagram_endpoint);
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'H':
            report = !0;
            headless = optarg;
            break;
        case 'I':
            pps = optarg;
            ppspin = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (ppspin < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'K':
            readonly = 0;
            direction = OUTPUT;
            break;
        case 'L':
            logging = optarg;
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
            assert(command != (struct Command *)0);
            command->acknak = !0;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'V':
            fprintf(stderr, "%s: version com-diag-hazer %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'W':
            readonly = 0;
            command = (struct Command *)malloc(sizeof(struct Command));
            assert(command != (struct Command *)0);
            command->acknak = 0;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'X':
            expire = !0;
            break;
        case 'Y':
            readonly = 0;
            surveyor_option = optarg;
            rc = diminuto_ipc_endpoint(surveyor_option, &surveyor_endpoint);
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'b':
            bitspersecond = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (bitspersecond == 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
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
        case 'g':
            datagram_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'h':
            rtscts = !0;
            break;
        case 'k':
            device_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
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
            strobepin = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (strobepin < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 's':
            xonxoff = !0;
            break;
        case 't':
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0) || (timeout > HAZER_GNSS_SECONDS)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'u':
            unknown = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        case 'y':
            keepalive = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (keepalive < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case '?':
            fprintf(stderr, "usage: %s "
                           "[ -d ] [ -v ] [ -u ] [ -V ] [ -X ] [ -C ] "
                           "[ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] | -S FILE ] [ -B BYTES ]"
                           "[ -t SECONDS ] "
                           "[ -I PIN | -c ] [ -p PIN ] "
                           "[ -U STRING ... ] [ -W STRING ... ] "
                           "[ -R | -E | -F | -H HEADLESS ] "
                           "[ -L LOG ] "
                           "[ -G [ IP:PORT | :PORT [ -g MASK ] ] ] "
                           "[ -Y [ IP:PORT [ -y SECONDS ] | :PORT ] ] "
                           "[ -K [ -k MASK ] ] "
                           "\n", Program);
            fprintf(stderr, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(stderr, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(stderr, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(stderr, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(stderr, "       -B BYTES    Set the input Buffer size to BYTES bytes.\n");
            fprintf(stderr, "       -C          Ignore bad Checksums.\n");
            fprintf(stderr, "       -D DEVICE   Use DEVICE for input or output.\n");
            fprintf(stderr, "       -E          Like -R but use ANSI Escape sequences.\n");
            fprintf(stderr, "       -F          Like -R but reFresh at 1Hz.\n");
            fprintf(stderr, "       -G IP:PORT  Use remote IP and PORT as dataGram sink.\n");
            fprintf(stderr, "       -G PORT     Use local PORT as dataGram source.\n");
            fprintf(stderr, "       -H HEADLESS Like -R but writes each iteration to HEADLESS file.\n");
            fprintf(stderr, "       -I PIN      Take 1PPS from GPIO Input PIN (requires -D).\n");
            fprintf(stderr, "       -K          Write input to DEVICE sinK from datagram source.\n");
            fprintf(stderr, "       -L LOG      Write input to LOG file.\n");
            fprintf(stderr, "       -R          Print a Report on standard output.\n");
            fprintf(stderr, "       -S SOURCE   Use SOURCE file or named pipe for input.\n");
            fprintf(stderr, "       -U STRING   Like -W except expect UBX ACK or NAK response.\n");
            fprintf(stderr, "       -U ''       Exit when this empty UBX STRING is processed.\n");
            fprintf(stderr, "       -V          Print release, Vintage, and revision on standard output.\n");
            fprintf(stderr, "       -W STRING   Collapse STRING, append checksum, Write to DEVICE.\n");
            fprintf(stderr, "       -W ''       Exit when this empty Write STRING is processed.\n");
            fprintf(stderr, "       -X          Enable message eXpiration test mode.\n");
            fprintf(stderr, "       -Y IP:PORT  Use remote IP and PORT as keepalive sink and surveYor source.\n");
            fprintf(stderr, "       -Y PORT     Use local PORT as surveYor source.\n");
            fprintf(stderr, "       -b BPS      Use BPS bits per second for DEVICE.\n");
            fprintf(stderr, "       -c          Take 1PPS from DCD (requires -D and implies -m).\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
            fprintf(stderr, "       -e          Use Even parity for DEVICE.\n");
            fprintf(stderr, "       -g MASK     Set dataGram sink mask (NMEA=%u, UBX=%u, RTCM=%u) default NMEA.\n", NMEA, UBX, RTCM);
            fprintf(stderr, "       -h          Use RTS/CTS Hardware flow control for DEVICE.\n");
            fprintf(stderr, "       -k MASK     Set device sinK mask (NMEA=%u, UBX=%u, RTCM=%u) default NMEA.\n", NMEA, UBX, RTCM);
            fprintf(stderr, "       -l          Use Local control for DEVICE.\n");
            fprintf(stderr, "       -m          Use Modem control for DEVICE.\n");
            fprintf(stderr, "       -o          Use Odd parity for DEVICE.\n");
            fprintf(stderr, "       -p PIN      Assert GPIO outPut PIN with 1PPS (requires -D and -I or -c).\n");
            fprintf(stderr, "       -n          Use No parity for DEVICE.\n");
            fprintf(stderr, "       -s          Use XON/XOFF (control-Q/control-S) for DEVICE.\n");
            fprintf(stderr, "       -t SECONDS  Timeout GNSS data after SECONDS seconds.\n");
            fprintf(stderr, "       -u          Note Unprocessed input on standard error.\n");
            fprintf(stderr, "       -v          Display Verbose output on standard error.\n");
            fprintf(stderr, "       -y SECONDS  Send surveYor a keep alive every SECONDS seconds.\n");
            return 1;
            break;
        }
    }

    if (error) {
        return 1;
    }

    /**
     ** INITIALIZATION
     **/

    /*
     * Are we logging every valid sentence or packet to an output file?
     */

    if (logging == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(logging, "-") == 0) {
        log_fp = stdout;
    } else {
        log_fp = fopen(logging, "ab");
        if (log_fp == (FILE *)0) { diminuto_perror(logging); }
        assert(log_fp != (FILE *)0);
    }

    /*
     * Are we consuming GPS data from an IP port, or producing GPS data to an
     * IP host and port? This feature is useful for forwarding data from a
     * mobile receiver to a stationary server, for example a vehicle tracking
     * application, or an unattended survey unit in the field that is monitored
     * remotely.
     */

    if (datagram_option == (const char *)0) {
        /* Do nothing. */
    } else if (datagram_endpoint.udp == 0) {
        /* Do nothing. */
    } else if (!diminuto_ipc6_is_unspecified(&datagram_endpoint.ipv6)) {

        datagram_protocol = IPV6;

        datagram_fd = diminuto_ipc6_datagram_peer(0);
        assert(datagram_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(datagram_fd, !0);
        assert(rc >= 0);

        role = PRODUCER;

    } else if (!diminuto_ipc4_is_unspecified(&datagram_endpoint.ipv4)) {

        datagram_protocol = IPV4;

        datagram_fd = diminuto_ipc4_datagram_peer(0);
        assert(datagram_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(datagram_fd, !0);
        assert(rc >= 0);

        role = PRODUCER;

    } else {

        datagram_fd = diminuto_ipc6_datagram_peer(datagram_endpoint.udp);
        assert(datagram_fd >= 0);

        rc = diminuto_mux_register_read(&mux, datagram_fd);
        assert(rc >= 0);

        role = CONSUMER;

    }

    if (datagram_fd >= 0) { DIMINUTO_LOG_INFORMATION("Datagram (%d) IPv%d \"%s\" %s:%d", datagram_fd, datagram_protocol, datagram_option, (datagram_protocol == IPV6) ? diminuto_ipc6_address2string(datagram_endpoint.ipv6, ipv6, sizeof(ipv6)) : (datagram_protocol == IPV4) ? diminuto_ipc4_address2string(datagram_endpoint.ipv4, ipv4, sizeof(ipv4)) : "", datagram_endpoint.udp); }

    /*
     * Are we receiving RTK corrections in the form of RTCM messages from a
     * stationary base station doing a survey? This is useful for DGNSS (DGPS),
     * which can achieve a very high degree of precision (centimeters, or even
     * less). If an optional host or address is also specified, then we are
     * presumably sending keepalives too. Note that it is possible that a
     * DNS resolved a FQDN to both an IPv6 and an IPv4 address, which is why
     * we check the IPv6 form - our preferred form - first.
     */

    if (surveyor_option == (const char *)0) {
        /* Do nothing. */
    } else if (surveyor_endpoint.udp == 0) {
        /* Do nothing. */
    } else if (!diminuto_ipc6_is_unspecified(&surveyor_endpoint.ipv6)) {

        /*
         * Sending keepalives and receiving updates via IPv6.
         */

        surveyor_protocol = IPV6;

        surveyor_fd = diminuto_ipc6_datagram_peer(0);
        assert(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        assert(rc >= 0);

    } else if (!diminuto_ipc4_is_unspecified(&surveyor_endpoint.ipv4)) {

        /*
         * Sending keepalives and receiving updates via IPv4.
         */

        surveyor_protocol = IPV4;

        surveyor_fd = diminuto_ipc4_datagram_peer(0);
        assert(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        assert(rc >= 0);

    } else {

        /*
         * Receiving updates passively via IPv6 with keepalives disabled.
         */

        surveyor_fd = diminuto_ipc6_datagram_peer(surveyor_endpoint.udp);
        assert(surveyor_fd >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        assert(rc >= 0);

        keepalive = -1;

    }

    if (surveyor_fd >= 0) { DIMINUTO_LOG_INFORMATION("Surveyor (%d) IPv%d \"%s\" %s:%d", surveyor_fd, surveyor_protocol, surveyor_option, (surveyor_protocol == IPV6) ? diminuto_ipc6_address2string(surveyor_endpoint.ipv6, ipv6, sizeof(ipv6)) : (surveyor_protocol == IPV4) ? diminuto_ipc4_address2string(surveyor_endpoint.ipv4, ipv4, sizeof(ipv4)) : "", surveyor_endpoint.udp); }

    /*
     * Are we strobing a GPIO pin with the one pulse per second (1PPS)
     * indication we receive via either another GPIO pin or Data Carrier
     * Detect (DCD) on the serial line? This is useful for passing 1PPS
     * along to another application or device.
     */

    if (strobe != (const char *)0) {

        strobe_fp = diminuto_pin_output(strobepin);
        assert(strobe_fp != (FILE *)0);

        rc = diminuto_pin_clear(strobe_fp);
        assert(rc >= 0);

    }

    /*
     * Are we monitoring 1PPS from a General Purpose Input/Output pin?
     * A thread polls the pin until it has changed. The GPIO output of the
     * USB-Port-GPS doesn't appear to correlate with its serial output in any
     * way, nor is polling it when we do character I/O sufficient. So it's
     * interrogated in a separate thread. This is useful for GPS-disciplined
     * clocks using a receiver that has a separate 1PPS digital output pin.
     */

    if (pps != (const char *)0) {

        rc = diminuto_pin_export(ppspin);
        assert(rc >= 0);

        rc = diminuto_pin_direction(ppspin, 0);
        assert(rc >= 0);

        rc = diminuto_pin_active(ppspin, !0);
        assert(rc >= 0);

        rc = diminuto_pin_edge(ppspin, DIMINUTO_PIN_EDGE_BOTH);
        assert(rc >= 0);

        pps_fp = diminuto_pin_open(ppspin);
        assert(pps_fp != (FILE *)0);

        rc = diminuto_pin_get(pps_fp);
        assert(rc >= 0);

        poller.ppsfp = pps_fp;
        poller.strobefp = strobe_fp;
        poller.onepps = 0;
        poller.done = 0;

        pthreadrc = pthread_create(&thread, 0, gpiopoller, &poller);
        if (pthreadrc != 0) {
            errno = pthreadrc;
            diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);

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

        dev_fd = open(device, readonly ? O_RDONLY : O_RDWR);
        if (dev_fd < 0) { diminuto_perror(device); }
        assert(dev_fd >= 0);

        rc = diminuto_serial_set(dev_fd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
        assert(rc == 0);

        rc = diminuto_serial_raw(dev_fd);
        assert(rc == 0);

        dev_fp = fdopen(dev_fd, readonly ? "r" : "a+");
        if (dev_fp == (FILE *)0) { diminuto_perror(device); }
        assert(dev_fp != (FILE *)0);

        /*
         * Note that we set our input file pointer provisionally; we may
         * change it below.
         */

        in_fp = dev_fp;

    }

    /*
     * If we are using some other source of input (e.g. a file, a FIFO, etc.),
     * open it here.
     */

    if (source == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(source, "-") == 0) {
        in_fp = stdin;
    } else {
        in_fp = fopen(source, "r");
        if (in_fp == (FILE *)0) { diminuto_perror(source); }
        assert(in_fp != (FILE *)0);
    }

    /*
     * Our input source is either standard input (either implicitly or
     * explicitly), a serial(ish) device, or a file or maybe a FIFO
     * a.k.a. a named pipe, remarkably useful BTW, see mkfifo(1). So
     * now we can get its underlying file descriptor. We also mess around
     * with the input stream standard I/O buffer.
     */

    in_fd = fileno(in_fp);

    rc = diminuto_mux_register_read(&mux, in_fd);
    assert(rc >= 0);

    io_buffer = malloc(io_size);
    assert(io_buffer != (void *)0);
    setvbuf(in_fp, io_buffer, _IOFBF, io_size);

    /*
     * If we are running headless, create our temporary output file using the
     * provided prefix.
     */

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_create(headless, &temporary);
        assert(out_fp != (FILE *)0);
    }

    /*
     * Are we monitoring 1PPS via Data Carrier Detect (DCD) on a serial line?
     * A thread blocks until it is asserted. The GR-701W asserts DCD just
     * before it unloads a block of sentences. The leading edge of DCD
     * indicates 1PPS. We interrogate DCD in a separate thread to decouple
     * it from our serial input. This is useful for GPS-disciplined
     * clocks using any receiver that toggles DCD on its serial port to
     * indicate 1PPS.
     */

    if (dev_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (!modemcontrol) {
        /* Do nothing. */
    } else if (!carrierdetect) {
        /* Do nothing. */
    } else {

        poller.ppsfp = dev_fp;
        poller.strobefp = strobe_fp;
        poller.onepps = 0;
        poller.done = 0;

        pthreadrc = pthread_create(&thread, 0, dcdpoller, &poller);
        if (pthreadrc != 0) {
            errno = pthreadrc;
            diminuto_perror("pthread_create");
        }
        assert(pthreadrc == 0);

    }

    /*
     * Install our signal handlers.
     */

    rc = diminuto_terminator_install(0);
    assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    assert(rc >= 0);

    /*
     * Initialize our time zone. The underlying tzset(3) call is relatively
     * expensive (it accesses the file system). But at least some
     * implementations memoize (a.k.a. cache) the information gleaned from
     * the file system and from the environment. So we'll call it here to
     * do that so when print_local() calls it, it doesn't introduce a bunch
     * of latency while we're processing the NMEA stream. IMPORTANT TIP: if
     * your Hazer application is in a system that routinely crosses (perhaps
     * many) time zones - as at least four of the aircraft-based products I've
     * worked on do - or if your application is stationary but distributed
     * (perhaps internationally) across time zones - as one of the enterprise
     * telecommunications sytems I've worked on can be - consider setting the
     * time zone of your system to UTC. If nothing else, your field support
     * people may thank you.
     */

    (void)diminuto_time_timezone(diminuto_time_clock());

    /*
     * How much of each packet do we display? Depends on whether we're doing
     * cursor control or not.
     */

    limitation = escape ? LIMIT : UNLIMITED;

    /*
     * Initialize screen.
     */

    if (escape) {
        fputs("\033[1;1H\033[0J", out_fp);
        if (report) {
            fprintf(out_fp, "INP [%3d]\n", 0);
            fprintf(out_fp, "OUT [%3d]\n", 0);
            print_local(out_fp, timetofirstfix);
            fflush(out_fp);
        }
    }

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

    rc = tumbleweed_initialize();
    assert(rc == 0);

    if (debug) {
        hazer_debug(stderr);
        yodel_debug(stderr);
        tumbleweed_debug(stderr);
    }

    /*
     * Start the clock.
     */

    display_now = display_was = expiration_now = expiration_was = keepalive_now = keepalive_was = (epoch = diminuto_time_elapsed()) / (frequency = diminuto_frequency());

    /*
     * Initialize all state machines to attempt synchronization with the
     * input stream.
     */

    nmea_state = HAZER_STATE_START;
    ubx_state = YODEL_STATE_START;
    rtcm_state = TUMBLEWEED_STATE_START;

    sync = 0;

    frame = 0;

    /*
     * Enter the work loop.
     */


	while (!0) {

	    /*
	     * We keep working until out input goes away (end of file), or until
	     * we are interrupted by a SIGINT or terminated by a SIGTERM. We
	     * also check for SIGHUP, which I might use for something in the
	     * future.
	     */

		if (diminuto_terminator_check()) {
			break;
		}

		if (diminuto_interrupter_check()) {
			break;
		}

		if (diminuto_hangup_check()) {
			/* Do nothing. */
		}

		/**
         ** INPUT
         **/

		/*
		 * We keep looking for input from one of our sources until one of them
		 * tells us we have a buffer to process. It could be a NMEA sentence,
		 * a UBX packet, or an RTCM message. It is also possible that the
		 * select(2) timed out, and no file descriptor will be returned, in
		 * which case we have other work to do further below. Or it may be
		 * that the select(2) was interrupted, so we need to interrogate our
		 * signal handlers.
		 */

		if ((io_available = diminuto_file_ready(in_fp)) > 0) {
			fd = in_fd;
		} else if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
			/* Do nothing. */
		} else if ((ready = diminuto_mux_wait(&mux, 0 /* frequency */)) == 0) {
			fd = -1;
		} else if (ready > 0) {
			fd = diminuto_mux_ready_read(&mux);
		} else if (errno == EINTR) {
			continue;
		} else {
			assert(0);
		}

		buffer = (uint8_t *)0;

		if (fd < 0) {

			/*
			 * No input, so do something else.
			 */

		} else if (fd == in_fd) {

			/*
			 * Consume bytes of NMEA, UBX, or RTCM from the input stream until
			 * the current input stream buffer is empty or until a complete
			 * buffer is assembled.
			 */

			do {

				if (io_available > io_maximum) { io_maximum = io_available; }

				ch = fgetc(in_fp);
				if (ch == EOF) {
		        	DIMINUTO_LOG_INFORMATION("EOF");
					eof = !0;
					break;
				}


				/*
				 * We just received a character from the input stream.
				 * If we're synchronized (most recently received a complete
				 * and valid NMEA sentence, UBX packet, or RTCM message), and
				 * are at the beginning of a new sentence, packet, or message,
				 * then we will guess what the next format will be based on
				 * this one character and only activate the state machine
				 * that we need. If we don't recognize that character, then
				 * we're lost synchronization and need to reestablish it.
				 */

				if (!sync) {

					/* Do nothing. */

				} else if (!frame) {

					/* Do nothing. */

				} else if ((ch == HAZER_STIMULUS_START) || (ch == HAZER_STIMULUS_ENCAPSULATION)) {

					frame = 0;

					nmea_state = HAZER_STATE_START;
					ubx_state = YODEL_STATE_STOP;
					rtcm_state = TUMBLEWEED_STATE_STOP;

				} else if (ch == YODEL_STIMULUS_SYNC_1) {

					frame = 0;

					nmea_state = HAZER_STATE_STOP;
					ubx_state = YODEL_STATE_START;
					rtcm_state = TUMBLEWEED_STATE_STOP;

				} else if (ch == TUMBLEWEED_STIMULUS_PREAMBLE) {

					frame = 0;

					nmea_state = HAZER_STATE_STOP;
					ubx_state = YODEL_STATE_STOP;
					rtcm_state = TUMBLEWEED_STATE_START;

				} else {

					DIMINUTO_LOG_WARNING("Sync Lost 0x%02x\n", ch);

					sync = 0;
					frame = 0;

					nmea_state = HAZER_STATE_START;
					ubx_state = YODEL_STATE_START;
					rtcm_state = TUMBLEWEED_STATE_START;

				}

				nmea_state = hazer_machine(nmea_state, ch, nmea_buffer, sizeof(nmea_buffer), &nmea_context);
				if (nmea_state == HAZER_STATE_END) {
					buffer = nmea_buffer;
					size = hazer_size(&nmea_context);
					length = size - 1;
					format = NMEA;
					if (!sync) {
						DIMINUTO_LOG_NOTICE("Sync NMEA\n");
						sync = !0;
					}
					frame = !0;
					break;
				}

				ubx_state = yodel_machine(ubx_state, ch, ubx_buffer, sizeof(ubx_buffer), &ubx_context);
				if (ubx_state == YODEL_STATE_END) {
					buffer = ubx_buffer;
					size = yodel_size(&ubx_context);
					length = size - 1;
					format = UBX;
					if (!sync) {
						DIMINUTO_LOG_NOTICE("Sync UBX\n");
						sync = !0;
					}
					frame = !0;
					break;
				}

				rtcm_state = tumbleweed_machine(rtcm_state, ch, rtcm_buffer, sizeof(rtcm_buffer), &rtcm_context);
				if (rtcm_state == TUMBLEWEED_STATE_END) {
					buffer = rtcm_buffer;
					size = tumbleweed_size(&rtcm_context);
					length = size - 1;
					format = RTCM;
					if (!sync) {
						DIMINUTO_LOG_NOTICE("Sync RTCM\n");
						sync = !0;
					}
					frame = !0;
					break;
				 }

				/*
				 * If all the state machines have stopped, then either we have
				 * never had synchronization, or we lost synchronization.
				 * Restart all of them.
				 */

				if (nmea_state != HAZER_STATE_STOP) {
					/* Do nothing. */
				} else if (ubx_state != YODEL_STATE_STOP) {
					/* Do nothing. */
				} else if (rtcm_state != TUMBLEWEED_STATE_STOP) {
					/* Do nothing. */
				} else {
					if (sync) {
						DIMINUTO_LOG_WARNING("Sync Stop\n");
						sync = 0;
					}
				    frame = 0;
				    nmea_state = HAZER_STATE_START;
				    ubx_state = YODEL_STATE_START;
				    rtcm_state = TUMBLEWEED_STATE_START;
				}

			} while ((io_available = diminuto_file_ready(in_fp)) > 0);

			/*
			 * If we detected End Of File from our input source, we're
			 * done.
			 */

			if (eof) {
				break;
			}

		} else if (fd == datagram_fd) {

			/*
			 * Receive a NMEA, UBX, or RTCM datagram from a remote gpstool.
			 * We make a rule that the datagram must be a complete NMEA
			 * sentence, UBX packet, or RTCM message, complete with a valid
			 * checksum or cyclic redundancy check, with no extra leading or
			 * trailing bytes. If we do receive an invalid datagram, that
			 * is a serious bug either in this software or in the transport.
			 */

			if ((datagram_size = receive_datagram(datagram_fd, datagram_buffer, sizeof(datagram_buffer))) <= 0) {

				/* Do nothing. */

			} else if ((datagram_length = hazer_validate(datagram_buffer, datagram_size)) > 0) {

				buffer = datagram_buffer;
				size = datagram_size;
				length = datagram_length;
				format = NMEA;
				break;

			} else if ((datagram_length = yodel_validate(datagram_buffer, datagram_size)) > 0) {

				buffer = datagram_buffer;
				size = datagram_size;
				length = datagram_length;
				format = UBX;
				break;

			} else if ((datagram_length = tumbleweed_validate(datagram_buffer, datagram_size)) > 0) {

				buffer = datagram_buffer;
				size = datagram_size;
				length = datagram_length;
				format = RTCM;
				break;

			} else {

				DIMINUTO_LOG_WARNING("Remote (%d) [%zd] [%zd] 0x%02x\n", datagram_fd, datagram_size, datagram_length, datagram_buffer[0]);

			}

		} else if (fd == surveyor_fd) {

			/*
			 * Receive an RTCM message from a remote gpstool doing a survey.
			 */

			if ((surveyor_size = receive_datagram(surveyor_fd, surveyor_buffer, sizeof(surveyor_buffer))) <= 0) {

				/* Do nothing. */

			} else if ((surveyor_length = tumbleweed_validate(surveyor_buffer, surveyor_size)) <= 0) {

				DIMINUTO_LOG_WARNING("Surveyor (%d) [%zd] [%zd] 0x%02x\n", surveyor_fd, surveyor_size, surveyor_length, surveyor_buffer[0]);

			} else if (dev_fp == (FILE *)0) {

				/* Do nothing. */

			} else {

				if (verbose) { fprintf(stderr, "%s: RTCM <%d> [%zd]\n", Program, tumbleweed_message(surveyor_buffer, surveyor_length), surveyor_length); }
				write_buffer(dev_fp, surveyor_buffer, surveyor_length);

			}

		} else {

			/*
			 * The select(2) system call returned a file descriptor which
			 * was not one we know about; that should be impossible.
			 */

			DIMINUTO_LOG_ERROR("Multiplexing %d ( %d %d %d )\n", fd, dev_fd, datagram_fd, surveyor_fd);
			assert(0);

		}

        /*
         * If one of the state machines indicated end of file, we're done.
         */

        if (eof) {
            break;
        }

        /**
         ** KEEPALIVE
         **/

        /*
         * If our keep alive interval has expired, send a keep alive
         * (an RTCM message with a zero-length payload) to the surveyor. This
         * is necessary to establish and maintain the return path for datagram
         * streams that go through NATting firewalls. The surveyor we are
         * talking to probably isn't another gpstool; it's an rtktool that has
         * a static address, or at least a dynamic DNS (DDNS) address, and which
         * handles the routing of RTK updates from the stationary base station
         * in survey mode and one or more mobile rovers. I borrowed this
         * technique from SIP, where VoIP phones issue keepalives to PBXen like
         * Asterisk every twenty-five seconds, under the assumption that a
         * typical firewall UDP "connection" timeout is thirty seconds.
         */

        if (surveyor_fd < 0) {
            /* Do nothing. */
        } else if (keepalive < 0) {
            /* Do nothing. */
        } else if (((keepalive_now = ticktock(frequency)) - keepalive_was) < keepalive) {
            /* Do nothing. */
        } else {
            send_datagram(surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv4, &surveyor_endpoint.ipv6, surveyor_endpoint.udp, TUMBLEWEED_KEEPALIVE, sizeof(TUMBLEWEED_KEEPALIVE));
            keepalive_was = keepalive_now;
        }

        /**
         ** CONFIGURATION
         **/

        /*
         * If we have any initialization strings to send, and we have a device,
         * do so one at a time. Because
         * this queue of writes is checked every time we reiterate in the
         * work loop, later code can enqueue new commands to be written
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

        if (dev_fp == (FILE *)0) {
            /* Do nothing. */
        } else if (acknakpending > 0) {
            /* Do nothing. */
        } else if (diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else {
            command_node = diminuto_list_dequeue(&command_list);
            assert(command_node != (diminuto_list_t *)0);
            command = diminuto_containerof(struct Command, link, command_node);
            command_payload = diminuto_list_data(command_node);
            assert(command_payload != (uint8_t *)0);
            if (command_payload[0] == '\0') {
                DIMINUTO_LOG_INFORMATION("Zero");
                free(command_node);
                eof = !0;
            } else {
                command_size = strlen(command_payload) + 1;
                command_length = diminuto_escape_collapse(command_payload, command_payload, command_size);
                if (command_payload[0] == HAZER_STIMULUS_START) {
                    emit_sentence(dev_fp, command_payload, command_length);
                    rc = 0;
                } else if ((command_payload[0] == YODEL_STIMULUS_SYNC_1) && (command_payload[1] == YODEL_STIMULUS_SYNC_2)) {
                    emit_packet(dev_fp, command_payload, command_length);
                    rc = 0;
                } else {
                	DIMINUTO_LOG_WARNING("Command 0x%02x%02x [%zd]", command_payload[0], command_payload[1], command_length);
                    rc = -1;
                }
                if (rc == 0) {
                    if (command->acknak) { acknakpending += 1; }
                    if (verbose) { print_buffer(stderr, command_payload, command_length, UNLIMITED); }
                    if (escape) { fputs("\033[2;1H\033[0K", out_fp); }
                    if (report) { fprintf(out_fp, "OUT [%3zd] ", command_length); print_buffer(out_fp, command_payload, command_length, limitation); fflush(out_fp); }
                }
                free(command_node);
            }
        }

        /*
         * If we don't have a buffer to consume, keep trying.
         */

        if (buffer == (uint8_t *)0) {
            continue;
        }

        /*
         * At this point:
         *
         * format indicates NMEA, UBX, or RTCM;
         *
         * buffer points to a buffer containing an NMEA sentence, a UBX packet,
         * or an RTCM message, with a valid checksum or CRC;
         *
         * size is the size of the data in the buffer in bytes including the
         * trailing NUL (which is added even to buffers containing binary
         * UBX or RTCM data).
         *
         * length is the number of bytes in the buffer as determined by the
         * format-specific validation and is typically based on the a value
         * extracted from the data in the buffer. Unless the format requires
         * it (none currently do), it does not include the trailing NUL.
         */

        if (verbose) { print_buffer(stderr, buffer, length, UNLIMITED); }
        if (escape) { fputs("\033[1;1H\033[0K", out_fp); }
        if (report) { fprintf(out_fp, "INP [%3zd] ", length); print_buffer(out_fp, buffer, length, limitation); fflush(out_fp); }

        /**
         ** FORWARD
         **/

        /*
         * We forward anything whose format is enabled in the forwarding
         * mask. Note that we don't forward the terminating NUL (using length
         * instead of size) that terminate all input of any format (whether
         * that's useful or not). This is kinda iffy since UDP can not only
         * drop datagrams, but reorder them. But the ensured delivery of TCP
         * can (and has, in testing over LTE networks) add substantial latency
         * to the data. Sometimes it is truly "better never than late".
         */

        if (datagram_fd < 0) {
            /* Do nothing. */
        } else if (role != PRODUCER) {
            /* Do nothing. */
        } else if ((datagram_mask & format) == 0) {
            /* Do nothing. */
        } else {
            send_datagram(datagram_fd, datagram_protocol, &datagram_endpoint.ipv4, &datagram_endpoint.ipv6, datagram_endpoint.udp, buffer, length);
        }

        /**
         ** WRITE
         **/

        /*
         * We write the validated input to the device in the case in which
         * we received the original data via UDP or from standard input; in
         * other cases the device is our input source. Time must monotonically
         * increase (UDP can reorder packets), and we have to have gotten an
         * RMC sentence to set the date before we pass the data along; doing
         * anything else confuses Google Earth, and perhaps other applications.
         */

        if (dev_fp == (FILE *)0) {
            /* Do nothing. */
        } else if (direction != OUTPUT) {
            /* Do nothing. */
        } else if ((device_mask & format) == 0) {
            /* Do nothing. */
        } else if (!dmyokay) {
            /* Do nothing. */
        } else if (!totokay) {
            /* Do nothing. */
        } else {
            write_buffer(dev_fp, buffer, length);
        }

        /**
         ** LOG
         **/

        if (log_fp != (FILE *)0) {
            write_buffer(log_fp, buffer, length);
        }

        /**
         ** EXPIRE
         **/

        /*
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

        expiration_was = expiration_now;
        expiration_now = ticktock(frequency);
        elapsed = (expiration_now > expiration_was) ? expiration_now - expiration_was : 0;

        if (elapsed > 0) {
        	int ii;

            for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                countdown(&position[ii].ticks, elapsed);
                countdown(&active[ii].ticks, elapsed);
                countdown(&view[ii].ticks, elapsed);
            }

            countdown(&solution.ticks, elapsed);
            countdown(&hardware.ticks, elapsed);
            countdown(&status.ticks, elapsed);
            countdown(&base.ticks, elapsed);
            countdown(&rover.ticks, elapsed);
            countdown(&kinematics.ticks, elapsed);

        }

        /**
         ** PROCESS
         **/

        switch (format) {

        case NMEA:

            /*
             * NMEA SENTENCES
             */

            /*
             * We tokenize the a copy of the NMEA sentence so we can parse it.
             * We make a copy because the tokenization modifies the body
             * of the sentence in place and we may want to display the original
             * sentence later.
             */

        	strncpy(tokenized, buffer, sizeof(tokenized));
        	tokenized[sizeof(tokenized) - 1] = '\0';
            count = hazer_tokenize(vector, countof(vector), tokenized, length);
            assert(count >= 0);
            assert(vector[count - 1] == (char *)0);
            assert(count <= countof(vector));

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
                    DIMINUTO_LOG_WARNING("Talker \"%c%c\"", vector[0][1], vector[0][2]);
                }

                continue;

            } else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {

                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    DIMINUTO_LOG_WARNING("Constellation \"%c%c\"\n", vector[0][1], vector[0][2]);
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

                DIMINUTO_LOG_INFORMATION("TXT \"%*s\"", length - 2 /* Exclude CR and LF. */, buffer);

            } else if (unknown) {

                DIMINUTO_LOG_WARNING("NMEA \"%s\"\n", vector[0]);

            } else {

                /* Do nothing. */

            }

            break;

        case UBX:

            /*
             * UBX PACKETS
             */

            if (verbose) { diminuto_dump(stderr, buffer, length); }

            if (yodel_ubx_nav_hpposllh(&(solution.payload), buffer, length) == 0) {

                solution.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_mon_hw(&(hardware.payload), buffer, length) == 0) {

                hardware.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_nav_status(&(status.payload), buffer, length) == 0) {

                status.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_ack(&acknak, buffer, length) == 0) {

                refresh = !0;

                DIMINUTO_LOG_INFORMATION("UBX %s 0x%02x 0x%02x (%d)\n", acknak.state ? "ACK" : "NAK", acknak.clsID, acknak.msgID, acknakpending);

                if (acknakpending > 0) { acknakpending -= 1; }

            } else if (yodel_ubx_cfg_valget(buffer, length) == 0) {

                /*
                 * All of the validity checking and byte swapping is done in
                 * yodel_ubx_cfg_valget(). The parse function doesn't accept
                 * the message unless it checks out. This is also why the
                 * buffer is passed as non-const; the variable length payload
                 * is byteswapped in-place.
                 */

                yodel_ubx_cfg_valget_t * pp = (yodel_ubx_cfg_valget_t *)&(buffer[YODEL_UBX_PAYLOAD]);
                const char * bb = (const char *)0;
                const char * ee = &buffer[length - YODEL_UBX_CHECKSUM];
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
                        DIMINUTO_LOG_INFORMATION("UBX CFG VALGET v%d %s [%d] 0x%08x 0x%01x\n", pp->version, layer, ii, kk, vv1);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_ONE:
                        memcpy(&vv1, bb, sizeof(vv1));
                        DIMINUTO_LOG_INFORMATION("UBX CFG VALGET v%d %s [%d] 0x%08x 0x%02x\n", pp->version, layer, ii, kk, vv1);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_TWO:
                        memcpy(&vv16, bb, sizeof(vv16));
                        DIMINUTO_LOG_INFORMATION("UBX CFG VALGET v%d %s [%d] 0x%08x 0x%04x\n", pp->version, layer, ii, kk, vv16);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_FOUR:
                        memcpy(&vv32, bb, sizeof(vv32));
                        DIMINUTO_LOG_INFORMATION("UBX CFG VALGET v%d %s [%d] 0x%08x 0x%08x\n", pp->version,layer, ii, kk, vv32);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_EIGHT:
                        memcpy(&vv64, bb, sizeof(vv64));
                        DIMINUTO_LOG_INFORMATION("UBX CFG VALGET v%d %s [%d] 0x%08x 0x%016llx\n", pp->version, layer, ii, kk, (unsigned long long)vv64);
                        break;
                    }

                    ++ii;

                }

            } else if (yodel_ubx_mon_ver(buffer, length) == 0) {

                const char * bb = &buffer[YODEL_UBX_PAYLOAD];
                const char * ee = &buffer[length - YODEL_UBX_CHECKSUM];

                refresh = !0;

                do {

                    if (bb >= ee) { break; }
                    DIMINUTO_LOG_INFORMATION("UBX MON VER SW \"%s\"\n", bb);
                    bb += YODEL_UBX_MON_VER_swVersion_LENGTH;

                    if (bb >= ee) { break; }
                    DIMINUTO_LOG_INFORMATION("UBX MON VER HW \"%s\"\n", bb);
                    bb += YODEL_UBX_MON_VER_hwVersion_LENGTH;

                    while (bb < ee) {
                        DIMINUTO_LOG_INFORMATION("UBX MON VER EX \"%s\"\n", bb);
                        bb += YODEL_UBX_MON_VER_extension_LENGTH;
                    }

                } while (0);

            } else if (yodel_ubx_nav_svin(&base.payload, buffer, length) == 0) {

                base.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_rxm_rtcm(&rover.payload, buffer, length) == 0) {

                rover.ticks = timeout;
                refresh = !0;

            } else if (unknown) {

                DIMINUTO_LOG_WARNING("UBX 0x%02x%02x%02x%02x\n", buffer[YODEL_UBX_SYNC_1], buffer[YODEL_UBX_SYNC_2], buffer[YODEL_UBX_CLASS], buffer[YODEL_UBX_ID]);

            } else {

                /* Do nothing. */

            }

            break;

        case RTCM:

            /*
             * RTCM MESSAGES
             */

            if (verbose) { diminuto_dump(stderr, buffer, length); }

            kinematics.number = tumbleweed_message(buffer, length);
            if (kinematics.number < 0) { kinematics.number = 9999; }
            kinematics.length = length;
            if (length < kinematics.minimum) { kinematics.minimum = length; }
            if (length > kinematics.maximum) { kinematics.maximum = length; }

            if (verbose) { fprintf(stderr, "%s: RTCM <%d> [%zd] [%zd] [%zd]\n", Program, kinematics.number, kinematics.minimum, kinematics.length, kinematics.maximum); }

            kinematics.ticks = timeout;
            refresh = !0;

            break;

        case FORMAT:

            /* Do nothing. */

            break;

        }

        /*
         * Calculate our time to first fix.
         */

        if (fix < 0) {
            /* Do nothing. */
        } else if (timetofirstfix >= 0) {
            /* Do nothing. */
        } else {
            timetofirstfix = fix - epoch;
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
            int ii;

            if (crowbar <= 0) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    position[ii].ticks = 0;
                }
            }
            if (crowbar <= 100) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    active[ii].ticks = 0;
                 }
            }
            if (crowbar <= 200) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    view[ii].ticks = 0;
                }
            }
            if (crowbar <= 300) {
                hardware.ticks = 0;
            }
            if (crowbar <= 400) {
                status.ticks = 0;
            }
            if (crowbar <= 500) {
                base.ticks = 0;
            }
            if (crowbar <= 600) {
                rover.ticks = 0;
            }
            if (crowbar <= 700) {
                kinematics.ticks = 0;
            }
            if (crowbar > 0) {
                crowbar -= 1;
            }
        }

        /*
         * DISPLAY
         */

        if (!refresh) {
            /* Do nothing: nothing changed. */
        } else if ((dev_fp != (FILE *)0) && (diminuto_serial_available(fileno(dev_fp)) > 0)) {
            /* Do nothing: we still have real-time input waiting. */
        } else if (slow && (display_was == (display_now = ticktock(frequency)))) {
            /* Do nothing: slow display cannot handle real-time refresh rate. */
        } else {

            if (escape) { fputs("\033[3;1H", out_fp); }
            if (report) {
                DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
                    onepps = poller.onepps;
                    poller.onepps = 0;
                DIMINUTO_CRITICAL_SECTION_END;
                print_hardware(out_fp, &hardware);
                print_status(out_fp, &status);
                print_local(out_fp, timetofirstfix);
                print_positions(out_fp, position, onepps, dmyokay, totokay);
                print_solution(out_fp, &solution);
                print_corrections(out_fp, &base, &rover, &kinematics);
                print_actives(out_fp, active);
                print_views(out_fp, view, active);
            }
            if (escape) { fputs("\033[0J", out_fp); }
            if (report) { fflush(out_fp); }

            /*
             * If we're running headless, commit this observation to the
             * file system and start a new observation in a temporary file.
             */

            if (headless != (const char *)0) {
                out_fp = diminuto_observation_commit(out_fp, &temporary);
                assert(out_fp == (FILE *)0);
                out_fp = diminuto_observation_create(headless, &temporary);
                assert(out_fp != (FILE *)0);
            }

            display_was = display_now;

            refresh = 0;
        }

    }

    /**
     ** FINIALIZATION
     **/

	DIMINUTO_LOG_INFORMATION("End");

    rc = tumbleweed_finalize();
    assert(rc == 0);

    rc = yodel_finalize();
    assert(rc == 0);

    rc = hazer_finalize();
    assert(rc == 0);

    diminuto_mux_fini(&mux);

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

    if (pps_fp != (FILE *)0) {
        pps_fp = diminuto_pin_unused(pps_fp, ppspin);
        assert(pps_fp == (FILE *)0);
    }

    if (strobe_fp != (FILE *)0) {
        strobe_fp = diminuto_pin_unused(strobe_fp, strobepin);
        assert(strobe_fp == (FILE *)0);
    }

    if (datagram_fd >= 0) {
        rc = diminuto_ipc_close(datagram_fd);
        assert(rc >= 0);
    }

    if (log_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (log_fp == stdout) {
        /* Do nothing. */
    } else {
        rc = fclose(log_fp);
        assert(rc != EOF);
    }

    rc = fclose(in_fp);
    assert(rc != EOF);

    DIMINUTO_LOG_INFORMATION("Buffer size=%llu maximum=%llu\n", io_size, io_maximum);
    free(io_buffer);

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_commit(out_fp, &temporary);
        assert(out_fp == (FILE *)0);
    } else {
        rc = fclose(out_fp);
        assert(rc != EOF);
    }

    while (!diminuto_list_isempty(&command_list)) {
    	command_node = diminuto_list_dequeue(&command_list);
    	assert(command_node != (diminuto_list_t *)0);
    	free(command_node);
    }

    fflush(stderr);

	DIMINUTO_LOG_INFORMATION("Exit");

    return 0;
}
