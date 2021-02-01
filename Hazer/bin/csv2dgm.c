/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes and displays the checksum of an NMEA, UBX, or RTCM message.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Forwards a fixed subset of the CSV output as a datagram in JSON format
 * to a UDP endpoint.
 *
 * Developed for use with Tesoro, my OpenStreetMaps tile server project.
 *
 * USAGE
 * 
 * csv2dgm [ -d ] [ -c | -j | -q | -v | -y | -x ] [ -F FILE ] [ -U HOST:PORT ]
 *
 * EXAMPLE
 *
 * socat -u UDP6-RECV:8080 - & csv2dgm -U localhost:8080 < ./dat/yodel/20200903/vehicle.csv
 *
 * INPUT (CSV)
 *
 *      "neon", 2, 3, 0, 11, 1599145249.632000060, 1599145249.000000000, 39.7943071, -105.1533805, 0., 1710.300, 1688.800, 0., 0.005000, 0., 0., 0., 0., 0., 0., 0., 0, 0.\n
 *
 * OUTPUT (default)
 *
 *      1599145249 39.7943071 -105.1533805 1710.300 20200903T150049Z\n
 *
 * OUTPUT (-c)
 *
 *      1599145249, 39.7943071, -105.1533805, 1710.300, "20200903T150049Z"\n
 *
 * OUTPUT (-j)
 *
 *      { "TIM": 1599145249, "LAT": 39.7943071, "LON": -105.1533805, "MSL": 1710.300, "LBL": "20200903T150049Z" }\n
 *
 * OUTPUT (-q)
 *
 *      ?TIM=1599145249&LAT=39.7943071&LON=-105.1533805&MSL=1710.300&LBL=20200903T150049Z\n
 *
 * OUTPUT (-v)
 *
 *      TIM=1599145249; LAT=39.7943071; LON=-105.1533805; MSL=1710.300; LBL="20200903T150049Z"\n
 *
 * OUTPUT (-y)
 *
 *      TIM: 1599145249\nLAT: 39.7943071\nLON: -105.1533805\nMSL: 1710.3\n LBL: 20200903T150049Z\n
 *
 * OUTPUT (-x)
 *
 *      <?xml version="1.0" encoding="UTF-8" ?><TIM>1599145249</TIM><LAT>39.7943071</LAT><LON>-105.1533805</LON><MSL>1710.300</MSL><LBL>20200903T150049Z</LBL>\n
 *
 * REFERENCES
 *
 * https://github.com/coverclock/com-diag-tesoro
 *
 * https://jsonformatter.org
 */

#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_ipc.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char * expand(char * to, const char * from, size_t tsize, size_t fsize) {
    (void)diminuto_escape_expand(to, from, tsize, fsize, (const char *)0);
    return to;
}

int main(int argc, char * argv[])
{
    int xc = 1;
    const char * program = (const char *)0;
    enum Type { CSV = 'c', DEFAULT = 'd', HTML = 'h',  JSON = 'j', QUERY='q', VAR = 'v', YAML = 'y', XML = 'x', } type = DEFAULT;
    enum Tokens { TIM = 6, LAT = 7, LON = 8, MSL = 10, };
    int opt = -1;
    int error = 0;
    int debug = 0;
    int sock = -1;
    int rc = -1;
    int ii = -1;
    diminuto_ipc_endpoint_t endpoint = { 0, };
    char * endpointname = (char *)0;
    char * filename = (char *)0;
    char * token[23] = { 0, };
    char * here = (char *)0;
    char * temp = (char *)0;
    const char * format = (const char *)0;
    FILE * fp = (FILE *)0;
    size_t length = 0;
    ssize_t size = 0;
    char input[1024] = { '\0', };
    char output[1024] = { '\0', };
    char buffer[1024] = { '\0', };
    diminuto_ipv4_buffer_t ipv4buffer = { '\0', }; \
    diminuto_ipv6_buffer_t ipv6buffer = { '\0', }; \
    diminuto_sticks_t ticks = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    diminuto_ticks_t fraction = 0;
    extern char * optarg;
    extern int optind;
    extern int opterr;
    extern int optopt;

    do {

        diminuto_log_setmask();

        /*
         * Parse the command line.
         */

        program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

        while ((opt = getopt(argc, argv, "U:F:cdhjqvyx")) >= 0) {
            switch (opt) {
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
            case 'U':
                endpointname = optarg;
                if (
                        (diminuto_ipc_endpoint(endpointname, &endpoint) != 0) ||
                        ((endpoint.type != DIMINUTO_IPC_TYPE_IPV4) &&
                            (endpoint.type != DIMINUTO_IPC_TYPE_IPV6)) ||
                        ((diminuto_ipc4_is_unspecified(&endpoint.ipv4) &&
                            diminuto_ipc6_is_unspecified(&endpoint.ipv6))) ||
                        (endpoint.udp == 0)
                ) {
                    errno = EINVAL;
                    diminuto_perror(endpointname);
                    error = !0;
                }
                break;
            case 'c':
                type = CSV;
                break;
            case 'd':
                debug = !0;
                break;
            case 'h':
                type = HTML;
                break;
            case 'j':
                type = JSON;
                break;
            case 'q':
                type = QUERY;
                break;
            case 'v':
                type = VAR;
                break;
            case 'y':
                type = YAML;
                break;
            case 'x':
                type = XML;
                break;
            case '?':
            default:
                fprintf(stderr, "usage: %s [ -d ] [ -c | -h | -j | | -q | -v | -y | -x ] [ -F FILE ] [ -U HOST:PORT ]\n", program);
                error = !0;
                break;
            }
        }

        if (error) {
            errno = EINVAL;
            diminuto_perror(program);
            break;
        }

        if (!debug) {
            /* Do nothing. */
        } else if (endpointname == (char *)0) {
            /* Do nothing. */
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
            fprintf (stderr, "%s: endpoint=\"%s\"=%s:%u\n", program, endpointname, diminuto_ipc4_address2string(endpoint.ipv4, ipv4buffer, sizeof(ipv4buffer)), endpoint.udp);
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
            fprintf (stderr, "%s: endpoint=\"%s\"=[%s]:%u\n", program, endpointname, diminuto_ipc6_address2string(endpoint.ipv6, ipv6buffer, sizeof(ipv6buffer)), endpoint.udp);
        } else {
            fprintf (stderr, "%s: endpoint=\"%s\"\n", program, endpointname);
        }

        if (!debug) {
            /* Do nothing. */
        } else if (filename == (char *)0) {
            /* Do nothing. */
        } else if (fp != (FILE *)0) {
            fprintf(stderr, "%s: file=\"%s\" fd=%d\n", program, filename, fileno(fp));
        } else {
            fprintf(stderr, "%s: file=\"%s\"\n", program, filename);
        }
 
        /*
         * Create a datagram socket with an ephemeral port number.
         */

        if (endpointname == (char *)0) {
            /* Do nothing. */
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
            sock = diminuto_ipc4_datagram_peer(0);
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
            sock = diminuto_ipc6_datagram_peer(0);
        } else {
            errno = EINVAL;
            diminuto_perror(endpointname);
            break;
        }

        if (endpointname == (char *)0) {
            /* Do nothing. */
        } else if (sock >= 0) {
            /* Do nothing. */
        } else {
            break;
        }

        /*
         * Select the appropriate output format.
         */

        switch (type) {
        case CSV:
            format = "%s, %s, %s, %s, \"%04d%02d%02dT%02d%02d%02dZ\"\n";
            break;
        case HTML:
            format = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta http-equiv=\"Content-Style-Type\" content=\"text/css\"></head><body><h1>TIM</h1><p>%s</p><h1>LAT</h1><p>%s</p><h1>LON</h1><p>%s</p><h1>MSL</h1><p>%s</p><h1>LBL</h1><p>%04d%02d%02dT%02d%02d%02dZ</p></body></html>\n";
            break;
        case JSON:
            format = "{ \"TIM\": %s, \"LAT\": %s, \"LON\": %s, \"MSL\": %s, \"LBL\": \"%04d%02d%02dT%02d%02d%02dZ\" }\n";
            break;
        case QUERY:
            format = "?TIM=%s&LAT=%s&LON=%s&MSL=%s&LBL=%04d%02d%02dT%02d%02d%02dZ\n";
            break;
        case VAR:
            format = "TIM=%s; LAT=%s; LON=%s; MSL=%s; LBL=\"%04d%02d%02dT%02d%02d%02dZ\"\n";
            break;
        case YAML:
            format = "TIM: %s\nLAT: %s\nLON: %s\nMSL: %s\nLBL: %04d%02d%02dT%02d%02d%02dZ\n";
            break;
        case XML:
            format = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><TIM>%s</TIM><LAT>%s</LAT><LON>%s</LON><MSL>%s</MSL><LBL>%04d%02d%02dT%02d%02d%02dZ</LBL>\n";
            break;
        default:
            format = "%s %s %s %s %04d%02d%02dT%02d%02d%02dZ\n";
            break;
        }

        /*
         * Install the signal handlers.
         */

        if (diminuto_interrupter_install(0) < 0) {
            break;
        }

        if (diminuto_pipe_install(0) < 0) {
            break;
        }

        if (diminuto_terminator_install(0) < 0) {
            break;
        }

        /*
         * Enter the work loop.
         */

        for (;;) {

            if (diminuto_interrupter_check()) {
                if (debug) { fprintf(stderr, "%s: SIGINT!\n", program); }
                break;
            }

            if (diminuto_pipe_check()) {
                if (debug) { fprintf(stderr, "%s: SIGPIPE!\n", program); }
                break;
            }

            if (diminuto_terminator_check()) {
                if (debug) { fprintf(stderr, "%s: SIGTERM!\n", program); }
                break;
            }

            /*
             * Read an entire line terminated by a newline.
             */

            if (fgets(input, sizeof(input), stdin) == (char *)0) {
                xc = 0;
                break;
            }
            input[sizeof(input) - 1] = '\0';

            /*
             * Parse the input line into tokens.
             */

            for (ii = 0; ii < diminuto_countof(token); ++ii) {
                if (ii == 0) {
                    token[ii] = strtok_r(input, ", ", &here);
                } else if (ii == (diminuto_countof(token) - 1)) {
                    token[ii] = strtok_r(here, "\n", &here);
                } else {
                    token[ii] = strtok_r(here, ", ", &here);
                }
                if (token[ii] == (char *)0) {
                    break;
                }
                if (debug) {
                    fprintf(stderr, "%s: token[%d]=\"%s\"\n", program, ii, token[ii]);
                }
            }

            /*
             * If there aren't the right number of tokens, try again.
             */

            if (ii != diminuto_countof(token)) {
                errno = EIO;
                diminuto_perror("strtok_r");
                continue;
            }

            /*
             * If the first token looks like a column header for the
             * CSV data, try again.
             */

            if (strncmp(token[0], "NAM, ", sizeof("NAM, ")  - 1) == 0) {
                continue;
            }

            /*
             * If the first token doesn't look like a valid value for
             * that CSV field, try again.
             */

            length = strnlen(token[0], 14);
            if (length < 2) {
                continue;
            } else if ((token[0][0] == '"') && (token[0][length - 1] == '"')) {
                /* Do nothing. */
            } else {
                continue;
            }

            /*
             * Truncate the fractional portion of the TIM field because it
             * should always be all zeros.
             */

            ticks = strtoull(token[TIM], &here, 10);
            if ((here == (char *)0) || (*here != '.')) {
                errno = EINVAL;
                diminuto_perror(token[TIM]);
                continue;
            }
            *here = '\0';

            /*
             * A UTC timestamp is generated mostly so that the farend can use
             * it as a label. The format is {YYYY}{MM}{DD}T{HH}{MM}{SS}Z.
             */

            ticks *= diminuto_time_frequency();
            rc = diminuto_time_zulu(ticks, &year, &month, &day, &hour, &minute, &second, &fraction);
            if (rc != 0) {
                errno = EINVAL;
                diminuto_perror(token[TIM]);
                continue;
            }

            /*
             * Generate an output line using specific fields.
             */

            snprintf(output, sizeof(output), format, token[TIM], token[LAT], token[LON], token[MSL], year, month, day, hour, minute, second);
            output[sizeof(output) - 1] = '\0';
            length = strnlen(output, sizeof(output));

            if (debug) { fprintf(stderr, "%s: output=\"%s\"\n", program, expand(buffer, output, sizeof(buffer), length)); }

            /*
             * Send the output line as an IPv4 or IPv6 datagram.
             */

            if (sock < 0) {
                /* Do nothing. */
            } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
                size = diminuto_ipc4_datagram_send(sock, output, length, endpoint.ipv4, endpoint.udp);
            } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
                size = diminuto_ipc6_datagram_send(sock, output, length, endpoint.ipv6, endpoint.udp);
            } else {
                /* Do nothing. */
            }

            if (sock < 0) {
                /* Do nothing. */
            } else if (size > 0) {
                /* Do nothing. */
            } else {
                break;
            }

            /*
             * Write the output line to the file.
             */

            if (fp != (FILE *)0) {
                fputs(output, fp);
                fflush(fp);
            }

            if (fp == (FILE *)0) {
                /* Do nothing. */
            } else if (fp == stdout) {
                /* Do nothing. */
            } else if ((fp = diminuto_observation_commit(fp, &temp)) != (FILE *)0) {
                break;
            } else if ((fp = diminuto_observation_create(filename, &temp)) == (FILE *)0) {
                break;
            } else {
                /* Do nothing. */
            }

        }

        /*
         * Upon EOF on the input stream, send a zero length datagram and
         * close the socket.
         */

        if (sock < 0) {
            /* Do nothing. */
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
            (void)diminuto_ipc4_datagram_send(sock, "", 0, endpoint.ipv4, endpoint.udp);
            (void)diminuto_ipc4_close(sock);
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
            (void)diminuto_ipc6_datagram_send(sock, "", 0, endpoint.ipv6, endpoint.udp);
            (void)diminuto_ipc6_close(sock);
        } else {
            /* Do nothing. */
        }

        if (fp == (FILE *)0) {
            /* Do nothing. */
        } else if (fp == stdout) {
            /* Do nothing. */
        } else {
            fp = diminuto_observation_discard(fp, &temp);
        }

    } while (0);

    return xc;
}
