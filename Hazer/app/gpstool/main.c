/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool main program.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
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
 *      -H headless.out -t 10 \
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

#include "com/diag/diminuto/diminuto_absolute.h"
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_coherentsection.h"
#include "com/diag/diminuto/diminuto_command.h"
#include "com/diag/diminuto/diminuto_containerof.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_criticalsection.h"
#include "com/diag/diminuto/diminuto_daemon.h"
#include "com/diag/diminuto/diminuto_delay.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_fd.h"
#include "com/diag/diminuto/diminuto_file.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_hangup.h"
#include "com/diag/diminuto/diminuto_interrupter.h"
#include "com/diag/diminuto/diminuto_ipc4.h"
#include "com/diag/diminuto/diminuto_ipc6.h"
#include "com/diag/diminuto/diminuto_lock.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/diminuto/diminuto_pin.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_thread.h"
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
#include <locale.h>
#include <wchar.h>
#include <ctype.h>
#include "com/diag/hazer/hazer_release.h"
#include "com/diag/hazer/hazer_revision.h"
#include "com/diag/hazer/hazer_vintage.h"
#include "buffer.h"
#include "datagram.h"
#include "emit.h"
#include "sync.h"
#include "helpers.h"
#include "print.h"
#include "threads.h"
#include "types.h"
#include "constants.h"
#include "globals.h"

/**
 * Run the main program.
 * @param argc is the number of tokens on the command line argument list.
 * @param argv contains the tokens on the command line argument list.
 * @return the exit value of the main program.
 */
int main(int argc, char * argv[])
{
    int xc = 0;
    /*
     * Command line options and parameters with defaults.
     */
    const char * source = (const char *)0;
    const char * sink = (const char *)0;
    const char * strobe = (const char *)0;
    const char * logging = (const char *)0;
    const char * headless = (const char *)0;
    const char * arp = (const char *)0;
    const char * tracing = (const char *)0;
    const char * identity = (const char *)0;
    int opt = -1;
    int debug = 0;
    int verbose = 0;
    int escape = 0;
    int report = 0;
    int process = 0;
    int strobepin = (((int)1)<<((sizeof(int)*8)-1));
    int ppspin = (((int)1)<<((sizeof(int)*8)-1));
    int expire = 0;
    int unknown = 0;
    int serial = 0;
    int daemon = 0;
    timeout_t slow = 0;
    timeout_t timeout = HAZER_GNSS_SECONDS;
    timeout_t keepalive = TUMBLEWEED_KEEPALIVE_SECONDS;
    timeout_t frequency = 1;
    timeout_t postpone = 0;
    timeout_t check = -1;
    /*
     * Configuration command variables.
     */
    command_t * command = (command_t *)0;
    diminuto_list_t * command_node = (diminuto_list_t *)0;
    diminuto_list_t command_list = DIMINUTO_LIST_NULLINIT(&command_list);
    uint8_t * command_string = (uint8_t *)0;
    uint8_t * command_buffer = (uint8_t *)0;
    ssize_t command_size = 0;
    ssize_t command_length = 0;
    ssize_t command_total = 0;
    /*
     * FILE pointer variables.
     */
    FILE * in_fp = stdin;
    FILE * out_fp = stdout;
    FILE * dev_fp = (FILE *)0;
    FILE * sink_fp = (FILE *)0;
    FILE * log_fp = (FILE *)0;
    FILE * strobe_fp = (FILE *)0;
    FILE * pps_fp = (FILE *)0;
    FILE * trace_fp = (FILE *)0;
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
    diminuto_thread_t thread = DIMINUTO_THREAD_INITIALIZER((diminuto_thread_function_t *)0);
    diminuto_thread_t * threadp = (diminuto_thread_t *)0;
    int threadrc = -1;
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
    yodel_ubx_mon_comms_t ports = YODEL_UBX_MON_COMMS_INITIALIZER;
    yodel_attitude_t attitude = YODEL_ATTITUDE_INITIALIZER;
    yodel_odometer_t odometer = YODEL_ODOMETER_INITIALIZER;
    yodel_posveltim_t posveltim = YODEL_POSVELTIM_INITIALIZER;
    int acknakpending = 0;
    int nakquit = 0;
    int nominal = 0;
    /*
     * RTCM state databases.
     */
    tumbleweed_message_t kinematics = TUMBLEWEED_MESSAGE_INITIALIZER;
    tumbleweed_updates_t updates = TUMBLEWEED_UPDATES_INITIALIZER;
    /*
     * Time keeping variables.
     */
    diminuto_sticks_t delay = 0;
    diminuto_sticks_t elapsed = 0;
    diminuto_sticks_t epoch = 0;
    diminuto_sticks_t fix = -1;
    diminuto_sticks_t timetofirstfix = -1;
    seconds_t expiration_was = 0;
    seconds_t expiration_now = 0;
    seconds_t display_last = 0;
    seconds_t keepalive_last = 0;
    seconds_t trace_last = 0;
    seconds_t command_last = 0;
    seconds_t check_last = 0;
    /*
     * I/O buffer variables.
     */
    void * io_buffer = (void *)0;
    size_t io_size = BUFSIZ;
    size_t io_maximum = 0;
    size_t io_total = 0;
    ssize_t io_available = 0;
    ssize_t io_peak = 0;
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
    int checkpoint = 0;
    /**
     * Control variables.
     */
    int eof = 0;        /** If true then the input stream hit end of file. */
    int sync = 0;       /** If true then the input stream is synchronized. */
    int frame = 0;      /** If true then the input stream is at frame start. */
    int refresh = !0;   /** If true then the display needs to be refreshed. */
    int trace = 0;      /** If true then the trace needs to be emitted. */
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
    size_t sz = 0;
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
    static const char OPTIONS[] = "1278B:C:D:EF:G:H:I:KL:MN:O:PRS:T:U:VW:XY:Z:b:cdef:g:hi:k:lmnop:st:uvxw:y:?";

    /**
     ** PREINITIALIZATION
     **/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;
    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);
    diminuto_log_setmask();

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case '1':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            stopbits = 1;
            serial = !0;
            break;
        case '2':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            stopbits = 2;
            serial = !0;
            break;
        case '7':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            databits = 7;
            serial = !0;
            break;
        case '8':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            databits = 8;
            serial = !0;
            break;
        case 'B':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            io_size = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (io_size < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'C':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            sink = optarg;
            break;
        case 'D':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            device = optarg;
            break;
        case 'E':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            report = !0;
            escape = !0;
            process = !0;
            break;
        case 'F':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            slow = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            report = !0;
            process = !0;
            break;
        case 'G':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            remote_option = optarg;
            rc = diminuto_ipc_endpoint(optarg, &remote_endpoint);
            if (remote_endpoint.udp <= 0) { rc = -1; errno = EINVAL; }
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'H':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            report = !0;
            process = !0;
            headless = optarg;
            break;
        case 'I':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            pps = optarg;
            ppspin = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'K':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            readonly = 0;
            direction = OUTPUT;
            break;
        case 'L':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            logging = optarg;
            break;
        case 'M':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            daemon = !0;
            break;
        case 'N':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            arp = optarg;
            break;
        case 'O':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            identity = optarg;
            break;
        case 'P':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            process = !0;
            break;
        case 'R':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            report = !0;
            process = !0;
            break;
        case 'S':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            source = optarg;
            break;
        case 'T':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            tracing = optarg;
            process = !0; /* Have to process trace. */
            break;
        case 'U':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_assert(command != (command_t *)0);
            command->emission = OPT_U;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            process = !0; /* Have to process ACK/NAKs. */
            break;
        case 'V':
            DIMINUTO_LOG_NOTICE("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE, COM_DIAG_HAZER_VINTAGE, COM_DIAG_HAZER_REVISION);
            break;
        case 'W':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_assert(command != (command_t *)0);
            command->emission = OPT_W;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'X':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            expire = !0;
            break;
        case 'Y':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            surveyor_option = optarg;
            rc = diminuto_ipc_endpoint(surveyor_option, &surveyor_endpoint);
            if (surveyor_endpoint.udp <= 0) { rc = -1; errno = EINVAL; }
            if (rc < 0) { diminuto_perror(optarg); error = !0; }
            break;
        case 'Z':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_assert(command != (command_t *)0);
            command->emission = OPT_Z;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'b':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            bitspersecond = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (bitspersecond == 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            serial = !0;
            break;
        case 'c':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            modemcontrol = !0;
            carrierdetect = !0;
            serial = !0;
            break;
        case 'd':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            debug = !0;
            break;
        case 'e':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            paritybit = 2;
            serial = !0;
            break;
        case 'f':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            frequency = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (frequency < 1)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'g':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            remote_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'h':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            rtscts = !0;
            serial = !0;
            break;
        case 'i':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            check = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'k':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            device_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'l':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            modemcontrol = 0;
            serial = !0;
            break;
        case 'm':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            modemcontrol = !0;
            serial = !0;
            break;
        case 'n':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            paritybit = 0;
            serial = !0;
            break;
        case 'o':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            paritybit = 1;
            serial = !0;
            break;
        case 'p':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            strobe = optarg;
            strobepin = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 's':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            xonxoff = !0;
            serial = !0;
            break;
        case 't':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0) || (timeout > HAZER_GNSS_SECONDS)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'u':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            unknown = !0;
            break;
        case 'v':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            verbose = !0;
            break;
        case 'w':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            postpone = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (postpone < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case 'x':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            nakquit = !0;
            break;
        case 'y':
            DIMINUTO_LOG_DEBUG("Option -%c \"%s\"\n", opt, optarg);
            keepalive = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (keepalive < 0)) { errno = EINVAL; diminuto_perror(optarg); error = !0; }
            break;
        case '?':
            DIMINUTO_LOG_DEBUG("Option -%c\n", opt);
            fprintf(stderr, "usage: %s"
                " [ -d ] [ -v ] [ -M ] [ -u ] [ -V ] [ -X ] [ -x ]"
                " [ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] | -S FILE ] [ -B BYTES ]"
                " [ -O FILE ]"
                " [ -C FILE ]"
                " [ -t SECONDS ]"
                " [ -I PIN | -c ] [ -p PIN ]"
                " [ -U STRING ... ] [ -W STRING ... ] [ -Z STRING ... ] [ -w SECONDS ]"
                " [ -R | -E | -H HEADLESS | -P ] [ -F SECONDS ] [ -i SECONDS ]"
                " [ -L LOG ]"
                " [ -G [ IP:PORT | :PORT [ -g MASK ] ] ]"
                " [ -Y [ IP:PORT [ -y SECONDS ] | :PORT ] ]"
                " [ -K [ -k MASK ] ]"
                " [ -N FILE ]"
                " [ -T FILE [ -f SECONDS ] ]"
                          "\n", Program);
            fprintf(stderr, "       -1          Use one stop bit for DEVICE.\n");
            fprintf(stderr, "       -2          Use two stop bits for DEVICE.\n");
            fprintf(stderr, "       -7          Use seven data bits for DEVICE.\n");
            fprintf(stderr, "       -8          Use eight data bits for DEVICE.\n");
            fprintf(stderr, "       -B BYTES    Set the input Buffer size to BYTES bytes.\n");
            fprintf(stderr, "       -C FILE     Catenate input to FILE or named pipe.\n");
            fprintf(stderr, "       -D DEVICE   Use DEVICE for input or output.\n");
            fprintf(stderr, "       -E          Like -R but use ANSI Escape sequences.\n");
            fprintf(stderr, "       -F SECONDS  Set report Frequency to 1/SECONDS, 0 for no delay.\n");
            fprintf(stderr, "       -G IP:PORT  Use remote IP and PORT as dataGram sink.\n");
            fprintf(stderr, "       -G :PORT    Use local PORT as dataGram source.\n");
            fprintf(stderr, "       -H HEADLESS Like -R but writes each iteration to HEADLESS file.\n");
            fprintf(stderr, "       -I PIN      Take 1PPS from GPIO Input PIN (requires -D) (<0 active low).\n");
            fprintf(stderr, "       -K          Write input to DEVICE sinK from datagram source.\n");
            fprintf(stderr, "       -L LOG      Write pretty-printed input to LOG file.\n");
            fprintf(stderr, "       -M          Run in the background as a daeMon.\n");
            fprintf(stderr, "       -N FILE     Use fix FILE to save ARP LLH for subsequeNt fixed mode.\n");
            fprintf(stderr, "       -O FILE     Save process identifier in FILE.\n");
            fprintf(stderr, "       -P          Process incoming data even if no report is being generated.\n");
            fprintf(stderr, "       -R          Print a Report on standard output.\n");
            fprintf(stderr, "       -S FILE     Use source FILE or named pipe for input.\n");
            fprintf(stderr, "       -T FILE     Save the PVT CSV Trace to FILE.\n");
            fprintf(stderr, "       -U STRING   Collapse STRING, append Ubx end matter, write to DEVICE, expect response.\n");
            fprintf(stderr, "       -U ''       Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -V          Log Version in the form of release, vintage, and revision.\n");
            fprintf(stderr, "       -W STRING   Collapse STRING, append NMEA end matter, Write to DEVICE.\n");
            fprintf(stderr, "       -W ''       Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -X          Enable message eXpiration test mode.\n");
            fprintf(stderr, "       -Y IP:PORT  Use remote IP and PORT as keepalive sink and surveYor source.\n");
            fprintf(stderr, "       -Y :PORT    Use local PORT as surveYor source.\n");
            fprintf(stderr, "       -Z STRING   Collapse STRING, write to DEVICE.\n");
            fprintf(stderr, "       -Z ''       Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -X          Enable message eXpiration test mode.\n");
            fprintf(stderr, "       -b BPS      Use BPS bits per second for DEVICE.\n");
            fprintf(stderr, "       -c          Take 1PPS from DCD (requires -D and implies -m).\n");
            fprintf(stderr, "       -d          Display Debug output on standard error.\n");
            fprintf(stderr, "       -e          Use Even parity for DEVICE.\n");
            fprintf(stderr, "       -f SECONDS  Set trace Frequency to 1/SECONDS.\n");
            fprintf(stderr, "       -g MASK     Set dataGram sink mask (NMEA=%u, UBX=%u, RTCM=%u) default NMEA.\n", NMEA, UBX, RTCM);
            fprintf(stderr, "       -h          Use RTS/CTS Hardware flow control for DEVICE.\n");
            fprintf(stderr, "       -i SECONDS  Bypass input check every SECONDS seconds, 0 for always, -1 for never.\n");
            fprintf(stderr, "       -k MASK     Set device sinK mask (NMEA=%u, UBX=%u, RTCM=%u) default NMEA.\n", NMEA, UBX, RTCM);
            fprintf(stderr, "       -l          Use Local control for DEVICE.\n");
            fprintf(stderr, "       -m          Use Modem control for DEVICE.\n");
            fprintf(stderr, "       -o          Use Odd parity for DEVICE.\n");
            fprintf(stderr, "       -p PIN      Assert GPIO outPut PIN with 1PPS (requires -D and -I or -c) (<0 active low).\n");
            fprintf(stderr, "       -n          Use No parity for DEVICE.\n");
            fprintf(stderr, "       -s          Use XON/XOFF (control-Q/control-S) for DEVICE.\n");
            fprintf(stderr, "       -t SECONDS  Timeout GNSS data after SECONDS seconds.\n");
            fprintf(stderr, "       -u          Note Unprocessed input on standard error.\n");
            fprintf(stderr, "       -v          Display Verbose output on standard error.\n");
            fprintf(stderr, "       -w SECONDS  Write STRING to DEVICE no more than every SECONDS seconds.\n");
            fprintf(stderr, "       -x          EXit if a NAK is received.\n");
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
        diminuto_assert(rc == 0);
        Process = getpid();
        DIMINUTO_LOG_NOTICE("Daemon %s %d %d %d %d", Program, rc, (int)Process, (int)getppid(), (int)getsid(Process));
    } else {
        Process = getpid();
    }
    diminuto_assert(Process >= 0);

    DIMINUTO_LOG_NOTICE("Begin");

    if (daemon) {
        size_t commandlength = 0;
        char * commandline = (char *)0;
        size_t commandresult = 0;

        commandlength = diminuto_command_length(argc, (const char **)argv);
        diminuto_assert(commandlength > 0);
        commandline = (char *)malloc(commandlength);
        diminuto_assert(commandline != (char *)0);
        commandresult = diminuto_command_line(argc, (const char **)argv, commandline, commandlength);
        diminuto_assert(commandresult == commandlength);
        DIMINUTO_LOG_INFORMATION("Command \"%s\"\n", commandline);
        free(commandline);
    }

    (void)gethostname(Hostname, sizeof(Hostname));
    Hostname[sizeof(Hostname) - 1] = '\0';
    if (Hostname[0] == '\0') {
        strncpy(Hostname, "hostname", sizeof(Hostname));
    }
    DIMINUTO_LOG_INFORMATION("Hostname \"%s\"\n", Hostname);

    /*
     * Necessary to get stuff like wchar_t and the "%lc" format to work,
     * which we use to display stuff like the degree sign.
     */

    (void)setenv("LC_ALL", "en_US.utf8", 0);
    if ((locale = setlocale(LC_ALL, "")) != (char *)0) {
        DIMINUTO_LOG_INFORMATION("Locale \"%s\"", locale);
    } else {
        DIMINUTO_LOG_WARNING("Locale %p", locale);
    }

    if (identity != (const char *)0) {
        rc = diminuto_lock_file(identity);
        diminuto_assert(rc >= 0);
    }
        

    if (process) {
        DIMINUTO_LOG_NOTICE("Processing");
    }

    /*
     * Are we logging every valid sentence or packet to an output file?
     */

    if (logging == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(logging, "-") == 0) {
        log_fp = stderr;
    } else if ((log_fp = fopen(logging, "ab")) != (FILE *)0) {
        /* Do nothing. */
    } else {
        diminuto_perror(logging);
        diminuto_assert(log_fp != (FILE *)0);
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
        diminuto_assert(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        diminuto_assert(rc >= 0);

        role = PRODUCER;

    } else if (!diminuto_ipc4_is_unspecified(&remote_endpoint.ipv4)) {

        remote_protocol = IPV4;

        remote_fd = diminuto_ipc4_datagram_peer(0);
        diminuto_assert(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        diminuto_assert(rc >= 0);

        role = PRODUCER;

    } else {

        Device = remote_option;

        remote_protocol = IPV6;

        remote_fd = diminuto_ipc6_datagram_peer(remote_endpoint.udp);
        diminuto_assert(remote_fd >= 0);

        rc = diminuto_mux_register_read(&mux, remote_fd);
        diminuto_assert(rc >= 0);

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
        diminuto_assert(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        diminuto_assert(rc >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_assert(rc >= 0);

    } else if (!diminuto_ipc4_is_unspecified(&surveyor_endpoint.ipv4)) {

        /*
         * Sending keepalives and receiving updates via IPv4.
         */

        surveyor_protocol = IPV4;

        surveyor_fd = diminuto_ipc4_datagram_peer(0);
        diminuto_assert(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        diminuto_assert(rc >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_assert(rc >= 0);

    } else {

        /*
         * Receiving updates passively via IPv6 with keepalives disabled.
         */

        surveyor_fd = diminuto_ipc6_datagram_peer(surveyor_endpoint.udp);
        diminuto_assert(surveyor_fd >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_assert(rc >= 0);

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
        int activehigh = !0;

        if (strobepin < 0) {
            activehigh = 0;
            strobepin = -strobepin;
        }

        (void)diminuto_pin_unexport_ignore(strobepin);

        rc = diminuto_pin_export(strobepin);
        diminuto_assert(rc >= 0);

        rc = diminuto_pin_direction(strobepin, !0);
        diminuto_assert(rc >= 0);

        rc = diminuto_pin_active(strobepin, activehigh);
        diminuto_assert(rc >= 0);

        strobe_fp = diminuto_pin_open(strobepin, !0);
        diminuto_assert(strobe_fp != (FILE *)0);

        rc = diminuto_pin_clear(strobe_fp);
        diminuto_assert(rc >= 0);
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
        int activehigh = !0;

        if (ppspin < 0) {
            activehigh = 0;
            ppspin = -ppspin;
        }

        (void)diminuto_pin_unexport_ignore(ppspin);

        rc = diminuto_pin_export(ppspin);
        diminuto_assert(rc >= 0);

        rc = diminuto_pin_direction(ppspin, 0);
        diminuto_assert(rc >= 0);

        rc = diminuto_pin_active(ppspin, activehigh);
        diminuto_assert(rc >= 0);

        rc = diminuto_pin_edge(ppspin, DIMINUTO_PIN_EDGE_BOTH);
        diminuto_assert(rc >= 0);

        pps_fp = diminuto_pin_open(ppspin, 0);
        diminuto_assert(pps_fp != (FILE *)0);

        rc = diminuto_pin_get(pps_fp);
        diminuto_assert(rc >= 0);

        poller.ppsfp = pps_fp;
        poller.strobefp = strobe_fp;
        poller.onepps = 0;
        poller.done = 0;

        threadp = diminuto_thread_init(&thread, gpiopoller);
        diminuto_assert(threadp == &thread);

        threadrc = diminuto_thread_start(&thread, &poller);
        diminuto_assert(threadrc == 0);
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
     * diminuto_assert(3) will fire, and the application will dump core. I do
     * this routinely, alas. Maybe in the future I'll add a check, a delay, and
     * a retry.
     */

    if (device == (const char *)0) {

        /* Do nothing. */

    } else if (strcmp(device, "-") == 0) {

        Device = device;

        in_fp = stdin;

    } else {

        Device = strrchr(device, '/');
        if (Device != (const char *)0) {
            Device += 1;
        } else {
            Device = device;
        }

        dev_fd = open(device, readonly ? O_RDONLY : O_RDWR);
        if (dev_fd < 0) { diminuto_perror(device); }
        diminuto_assert(dev_fd >= 0);

        if (serial) {

            DIMINUTO_LOG_INFORMATION("Device (%d) \"%s\" %s \"%s\" %d %d%c%d%s%s%s\n", dev_fd, device, readonly ? "ro" : "rw", Device, bitspersecond, databits, (paritybit == 0) ? 'N' : ((paritybit % 2) == 0) ? 'E' : 'O', stopbits, modemcontrol ? " modem" : " local", xonxoff ? " xonoff" : "", rtscts ? " rtscts" : "");

            rc = diminuto_serial_set(dev_fd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
            diminuto_assert(rc == 0);

            rc = diminuto_serial_raw(dev_fd);
            diminuto_assert(rc == 0);

        }

        /*
         * Remarkably, below, some USB receivers will work with a mode of "w+"
         * and some will return a fatal I/O error and require "a+". "a+" seems
         * to work in either case. Weird.
         */

        dev_fp = fdopen(dev_fd, readonly ? "r" : "a+");
        if (dev_fp == (FILE *)0) { diminuto_perror(device); }
        diminuto_assert(dev_fp != (FILE *)0);

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

        Device = source;

        in_fp = stdin;

    } else {

        Device = strrchr(source, '/');
        if (Device != (const char *)0) {
            Device += 1;
        } else {
            Device = source;
        }

        if ((in_fp = fopen(source, "r")) != (FILE *)0) {
            /* Do nothing. */
        } else {
            diminuto_perror(source);
            diminuto_assert(in_fp != (FILE *)0);
        }

    }

    if (!serial) {
            DIMINUTO_LOG_INFORMATION("Device (%d) \"%s\" %s \"%s\"\n", dev_fd, device, readonly ? "ro" : "rw", Device);
    }

    /*
     * If we are using some other sink of output (e.g. a file, a FIFO, etc.),
     * open it here.
     */

    if (sink == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(sink, "-") == 0) {
        sink_fp = stdout;
    } else if ((sink_fp = fopen(sink, "a")) != (FILE *)0) {
        /* Do nothing. */
    } else {
        diminuto_perror(sink);
        diminuto_assert(sink_fp != (FILE *)0);
    }

    /*
     * Our input source is either standard input (either implicitly or
     * explicitly), a serial(ish) device, or a file or maybe a FIFO
     * a.k.a. a named pipe, remarkably useful BTW, see mkfifo(1). So
     * now we can get its underlying file descriptor. We also mess around
     * with the input stream standard I/O buffer.
     */

    in_fd = fileno(in_fp);

    serial = diminuto_serial_valid(in_fd);

    rc = diminuto_mux_register_read(&mux, in_fd);
    diminuto_assert(rc >= 0);

    io_buffer = malloc(io_size);
    diminuto_assert(io_buffer != (void *)0);
    rc = setvbuf(in_fp, io_buffer, _IOFBF, io_size);
    diminuto_assert(rc == 0);

    DIMINUTO_LOG_INFORMATION("Buffer (%d) [%zu] [%zu]\n", in_fd, io_size, (size_t)BUFSIZ);

    /*
     * If we are running headless, create our temporary output file using the
     * provided prefix.
     */

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_create(headless, &temporary);
        diminuto_assert(out_fp != (FILE *)0);
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

        threadp = diminuto_thread_init(&thread, dcdpoller);
        diminuto_assert(threadp == &thread);

        threadrc = diminuto_thread_start(&thread, &poller);
        diminuto_assert(threadrc == 0);

    }

    /*
     * If we are saving the track, open the track file.
     */

    if (tracing == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(tracing, "-") == 0) {
        trace_fp = stdout;
        DIMINUTO_LOG_INFORMATION("Tracing enabled\n");
    } else if ((trace_fp = fopen(tracing, "a")) != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Tracing enabled\n");
    } else {
        diminuto_perror(tracing);
        diminuto_assert(trace_fp != (FILE *)0);
    }

    /*
     * Install our signal handlers.
     */

    rc = diminuto_terminator_install(0);
    diminuto_assert(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    diminuto_assert(rc >= 0);

    rc = diminuto_hangup_install(!0);
    diminuto_assert(rc >= 0);

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

    Now = diminuto_time_clock();
    diminuto_assert(Now >= 0);

    (void)diminuto_time_timezone();

    /*
     * How much of each packet do we display? Depends on whether we're doing
     * cursor control or not.
     */

    limitation = (escape || headless) ? LIMIT : UNLIMITED;

    /*
     * Initialize the NMEA (Hazer) and UBX (Yodel) parsers. If you're into this
     * kind of thing, these parsers are effectively a single non-deterministic
     * finite state automata, an FSA that can be in more than one state at a
     * time, with both state machines racing to see who can recognize a valid
     * statement in their own grammar first.
     */

    rc = hazer_initialize();
    diminuto_assert(rc == 0);

    rc = yodel_initialize();
    diminuto_assert(rc == 0);

    rc = tumbleweed_initialize();
    diminuto_assert(rc == 0);

    if (debug) {
        hazer_debug(stderr);
        yodel_debug(stderr);
        tumbleweed_debug(stderr);
    }

    /*
     * Start the clock. For some time periods (e.g. display) we want to
     * delay initially; for others (e.g. keepalive) we do not.
     */

    epoch = diminuto_time_elapsed();

    expiration_now = expiration_was =
        display_last =
            trace_last =
                check_last =
                    command_last = ticktock();

    keepalive_last = 0;

    delay = diminuto_frequency();

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
     * Initialize screen iff we're doing full-screen stuff with
     * ANSI escape sequences.
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

    DIMINUTO_LOG_NOTICE("Start");

    while (!0) {

        /*
         * We keep working until out input goes away (end of file), or until
         * we are interrupted by a SIGINT or terminated by a SIGTERM. We
         * also check for SIGHUP, which checkpoints the headless output.
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
            /*
             * Using SIGHUP is actually a little problematic, since I
             * routinely start gpstool interactively, switch it to the
             * background, and later disconnect my terminal session and
             * let it run.
             */
            DIMINUTO_LOG_INFORMATION("SIGHUP");
            checkpoint = !0;
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
            diminuto_assert(fd >= 0);
        } else if (errno == EINTR) {
            continue;
        } else {
            diminuto_assert(0);
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
                if (ch != EOF) {
                    /* Do nothing. */
                } else if (ferror(in_fp)) {
                    DIMINUTO_LOG_WARNING("ERROR");
                    clearerr(in_fp);
                    continue;
                } else if (feof(in_fp)) {
                    DIMINUTO_LOG_NOTICE("EOF");
                    eof = !0;
                    break;
                } else {
                    DIMINUTO_LOG_ERROR("FAILURE");
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

                    if (verbose) { sync_out(ch); }

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

                    if (isprint(ch)) {
                        DIMINUTO_LOG_WARNING("Sync Lost 0x%016llx 0x%02x '%c'\n", (unsigned long long)io_total, ch, ch);
                    } else {
                        DIMINUTO_LOG_WARNING("Sync Lost 0x%016llx 0x%02x\n", (unsigned long long)io_total, ch);
                    }

                    sync = 0;
                    if (verbose) { sync_out(ch); }

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

                    if (!sync) {
                        DIMINUTO_LOG_NOTICE("Sync NMEA 0x%016llx\n", (unsigned long long)io_total);
                        sync = !0;
                        if (verbose) { sync_in(length); }
                    }

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

                    if (!sync) {
                        DIMINUTO_LOG_NOTICE("Sync UBX 0x%016llx\n", (unsigned long long)io_total);
                        sync = !0;
                        if (verbose) { sync_in(length); }
                    }

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

                    if (!sync) {
                        DIMINUTO_LOG_NOTICE("Sync RTCM 0x%016llx\n", (unsigned long long)io_total);
                        sync = !0;
                        if (verbose) { sync_in(length); }
                    }

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

                    if (sync) {
                        DIMINUTO_LOG_WARNING("Sync Stop 0x%016llx 0x%02x\n", (unsigned long long)io_total, ch);
                        sync = 0;
                        if (verbose) { sync_out(ch); }
                    }

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
                collect(kinematics.number, &updates);

                kinematics.length = surveyor_length;

                kinematics.ticks = timeout;
                refresh = !0;

                DIMINUTO_LOG_DEBUG("Surveyor RTCM [%zd] [%zd] [%zd] <%d>\n", surveyor_total, surveyor_size, surveyor_length, kinematics.number);

                if (verbose) { fputs("NET:\n", stderr); diminuto_dump(stderr, &surveyor_buffer, surveyor_total); }
                write_buffer(dev_fp, surveyor_buffer.payload.rtcm, surveyor_length);

            }

        } else {

            /*
             * The multiplexor returned a file descriptor which was not one we
             * recognize; that should be impossible.
             */

            DIMINUTO_LOG_ERROR("Multiplexor Fail [%d] (%d) <%d %d %d>\n", ready, fd, dev_fd, remote_fd, surveyor_fd);
            diminuto_assert(0);

        }

        /*
         * If one of the input sources indicated end of file, we're done.
         * (This may be the only time I've found a legitimate use for a goto.)
         */

        if (eof) { goto render; }

        /*
         * At this point, either we have a buffer with a complete and validated
         * NMEA sentence, UBX packet, or RTCM message ready to process, acquired
         * either from a state machine or a socket, or there is no input pending
         * and maybe this is a good time to update the display. It is also a
         * good time to make a note of the current system (not GPS) time.
         */

        Now = diminuto_time_clock();
        diminuto_assert(Now >= 0);

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
        } else if (acknakpending > 0) {
            /* Do nothing. */
        } else if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (!dingdong(&keepalive_last, keepalive)) {
            /* Do nothing. */
        } else {

            datagram_stamp(&keepalive_buffer.header, &keepalive_sequence);
            surveyor_total = send_datagram(surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv4, &surveyor_endpoint.ipv6, surveyor_endpoint.udp, &keepalive_buffer, sizeof(keepalive_buffer));
            if (surveyor_total > 0) { network_total += surveyor_total; }

            DIMINUTO_LOG_DEBUG("Surveyor RTCM keepalive sent");

        }

        /**
         ** COMMANDS
         **/

        /*
         * If we have any initialization strings to send, and we have a device,
         * do so one at a time. Because this queue of writes is checked every
         * time we reiterate in the work loop, later code can enqueue new
         * commands to be written to the device. Because this is a doubly-linked
         * list, queued commands can be removed from the queue before they are
         * processed. And the list header can be prepended onto a command string
         * as part of a dynamically allocated structure, and this code will
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
        } else if (!dingdong(&command_last, postpone)) {
            /* Do nothing. */
        } else {

            command_node = diminuto_list_dequeue(&command_list);
            diminuto_assert(command_node != (diminuto_list_t *)0);

            command = diminuto_containerof(command_t, link, command_node);
            command_string = diminuto_list_data(command_node);
            diminuto_assert(command_string != (uint8_t *)0);

            if (command_string[0] == '\0') {

                free(command_node);
                DIMINUTO_LOG_NOTICE("Done");
                eof = !0;

            } else {

                command_size = strlen(command_string) + 1;
                DIMINUTO_LOG_NOTICE("Out \'%s\'[%zd]", command_string, command_size);
                command_buffer = (uint8_t *)malloc(command_size + 8 /* e.g. *, CHECKSUMA, CHECKSUMB, CR, LF, NUL. */);
                diminuto_assert(command_buffer != (uint8_t *)0);
                command_length = diminuto_escape_collapse(command_buffer, command_string, command_size);

                /*
                 * Since collapse() always includes a terminating NUL, the
                 * length will always be at least one. But if it is short,
                 * wackiness ensues below, so we check it anyway.
                 */

                diminuto_assert(command_length > 1);

                switch (command->emission) {
                case OPT_W:
                    command_total = emit_sentence(dev_fp, command_buffer, command_length);
                    break;
                case OPT_U:
                    command_total = emit_packet(dev_fp, command_buffer, command_length);
                    if (command_total > 0) { acknakpending += 1; }
                    break;
                case OPT_Z:
                    command_total = emit_data(dev_fp, command_buffer, command_length);
                    break;
                default:
                    command_total = -1;
                    break;
                }

                diminuto_assert(command_total > 1);

                 if (verbose) { fputs("OUT:\n", stderr); diminuto_dump(stderr, command_buffer, ((command_total > command_length) ? command_total : command_length) - 1 /* Minus terminating nul. */); }

                if (escape) { fputs("\033[2;1H\033[0K", out_fp); }
                if (report) { fprintf(out_fp, "OUT [%3zd] ", command_total - 1); print_buffer(out_fp, command_buffer, command_total - 1 /* Minus terminating nul. */, limitation); fflush(out_fp); }

                free(command_buffer);
                free(command_node);

            }

        }

        if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (acknakpending > 0) {
            /* Do nothing. */
        } else if (nominal) {
            /* Do nothing. */
        } else {
            DIMINUTO_LOG_NOTICE("Ready");
            nominal = !0;
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
         ** CATENATE
         **/

        if (sink_fp != (FILE *)0) {
            sz = fwrite(buffer, 1, length, sink_fp);
            diminuto_assert(sz == length);
        }

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

        if (log_fp != (FILE *)0) { print_buffer(log_fp, buffer, length, UNLIMITED); }
        if (verbose) { fputs("INP:\n", stderr); diminuto_dump(stderr, buffer, length); }
        if (escape) { fputs("\033[1;1H\033[0K", out_fp); }
        if (report) { fprintf(out_fp, "INP [%3zd] ", length); print_buffer(out_fp, buffer, length, limitation); fflush(out_fp); }

        /**
         ** ITERATE
         **/

        if (!process) { continue; }

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
        expiration_now = ticktock();
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
            countdown(&attitude.ticks, elapsed);
            countdown(&odometer.ticks, elapsed);
            countdown(&posveltim.ticks, elapsed);
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
             * sentence later. Note that the count returned by the tokenizer
             * includes a NULL pointer in the last used slot to terminate
             * the array in an argv[][] manner.
             */

            strncpy(tokenized, buffer, sizeof(tokenized));
            tokenized[sizeof(tokenized) - 1] = '\0';
            count = hazer_tokenize(vector, diminuto_countof(vector), tokenized, length);
            diminuto_assert(count > 0);
            diminuto_assert(vector[count - 1] == (char *)0);
            diminuto_assert(count <= diminuto_countof(vector));

            /*
             * Make sure it's a talker and a GNSS that we care about.
             * As a special case, if we receive an update on active satellites
             * or satellites in view from something we don't recognize, then
             * we have a new GNSS that isn't supported. That's worth noting.
             * Three other special cases: PUBX (u-blox), PMTK (Gtop/MTK),
             * and PSRF (SiRF) proprietary messages that are encoded like
             * NMEA sentences.
             */

            if (count < 2) {

                continue;

            } else if ((talker = hazer_parse_talker(vector[0])) >= HAZER_TALKER_TOTAL) {

                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    DIMINUTO_LOG_INFORMATION("Parse NMEA Talker Other \"%c%c\"", vector[0][1], vector[0][2]);
                }
                continue;

            } else if ((talker == HAZER_TALKER_PUBX) || (talker == HAZER_TALKER_PMTK) || (talker == HAZER_TALKER_PSRF)) {

                DIMINUTO_LOG_INFORMATION("Parse NMEA %s \"%.*s\"", HAZER_TALKER_NAME[talker], length - 2 /* Exclude CR and LF. */, buffer);
                continue;

            } else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {

                if ((vector[0][3] == 'G') && (vector[0][4] == 'S') && ((vector[0][5] == 'A') || (vector[0][5] == 'V'))) {
                    DIMINUTO_LOG_INFORMATION("Parse NMEA System Other \"%c%c\"\n", vector[0][1], vector[0][2]);
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
                trace = !0;
                fix = diminuto_time_elapsed();
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_rmc(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                trace = !0;
                fix = diminuto_time_elapsed();
                dmyokay = (position[system].dmy_nanoseconds > 0);
                totokay = (position[system].tot_nanoseconds >= position[system].old_nanoseconds);

            } else if (hazer_parse_gll(&position[system], vector, count) == 0) {

                position[system].ticks = timeout;
                refresh = !0;
                trace = !0;
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
                 * Below is a special case for the Ublox 8 used in devices like
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

                DIMINUTO_LOG_INFORMATION("Parse NMEA Other \"%s\"\n", vector[0]);

            } else {

                /* Do nothing. */

            }

            /*
             * Calculate our time to first fix if the code above established
             * a fix.
             */

            if (position[system].ticks == 0) {
                /* Do nothing. */
            } else if (position[system].utc_nanoseconds == 0) {
                /* Do nothing. */
            } else if (position[system].dmy_nanoseconds == 0) {
                /* Do nothing. */
            } else if (fix < 0) {
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

            if (yodel_ubx_nav_hpposllh(&(solution.payload), buffer, length) == 0) {

                solution.ticks = timeout;
                refresh = !0;
                trace = !0;

            } else if (yodel_ubx_mon_hw(&(hardware.payload), buffer, length) == 0) {

                hardware.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_nav_status(&(status.payload), buffer, length) == 0) {

                status.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_ack(&acknak, buffer, length) == 0) {

                refresh = !0;

                if (acknak.state) {
                    DIMINUTO_LOG_INFORMATION("Parse UBX ACK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                } else if (!nakquit) {
                    DIMINUTO_LOG_INFORMATION("Parse UBX NAK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                } else {
                    DIMINUTO_LOG_WARNING("Parse UBX NAK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                    xc = 1;
                    eof = !0;
                }

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

            } else if (yodel_ubx_nav_att(&(attitude.payload), buffer, length) == 0) {

                attitude.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_nav_odo(&(odometer.payload), buffer, length) == 0) {

                odometer.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_nav_pvt(&(posveltim.payload), buffer, length) == 0) {

                posveltim.ticks = timeout;
                refresh = !0;

            } else if (yodel_ubx_rxm_rtcm(&rover.payload, buffer, length) == 0) {

                rover.ticks = timeout;
                refresh = !0;

            } else if ((rc = yodel_ubx_mon_comms(&ports, buffer, length)) >= 0) {
                int ii = 0;
                int jj = 0;

                diminuto_assert(sizeof(ports.prefix) == 8);
                diminuto_assert(sizeof(ports.port[0]) == 40);
                diminuto_assert(sizeof(ports) == (8 + (5 * 40)));

                DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS version = %u\n", ports.prefix.version);
                DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS nPorts = %u\n", ports.prefix.nPorts);
                DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS txErrors = 0x%02x\n", ports.prefix.txErrors);
                for (ii = 0; ii < countof(ports.prefix.protIds); ++ii) {
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS protIds[%d] = %u\n", ii, ports.prefix.protIds[ii]);
                }
                for (ii = 0; ii < rc; ++ii) {
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] portId = 0x%04x\n", ii, ports.port[ii].portId);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] txPending = %u\n", ii, ports.port[ii].txPending);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] txBytes = %u\n", ii, ports.port[ii].txBytes);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] txUsage = %u\n", ii, ports.port[ii].txUsage);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] txPeakUsage = %u\n", ii, ports.port[ii].txPeakUsage);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] rxPending = %u\n", ii, ports.port[ii].rxPending);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] rxBytes = %u\n", ii, ports.port[ii].rxBytes);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] rxUsage = %u\n", ii, ports.port[ii].rxUsage);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] rxPeakUsage = %u\n", ii, ports.port[ii].rxPeakUsage);
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] overrunErrs = %u\n", ii, ports.port[ii].overrunErrs);
                    for (jj = 0; jj < countof(ports.port[ii].msgs); ++jj) {
                        DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] msgs[%d] = %u\n", ii, jj, ports.port[ii].msgs[jj]);
                    }
                    DIMINUTO_LOG_INFORMATION("Parse UBX MON COMMS port[%d] skipped = %u\n", ii, ports.port[ii].skipped);
                }

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

            kinematics.source = DEVICE;

            kinematics.number = tumbleweed_message(buffer, length);
            if (kinematics.number < 0) { kinematics.number = 9999; }
            collect(kinematics.number, &updates);

            kinematics.length = length;

            kinematics.ticks = timeout;
            refresh = !0;

            break;

        case FORMAT:

            /* Do nothing. */

            break;

        }

        if (eof) { break; }

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
        } else if (!emit_solution(arp, &base, &solution)) {
            /* Do nothing. */
        } else {
            arp = (const char *)0;
        }

        /*
         * If tracing is enabled and we have a latitude, longitude
         * and altitude solution, emit the trace.
         */

        if (trace_fp == (FILE *)0) {
            /* Do nothing. */
        } else if (!trace) {
            /* Do nothing. */
        } else if (!dingdong(&trace_last, frequency)) {
            /* Do nothing. */
        } else {
            emit_trace(trace_fp, position, &solution, &attitude, &posveltim, &base);
            trace = 0;
        }

        /*
         * If tracing is enabled and we transitioned from an active
         * survey to a valid fix, disable tracing. This allows us to
         * trace until the fix is established and no longer changing.
         */

        if (trace_fp == (FILE *)0) {
            /* Do nothing. */
        } else if (base.ticks == 0) {
            /* Do nothing. */
        } else if (base.payload.active) {
            /* Do nothing. */
        } else if (!base.payload.valid) {
            /* Do nothing. */
        } else if (trace_fp == stdout) {
            trace_fp = (FILE *)0;
            DIMINUTO_LOG_NOTICE("Tracing disabled\n");
        } else if ((rc = fclose(trace_fp)) != EOF) {
            trace_fp = (FILE *)0;
            DIMINUTO_LOG_NOTICE("Tracing disabled\n");
        } else {
            diminuto_perror("fclose(trace_fp)");
            trace_fp = (FILE *)0;
        }

        /*
         * We usually give priority to reading input from the device or a
         * socket. Generating the report can take a long time, particularly
         * with slow displays or serial consoles (partly what the -F flag is
         * all about). So if there is still data waiting to be read, we
         * short circuit the report code and instead try to assemble another
         * complete sentence, packet, or message that we can forward, write,
         * log, or use to update our databases. OTOH, why might we choose
         * not to do this, despite the risk of data loss? I have tested
         * GNSS devices whose output was so evenly distributed throughout
         * their cycle time (e.g. 1Hz) that there is never a time that there
         * isn't data in the standard I/O buffer. In such devices, this
         * code would continously loop back to read and process more data,
         * and never render a report. (Perhaps a better approach would be
         * to add a timeout interval, kind of the opposite of what the
         * report frequency options achieves.)
         */

        if ((dev_fp == (FILE *)0) && (remote_fd < 0)) {
            /* Do nothing. */
        } else if (dingdong(&check_last, check)) {
            /* Do nothing. */
        } else if ((io_available = diminuto_file_ready(in_fp)) > 0) {
            if (io_available > io_peak) { io_peak = io_available; }
            DIMINUTO_LOG_DEBUG("Ready file [%zu] [%zu]\n", io_available, io_peak);
            if (io_available >= io_size) { DIMINUTO_LOG_WARNING("Full file [%zd] [%zu]\n", io_available, io_size); }
            continue;
        } else if (serial && (io_available = diminuto_serial_available(in_fd)) > 0) {
            if (io_available > io_peak) { io_peak = io_available; }
            DIMINUTO_LOG_DEBUG("Ready device [%zu] [%zu]\n", io_available, io_peak);
            continue;
        } else if ((io_available = diminuto_mux_wait(&mux, 0 /* POLL */)) > 0) {
            DIMINUTO_LOG_DEBUG("Ready socket\n");
            continue;
        } else {
            DIMINUTO_LOG_DEBUG("Ready empty [0] [%zu]\n", io_peak);
        }

render:

        /**
         ** REPORT
         **/

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
         * Generate the display if necessary and sufficient reasons exist.
         */

        if (!refresh) {
            /* Do nothing. */
        } else if (!dingdong(&display_last, slow)) {
            /* Do nothing. */
        } else {

            if (escape) { fputs("\033[3;1H", out_fp); }
            if (report) {
                DIMINUTO_CRITICAL_SECTION_BEGIN(&mutex);
                    onepps = poller.onepps;
                    poller.onepps = 0;
                DIMINUTO_CRITICAL_SECTION_END;
                print_local(out_fp, timetofirstfix);
                print_positions(out_fp, position, onepps, dmyokay, totokay, network_total);
                print_hardware(out_fp, &hardware);
                print_status(out_fp, &status);
                print_solution(out_fp, &solution);
                print_attitude(out_fp, &attitude);
                print_odometer(out_fp, &odometer);
                print_posveltim(out_fp, &posveltim);
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
                if (checkpoint) {
                    out_fp = diminuto_observation_checkpoint(out_fp, &temporary);
                    diminuto_assert(out_fp != (FILE *)0);
                    checkpoint = 0;
                }
                out_fp = diminuto_observation_commit(out_fp, &temporary);
                diminuto_assert(out_fp == (FILE *)0);
                out_fp = diminuto_observation_create(headless, &temporary);
                diminuto_assert(out_fp != (FILE *)0);
            }

            refresh = 0;
        }

        if (eof) { break; }

    }

    /**
     ** FINIALIZATION
     **/

    DIMINUTO_LOG_NOTICE("Stop");

    if (verbose) { sync_end(); }

    DIMINUTO_LOG_INFORMATION("Counters Remote=%lu Surveyor=%lu Keepalive=%lu OutOfOrder=%u Missing=%u", (unsigned long)remote_sequence, (unsigned long)surveyor_sequence, (unsigned long)keepalive_sequence, outoforder_counter, missing_counter);

    rc = tumbleweed_finalize();
    diminuto_assert(rc == 0);

    rc = yodel_finalize();
    diminuto_assert(rc == 0);

    rc = hazer_finalize();
    diminuto_assert(rc == 0);

    diminuto_mux_fini(&mux);

    if (threadrc == 0) {
        DIMINUTO_COHERENT_SECTION_BEGIN;
            poller.done = !0;
        DIMINUTO_COHERENT_SECTION_END;
        DIMINUTO_THREAD_BEGIN(&thread);
            threadrc = diminuto_thread_notify(&thread);
        DIMINUTO_THREAD_END;
        threadrc = diminuto_thread_join(&thread, &result);
    }

    if (pps_fp != (FILE *)0) {
        pps_fp = diminuto_pin_unused(pps_fp, ppspin);
        diminuto_assert(pps_fp == (FILE *)0);
    }

    if (strobe_fp != (FILE *)0) {
        strobe_fp = diminuto_pin_unused(strobe_fp, strobepin);
        diminuto_assert(strobe_fp == (FILE *)0);
    }

    if (remote_fd >= 0) {
        rc = diminuto_ipc_close(remote_fd);
        diminuto_assert(rc >= 0);
    }

    if (trace_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (trace_fp == stdout) {
        /* Do nothing. */
    } else if ((rc = fclose(trace_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(trace_fp)");
    }

    if (log_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (log_fp == stderr) {
        /* Do nothing. */
    } else if ((rc = fclose(log_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(log_fp)");
    }

    if (dev_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (dev_fp == stdout) {
        /* Do nothing. */
    } else if ((rc = fclose(dev_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(dev_fp)");
    }

    DIMINUTO_LOG_INFORMATION("Buffer size=%lluB maximum=%lluB total=%lluB speed=%lluBPS peak=%lldB\n", (unsigned long long)io_size, (unsigned long long)io_maximum, (unsigned long long)io_total, (unsigned long long)((io_total * diminuto_frequency()) / (diminuto_time_elapsed() - epoch)), (long long)io_peak);

    free(io_buffer);

    if (sink_fp == (FILE *)0) {
        /* Do nothing. */
    } else if ((rc = fclose(sink_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(sink_fp)");
    }

    if (in_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (in_fp == dev_fp) {
        /* Do nothing. */
    } else if ((rc = fclose(in_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(in_fp)");
    }

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_commit(out_fp, &temporary);
        diminuto_assert(out_fp == (FILE *)0);
    } else if (out_fp == dev_fp) {
        /* Do nothing. */
    } else if ((rc = fclose(out_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(out_fp)");
    }

    if (identity != (const char *)0) {
        (void)diminuto_lock_unlock(identity);
    }

    while (!diminuto_list_isempty(&command_list)) {
        command_node = diminuto_list_dequeue(&command_list);
        diminuto_assert(command_node != (diminuto_list_t *)0);
        free(command_node);
    }

    DIMINUTO_LOG_NOTICE("End");

    fflush(stderr);

    return xc;
}
