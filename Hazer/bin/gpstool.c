/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 *
 * EXAMPLES
 *
 * gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E -6 -A ::1 -P 5555
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
#include "com/diag/hazer/hazer.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_phex.h"

typedef enum Role { NONE = 0, PRODUCER = 1, CONSUMER = 2 } role_t;

typedef enum Protocol { IPV4 = 4, IPV6 = 6, } protocol_t;

static int emit_sentence(FILE * fp, const char * string)
{
    int rc = -1;
    uint8_t cs = 0;
    char msn = '\0';
    char lsn = '\0';

    do {

        cs = hazer_checksum(string, strlen(string));
        if (hazer_checksum2characters(cs, &msn, &lsn) < 0) { break; }

        if (fprintf(fp, "%s%c%c%c\r\n", string, HAZER_STIMULUS_CHECKSUM, msn, lsn) < 0) { break; }
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
        if (rc < 0) { perror("diminuto_ipc4_datagram_send"); }
        break;
    case IPV6:
        rc = diminuto_ipc6_datagram_send(sock, buffer, size, *ipv6p, port);
        if (rc < 0) { perror("diminuto_ipc6_datagram_send"); }
        break;
    default:
        assert(0);
        break;
    }

}

static void print_sentence(FILE *fp, const void * buffer, size_t size)
{
    const char * bb = (const char *)0;
    size_t current = 0;
    int end = 0;

    for (bb = buffer; size > 0; --size) {
        diminuto_phex_emit(fp, *(bb++), ~(size_t)0, 0, 0, 0, &current, &end, 0);
    }
    fputc('\r', fp);
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

    if (pps) { fprintf(fp, " 1PPS"); }

    fputc('\n', fp);
}

static void print_datum(FILE * fp, const char * name, const hazer_position_t * pp) {
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

int main(int argc, char * argv[])
{
    const char * program = (const char *)0;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
    hazer_state_t state = HAZER_STATE_EOF;
    hazer_state_t prior = HAZER_STATE_START;
    hazer_buffer_t buffer = { 0 };
    hazer_buffer_t datagram = { 0 };
    hazer_vector_t vector = { 0 };
    hazer_position_t position = { 0 };
    hazer_solution_t solution = { 0 };
    hazer_constellation_t constellation[HAZER_TALKER_TOTAL] = { {  0 } };
    FILE * infp = stdin;
    FILE * outfp = stdout;
    FILE * errfp = stderr;
    FILE * devfp = stdout;
    int fd = STDIN_FILENO;
    int sock = -1;
    int rc = 0;
    int ch = EOF;
    char * bb = (char *)0;
    ssize_t size = 0;
    ssize_t ss = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    ssize_t count = 0;
    char ** vv = (char **)0;
    uint8_t cs = 0;
    uint8_t ck = 0;
    char msn = '\0';
    char lsn = '\0';
    hazer_talker_t talker = HAZER_TALKER_NA;
    int opt = -1;
    const char * device = (const char *)0;
    int bitspersecond = 4800;
    int databits = 8;
    int paritybit = 0;
    int stopbits = 1;
    int modemcontrol = 0;
    int rtscts = 0;
    int xonxoff = 0;
    int readonly = !0;
    int carrierdetect = 0;
    role_t role = NONE;
    protocol_t protocol = IPV4;
    const char * host = (const char *)0;
    const char * service = (const char *)0;
    diminuto_ipv4_t ipv4 = 0;
    diminuto_ipv6_t ipv6 = { 0 };
    diminuto_port_t port = 0;
    int output = 0;
    int emit = 0;
    int pps = 0;
    uint64_t nanoseconds = 0;
    static const char OPTIONS[] = "124678A:D:EOP:RW:b:cdehlmnorsv?";
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

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
        case 'D':
            device = optarg;
            break;
        case 'E':
            report = !0;
            escape = !0;
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
            emit = !0;
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
        case 'r':
            outfp = stderr;
            errfp = stdout;
            break;
        case 's':
            xonxoff = !0;
            break;
        case 'v':
            verbose = !0;
            break;
        case '?':
            fprintf(stderr, "usage: %s [ -d ] [ -v ] [ -D DEVICE ] [ -b BPS ] [ -7 | -8 ]  [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] [ -c ] [ -W NMEA ] [ -R | -E ] [ -A ADDRESS ] [ -P PORT ] [ -O ]\n", program);
            fprintf(stderr, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(stderr, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(stderr, "       -4          Use IPv4 for ADDRESS, PORT.\n");
            fprintf(stderr, "       -6          Use IPv6 for ADDRESS, PORT.\n");
            fprintf(stderr, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(stderr, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(stderr, "       -A ADDRESS  Send sentences to ADDRESS.\n");
            fprintf(stderr, "       -D DEVICE   Use DEVICE.\n");
            fprintf(stderr, "       -E          Like -R but use ANSI escape sequences.\n");
            fprintf(stderr, "       -O          Output sentences to DEVICE.\n");
            fprintf(stderr, "       -P PORT     Send to or receive from PORT.\n");
            fprintf(stderr, "       -R          Print a report on standard output.\n");
            fprintf(stderr, "       -W NMEA     Append * and checksum to NMEA and write to DEVICE.\n");
            fprintf(stderr, "       -b BPS      Use BPS bits per second for DEVICE.\n");
            fprintf(stderr, "       -c          Wait for DCD to be asserted (implies -m).\n");
            fprintf(stderr, "       -d          Display debug output on standard error.\n");
            fprintf(stderr, "       -e          Use even parity for DEVICE.\n");
            fprintf(stderr, "       -l          Use local control for DEVICE.\n");
            fprintf(stderr, "       -m          Use modem control for DEVICE.\n");
            fprintf(stderr, "       -o          Use odd parity for DEVICE.\n");
            fprintf(stderr, "       -n          Use no parity for DEVICE.\n");
            fprintf(stderr, "       -h          Use RTS/CTS for DEVICE.\n");
            fprintf(stderr, "       -r          Reverse use of standard output and error.\n");
            fprintf(stderr, "       -s          Use XON/XOFF for DEVICE.\n");
            fprintf(stderr, "       -v          Display verbose output on standard error.\n");
            return 1;
            break;
        }
    }

    if (device != (const char *)0) {

        fd = open(device, readonly ? O_RDONLY : O_RDWR);
        if (fd < 0) { perror(device); }
        assert(fd >= 0);

        rc = diminuto_serial_set(fd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
        assert(rc == 0);

        rc = diminuto_serial_raw(fd);
        assert(rc == 0);

        devfp = fdopen(fd, readonly ? "r" : "a+");
        if (devfp == (FILE *)0) { perror(device); }
        assert(devfp != (FILE *)0);
        infp = devfp;

        if (emit) {
            while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
                if (opt == 'W') {
                    rc = emit_sentence(devfp, optarg);
                    if (rc < 0) { fprintf(stderr, "%s: ERR\n", program); }
                }
            }
        }

    }



    if (service == (const char *)0) {

        role = NONE;

    } else if (host == (const char *)0) {

        switch (protocol) {
        case IPV4:
            port = diminuto_ipc4_port(service, "udp");
            if (port == 0) { perror(service); break; }
            sock = diminuto_ipc4_datagram_peer(port);
            if (sock < 0) { perror(service); }
            break;
        case IPV6:
            port = diminuto_ipc6_port(service, "udp");
            if (port == 0) { perror(service); break; }
            sock = diminuto_ipc6_datagram_peer(port);
            if (sock < 0) { perror(service); }
            break;
        default:
            break;
        }
        assert(sock >= 0);

        role = CONSUMER;

    } else {

        switch (protocol) {
        case IPV4:
            ipv4 = diminuto_ipc4_address(host);
            if (diminuto_ipc4_is_unspecified(&ipv4)) { perror(host); break; }
            port = diminuto_ipc4_port(service, "udp");
            if (port == 0) { perror(service); break; }
            sock = diminuto_ipc4_datagram_peer(0);
            if (sock < 0) { perror(service); }
            break;
        case IPV6:
            ipv6 = diminuto_ipc6_address(host);
            if (diminuto_ipc6_is_unspecified(&ipv6)) { perror(host); break; }
            port = diminuto_ipc6_port(service, "udp");
            if (port == 0) { perror(service); break; }
            sock = diminuto_ipc6_datagram_peer(0);
            if (sock < 0) { perror(service); }
            break;
        default:
            break;
        }
        assert(sock >= 0);

        rc = diminuto_ipc_set_nonblocking(sock, !0);
        if (rc < 0) { perror(service); }
        assert(rc >= 0);

        role = PRODUCER;

    }

    if (debug) {
        hazer_debug(stderr);
    }

    rc = hazer_initialize();
    assert(rc == 0);

    if (escape) { fputs("\033[1;1H\033[0J", outfp); }

    while (!0) {

        state = HAZER_STATE_START;

        if (role != CONSUMER) {

        	pps = 0;
        	if (!modemcontrol) {
        		/* Do nothing. */
        	} else if (!carrierdetect) {
        		/* Do nothing. */
        	} else if (state != HAZER_STATE_START) {
        		/* Do nothing. */
        	} else if (diminuto_serial_available(fd) > 0) {
        		/* Do nothing. */
        	} else {
        		rc = diminuto_serial_wait(fd);
        		assert(rc >= 0);
        		pps = 1;
        	}

            while (!0) {
                ch = fgetc(infp);
                prior = state;
                state = hazer_machine(state, ch, buffer, sizeof(buffer), &bb, &ss);
                if (state == HAZER_STATE_END) {
                    break;
                } else if  (state == HAZER_STATE_EOF) {
                    break;
                } else if ((prior != HAZER_STATE_START) && (state == HAZER_STATE_START)) {
                    /* State machine restarted. */
                    fprintf(stderr, "%s: ERR\n", program);
                } else {
                    /* Do nothing. */
                }
            }

            if (state == HAZER_STATE_EOF) {
                fprintf(stderr, "%s: EOF\n", program);
                break;
            }

            assert(state == HAZER_STATE_END);

            size = ss;
            assert(size > 0);

        } else if (protocol == IPV4) {

            size = diminuto_ipc4_datagram_receive(sock, buffer, sizeof(buffer) - 1);
            assert(size > 0);
            buffer[size++] = '\0';

        } else if (protocol == IPV6) {

            size = diminuto_ipc6_datagram_receive(sock, buffer, sizeof(buffer) - 1);
            assert(size > 0);
            buffer[size++] = '\0';

        } else {

            assert(0);

        }

        assert(buffer[0] == '$');
        assert(buffer[size - 1] == '\0');
        assert(buffer[size - 2] == '\n');
        assert(buffer[size - 3] == '\r');
        assert(buffer[size - 6] == '*');

        cs = hazer_checksum(buffer, size);

        rc = hazer_characters2checksum(buffer[size - 5], buffer[size - 4], &ck);
        assert(rc >= 0);

        if (ck != cs) {
            /* Checksum failed. */
            fprintf(stderr, "%s: SUM 0x%02x 0x%02x\n", program, cs, ck);
            continue;
        }

        if (verbose) {
            fputs(buffer, errfp);
            fflush(errfp);
        }

        if (escape) { fputs("\033[1;1H\033[0K", outfp); }
        if (report) { print_sentence(outfp, buffer, size - 1); }

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]),  buffer, size);
        assert(count >= 0);
        assert(vector[count - 1] == (char *)0);
        assert(count <= (sizeof(vector) / sizeof(vector[0])));

        size = hazer_serialize(datagram, sizeof(datagram), vector, count);
        assert(size <= (sizeof(datagram) - 4));
        assert(datagram[size - 1] == '\0');
        assert(datagram[size - 2] == '*');
        cs = hazer_checksum(datagram, size);
        hazer_checksum2characters(cs, &msn, &lsn);
        bb = &datagram[size - 1];
        *(bb++) = msn;
        *(bb++) = lsn;
        *(bb++) = '\r';
        *(bb++) = '\n';
        *(bb++) = '\0';
        size += 4;
        assert(size <= sizeof(datagram));

        if (role == PRODUCER) {
            send_sentence(sock, protocol, &ipv4, &ipv6, port, datagram, size - 1);
        }

        if (escape) { fputs("\033[2;1H\033[0K", outfp); }
        if (report) { print_sentence(outfp, datagram, size - 1); }

        if ((talker = hazer_parse_talker(vector, count)) == HAZER_TALKER_NA) {
            /* Do nothing. */
        } else if (hazer_parse_gga(&position, vector, count) == 0) {
            if (escape) { fputs("\033[3;1H\033[0K", outfp); }
            if (report) { print_position(outfp, "MAP",  &position, pps); }
            if (escape) { fputs("\033[4;1H\033[0K", outfp); }
            if (report) { print_datum(outfp, "GGA",  &position); }
        } else if (hazer_parse_rmc(&position, vector, count) == 0) {
            if (escape) { fputs("\033[3;1H\033[0K", outfp); }
            if (report) { print_position(outfp, "MAP", &position, pps); }
            if (escape) { fputs("\033[4;1H\033[0K", outfp); }
            if (report) { print_datum(outfp, "RMC",  &position); }
        } else if (hazer_parse_gsa(&solution, vector, count) == 0) {
            if (escape) { fputs("\033[5;1H\033[0K", outfp); }
            if (report) { print_active(outfp, "GSA", &solution); }
        } else if (hazer_parse_gsv(&constellation[talker], vector, count) == 0) {
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

    }

    rc = hazer_finalize();
    assert(rc == 0);

    if (sock >= 0) { diminuto_ipc_close(sock); }
    fclose(infp);
    fclose(outfp);
    fclose(errfp);

    return 0;
}
