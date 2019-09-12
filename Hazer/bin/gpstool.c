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
 *  gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -L save.dat
 *
 *  gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -G localhost:5555
 *
 *  gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -G 128.0.0.1:5555
 *
 *  gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -G [::1]:5555
 *
 *  gpstool -G 5555 -E
 *
 *  gpstool -d -v
 *
 *  gpstool -D /dev/ttyACM0 -b 9600 -8 -n -1 -E -t 10 -W '\$PUBX,40,GSV,0,0,0,1,0,0' -W '\$PUBX,40,VTG,0,0,0,1,0,0'
 *
 *  gpstool -P < input.dat
 *
 *  gpstool -S - -P < input.dat
 *
 *  gpstool -S input.dat -P
 *
 *  gpstool -D /dev/ttyACM0 -b 115200 -8 -n -1 \
 *      -G tumbleweed:21010 -g 4 \
 *      -F -H headless.out -t 10 \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x10\x00\x03\x40\x2c\x01\x00\x00' \
 *      -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x11\x00\x03\x40\x10\x27\x00\x00' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xc0\x02\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x61\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x66\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x70\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x00' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x8b\x00\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
 *      -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01'
 *
 *  gpstool -D /dev/ttyACM0 -b 115200 -8 -n -1 \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x10\x00\x03\x40\x2c\x01\x00\x00' \
 *      -U '\xb5\x62\x06\x8a\x0c\x00\x00\x01\x00\x00\x11\x00\x03\x40\x10\x27\x00\x00' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\xc0\x02\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x61\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x66\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x70\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x06\x03\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x00' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x8b\x00\x91\x20\x01' \
 *      -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
 *      -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
 *      -U ''
 *
 *  gpstool -D /dev/ttyACM0 E 2> >(log -S)
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
#include <wchar.h>
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
#include "com/diag/diminuto/diminuto_daemon.h"

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

/**
 * Return monotonic time in seconds.
 * @param frequency is the underlying clock frequency.
 * @return monotonic time in seconds.
 */
static inline long ticktock(diminuto_sticks_t frequency)
{
    return diminuto_time_elapsed() / frequency;
}

/**
 * Common function to count down the expiration fields in the database.
 * @param ep points to the expiration field to count down.
 * @param elapsed is the number of ticks to count down.
 */
static inline void countdown(hazer_expiry_t * ep, diminuto_sticks_t elapsed)
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
 * TRANSCEIVERS
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
 * Print an NMEA sentence or UBX message to a stream, expanding all
 * characters into hexadecimal escape sequences.
 * @param fp points to the FILE stream.
 * @param ep points to the FILE stream for errors.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 */
static void dump_buffer(FILE * fp, const void * buffer, size_t size)
{
    const unsigned char * bb = (const char *)0;

    for (bb = buffer; size > 0; --size) {
        fprintf(fp, "\\x%2.2x", *(bb++));
    }
    fputc('\n', fp);
}

/**
 * Log connection information.
 * @param label is a label prepended to the output.
 * @param option is the command line endpoint option argument.
 * @param fd is the socket.
 * @param protocol is selects which address to show.
 * @param ipv6p points to an IPv6 address.
 * @param ipv4p points to an IPv4 address.
 * @param port is a port number.
 */
static void show_connection(const char * label, const char * option, int fd, protocol_t protocol, const diminuto_ipv6_t * ipv6p, const diminuto_ipv4_t * ipv4p, diminuto_port_t port)
{
    diminuto_ipv4_buffer_t ipv4;
    diminuto_ipv6_buffer_t ipv6;

    switch (protocol) {

    case IPV6:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\" [%s]:%d", label, fd, option, diminuto_ipc6_address2string(*ipv6p, ipv6, sizeof(ipv6)), port);
        break;

    case IPV4:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\" %s:%d", label, fd, option, diminuto_ipc4_address2string(*ipv4p, ipv4, sizeof(ipv4)), port);
        break;

    case PROTOCOL:
        DIMINUTO_LOG_INFORMATION("%s (%d) \"%s\"", label, fd, option);
        break;

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
 * @return the size of the sent datagram in bytes or <0 if an error occurred.
 */
static ssize_t send_datagram(int fd, protocol_t protocol, const diminuto_ipv4_t * ipv4p, const diminuto_ipv6_t * ipv6p, diminuto_port_t port, const void * buffer, size_t size)
{
    ssize_t length = 0;

    if (size <= 0) {
        /* Do nothing. */
    } else if (protocol == IPV4) {
        length = diminuto_ipc4_datagram_send(fd, buffer, size, *ipv4p, port);
    } else if (protocol == IPV6) {
        length = diminuto_ipc6_datagram_send(fd, buffer, size, *ipv6p, port);
    } else {
        /* Do nothing. */
    }

    return length;
}

/**
 * Receive a datagram from a UDP port. The datagram will be NUL terminated.
 * The provided buffer must be sized one more byte than the received datagram.
 * @param fd is an open socket.
 * @param buffer points to the buffer.
 * @param size is the size of the buffer in bytes.
 * @return the size of the received datagram in bytes or <0 if an error occurred.
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

/**
 * Track RTK updates by encoding each received RTCM message as a single
 * character in a shifting string.
 * @param number is the RTCM message number.
 * @param up points to the updates union.
 */
static void collect_update(int number, tumbleweed_updates_t * up)
{
    update_t update = UPDATE;

    switch (number) {

    case 1005:
        update = RTCM_TYPE_1005;
        break;

    case 1074:
        update = RTCM_TYPE_1074;
        break;

    case 1084:
        update = RTCM_TYPE_1084;
        break;

    case 1094:
        update = RTCM_TYPE_1094;
        break;

    case 1124:
        update = RTCM_TYPE_1124;
        break;

    case 1230:
        update = RTCM_TYPE_1230;
        break;

    case 9999:
        update = RTCM_TYPE_9999;
        break;

    default:
        /* Do nothing. */
        break;

    }

    up->word = (up->word << 8) | update;

}

/**
 * If the caller has passed a valid file name, and the solution is not active
 * yet valid, emit the appropriate UBX messages minus checksums for feeding
 * this solution into this programming running in fixed mode.
 * @param arp points to a Antenna Reference Point file name.
 * @param bp points to a base structure.
 * @param sp points to a solution structure.
 * @return true if the solution was emitted, false otherwise.
 */
static int save_solution(const char * arp, const yodel_base_t * bp, const yodel_solution_t * sp)
{
    int rc = 0;
    FILE * fp = (FILE *)0;
    char * temporary = (char *)0;
    int64_t value = 0;
    int32_t lat = 0;
    int8_t latHp = 0;
    int32_t lon = 0;
    int8_t lonHp = 0;
    int32_t height = 0;
    int8_t heightHp = 0;

    if (bp->ticks == 0) {
        /* Do nothing. */
    } else if (sp->ticks == 0) {
        /* Do nothing. */
    } else if (bp->payload.active) {
        /* Do nothing. */
    } else if (!bp->payload.valid) {
        /* Do nothing. */
    } else if ((fp = diminuto_observation_create(arp, &temporary)) == (FILE *)0) {
        /* Do nothing. */
    } else {

        lat = sp->payload.lat;
        latHp = sp->payload.latHp;
        lon = sp->payload.lon;
        lonHp = sp->payload.lonHp;
        height  = sp->payload.height; /* mm == 10^-3m */
        heightHp = sp->payload.heightHp; /* 0.1mm == 10^-4m (-9..+9) */

        /*
         * Remarkably, the output high precision height in SURVEY-IN mode is
         * reported in slightly different units than its input configuration
         * in FIXED mode. This seems wrong. [UBX ZED-F9P Interface,
         * pp. 145, 226..227]
         */

        value = (height * 10) + heightHp; /* 0.1mm == 10^-4m */
        height = value / 100; /* cm == 10^-4m * 10^2 == 10^-2m */
        heightHp = value % 100; /* 0.1mm == 10^-4m (-99..+99) */

        DIMINUTO_LOG_INFORMATION("Fix Emit lat 0x%8.8x 0x%2.2x lon 0x%8.8x 0x%2.2x alt 0x%8.8x 0x%2.2x\n", (uint32_t)lat, (uint8_t)latHp, (uint32_t)lon, (uint8_t)lonHp, (uint32_t)height, (uint8_t)heightHp);

        COM_DIAG_YODEL_HTOLE(lat);
        COM_DIAG_YODEL_HTOLE(lon);
        COM_DIAG_YODEL_HTOLE(height);

        dump_buffer(fp, &lat, sizeof(lat));
        dump_buffer(fp, &latHp, sizeof(latHp));
        dump_buffer(fp, &lon, sizeof(lon));
        dump_buffer(fp, &lonHp, sizeof(lonHp));
        dump_buffer(fp, &height, sizeof(height));
        dump_buffer(fp, &heightHp, sizeof(heightHp));

        rc = (diminuto_observation_commit(fp, &temporary) == (FILE *)0);

    }

    return rc;
}

/*******************************************************************************
 * REPORTERS
 ******************************************************************************/

/**
 * Print all of the active satellites used for the most recent fix.
 * @param fp points to the FILE stream.
 * @param aa points to the array of active satellites.
 */
static void print_actives(FILE * fp, const hazer_active_t aa[])
{
    static const unsigned int IDENTIFIERS = diminuto_countof(aa[0].id);
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
    static const unsigned int SATELLITES = diminuto_countof(va[0].sat);
    static const unsigned int IDENTIFIERS = diminuto_countof(aa[0].id);
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
            DIMINUTO_LOG_WARNING("VIEW \"%s\" %u %u %u\n", HAZER_SYSTEM_NAME[system], va[system].pending, va[system].channels, va[system].view);
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
            DIMINUTO_LOG_NOTICE("Signal UBX MON jamming %u indicator %u\n", value, hp->payload.jamInd);
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
            DIMINUTO_LOG_NOTICE("Signal UBX NAV spoofing %u\n", value);
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
 * @param bytes is the total number of bytes sent and received over the network.
 */
static void print_positions(FILE * fp, const hazer_position_t pa[], int pps, int dmyokay, int totokay, uint64_t bytes)
{
    unsigned int system = 0;

    {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        uint64_t billionths = 0;
        char zone = '\0';
        static int once = 0;

        zone = diminuto_time_zonename(0);

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == 0) { continue; }
            if (pa[system].dmy_nanoseconds == 0) { continue; }

            fputs("TIM", fp);

            hazer_format_nanoseconds2timestamp(pa[system].tot_nanoseconds, &year, &month, &day, &hour, &minute, &second, &billionths);
            assert((1 <= month) && (month <= 12));
            assert((1 <= day) && (day <= 31));
            assert((0 <= hour) && (hour <= 23));
            assert((0 <= minute) && (minute <= 59));
            assert((0 <= second) && (second <= 59));
            assert((0 <= billionths) && (billionths < 1000000000ULL));
            fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02d.000-00:00+00%c", year, month, day, hour, minute, second, zone);

            fprintf(fp, " %cpps", pps ? '1' : '0');

            fprintf(fp, "%28s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

            if (!once) {
                DIMINUTO_LOG_NOTICE("Time %04d-%02d-%02d %02d:%02d:%02d%c", year, month, day, hour, minute, second, zone);
                once = !0;
            }

        }
    }

    {
        int degrees = 0;
        int minutes = 0;
        int seconds = 0;
        int thousandths = 0;
        int direction = 0;
        uint64_t tenmillionths = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == 0) { continue; }

            fputs("POS", fp);

            hazer_format_nanominutes2position(pa[system].lat_nanominutes, &degrees, &minutes, &seconds, &thousandths, &direction);
            assert((0 <= degrees) && (degrees <= 90));
            assert((0 <= minutes) && (minutes <= 59));
            assert((0 <= seconds) && (seconds <= 59));
            assert((0 <= thousandths) && (thousandths <= 999));
            fprintf(fp, " %2d%lc%02d'%02d.%03d\"%c,", degrees, DEGREE, minutes, seconds, thousandths, (direction < 0) ? 'S' : 'N');

            hazer_format_nanominutes2position(pa[system].lon_nanominutes, &degrees, &minutes, &seconds, &thousandths, &direction);
            assert((0 <= degrees) && (degrees <= 180));
            assert((0 <= minutes) && (minutes <= 59));
            assert((0 <= seconds) && (seconds <= 59));
            assert((0 <= thousandths) && (thousandths <= 999));
            fprintf(fp, " %3d%lc%02d'%02d.%03d\"%c", degrees, DEGREE, minutes, seconds, thousandths, (direction < 0) ? 'W' : 'E');

            fputc(' ', fp);

            hazer_format_nanominutes2degrees(pa[system].lat_nanominutes, &degrees, &tenmillionths);
            assert((-90 <= degrees) && (degrees <= 90));
            assert((0 <= tenmillionths) && (tenmillionths <= 9999999));
            fprintf(fp, " %4d.%07llu,", degrees, (long long unsigned int)tenmillionths);

            hazer_format_nanominutes2degrees(pa[system].lon_nanominutes, &degrees, &tenmillionths);
            assert((-180 <= degrees) && (degrees <= 180));
            fprintf(fp, " %4d.%07llu", degrees, (long long unsigned int)tenmillionths);
            assert((0 <= tenmillionths) && (tenmillionths <= 9999999));

            fprintf(fp, "%7s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int64_t meters = 0;
        uint64_t thousandths = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == 0) { continue; }

            fputs("ALT", fp);

            fprintf(fp, " %10.2lf'", pa[system].alt_millimeters * 3.2808 / 1000.0);

            meters = pa[system].alt_millimeters / 1000LL;
            thousandths = abs64(pa[system].alt_millimeters) % 1000ULL;
            fprintf(fp, " %6lld.%03llum", (long long signed int)meters, (long long unsigned int)thousandths);

            fprintf(fp, "%43s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int64_t degrees = 0;
        uint64_t billionths = 0;
        const char * compass = (const char *)0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == 0) { continue; }

            fputs("COG", fp);

            assert((0LL <= pa[system].cog_nanodegrees) && (pa[system].cog_nanodegrees <= 360000000000LL));

            compass = hazer_format_nanodegrees2compass8(pa[system].cog_nanodegrees);
            assert(compass != (const char *)0);
            assert(strlen(compass) <= 4);
            fprintf(fp, " %-2s", compass);

            degrees = pa[system].cog_nanodegrees / 1000000000LL;
            billionths = abs64(pa[system].cog_nanodegrees) % 1000000000LLU;
            fprintf(fp, " %4lld.%09llu%lcT", (long long signed int)degrees, (long long unsigned int)billionths, DEGREE);

            degrees = pa[system].mag_nanodegrees / 1000000000LL;
            billionths = abs64(pa[system].mag_nanodegrees) % 1000000000LLU;
            fprintf(fp, " %4lld.%09llu%lcM", (long long signed int)degrees, (long long unsigned int)billionths, DEGREE);

            fprintf(fp, "%30s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int64_t knots = 0;
        int64_t kilometersperhour = 0;
        uint64_t millionths = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == 0) { continue; }

            fputs("SOG", fp);

            fprintf(fp, " %11.3lfmph", pa[system].sog_microknots * 1.150779 / 1000000.0);

            knots = pa[system].sog_microknots / 1000000LL;
            millionths = abs64(pa[system].sog_microknots) % 1000000ULL;
            fprintf(fp, " %7lld.%06lluknots", (long long signed int)knots, (long long unsigned int)millionths);

            kilometersperhour = pa[system].sog_millimeters / 1000000LL;
            millionths = abs64(pa[system].sog_millimeters) % 1000000ULL;
            fprintf(fp, " %7lld.%06llukph", (long long signed int)kilometersperhour, (long long unsigned int)millionths);

            fprintf(fp, "%14s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (pa[system].ticks == 0) { continue; }

            fputs("INT", fp);

            fprintf(fp, " %s", pa[system].label);
            fprintf(fp, " [%2u]", pa[system].sat_used);
            fprintf(fp, " %ddmy", dmyokay);
            fprintf(fp, " %dinc", totokay);
            fprintf(fp, " ( %2d %2d %2d %2d %2d %2d %2d )", pa[system].lat_digits, pa[system].lon_digits, pa[system].alt_digits, pa[system].cog_digits, pa[system].mag_digits, pa[system].sog_digits, pa[system].smm_digits);
            fprintf(fp, " %20llub", (unsigned long long)bytes); /* (2^64)-1 == 0xFFFFFFFFFFFFFFFF == 18,446,744,073,709,551,615. */

            fprintf(fp, "%1s", "");

            fprintf(fp, " %-8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

}

/**
 * Print information about the base and the rover that communicate via RTCM.
 * @param fp points to the FILE stream.
 * @param bp points to the base structure.
 * @param rp points to the rover structure.
 * @param mp points to the kinematics structure.
 */
static void print_corrections(FILE * fp, const yodel_base_t * bp, const yodel_rover_t * rp, const tumbleweed_message_t * kp, const tumbleweed_updates_t * up)
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
         fprintf(fp, " %4u [%4zu] %-8s <%8.8s>", kp->number, kp->length, (kp->source == DEVICE) ? "base" : (kp->source == NETWORK) ? "rover" : "unknown", up->bytes);
         fprintf(fp, "%36s", "");
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
    int32_t decimaldegrees = 0;
    uint32_t degrees = 0;
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    uint64_t billionths = 0;
    int direction = 0;
    int32_t meters = 0;
    uint32_t tenthousandths = 0;

    if (sp->ticks != 0) {

        fputs("HPP", fp);

        yodel_format_hppos2degrees(sp->payload.lat, sp->payload.latHp, &decimaldegrees, &billionths);
        fprintf(fp, " %4d.%09llu,", decimaldegrees, (long long unsigned int)billionths);

        yodel_format_hppos2degrees(sp->payload.lon, sp->payload.lonHp, &decimaldegrees, &billionths);
        fprintf(fp, " %4d.%09llu", decimaldegrees, (long long unsigned int)billionths);

        yodel_format_hpacc2accuracy(sp->payload.hAcc, &meters, &tenthousandths);
        fprintf(fp, " %lc%6lld.%04llum", PLUSMINUS, (long long signed int)meters, (long long unsigned int)tenthousandths);

        fprintf(fp, "%22s", "");

        fprintf(fp, " %-8s", "GNSS");

        fputc('\n', fp);

        fputs("HPA", fp);

        yodel_format_hpalt2aaltitude(sp->payload.hMSL, sp->payload.hMSLHp, &meters, &tenthousandths);
        fprintf(fp, " %6lld.%04llum MSL", (long long signed int)meters, (long long unsigned int)tenthousandths);

        yodel_format_hpalt2aaltitude(sp->payload.height, sp->payload.heightHp, &meters, &tenthousandths);
        fprintf(fp, " %6lld.%04llum WGS84", (long long signed int)meters, (long long unsigned int)tenthousandths);

        yodel_format_hpacc2accuracy(sp->payload.vAcc, &meters, &tenthousandths);
        fprintf(fp, " %lc%6lld.%04llum", PLUSMINUS, (long long signed int)meters, (long long unsigned int)tenthousandths);

        fprintf(fp, "%17s", "");

        fprintf(fp, " %-8s", "GNSS");

        fputc('\n', fp);

        fputs("NGS", fp);

        yodel_format_hppos2position(sp->payload.lat, sp->payload.latHp, &degrees, &minutes, &seconds, &tenthousandths, &direction);
        fprintf(fp, " %3u %02u %02u.%05u(%c)", degrees, minutes, seconds, tenthousandths, (direction < 0) ? 'S' : 'N');

        yodel_format_hppos2position(sp->payload.lon, sp->payload.lonHp, &degrees, &minutes, &seconds, &tenthousandths, &direction);
        fprintf(fp, " %3u %02u %02u.%05u(%c)", degrees, minutes, seconds, tenthousandths, (direction < 0) ? 'W' : 'E');

        fprintf(fp, "%29s", "");

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
    poller_t * ctxp = (poller_t *)0;
    int done = 0;
    int rc = -1;
    int nowpps = 0;
    int waspps = 0;

    ctxp = (poller_t *)argp;

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
    poller_t * pollerp = (poller_t *)0;
    diminuto_mux_t mux = { 0 };
    int ppsfd = -1;
    int done = 0;
    int rc = -1;
    int fd = -1;
    int nowpps = 0;
    int waspps = 0;

    pollerp = (poller_t *)argp;

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
    const char * arp = (const char *)0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
    int process = 0;
    int strobepin = -1;
    int ppspin = -1;
    int slow = 0;
    int expire = 0;
    int unknown = 0;
    int serial = 0;
    int daemon = 0;
    long timeout = HAZER_GNSS_SECONDS;
    long keepalive = TUMBLEWEED_KEEPALIVE_SECONDS;
    /*
     * Configuration command variables.
     */
    command_t * command = (command_t *)0;
    diminuto_list_t * command_node = (diminuto_list_t *)0;
    diminuto_list_t command_list = DIMINUTO_LIST_NULLINIT(&command_list);
    uint8_t * command_string = (uint8_t *)0;
    ssize_t command_size = 0;
    ssize_t command_length = 0;
    /*
     * FILE pointer variables.
     */
    FILE * in_fp = stdin;
    FILE * out_fp = stdout;
    FILE * dev_fp = (FILE *)0;
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
    int carrierdetect = 0;
    int readonly = !0;
    long device_mask = NMEA;
    /*
     * Remote variables.
     */
    protocol_t remote_protocol = PROTOCOL;
    datagram_buffer_t remote_buffer = DATAGRAM_BUFFER_INITIALIZER;
    ssize_t remote_total = 0;
    ssize_t remote_size = 0;
    ssize_t remote_length = 0;
    datagram_sequence_t remote_sequence = 0;
    const char * remote_option = (const char *)0;
    diminuto_ipc_endpoint_t remote_endpoint = { 0, };
    long remote_mask = NMEA;
    role_t role = ROLE;
    /*
     * Surveyor variables.
     */
    protocol_t surveyor_protocol = PROTOCOL;
    datagram_buffer_t surveyor_buffer = DATAGRAM_BUFFER_INITIALIZER;
    ssize_t surveyor_total = 0;
    datagram_sequence_t surveyor_sequence = 0;
    const char * surveyor_option = (const char *)0;
    diminuto_ipc_endpoint_t surveyor_endpoint = { 0, };
    ssize_t surveyor_size = 0;
    ssize_t surveyor_length = 0;
    uint8_t surveyor_crc1 = 0;
    uint8_t surveyor_crc2 = 0;
    uint8_t surveyor_crc3 = 0;
    /*
     * Network variables.
     */
    uint64_t network_total = 0;
    /*
     * Keepalive variables.
     */
    struct { datagram_header_t header; uint8_t payload[sizeof(TUMBLEWEED_KEEPALIVE)]; } keepalive_buffer = { { 0, }, TUMBLEWEED_KEEPALIVE_INITIALIZER };
    datagram_sequence_t keepalive_sequence = 0;
    /*
     * File Descriptor variables.
     */
    int in_fd = -1;
    int dev_fd = -1;
    int remote_fd = -1;
    int surveyor_fd = -1;
    /*
     * 1PPS poller thread variables.
     */
    const char * pps = (const char *)0;
    poller_t poller = { 0 };
    void * result = (void *)0;
    pthread_t thread;
    int pthreadrc = -1;
    int onepps = 0;
    /*
     * NMEA parser state variables.
     */
    hazer_state_t nmea_state = HAZER_STATE_STOP;
    hazer_context_t nmea_context = { 0, };
    datagram_buffer_t nmea_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * UBX parser state variables.
     */
    yodel_state_t ubx_state = YODEL_STATE_STOP;
    yodel_context_t ubx_context = { 0, };
    datagram_buffer_t ubx_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * RTCM parser state variables.
     */
    tumbleweed_state_t rtcm_state = TUMBLEWEED_STATE_STOP;
    tumbleweed_context_t rtcm_context = { 0, };
    datagram_buffer_t rtcm_buffer = DATAGRAM_BUFFER_INITIALIZER;
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
    tumbleweed_updates_t updates = TUMBLEWEED_UPDATES_INITIALIZER;
    /*
     * Time keeping variables.
     */
    diminuto_sticks_t frequency = 0;
    diminuto_sticks_t delay = 0;
    diminuto_sticks_t elapsed = 0;
    diminuto_sticks_t epoch = 0;
    diminuto_sticks_t fix = -1;
    diminuto_sticks_t timetofirstfix = -1;
    long expiration_was = 0;
    long expiration_now = 0;
    long display_was = 0;
    long display_now = 0;
    long keepalive_was = 0;
    long keepalive_now = 0;
    /*
     * I/O buffer variables.
     */
    void * io_buffer = (void *)0;
    size_t io_size = BUFSIZ;
    size_t io_maximum = 0;
    size_t io_total = 0;
    /*
     * Source variables.
     */
    diminuto_mux_t mux = { 0, };
    int ch = EOF;
    int ready = 0;
    int fd = -1;
    ssize_t available = 0;
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
    int eof = 0;        /** If true then the input stream hit end of file. */
    int sync = 0;       /** If true then the input stream is synchronized. */
    int frame = 0;      /** If true then the input stream is at frame start. */
    int refresh = !0;   /** If true then the display needs to be refreshed. */
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
     * Counters.
     */
    unsigned int outoforder_counter = 0;
    unsigned int missing_counter = 0;
    /*
     * Miscellaneous variables.
     */
    int rc = 0;
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
    static const char OPTIONS[] = "1278B:D:EFG:H:I:KL:MN:OPRS:U:VW:XY:b:cdeg:hk:lmnop:st:uvy:?"; /* Unused: ACJNQTZ afijqrwxz Pairs: Aa Jj Qq Zz */

    /**
     ** PREINITIALIZATION
     **/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;

    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);

    diminuto_log_setmask();

    (void)gethostname(Hostname, sizeof(Hostname));
    Hostname[sizeof(Hostname) - 1] = '\0';

    locale = setlocale(LC_ALL, "");

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case '1':
            stopbits = 1;
            serial = !0;
            break;
        case '2':
            stopbits = 2;
            serial = !0;
            break;
        case '7':
            databits = 7;
            serial = !0;
            break;
        case '8':
            databits = 8;
            serial = !0;
            break;
        case 'B':
            io_size = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (io_size < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'D':
            device = optarg;
            break;
        case 'E':
            report = !0;
            escape = !0;
            process = !0;
            break;
        case 'F':
            report = !0;
            slow = !0;
            process = !0;
            break;
        case 'G':
            remote_option = optarg;
            rc = diminuto_ipc_endpoint(optarg, &remote_endpoint);
            if (remote_endpoint.udp <= 0) { rc = -1; errno = EINVAL; }
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'H':
            report = !0;
            headless = optarg;
            process = !0;
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
        case 'M':
            daemon = !0;
            break;
        case 'N':
            arp = optarg;
            break;
        case 'P':
            process = !0;
            break;
        case 'R':
            report = !0;
            process = !0;
            break;
        case 'S':
            source = optarg;
            break;
        case 'U':
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            assert(command != (command_t *)0);
            command->acknak = !0;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'V':
            DIMINUTO_LOG_INFORMATION("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'W':
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            assert(command != (command_t *)0);
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
            if (surveyor_endpoint.udp <= 0) { rc = -1; errno = EINVAL; }
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'b':
            bitspersecond = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (bitspersecond == 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            serial = !0;
            break;
        case 'c':
            modemcontrol = !0;
            carrierdetect = !0;
            serial = !0;
            break;
        case 'd':
            debug = !0;
            break;
        case 'e':
            paritybit = 2;
            serial = !0;
            break;
        case 'g':
            remote_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'h':
            rtscts = !0;
            serial = !0;
            break;
        case 'k':
            device_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'l':
            modemcontrol = 0;
            serial = !0;
            break;
        case 'm':
            modemcontrol = !0;
            serial = !0;
            break;
        case 'n':
            paritybit = 0;
            serial = !0;
            break;
        case 'o':
            paritybit = 1;
            serial = !0;
            break;
        case 'p':
            strobe = optarg;
            strobepin = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (strobepin < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 's':
            xonxoff = !0;
            serial = !0;
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
                           "[ -d ] [ -v ] [ -M ] [ -u ] [ -V ] [ -X ] "
                           "[ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] | -S FILE ] [ -B BYTES ]"
                           "[ -t SECONDS ] "
                           "[ -I PIN | -c ] [ -p PIN ] "
                           "[ -U STRING ... ] [ -W STRING ... ] "
                           "[ -R | -E | -F | -H HEADLESS | -P ] "
                           "[ -L LOG ] "
                           "[ -G [ IP:PORT | :PORT [ -g MASK ] ] ] "
                           "[ -Y [ IP:PORT [ -y SECONDS ] | :PORT ] ] "
                           "[ -K [ -k MASK ] ] "
                           "[ -N FILE ]"
                           "\n", Program);
            fprintf(stderr, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(stderr, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(stderr, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(stderr, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(stderr, "       -B BYTES    Set the input Buffer size to BYTES bytes.\n");
            fprintf(stderr, "       -D DEVICE   Use DEVICE for input or output.\n");
            fprintf(stderr, "       -E          Like -R but use ANSI Escape sequences.\n");
            fprintf(stderr, "       -F          Like -R but reFresh at 1Hz.\n");
            fprintf(stderr, "       -G IP:PORT  Use remote IP and PORT as dataGram sink.\n");
            fprintf(stderr, "       -G :PORT    Use local PORT as dataGram source.\n");
            fprintf(stderr, "       -H HEADLESS Like -R but writes each iteration to HEADLESS file.\n");
            fprintf(stderr, "       -I PIN      Take 1PPS from GPIO Input PIN (requires -D).\n");
            fprintf(stderr, "       -K          Write input to DEVICE sinK from datagram source.\n");
            fprintf(stderr, "       -L LOG      Write input to LOG file.\n");
            fprintf(stderr, "       -M          Run in the background as a daeMon.\n");
            fprintf(stderr, "       -N FILE     Use fix FILE to save ARP LLH for subsequeNt fixed mode.\n");
            fprintf(stderr, "       -P          Process incoming data even if no report is being generated.\n");
            fprintf(stderr, "       -R          Print a Report on standard output.\n");
            fprintf(stderr, "       -S FILE     Use source FILE or named pipe for input.\n");
            fprintf(stderr, "       -U STRING   Like -W except expect UBX ACK or NAK response.\n");
            fprintf(stderr, "       -U ''       Exit when this empty UBX STRING is processed.\n");
            fprintf(stderr, "       -V          Log Version in the form of release, vintage, and revision.\n");
            fprintf(stderr, "       -W STRING   Collapse STRING, append checksum, Write to DEVICE.\n");
            fprintf(stderr, "       -W ''       Exit when this empty Write STRING is processed.\n");
            fprintf(stderr, "       -X          Enable message eXpiration test mode.\n");
            fprintf(stderr, "       -Y IP:PORT  Use remote IP and PORT as keepalive sink and surveYor source.\n");
            fprintf(stderr, "       -Y :PORT    Use local PORT as surveYor source.\n");
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
            fprintf(stderr, "       -x          Run in the background as a daemon.\n");
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

    if (daemon) {
        rc = diminuto_daemon(Program);
        DIMINUTO_LOG_NOTICE("Daemon %s %d %d %d %d", Program, rc, (int)getpid(), (int)getppid(), (int)getsid(getpid()));
        assert(rc == 0);
    }

    DIMINUTO_LOG_INFORMATION("Begin");

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
     * Initialize the multiplexer.
     */

    diminuto_mux_init(&mux);

    /*
     * Are we consuming GPS data from an IP port, or producing GPS data to an
     * IP host and port? This feature is useful for forwarding data from a
     * mobile receiver to a stationary server, for example a vehicle tracking
     * application, or an unattended survey unit in the field that is monitored
     * remotely.
     */

    if (remote_option == (const char *)0) {
        /* Do nothing. */
    } else if (remote_endpoint.udp == 0) {
        /* Do nothing. */
    } else if (!diminuto_ipc6_is_unspecified(&remote_endpoint.ipv6)) {

        remote_protocol = IPV6;

        remote_fd = diminuto_ipc6_datagram_peer(0);
        assert(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        assert(rc >= 0);

        role = PRODUCER;

    } else if (!diminuto_ipc4_is_unspecified(&remote_endpoint.ipv4)) {

        remote_protocol = IPV4;

        remote_fd = diminuto_ipc4_datagram_peer(0);
        assert(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        assert(rc >= 0);

        role = PRODUCER;

    } else {

        remote_protocol = IPV6;

        remote_fd = diminuto_ipc6_datagram_peer(remote_endpoint.udp);
        assert(remote_fd >= 0);

        rc = diminuto_mux_register_read(&mux, remote_fd);
        assert(rc >= 0);

        role = CONSUMER;

    }

    if (remote_fd >= 0) { show_connection("Remote", remote_option, remote_fd, remote_protocol, &remote_endpoint.ipv6, &remote_endpoint.ipv4, remote_endpoint.udp); }

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

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
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

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
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

    if (surveyor_fd >= 0) { show_connection("Surveyor", surveyor_option, surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv6, &surveyor_endpoint.ipv4, surveyor_endpoint.udp); }

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

        if (serial) {

            DIMINUTO_LOG_INFORMATION("Device (%d) \"%s\" %s %d %d%c%d%s%s%s\n", dev_fd, device, readonly ? "ro" : "rw", bitspersecond, databits, (paritybit == 0) ? 'N' : ((paritybit % 2) == 0) ? 'E' : 'O', stopbits, modemcontrol ? " modem" : " local", xonxoff ? " xonoff" : "", rtscts ? " rtscts" : "");

            rc = diminuto_serial_set(dev_fd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
            assert(rc == 0);

            rc = diminuto_serial_raw(dev_fd);
            assert(rc == 0);

        } else {

            DIMINUTO_LOG_INFORMATION("Device (%d) \"%s\" %s\n", dev_fd, device, readonly ? "ro" : "rw");

        }

        /*
         * Remarkably, below, some USB receivers will work with a mode of "w+"
         * and some will return a fatal I/O error and require "a+". "a+" seems
         * to work in either case. Weird.
         */

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

    delay = frequency; /* May be mutatable some day. */

    keepalive_was -= keepalive;

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

    /**
     ** LOOP
     **/

    DIMINUTO_LOG_INFORMATION("Start");

    while (!0) {

        /*
         * We keep working until out input goes away (end of file), or until
         * we are interrupted by a SIGINT or terminated by a SIGTERM. We
         * also check for SIGHUP, which I might use for something in the
         * future.
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

        available = 0;
        fd = -1;
        ready = 0;

        if ((available = diminuto_file_ready(in_fp)) > 0) {
            fd = in_fd;
        } else if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {
            /* Do nothing. */
        } else if ((ready = diminuto_mux_wait(&mux, delay /* BLOCK */)) == 0) {
            /* Do nothing. */
        } else if (ready > 0) {
            fd = diminuto_mux_ready_read(&mux);
            assert(fd >= 0);
        } else if (errno == EINTR) {
            continue;
        } else {
            assert(0);
        }

        /*
         * At this point, either available > 0 (there is pending data in
         * the input stream buffer) or fd >= 0 (there is a file descriptor or
         * socket with pending data), or fd < 0 (there is no data pending).
         * The latter case is very unlikely since there was a long timeout in
         * the multiplexor wait unless our device has stopped generating data.
         */

        buffer = (uint8_t *)0;

        if (fd < 0) {

            /*
             * The multiplexor timed out; very unlikely but not impossible
             * if our device or remote stopped producing data.
             */

        } else if (fd == in_fd) {

            if (available > io_maximum) { io_maximum = available; }

            /*
             * Consume bytes of NMEA, UBX, or RTCM from the input stream until
             * the current input stream buffer is empty or until a complete
             * buffer is assembled.
             */

            do {

                ch = fgetc(in_fp);
                if (ch == EOF) {
                    DIMINUTO_LOG_NOTICE("EOF");
                    eof = !0;
                    break;
                }

                io_total += 1;

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

                    nmea_state = HAZER_STATE_START;
                    ubx_state = YODEL_STATE_STOP;
                    rtcm_state = TUMBLEWEED_STATE_STOP;

                } else if (ch == YODEL_STIMULUS_SYNC_1) {

                    nmea_state = HAZER_STATE_STOP;
                    ubx_state = YODEL_STATE_START;
                    rtcm_state = TUMBLEWEED_STATE_STOP;

                } else if (ch == TUMBLEWEED_STIMULUS_PREAMBLE) {

                    nmea_state = HAZER_STATE_STOP;
                    ubx_state = YODEL_STATE_STOP;
                    rtcm_state = TUMBLEWEED_STATE_START;

                } else {

                    DIMINUTO_LOG_WARNING("Sync Lost 0x%016llx 0x%02x\n", (unsigned long long)io_total, ch);

                    sync = 0;

                    nmea_state = HAZER_STATE_START;
                    ubx_state = YODEL_STATE_START;
                    rtcm_state = TUMBLEWEED_STATE_START;

                }

                frame = 0;

                nmea_state = hazer_machine(nmea_state, ch, nmea_buffer.payload.nmea, sizeof(nmea_buffer.payload.nmea), &nmea_context);
                if (nmea_state == HAZER_STATE_END) {

                    buffer = nmea_buffer.payload.nmea;
                    size = hazer_size(&nmea_context);
                    length = size - 1;
                    format = NMEA;
                    if (!sync) { DIMINUTO_LOG_NOTICE("Sync NMEA 0x%016llx\n", (unsigned long long)io_total); sync = !0; }
                    frame = !0;

                    DIMINUTO_LOG_DEBUG("Input NMEA [%zd] [%zd]", size, length);

                    break;

                }

                ubx_state = yodel_machine(ubx_state, ch, ubx_buffer.payload.ubx, sizeof(ubx_buffer.payload.ubx), &ubx_context);
                if (ubx_state == YODEL_STATE_END) {

                    buffer = ubx_buffer.payload.ubx;
                    size = yodel_size(&ubx_context);
                    length = size - 1;
                    format = UBX;
                    if (!sync) { DIMINUTO_LOG_NOTICE("Sync UBX 0x%016llx\n", (unsigned long long)io_total); sync = !0; }
                    frame = !0;

                    DIMINUTO_LOG_DEBUG("Input UBX [%zd] [%zd]", size, length);

                    break;
                }

                rtcm_state = tumbleweed_machine(rtcm_state, ch, rtcm_buffer.payload.rtcm, sizeof(rtcm_buffer.payload.rtcm), &rtcm_context);
                if (rtcm_state == TUMBLEWEED_STATE_END) {

                    buffer = rtcm_buffer.payload.rtcm;
                    size = tumbleweed_size(&rtcm_context);
                    length = size - 1;
                    format = RTCM;
                    if (!sync) { DIMINUTO_LOG_NOTICE("Sync RTCM 0x%016llx\n", (unsigned long long)io_total); sync = !0; }
                    frame = !0;

                    DIMINUTO_LOG_DEBUG("Input RTCM [%zd] [%zd]", size, length);

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
                    if (sync) { DIMINUTO_LOG_WARNING("Sync Stop 0x%016llx 0x%02x\n", (unsigned long long)io_total, ch); sync = 0; }
                    frame = 0;
                    nmea_state = HAZER_STATE_START;
                    ubx_state = YODEL_STATE_START;
                    rtcm_state = TUMBLEWEED_STATE_START;
                }

            } while (diminuto_file_ready(in_fp) > 0);

            /*
             * At this point, either we ran out of data in the input
             * stream buffer, or we assembled a complete NMEA sentence,
             * UBX packet, or NMEA message to process, or we hit end of file.
             */

        } else if (fd == remote_fd) {

            /*
             * Receive a NMEA, UBX, or RTCM datagram from a remote gpstool.
             * We make a rule that the datagram must be a complete NMEA
             * sentence, UBX packet, or RTCM message, complete with a valid
             * checksum or cyclic redundancy check, with no extra leading or
             * trailing bytes. If we do receive an invalid datagram, that
             * is a serious bug either in this software or in the transport.
             */

            remote_total = receive_datagram(remote_fd, &remote_buffer, sizeof(remote_buffer));
            if (remote_total > 0) { network_total += remote_total; }

            if (remote_total < sizeof(remote_buffer.header)) {

                DIMINUTO_LOG_WARNING("Remote Length [%zd]\n", remote_total);

            } else if ((remote_size = datagram_validate(&remote_sequence, &remote_buffer.header, remote_total, &outoforder_counter, &missing_counter)) < 0) {

                DIMINUTO_LOG_NOTICE("Remote Order [%zd] {%lu} {%lu}\n", remote_total, (unsigned long)remote_sequence, (unsigned long)ntohl(remote_buffer.header.sequence));

            } else if ((remote_length = hazer_validate(remote_buffer.payload.nmea, remote_size)) > 0) {

                buffer = remote_buffer.payload.nmea;
                size = remote_size;
                length = remote_length;
                format = NMEA;

                DIMINUTO_LOG_DEBUG("Remote NMEA [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else if ((remote_length = yodel_validate(remote_buffer.payload.ubx, remote_size)) > 0) {

                buffer = remote_buffer.payload.ubx;
                size = remote_size;
                length = remote_length;
                format = UBX;

                DIMINUTO_LOG_DEBUG("Remote UBX [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else if ((remote_length = tumbleweed_validate(remote_buffer.payload.rtcm, remote_size)) > 0) {

                buffer = remote_buffer.payload.rtcm;
                size = remote_size;
                length = remote_length;
                format = RTCM;

                DIMINUTO_LOG_DEBUG("Remote RTCM [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else {

                DIMINUTO_LOG_ERROR("Remote Other [%zd] [%zd] [%zd] 0x%02x\n", remote_total, remote_size, remote_length, remote_buffer.payload.data[0]);

            }

        } else if (fd == surveyor_fd) {

            /*
             * Receive an RTCM datagram from a remote gpstool doing a survey.
             */

            surveyor_total = receive_datagram(surveyor_fd, &surveyor_buffer, sizeof(surveyor_buffer));
            if (surveyor_total > 0) { network_total += surveyor_total; }

            if (surveyor_total < sizeof(surveyor_buffer.header)) {

                DIMINUTO_LOG_WARNING("Surveyor Length [%zd]\n", surveyor_total);

            } else if ((surveyor_size = datagram_validate(&surveyor_sequence, &surveyor_buffer.header, surveyor_total, &outoforder_counter, &missing_counter)) < 0) {

                DIMINUTO_LOG_NOTICE("Surveyor Order [%zd] {%lu} {%lu}\n", surveyor_total, (unsigned long)surveyor_sequence, (unsigned long)ntohl(surveyor_buffer.header.sequence));

            } else if ((surveyor_length = tumbleweed_validate(surveyor_buffer.payload.rtcm, surveyor_size)) < TUMBLEWEED_RTCM_SHORTEST) {

                DIMINUTO_LOG_ERROR("Surveyor Data [%zd] [%zd] [%zd] 0x%02x\n", surveyor_total, surveyor_size, surveyor_length, surveyor_buffer.payload.data[0]);

            } else if (surveyor_length == TUMBLEWEED_RTCM_SHORTEST) {

                DIMINUTO_LOG_DEBUG("Surveyor RTCM keepalive received");

            } else if (dev_fp == (FILE *)0) {

                /* Do nothing. */

            } else {

                kinematics.source = NETWORK;

                kinematics.number = tumbleweed_message(surveyor_buffer.payload.rtcm, surveyor_length);
                if (kinematics.number < 0) { kinematics.number = 9999; }
                collect_update(kinematics.number, &updates);

                kinematics.length = surveyor_length;

                kinematics.ticks = timeout;
                refresh = !0;

                DIMINUTO_LOG_DEBUG("Surveyor RTCM [%zd] [%zd] [%zd] <%d>\n", surveyor_total, surveyor_size, surveyor_length, kinematics.number);

                if (verbose) { diminuto_dump(stderr, &surveyor_buffer, surveyor_total); }
                write_buffer(dev_fp, surveyor_buffer.payload.rtcm, surveyor_length);

            }

        } else {

            /*
             * The multiplexor returned a file descriptor which was not one we
             * recognize; that should be impossible.
             */

            DIMINUTO_LOG_ERROR("Multiplexor Fail [%d] (%d) <%d %d %d>\n", ready, fd, dev_fd, remote_fd, surveyor_fd);
            assert(0);

        }

        /*
         * If one of the input sources indicated end of file, we're done.
         * (This may be the only time I've found a legitimate use for a goto.)
         */

        if (eof) { goto report; }

        /*
         * At this point, either we have a buffer with a complete and validated
         * NMEA sentence, UBX packet, or RTCM message ready to process, acquired
         * either from a state machine or a socket, or there is no input pending
         * and maybe this is a good time to update the display.
         */

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
         * typical firewall UDP "connection" timeout is thirty seconds. Also:
         * we delay sending keepalives until we have completed initializing
         * the device with any configuration, since it might not be ready to
         * receive RTCM messages until then.
         */

        if (surveyor_fd < 0) {
            /* Do nothing. */
        } else if (keepalive < 0) {
            /* Do nothing. */
        } else if (acknakpending > 0) {
            /* Do nothing. */
        } else if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (((keepalive_now = ticktock(frequency)) - keepalive_was) < keepalive) {
            /* Do nothing. */
        } else {

            datagram_stamp(&keepalive_buffer.header, &keepalive_sequence);
            surveyor_total = send_datagram(surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv4, &surveyor_endpoint.ipv6, surveyor_endpoint.udp, &keepalive_buffer, sizeof(keepalive_buffer));
            if (surveyor_total > 0) { network_total += surveyor_total; }
            keepalive_was = keepalive_now;

            DIMINUTO_LOG_DEBUG("Surveyor RTCM keepalive sent");

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

            command = diminuto_containerof(command_t, link, command_node);
            command_string = diminuto_list_data(command_node);
            assert(command_string != (uint8_t *)0);

            if (command_string[0] == '\0') {

                free(command_node);
                DIMINUTO_LOG_NOTICE("Done");
                eof = !0;

            } else {

                command_size = strlen(command_string) + 1;
                command_length = diminuto_escape_collapse(command_string, command_string, command_size);
                if (command_string[0] == HAZER_STIMULUS_START) {

                    emit_sentence(dev_fp, command_string, command_length);
                    rc = 0;

                } else if ((command_string[0] == YODEL_STIMULUS_SYNC_1) && (command_string[1] == YODEL_STIMULUS_SYNC_2)) {

                    emit_packet(dev_fp, command_string, command_length);
                    rc = 0;

                } else {

                    DIMINUTO_LOG_WARNING("Command Other 0x%02x%02x [%zd]", command_string[0], command_string[1], command_length);
                    rc = -1;

                }

                if (rc == 0) {
                    if (command->acknak) { acknakpending += 1; }
                    if (verbose) { print_buffer(stderr, command_string, command_length, UNLIMITED); }
                    if (escape) { fputs("\033[2;1H\033[0K", out_fp); }
                    if (report) { fprintf(out_fp, "OUT [%3zd] ", command_length); print_buffer(out_fp, command_string, command_length, limitation); fflush(out_fp); }
                }

                free(command_node);

                if (diminuto_list_isempty(&command_list)) { DIMINUTO_LOG_NOTICE("Ready"); }

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

        /**
         ** FORWARD
         **/

        /*
         * We forward anything whose format is enabled in the forwarding
         * mask. Note that we don't forward the terminating NUL (using length
         * instead of size) that terminate all input of any format (whether
         * that's useful or not). The ensured delivery of TCP can (and has, in
         * testing over LTE networks) add substantial latency to the data.
         * Sometimes it is truly "better never than late".
         */

        if (remote_fd < 0) {
            /* Do nothing. */
        } else if (role != PRODUCER) {
            /* Do nothing. */
        } else if ((remote_mask & format) == 0) {
            /* Do nothing. */
        } else {
            datagram_buffer_t * dp;
            dp = diminuto_containerof(datagram_buffer_t, payload, buffer);
            datagram_stamp(&(dp->header), &remote_sequence);
            remote_total = send_datagram(remote_fd, remote_protocol, &remote_endpoint.ipv4, &remote_endpoint.ipv6, remote_endpoint.udp, dp, sizeof(dp->header) + length);
            if (remote_total > 0) { network_total += remote_total; }
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

        if (log_fp != (FILE *)0) {  write_buffer(log_fp, buffer, length); }

        if (verbose) { print_buffer(stderr, buffer, length, UNLIMITED); }

        /*
         * At this point, if we are not generating a report or not otherwise
         * required to process the incoming data (maybe we're only forwarding
         * data to a remote via datagrams, or writing incoming datagrams to
         * a device), there is no point in continuing.
         */

        if (!process) { continue; }

        if (escape) { fputs("\033[1;1H\033[0K", out_fp); }
        if (report) { fprintf(out_fp, "INP [%3zd] ", length); print_buffer(out_fp, buffer, length, limitation); fflush(out_fp); }

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
            count = hazer_tokenize(vector, diminuto_countof(vector), tokenized, length);
            assert(count >= 0);
            assert(vector[count - 1] == (char *)0);
            assert(count <= diminuto_countof(vector));

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
                    DIMINUTO_LOG_WARNING("Parse NMEA Talker Other \"%c%c\"", vector[0][1], vector[0][2]);
                }

                continue;

            } else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {

                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    DIMINUTO_LOG_WARNING("Parse NMEA System Other \"%c%c\"\n", vector[0][1], vector[0][2]);
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

                DIMINUTO_LOG_INFORMATION("Parse NMEA TXT \"%.*s\"", length - 2 /* Exclude CR and LF. */, buffer);

            } else if (unknown) {

                DIMINUTO_LOG_WARNING("Parse NMEA Other \"%s\"\n", vector[0]);

            } else {

                /* Do nothing. */

            }

            /*
             * Calculate our time to first fix if the code above established
             * a fix.
             */

            if (fix < 0) {
                /* Do nothing. */
            } else if (timetofirstfix >= 0) {
                /* Do nothing. */
            } else {
                timetofirstfix = fix - epoch;
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

                DIMINUTO_LOG_INFORMATION("Parse UBX %s 0x%02x 0x%02x (%d)\n", acknak.state ? "ACK" : "NAK", acknak.clsID, acknak.msgID, acknakpending);

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
                        DIMINUTO_LOG_INFORMATION("Parse UBX CFG VALGET v%d %s [%d] 0x%08x 0x%01x\n", pp->version, layer, ii, kk, vv1);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_ONE:
                        memcpy(&vv1, bb, sizeof(vv1));
                        DIMINUTO_LOG_INFORMATION("Parse UBX CFG VALGET v%d %s [%d] 0x%08x 0x%02x\n", pp->version, layer, ii, kk, vv1);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_TWO:
                        memcpy(&vv16, bb, sizeof(vv16));
                        DIMINUTO_LOG_INFORMATION("Parse UBX CFG VALGET v%d %s [%d] 0x%08x 0x%04x\n", pp->version, layer, ii, kk, vv16);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_FOUR:
                        memcpy(&vv32, bb, sizeof(vv32));
                        DIMINUTO_LOG_INFORMATION("Parse UBX CFG VALGET v%d %s [%d] 0x%08x 0x%08x\n", pp->version,layer, ii, kk, vv32);
                        break;
                    case YODEL_UBX_CFG_VALGET_Size_EIGHT:
                        memcpy(&vv64, bb, sizeof(vv64));
                        DIMINUTO_LOG_INFORMATION("Parse UBX CFG VALGET v%d %s [%d] 0x%08x 0x%016llx\n", pp->version, layer, ii, kk, (unsigned long long)vv64);
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
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON VER SW \"%s\"\n", bb);
                    bb += YODEL_UBX_MON_VER_swVersion_LENGTH;

                    if (bb >= ee) { break; }
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON VER HW \"%s\"\n", bb);
                    bb += YODEL_UBX_MON_VER_hwVersion_LENGTH;

                    while (bb < ee) {
                        DIMINUTO_LOG_INFORMATION("Parse UBX MON VER EX \"%s\"\n", bb);
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

                DIMINUTO_LOG_WARNING("Parse UBX Other 0x%02x%02x%02x%02x\n", buffer[YODEL_UBX_SYNC_1], buffer[YODEL_UBX_SYNC_2], buffer[YODEL_UBX_CLASS], buffer[YODEL_UBX_ID]);

            } else {

                /* Do nothing. */

            }

            break;

        case RTCM:

            /*
             * RTCM MESSAGES
             */

            if (verbose) { diminuto_dump(stderr, buffer, length); }

            kinematics.source = DEVICE;

            kinematics.number = tumbleweed_message(buffer, length);
            if (kinematics.number < 0) { kinematics.number = 9999; }
            collect_update(kinematics.number, &updates);

            kinematics.length = length;

            kinematics.ticks = timeout;
            refresh = !0;

            break;

        case FORMAT:

            /* Do nothing. */

            break;

        }

        /**
         ** REPORT
         **/

        /*
         * We always give priority to reading input from the device or a
         * socket. Generating the report can take a long time, particularly
         * with slow displays or serial consoles (partly what the -F flag is
         * all about). So if there is still data waiting to be read, we
         * short circuit the report code and instead try to assemble another
         * complete sentence, packet, or message that we can forward, write,
         * log, or use to update our databases.
         */
        if ((dev_fp == (FILE *)0) && (remote_fd < 0)) {
            /* Do nothing. */
        } else if (diminuto_file_ready(in_fp) > 0) {
            continue;
        } else if (diminuto_mux_wait(&mux, 0 /* POLL */) > 0) {
            continue;
        } else {
            /* Do nothing. */
        }

        /*
         * This code is just for testing the expiration feature.
         * It turns out to be remarkably difficult to block the most recent
         * GPS receivers, e.g. the UBlox 8. Makes me wish I still had access
         * to those gigantic walk-in Faraday cages that several of my clients
         * have. Anyway, if some of the data are too old, we remove them from
         * the display. This is particularly useful (for me) for determining
         * when a base has stopped transmitting to a rover, making the rover's
         * high precision position fix problematic.
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
         * If we've generated a high precision solution in survey mode,
         * and have been asked to emit the solution to a file for later use
         * in fixed mode, do so now. We delay doing this until the device
         * is fully configured and has ACKed all of the configuration
         * commands.
         */

        if (arp == (const char *)0) {
            /* Do nothing. */
        } else if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (acknakpending > 0) {
            /* Do nothing. */
        } else if (!save_solution(arp, &base, &solution)) {
            /* Do nothing. */
        } else {
            arp = (const char *)0;
        }

        /*
         * Generate the display if necessary and sufficient reasons exist.
         */

report:

        if (!refresh) {
            /* Do nothing. */
        } else if (slow && (display_was == (display_now = ticktock(frequency)))) {
            /* Do nothing. */
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
                print_positions(out_fp, position, onepps, dmyokay, totokay, network_total);
                print_solution(out_fp, &solution);
                print_corrections(out_fp, &base, &rover, &kinematics, &updates);
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

        if (eof) { break; }

    }

    /**
     ** FINIALIZATION
     **/

    DIMINUTO_LOG_INFORMATION("Stop");

    DIMINUTO_LOG_INFORMATION("Counters Remote=%lu Surveyor=%lu Keepalive=%lu OutOfOrder=%u Missing=%u", (unsigned long)remote_sequence, (unsigned long)surveyor_sequence, (unsigned long)keepalive_sequence, outoforder_counter, missing_counter);

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

    if (remote_fd >= 0) {
        rc = diminuto_ipc_close(remote_fd);
        assert(rc >= 0);
    }

    if (log_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (log_fp == stdout) {
        /* Do nothing. */
    } else {
        rc = fclose(log_fp);
        if (rc == EOF) { diminuto_perror("fclose(log_fp)"); }
    }

    if (dev_fp != (FILE *)0) {
        rc = fclose(dev_fp);
        if (rc == EOF) { diminuto_perror("fclose(dev_fp)"); }
    }

    DIMINUTO_LOG_INFORMATION("Buffer size=%lluB maximum=%lluB total=%lluB speed=%lluBPS\n", (unsigned long long)io_size, (unsigned long long)io_maximum, (unsigned long long)io_total, (unsigned long long)((io_total * frequency) / (diminuto_time_elapsed() - epoch)));
    free(io_buffer);

    if (in_fp != dev_fp) {
        rc = fclose(in_fp);
        if (rc == EOF) { diminuto_perror("fclose(in_fp)"); }
    }

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_commit(out_fp, &temporary);
        assert(out_fp == (FILE *)0);
    } else if (out_fp != dev_fp) {
        rc = fclose(out_fp);
        if (rc == EOF) { diminuto_perror("fclose(out_fp)"); }
    } else {
        /* Do nothing. */
    }

    while (!diminuto_list_isempty(&command_list)) {
        command_node = diminuto_list_dequeue(&command_list);
        assert(command_node != (diminuto_list_t *)0);
        free(command_node);
    }

    DIMINUTO_LOG_INFORMATION("End");

    fflush(stderr);

    return 0;
}
