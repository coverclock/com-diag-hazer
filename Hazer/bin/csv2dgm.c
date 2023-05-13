/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Forwards a subset of CSV output into other formats as a datagram.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Forwards a fixed subset of the CSV output as a datagram in JSON format
 * to a UDP endpoint.
 *
 * Developed for use with Tesoro, an OpenStreetMaps tile server project.
 *
 * See tst/unittest-csv2dgm.sh for examples of all the output formats.
 *
 * USAGE
 * 
 * csv2dgm [ -d ] [ -v ] [ -t ] [ -c | -j | -q | -s | -x | -y ] [ -F FILE ] [ -M MODE ] [ -U HOST:PORT ] [ -D DEVICE [ -b BPS ] [ -
 *
 * EXAMPLE
 *
 * socat -u UDP6-RECV:8080 - &
 * csv2meter < ./dat/yodel/20200903/vehicle.csv | csv2dgm -U localhost:8080 -F Observation.json -M 0644 -j
 *
 * REFERENCES
 *
 * https://github.com/coverclock/com-diag-tesoro
 *
 * https://jsonformatter.org
 */

/*******************************************************************************
 * DEPENDENCIES
 ******************************************************************************/

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_ipc.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_types.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*******************************************************************************
 * SYMBOLS
 ******************************************************************************/

#define TIMESTAMP "%04d-%02d-%02dT%02d:%02d:%02dZ"

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static const char FORMAT_CSV[] =
    "\"%s\", "
    "%s, "
    "%s, "
    "%s, "
    "%s, "
    "%s, "
    "\"" TIMESTAMP "\""
    "\n";

static const char FORMAT_HTML[] =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\">"
    "<html>"
        "<head>"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"
            "<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">"
        "</head>"
        "<body>"
            "<h1>NAM</h1><p>%s</p>"
            "<h1>NUM</h1><p>%s</p>"
            "<h1>TIM</h1><p>%s</p>"
            "<h1>LAT</h1><p>%s</p>"
            "<h1>LON</h1><p>%s</p>"
            "<h1>MSL</h1><p>%s</p>"
            "<h1>LBL</h1><p>" TIMESTAMP "</p>"
        "</body>"
    "</html>"
    "\n";

static const char FORMAT_JSON[] =
    "{ "
        "\"NAM\": \"%s\", "
        "\"NUM\": %s, "
        "\"TIM\": %s, "
        "\"LAT\": %s, "
        "\"LON\": %s, "
        "\"MSL\": %s, "
        "\"LBL\": \"" TIMESTAMP "\" "
    "}"
    "\n";

static const char FORMAT_QUERY[] =
    "?NAM=%s"
    "&NUM=%s"
    "&TIM=%s"
    "&LAT=%s"
    "&LON=%s"
    "&MSL=%s"
    "&LBL=" TIMESTAMP 
    "\n";

static const char FORMAT_SHELL[] =
    "NAM=\"%s\"; "
    "NUM=%s; "
    "TIM=%s; "
    "LAT=%s; "
    "LON=%s; "
    "MSL=%s; "
    "LBL=\"" TIMESTAMP "\""
    "\n";

static const char FORMAT_YAML[] =
    "NAM: %s\n"
    "NUM: %s\n"
    "TIM: %s\n"
    "LAT: %s\n"
    "LON: %s\n"
    "MSL: %s\n"
    "LBL: " TIMESTAMP "\n"
    "\n";

static const char FORMAT_XML[] =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
    "<NAM>%s</NAM>"
    "<NUM>%s</NUM>"
    "<TIM>%s</TIM>"
    "<LAT>%s</LAT>"
    "<LON>%s</LON>"
    "<MSL>%s</MSL>"
    "<LBL>" TIMESTAMP "</LBL>"
    "\n";

static const char FORMAT_DEFAULT[] =
    "%s "
    "%s "
    "%s "
    "%s "
    "%s "
    "%s "
    TIMESTAMP
    "\n";

/*******************************************************************************
 * HELPERS
 ******************************************************************************/

/**
 * Return true if the argument string is numeric.
 * @param str points to the argument string.
 * @return true if numeric, false otherwise.
 */
static int numeric(const char * str)
{
    char * end = (char *)0;

    errno = 0;
    (void)strtod(str, &end);

    return ((*str != '\0') && (errno == 0) && (end != (char *)0) && (*end == '\0'));
}

/**
 * Change the coding of an empty CSV field from "0." to "0" for those
 * output formats that have issues with "0.". I'm lookin' at you, JSON.
 * @param str is a pointer to the input string.
 * @return pointer to the output string which may be str or a constant.
 */
static char * empty(char * str)
{
    return ((strcmp(str, "0.") == 0) ? "0" : str);
}

/**
 * Expand special characters into C=style escape sequences (for display).
 * @param to points to the destination buffer.
 * @param from points to the source buffer.
 * @param tsize is the size of the destination buffer in bytes.
 * @param fsize is the size of the source buffer in bytes.
 * @return a pointer to the start of the destination buffer.
 */
static const char * expand(char * to, const char * from, size_t tsize, size_t fsize)
{
    (void)diminuto_escape_expand(to, from, tsize, fsize, "\"");
    return to;
}

/*******************************************************************************
 * MAIN PROGRAM
 ******************************************************************************/

int main(int argc, char * argv[])
{
    int xc = 1;
    const char * program = (const char *)0;
    enum Type { CSV = 'c', DEFAULT = 'd', HTML = 'h',  JSON = 'j', QUERY='q', SHELL = 's', XML = 'x', YAML = 'y', } type = DEFAULT;
    enum Tokens { NAM = 0, NUM = 1, TIM = 6, LAT = 7, LON = 8, MSL = 10, };
    int opt = -1;
    int error = 0;
    int debug = 0;
    int unempty = 0;
    int verbose = 0;
    int out = 0;
    int sock = -1;
    int fd = -1;
    int rc = -1;
    int ii = -1;
    mode_t mode = COM_DIAG_DIMINUTO_OBSERVATION_MODE;
    diminuto_ipc_endpoint_t endpoint = { 0, };
    char * device = (char *)0;
    char * endpointname = (char *)0;
    char * filename = (char *)0;
    char * token[23] = { 0, };
    char * here = (char *)0;
    char * temp = (char *)0;
    char * end = (char *)0;
    const char * format = (const char *)0;
    FILE * fp = (FILE *)0;
    size_t length = 0;
    ssize_t size = 0;
    char input[1024] = { '\0', };
    char output[1024] = { '\0', };
    char buffer[1024] = { '\0', };
    diminuto_ipv4_buffer_t ipv4buffer = { '\0', }; \
    diminuto_ipv6_buffer_t ipv6buffer = { '\0', }; \
    int bps = 9600;
    int databits = 8;
    int stopbits = 1;
    int paritybit = 0;
    int modemcontrol = 0;
    int rtscts = 0;
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

        while ((opt = getopt(argc, argv, "1278D:F:M:U:b:cedhjmnoqrstvxy")) >= 0) {
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
            case 'D':
                device = optarg;
                break;
            case 'F':
                filename = optarg;
                break;
            case 'M':
                mode = strtoul(optarg, &end, 0) & 0777;
                if ((end == (char *)0) || (*end != '\0')) {
                    errno = EINVAL;
                    diminuto_perror(optarg);
                    error = !0;
                }
                break;
            case 'U':
                endpointname = optarg;
                break;
            case 'b':
                bps = strtoul(optarg, &end, 0);
                if ((end == (char *)0) || (*end != '\0')) {
                    errno = EINVAL;
                    diminuto_perror(optarg);
                    error = !0;
                }
                break;
            case 'c':
                type = CSV;
                break;
            case 'd':
                debug = !0;
                break;
            case 'e':
                paritybit = 2;
                break;
            case 'h':
                type = HTML;
                break;
            case 'j':
                type = JSON;
                unempty = !0;
                break;
            case 'm':
                modemcontrol = !0;
                break;
            case 'o':
                paritybit = 1;
                break;
            case 'n':
                paritybit = 0;
                break;
            case 'q':
                type = QUERY;
                break;
            case 'r':
                rtscts = !0;
                break;
            case 's':
                type = SHELL;
                break;
            case 't':
                out = !0;
                break;
            case 'v':
                verbose = !0;
                break;
            case 'x':
                type = XML;
                break;
            case 'y':
                type = YAML;
                break;
            default:
            case '?':
                fprintf(stderr, "usage: %s [ -d ] [ -v ] [ -c | -h | -j | | -q | -s | -x | -y ] [ -t ] [ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -1 | -2 ] [ -e | -o | -n ] [ -m ] [ -r ] ] [ -F FILE ] [ -M MODE ] [ -U HOST:PORT ]\n", program);
                fprintf(stderr, "       -1              Set DEVICE to 1 stop bit.\n");
                fprintf(stderr, "       -2              Set DEVICE to 2 stop bits.\n");
                fprintf(stderr, "       -7              Set DEVICE to 7 data bits.\n");
                fprintf(stderr, "       -8              Set DEVICE to 8 data bits.\n");
                fprintf(stderr, "       -D DEVICE       Write datagram to DEVICE.\n");
                fprintf(stderr, "       -F FILE         Save latest datagram in observation FILE.\n");
                fprintf(stderr, "       -M MODE         Set FILE mode to MODE.\n");
                fprintf(stderr, "       -U HOST:PORT    Forward datagrams to HOST:PORT.\n");
                fprintf(stderr, "       -b BPS          Set DEVICE to BPS bits per second.\n");
                fprintf(stderr, "       -c              Emit CSV.\n");
                fprintf(stderr, "       -d              Enable debug output.\n");
                fprintf(stderr, "       -e              Set DEVICE to even parity.\n");
                fprintf(stderr, "       -h              Emit HTML.\n");
                fprintf(stderr, "       -j              Emit JSON.\n");
                fprintf(stderr, "       -o              Set DEVICE to odd parity.\n");
                fprintf(stderr, "       -m              Set DEVICE to use modem control.\n");
                fprintf(stderr, "       -n              Set DEVICE to no parity.\n");
                fprintf(stderr, "       -q              Emit URL Query.\n");
                fprintf(stderr, "       -r              Set DEVICE to use hardware flow control.\n");
                fprintf(stderr, "       -s              Emit Shell commands.\n");
                fprintf(stderr, "       -t              Write to standard output.\n");
                fprintf(stderr, "       -v              Enable verbose output.\n");
                fprintf(stderr, "       -x              Emit XML.\n");
                fprintf(stderr, "       -y              Emit YAML.\n");
                error = !0;
                break;
            }
        }

        if (error) {
            break;
        }

        if (endpointname == (char *)0) {
            /* Do nothing. */
        } else if (diminuto_ipc_endpoint(endpointname, &endpoint) != 0) {
            diminuto_perror(endpointname);
            break;
        } else if (
            ((endpoint.type != DIMINUTO_IPC_TYPE_IPV4) &&
                (endpoint.type != DIMINUTO_IPC_TYPE_IPV6)) ||
            ((diminuto_ipc4_is_unspecified(&endpoint.ipv4) &&
                diminuto_ipc6_is_unspecified(&endpoint.ipv6))) ||
            (endpoint.udp == 0)
        ) {
            errno = EINVAL;
            diminuto_perror(endpointname);
            break;
        }

        if (!debug) {
            /* Do nothing. */
        } else if (endpointname == (char *)0) {
            /* Do nothing. */
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV4) {
            fprintf (stderr, "%s: endpoint4=\"%s\"=%s:%u\n", program, endpointname, diminuto_ipc4_address2string(endpoint.ipv4, ipv4buffer, sizeof(ipv4buffer)), endpoint.udp);
        } else if (endpoint.type == DIMINUTO_IPC_TYPE_IPV6) {
            fprintf (stderr, "%s: endpoint6=\"%s\"=[%s]:%u\n", program, endpointname, diminuto_ipc6_address2string(endpoint.ipv6, ipv6buffer, sizeof(ipv6buffer)), endpoint.udp);
        } else {
            /* Do nothing. */
        }

        if (filename == (char *)0) {
            /* Do nothing. */
        } else if (strcmp(filename, "-") == 0) {
            fp = stdout;
        } else if ((fp = diminuto_observation_create_generic(filename, &temp, mode)) == (FILE *)0) {
            diminuto_perror(filename);
            break;
        } else {
            /* Do nothing. */
        }

        if (!debug) {
            /* Do nothing. */
        } else if (filename == (char *)0) {
            /* Do nothing. */
        } else {
            fprintf(stderr, "%s: file=\"%s\" mode=0%03o fd=%d\n", program, filename, mode, fileno(fp));
        }

        if (device == (char *)0) {
            /* Do nothing. */
        } else if ((fd = open(device, O_WRONLY)) < 0) {
            diminuto_perror(device);
            break;
        } else if (!diminuto_serial_valid(fd)) {
            /* Do nothing. */
        } else if (diminuto_serial_set(fd, bps, databits, paritybit, stopbits, modemcontrol, 0, rtscts) < 0) {
            break;
        } else if (diminuto_serial_raw(fd) < 0) {
            break;
        } else {
            /* Do nothing. */
        }

        if (!debug) {
            /* Do nothing. */
        } else if (device == (char *)0) {
            /* Do nothing. */
        } else {
            fprintf(stderr, "%s: device=\"%s\" bps=%d databits=%d paritybit=%d stopbits=%d modemcontrol=%d rtscts=%d fd=%d\n", program, device, bps, databits, paritybit, stopbits, modemcontrol, rtscts, fd);
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
            format = FORMAT_CSV;
            break;
        case HTML:
            format = FORMAT_HTML;
            break;
        case JSON:
            format = FORMAT_JSON;
            break;
        case QUERY:
            format = FORMAT_QUERY;
            break;
        case SHELL:
            format = FORMAT_SHELL;
            break;
        case YAML:
            format = FORMAT_YAML;
            break;
        case XML:
            format = FORMAT_XML;
            break;
        default:
            format = FORMAT_DEFAULT;
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
            length = strnlen(input, sizeof(input));
            diminuto_contract((length > 0) && (input[length - 1] == '\n'));

            if (verbose) { fprintf(stderr, "%s: input=\"%s\"\n", program, expand(buffer, input, sizeof(buffer), length)); }

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
                if (verbose) {
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

            if (strncmp(token[0], "NAM", sizeof("NAM")  - 1) == 0) {
                continue;
            }

            /*
             * If the first token doesn't look like a valid value for
             * that CSV field, try again.
             */

            length = strnlen(token[NAM], 14);
            if ((length < (sizeof("\"?\"") - 1)) || (token[NAM][0] != '"') || (token[NAM][length - 1] != '"') || (strchr(token[NAM], ' ') != (char *)0)) {
                errno = EINVAL;
                diminuto_perror(token[NAM]);
                continue;
            }

            /*
             * Strip off the double quotes (which CSV expects) from the
             * NAM field. (We know they're there because of the check above.)
             */

            token[NAM][length - 1] = '\0';
            token[NAM] += 1;

            /*
             * If the numeric tokens aren't, try again.
             */

            if (!numeric(token[NUM])) {
                errno = EINVAL;
                diminuto_perror(token[NUM]);
                continue;
            } else if (!numeric(token[TIM])) {
                errno = EINVAL;
                diminuto_perror(token[TIM]);
                continue;
            } else if (!numeric(token[LAT])) {
                errno = EINVAL;
                diminuto_perror(token[LAT]);
                continue;
            } else if (!numeric(token[LON])) {
                errno = EINVAL;
                diminuto_perror(token[LON]);
                continue;
            } else if (!numeric(token[MSL])) {
                errno = EINVAL;
                diminuto_perror(token[MSL]);
                continue;
            } else {
                /* Do nothing. */
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
             * If the chosen output format has issues with "0." (JSON!),
             * change it to something that's more palatoble. "0." is a
             * special coding in Hazer gpstool for numeric fields that
             * are empty, versus fields that are legitimately zero.
             */

            if (unempty) {
                token[LAT] = empty(token[LAT]);
                token[LON] = empty(token[LON]);
                token[MSL] = empty(token[MSL]);
            }

            /*
             * Generate an output line using specific fields.
             */

            snprintf(output, sizeof(output), format, token[NAM], token[NUM], token[TIM], token[LAT], token[LON], token[MSL], year, month, day, hour, minute, second);
            output[sizeof(output) - 1] = '\0';
            length = strnlen(output, sizeof(output));
            diminuto_contract((length > 0) && (output[length - 1] == '\n'));

            if (verbose) { fprintf(stderr, "%s: output=\"%s\"\n", program, expand(buffer, output, sizeof(buffer), length)); }

            /*
             * Send the output line as an IPv4 or IPv6 datagram on the socket.
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
            } else if (size == length) {
                /* Do nothing. */
            } else if (size == 0) {
                /* Should be impossible with UDP. */
                fprintf(stderr, "diminuto_ipc_datagram_send: SHUTDOWN\n");
                break;
            } else if (size < length) {
                /* Should be impossible with UDP. */
                fprintf(stderr, "diminuto_ipc_datagram_send: SHORT\n");
                break;
            } else {
                break;
            }

            /*
             * Write the output line to the device.
             */

            if (fd < 0) {
                /* Do nothing. */
            } else if ((size = diminuto_fd_write(fd, output, length)) == length) {
                /* Do nothing. */
            } else if (size >= 0) {
                fprintf(stderr, "diminuto_fd_write: EOF\n");
                break;
            } else {
                break;
            }

            /*
             * Write the output line to the observation file and commit it.
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
            } else if ((fp = diminuto_observation_create_generic(filename, &temp, mode)) == (FILE *)0) {
                break;
            } else {
                /* Do nothing. */
            }

            /*
             * Write the output line to standard output.
             */

            if (out) {
                fputs(output, stdout);
                fflush(stdout);
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

        if (fd < 0) {
            /* Do nothing. */
        } else if (close(fd) < 0) {
            diminuto_perror("close");
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
