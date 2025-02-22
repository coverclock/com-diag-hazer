/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2024 Digital Aggregates Corporation, Colorado, USA.
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
 * gpstool started out as a functional test of the Hazer library, but it has
 * since taken on a life of its own. I've been writing C code for decades, and
 * usually take a more modular approach to design. But with gpstool, I used
 * a "work loop" approach that has resulted in a far larger main program than
 * I would perhaps have otherwise preferred. But I find it pretty easy to
 * maintain, modify, and debug. This is the only C program (other than some
 * Linux kernel hacking) that I've had reason to use gotos - in a very limited
 * way - to actually simplify the code.
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
#include "com/diag/diminuto/diminuto_line.h"
#include "com/diag/diminuto/diminuto_lock.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_minmaxof.h"
#include "com/diag/diminuto/diminuto_mux.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/diminuto/diminuto_pipe.h"
#include "com/diag/diminuto/diminuto_policy.h"
#include "com/diag/diminuto/diminuto_realtime.h"
#include "com/diag/diminuto/diminuto_serial.h"
#include "com/diag/diminuto/diminuto_terminator.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_timer.h"
#include "com/diag/diminuto/diminuto_thread.h"
#include "com/diag/diminuto/diminuto_types.h"
#include "com/diag/diminuto/diminuto_version.h"
#include "com/diag/hazer/common.h"
#include "com/diag/hazer/machine.h"
#include "com/diag/hazer/hazer_version.h"
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <wchar.h>
#include "ansi.h"
#include "buffer.h"
#include "constants.h"
#include "defaults.h"
#include "emit.h"
#include "endpoint.h"
#include "fix.h"
#include "globals.h"
#include "helper.h"
#include "log.h"
#include "print.h"
#include "process.h"
#include "sync.h"
#include "test.h"
#include "threads.h"
#include "time.h"
#include "types.h"

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
    const char * pps = (const char *)0;
    const char * strobe = (const char *)0;
    const char * listing = (const char *)0;
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
    char * strobepath = (char *)0;
    const char * strobedevice = (const char *)0;
    diminuto_line_offset_t strobeline = maximumof(diminuto_line_offset_t);
    int strobeinverted = 0;
    char * ppspath = (char *)0;
    const char * ppsdevice = (const char *)0;
    diminuto_line_offset_t ppsline = maximumof(diminuto_line_offset_t);
    int ppsinverted = 0;
    int test = 0;
    int serial = 0;
    int daemon = 0;
    int nakquit = 0;
    int syncquit = 0;
    int activefirst = 0;
    diminuto_policy_scheduler_t scheduler = DIMINUTO_POLICY_SCHEDULER_DEFAULT;
    int priority = DIMINUTO_POLICY_PRIORITY_DEFAULT;
    seconds_t slow = 0;
    seconds_t timeout = HAZER_GNSS_SECONDS;
    seconds_t keepalive = TUMBLEWEED_KEEPALIVE_SECONDS;
    seconds_t frequency = 1;
    seconds_t postpone = 0;
    seconds_t bypass = -1;
    protocol_t preference = PROTOCOL;
    uint32_t threshold = DEFAULT_THRESHOLD_CENTICENTIMETERS;
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
    FILE * dev_fp = (FILE *)0;
    FILE * in_fp = (FILE *)0;
    FILE * listing_fp = (FILE *)0;
    FILE * out_fp = stdout;
    FILE * queue_fp = (FILE *)0;
    FILE * sink_fp = (FILE *)0;
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
    long device_mask = ANY;
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
    long remote_mask = ANY;
    role_t role = ROLE;
    /*
     * Queue variables.
     */
    const char * queue_option = (const char *)0;
    long queue_mask = ANY;
    size_t queued = 0;
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
    /*
     * Network variables.
     */
    ssize_t network_total = 0;
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
    int source_fd = -1;
    int pps_fd = -1;
    int strobe_fd = -1;
    /*
     * 1PPS poller thread variables.
     */
    poller_t poller = { 0, };
    void * result = (void *)0;
    diminuto_thread_t thread = DIMINUTO_THREAD_INITIALIZER((diminuto_thread_function_t *)0);
    diminuto_thread_t * threadp = (diminuto_thread_t *)0;
    int threadrc = -1;
    int onepps = 0;
    bool pulsing = false;
    /*
     * 1Hz timer service variables.
     */
    diminuto_timer_t timer = DIMINUTO_TIMER_INITIALIZER((diminuto_timer_function_t *)0);
    diminuto_timer_t * timerp = (diminuto_timer_t *)0;
    diminuto_sticks_t timerticks = (diminuto_sticks_t)-1;
    int onehz = 0;
    /*
     * NMEA parser state variables.
     */
    hazer_state_t nmea_state = HAZER_STATE_STOP;
    hazer_context_t nmea_context = HAZER_CONTEXT_INITIALIZER;
    datagram_buffer_t nmea_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * UBX parser state variables.
     */
    yodel_state_t ubx_state = YODEL_STATE_STOP;
    yodel_context_t ubx_context = YODEL_CONTEXT_INITIALIZER;
    datagram_buffer_t ubx_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * RTCM parser state variables.
     */
    tumbleweed_state_t rtcm_state = TUMBLEWEED_STATE_STOP;
    tumbleweed_context_t rtcm_context = TUMBLEWEED_CONTEXT_INITIALIZER;
    datagram_buffer_t rtcm_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * CPO parser state variables.
     */
    calico_state_t cpo_state = CALICO_STATE_STOP;
    calico_context_t cpo_context = CALICO_CONTEXT_INITIALIZER;
    datagram_buffer_t cpo_buffer = DATAGRAM_BUFFER_INITIALIZER;
    /*
     * NMEA processing variables.
     */
    hazer_buffer_t tokenized = HAZER_BUFFER_INITIALIZER;
    hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;
    /*
     * System (constellation) database.
     */
    bool systems[HAZER_SYSTEM_TOTAL] = { false, };
    hazer_system_t maximum = HAZER_SYSTEM_GNSS;
    /*
     * NMEA state databases.
     */
    hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
    hazer_actives_t actives = HAZER_ACTIVES_INITIALIZER;
    hazer_views_t views = HAZER_VIEWS_INITIALIZER;
    /*
     * UBX state databases.
     */
    yodel_solution_t solution = YODEL_SOLUTION_INITIALIZER;
    yodel_hardware_t hardware = YODEL_HARDWARE_INITIALIZER;
    yodel_status_t status = YODEL_STATUS_INITIALIZER;
    yodel_base_t base = YODEL_BASE_INITIALIZER;
    yodel_rover_t rover = YODEL_ROVER_INITIALIZER;
    yodel_ubx_ack_t acknak = YODEL_UBX_ACK_INITIALIZER;
    yodel_attitude_t attitude = YODEL_ATTITUDE_INITIALIZER;
    yodel_odometer_t odometer = YODEL_ODOMETER_INITIALIZER;
    yodel_posveltim_t posveltim = YODEL_POSVELTIM_INITIALIZER;
    int acknakpending = 0;
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
    seconds_t expiration_was = 0;
    seconds_t expiration_now = 0;
    seconds_t slow_last = 0;
    seconds_t keepalive_last = 0;
    seconds_t frequency_last = 0;
    seconds_t postpone_last = 0;
    seconds_t bypass_last = 0;
    /*
     * I/O buffer variables.
     */
    void * io_buffer = (void *)0;
    size_t io_size = BUFSIZ;
    size_t io_maximum = 0;
    size_t io_total = 0;
    size_t io_waiting = 0;
    /*
     * Source variables.
     */
    diminuto_mux_t mux = { 0, };
    int ch = EOF;
    int ready = 0;
    int fd = -1;
    ssize_t available = 0;
    format_t format = FORMAT;
    uint8_t * buffer = (uint8_t *)0;
    ssize_t size = 0;
    ssize_t length = 0;
    size_t written = 0;
    /*
     * Display variables.
     */
    char * temporary = (char *)0;
    size_t limitation = 0;
    int hangup = 0;
    int checkpoint = 0;
    /*
     * Control variables.
     */
    int eof = 0;        /** If true then the input stream hit end of file. */
    int sync = 0;       /** If true then the input stream is synchronized. */
    int frame = 0;      /** If true then the input stream is at frame start. */
    int refresh = !0;   /** If true then the display needs to be refreshed. */
    int trace = 0;      /** If true then the trace needs to be emitted. */
    int horizontal = 0; /** If true then horizontal has converged. */
    int vertical = 0;   /** If true then vertical has converged. */
    /*
     * Command line processing variables.
     */
    int error = 0;
    char * end = (char *)0;
    /*
     * Data processing variables.
     */
    ssize_t count = 0;
    hazer_active_t active_cache = HAZER_ACTIVE_INITIALIZER;
    int time_valid = 0;
    int time_valid_prior = 0;
    protocol_t protocol = PROTOCOL;
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
    int ii = 0;
    int jj = 0;
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
    static const char OPTIONS[] = "124678A:B:C:D:EF:G:H:I:KL:MN:O:PQ:RS:T:U:VW:X:Y:Z:ab:cdef:g:hi:k:lmnop:q:rst:u:vxw:y:z?";

    /**
     ** INITIALIZATION
     **/

    Program = ((Program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : Program + 1;

    diminuto_log_open_syslog(Program, DIMINUTO_LOG_OPTION_DEFAULT, DIMINUTO_LOG_FACILITY_DEFAULT);
    (void)diminuto_log_setmask();
    (void)diminuto_log_importmask(LOG_MASK_PATH);

    DIMINUTO_LOG_NOTICE("Program %s\n", argv[0]);
    DIMINUTO_LOG_INFORMATION("Library Hazer %s\n", COM_DIAG_HAZER_RELEASE_VALUE);
    DIMINUTO_LOG_INFORMATION("Library Diminuto %s\n", COM_DIAG_DIMINUTO_RELEASE_VALUE);

    /*
     * OPTIONS
     */

    while ((opt = getopt(argc, argv, OPTIONS)) >= 0) {
        switch (opt) {
        case '1':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            stopbits = 1;
            break;
        case '2':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            stopbits = 2;
            break;
        case '4':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            preference = IPV4;
            break;
        case '6':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            preference = IPV6;
            break;
        case '8':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            databits = 8;
            break;
        case 'A':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_contract(command != (command_t *)0);
            command->emission = OPT_A;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            process = !0; /* Have to process ACK/NAKs. */
            break;
        case 'B':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            io_size = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (io_size < 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'C':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            sink = optarg;
            break;
        case 'D':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            device = optarg;
            break;
        case 'E':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            report = !0;
            escape = !0;
            process = !0;
            break;
        case 'F':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            slow = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            report = !0;
            process = !0;
            break;
        case 'G':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            if (diminuto_ipc_endpoint(optarg, &remote_endpoint) < 0) {
                error = !0;
            } else if (remote_endpoint.udp <= 0) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            } else {
                remote_option = optarg;
            }
            break;
        case 'H':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            report = !0;
            escape = 0;
            process = !0;
            headless = optarg;
            break;
        case 'I':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            pps = optarg;
            ppspath = (char *)malloc(sizeof(diminuto_path_t));
            diminuto_contract(ppspath != (char *)0);
            ppsdevice = diminuto_line_parse(pps, ppspath, sizeof(diminuto_path_t), &ppsline, &ppsinverted);
            if (ppsdevice == (const char *)0) {
                error = !0;
            }
            break;
        case 'K':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            readonly = 0;
            direction = OUTPUT;
            break;
        case 'L':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            listing = optarg;
            break;
        case 'M':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            daemon = !0;
            break;
        case 'N':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            arp = optarg;
            break;
        case 'O':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            identity = optarg;
            break;
        case 'P':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            process = !0;
            break;
        case 'Q':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            queue_option = optarg;
            break;
        case 'R':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            report = !0;
            process = !0;
            break;
        case 'S':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            source = optarg;
            break;
        case 'T':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            tracing = optarg;
            process = !0; /* Have to process trace. */
            break;
        case 'U':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_contract(command != (command_t *)0);
            command->emission = OPT_U;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'V':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            DIMINUTO_LOG_NOTICE("Version %s %s %s %s\n", Program, COM_DIAG_HAZER_RELEASE_VALUE, COM_DIAG_HAZER_VINTAGE_VALUE, COM_DIAG_HAZER_REVISION_VALUE);
            break;
        case 'W':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_contract(command != (command_t *)0);
            command->emission = OPT_W;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'X':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            test = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'Y':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            if (diminuto_ipc_endpoint(optarg, &surveyor_endpoint) < 0) {
                error = !0;
            } else if (surveyor_endpoint.udp <= 0) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            } else {
                readonly = 0;
                surveyor_option = optarg;
            }
            break;
        case 'Z':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            readonly = 0;
            command = (command_t *)malloc(sizeof(command_t));
            diminuto_contract(command != (command_t *)0);
            command->emission = OPT_Z;
            command_node = &(command->link);
            diminuto_list_datainit(command_node, optarg);
            diminuto_list_enqueue(&command_list, command_node);
            break;
        case 'a':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            activefirst = !0;
            break;
        case 'b':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            bitspersecond = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (bitspersecond == 0)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'c':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            modemcontrol = !0;
            carrierdetect = !0;
            break;
        case 'd':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            debug = !0;
            break;
        case 'e':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            paritybit = 2;
            break;
        case 'f':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            frequency = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (frequency < 1)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'g':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            remote_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'h':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            rtscts = !0;
            break;
        case 'i':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            bypass = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'k':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            device_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'l':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            modemcontrol = 0;
            break;
        case 'm':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            modemcontrol = !0;
            break;
        case 'n':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            paritybit = 0;
            break;
        case 'o':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            paritybit = 1;
            break;
        case 'p':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            strobe = optarg;
            strobepath = (char *)malloc(sizeof(diminuto_path_t));
            diminuto_contract(strobepath != (char *)0);
            strobedevice = diminuto_line_parse(optarg, strobepath, sizeof(diminuto_path_t),  &strobeline, &strobeinverted);
            if (strobedevice == (const char *)0) {
                error = !0;
            }
            break;
        case 'q':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            queue_mask = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'r':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            scheduler = DIMINUTO_POLICY_SCHEDULER_FIFO;
            priority = DIMINUTO_POLICY_PRIORITY_HIGH;
            break;
        case 's':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            xonxoff = !0;
            break;
        case 't':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            timeout = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0') || (timeout < 0) || (timeout > HAZER_GNSS_SECONDS)) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'u':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            threshold = strtoul(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'v':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            verbose = !0;
            break;
        case 'w':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            postpone = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'x':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            nakquit = !0;
            break;
        case 'y':
            DIMINUTO_LOG_INFORMATION("Option -%c \"%s\"\n", opt, optarg);
            keepalive = strtol(optarg, &end, 0);
            if ((end == (char *)0) || (*end != '\0')) {
                errno = EINVAL;
                diminuto_perror(optarg);
                error = !0;
            }
            break;
        case 'z':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            syncquit = !0;
            break;
        case '?':
            DIMINUTO_LOG_INFORMATION("Option -%c\n", opt);
            fprintf(stderr, "usage: %s\n"
                            "               [ -d ] [ -v ] [ -z ]\n"
                            "               [ -D DEVICE [ -b BPS ] [ -7 | -8 ] [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] | -S FILE ] [ -B BYTES ]\n"
                            "               [ -R | -E | -H HEADLESS | -P ] [ -F SECONDS ] [ -i SECONDS ] [ -t SECONDS ] [ -a ]\n"
                            "               [ -C FILE ]\n"
                            "               [ -O FILE ]\n"
                            "               [ -L FILE ]\n"
                            "               [ -T FILE [ -f SECONDS ] ]\n"
                            "               [ -N FILE ]\n"
                            "               [ -Q FILE [ -q MASK ] ]\n"
                            "               [ -K [ -k MASK ] ]\n"
                            "               [ -A STRING ... ] [ -U STRING ... ] [ -W STRING ... ] [ -Z STRING ... ] [ -w SECONDS ] [ -x ]\n"
                            "               [ -4 | -6 ]\n"
                            "               [ -G :PORT | -G HOST:PORT [ -g MASK ] ]\n"
                            "               [ -Y :PORT | -Y HOST:PORT [ -y SECONDS ] ]\n"
                            "               [ -I CHIP:LINE | -I NAME | -c ]\n"
                            "               [ -p CHIP:LINE | -p NAME ]\n"
                            "               [ -M ] [ -X MASK ] [ -V ]\n"
                            , Program);
            fprintf(stderr, "       -1              Use one stop bit for DEVICE.\n");
            fprintf(stderr, "       -2              Use two stop bits for DEVICE.\n");
            fprintf(stderr, "       -4              Prefer IPv4 for HOST.\n");
            fprintf(stderr, "       -6              Prefer IPv6 for HOST.\n");
            fprintf(stderr, "       -7              Use seven data bits for DEVICE.\n");
            fprintf(stderr, "       -8              Use eight data bits for DEVICE.\n");
            fprintf(stderr, "       -A STRING       Collapse STRING, append Ubx end matter, write to DEVICE, expect ACK/NAK.\n");
            fprintf(stderr, "       -A ''           Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -B BYTES        Set the input Buffer size to BYTES bytes.\n");
            fprintf(stderr, "       -C FILE         Catenate input to FILE or named pipe.\n");
            fprintf(stderr, "       -D DEVICE       Use DEVICE for input or output.\n");
            fprintf(stderr, "       -E              Like -R but use ANSI Escape sequences.\n");
            fprintf(stderr, "       -F SECONDS      Update report no more than every SECONDS seconds, 0 always, <0 never.\n");
            fprintf(stderr, "       -G HOST:PORT    Use remote HOST and PORT as dataGram sink.\n");
            fprintf(stderr, "       -G :PORT        Use local PORT as dataGram source.\n");
            fprintf(stderr, "       -H HEADLESS     Like -R but writes each iteration to HEADLESS file.\n");
            fprintf(stderr, "       -I CHIP:LINE    Take 1PPS from GPIO CHIP LINE (requires -D) (LINE<0 active low).\n");
            fprintf(stderr, "       -I NAME         Take 1PPS from GPIO NAME (requires -D) (-NAME active low).\n");
            fprintf(stderr, "       -K              Write input to DEVICE sinK from datagram source.\n");
            fprintf(stderr, "       -L FILE         Write pretty-printed input to Listing FILE.\n");
            fprintf(stderr, "       -M              Run in the background as a daeMon.\n");
            fprintf(stderr, "       -N FILE         Use fix FILE to save ARP LLH for subsequeNt fixed mode.\n");
            fprintf(stderr, "       -O FILE         Save process identifier in FILE.\n");
            fprintf(stderr, "       -P              Process incoming data even if no report is being generated.\n");
            fprintf(stderr, "       -Q FILE         Write validated input to FILE or named pipe.\n");
            fprintf(stderr, "       -R              Print a Report on standard output.\n");
            fprintf(stderr, "       -S FILE         Use source FILE or named pipe for input.\n");
            fprintf(stderr, "       -T FILE         Save the PVT CSV Trace to FILE.\n");
            fprintf(stderr, "       -U STRING       Collapse STRING, append Ubx end matter, write to DEVICE.\n");
            fprintf(stderr, "       -U ''           Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -V              Log Version in the form of release, vintage, and revision.\n");
            fprintf(stderr, "       -W STRING       Collapse STRING, append NMEA end matter, Write to DEVICE.\n");
            fprintf(stderr, "       -W ''           Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -X MASK         Enable special test modes via MASK.\n");
            fprintf(stderr, "       -Y HOST:PORT    Use remote HOST and PORT as keepalive sink and surveYor source.\n");
            fprintf(stderr, "       -Y :PORT        Use local PORT as surveYor source.\n");
            fprintf(stderr, "       -Z STRING       Collapse STRING, write to DEVICE.\n");
            fprintf(stderr, "       -Z ''           Exit when this empty STRING is processed.\n");
            fprintf(stderr, "       -a              Display Active satellite views first.\n");
            fprintf(stderr, "       -b BPS          Use BPS bits per second for DEVICE.\n");
            fprintf(stderr, "       -c              Take 1PPS from DCD (requires -D and implies -m).\n");
            fprintf(stderr, "       -d              Display Debug output on standard error.\n");
            fprintf(stderr, "       -e              Use Even parity for DEVICE.\n");
            fprintf(stderr, "       -f SECONDS      Set trace Frequency to 1/SECONDS.\n");
            fprintf(stderr, "       -g MASK         Set dataGram sink mask (NMEA=%u, UBX=%u, RTCM=%u, CPO=%u, default=%lu).\n", NMEA, UBX, RTCM, CPO, remote_mask);
            fprintf(stderr, "       -h              Use RTS/CTS Hardware flow control for DEVICE.\n");
            fprintf(stderr, "       -i SECONDS      Bypass input check every SECONDS seconds, 0 always, <0 never.\n");
            fprintf(stderr, "       -k MASK         Set device sinK mask (NMEA=%u, UBX=%u, RTCM=%u, CPO=%u, default=%lu).\n", NMEA, UBX, RTCM, CPO, device_mask);
            fprintf(stderr, "       -l              Use Local control for DEVICE.\n");
            fprintf(stderr, "       -m              Use Modem control for DEVICE.\n");
            fprintf(stderr, "       -n              Use No parity for DEVICE.\n");
            fprintf(stderr, "       -o              Use Odd parity for DEVICE.\n");
            fprintf(stderr, "       -p CHIP:LINE    Assert GPIO outPut CHIP LINE with 1PPS (requires -D and -I or -c) (LINE<0 active low).\n");
            fprintf(stderr, "       -p NAME         Assert GPIO outPut NAME with 1PPS (requires -D and -I or -c) (-NAME active low).\n");
            fprintf(stderr, "       -q MASK         Set Queue mask (NMEA=%u, UBX=%u, RTCM=%u, CPO=%u, default=%lu).\n", NMEA, UBX, RTCM, CPO, queue_mask);
            fprintf(stderr, "       -r              Use real-time scheduling if available and root.\n");
            fprintf(stderr, "       -s              Use XON/XOFF (c-Q/c-S) Software flow control for DEVICE.\n");
            fprintf(stderr, "       -t SECONDS      Timeout GNSS data after SECONDS seconds [0..255].\n");
            fprintf(stderr, "       -u CCM          Use CCM for convergence threshold in centicentimeters.\n");
            fprintf(stderr, "       -v              Display Verbose output on standard error.\n");
            fprintf(stderr, "       -w SECONDS      Write STRING to DEVICE no more than every SECONDS seconds, 0 always, <0 never.\n");
            fprintf(stderr, "       -x              EXit if a NAK is received.\n");
            fprintf(stderr, "       -y SECONDS      Send surveYor a keep alive every SECONDS seconds, 0 always, <0 never.\n");
            fprintf(stderr, "       -z              Exit if all state machines stop.\n");
            return 1;
            break;
        }
    }

    if (error) {
        return 1;
    }

    /**
     ** START
     **/

    if (daemon) {
        rc = diminuto_daemon(Program);
        diminuto_contract(rc == 0);
        Process = getpid();
        DIMINUTO_LOG_NOTICE("Daemon %s %d %d %d %d", Program, rc, (int)Process, (int)getppid(), (int)getsid(Process));
    } else {
        Process = getpid();
    }
    diminuto_contract(Process >= 0);

    Identity = geteuid();
    diminuto_contract(Identity >= 0);
    DIMINUTO_LOG_INFORMATION("Identity pid %d uid %d euid %d gid %d euid %d\n", Process, (int)getuid(), Identity, (int)getgid(), (int)getegid());

    Realtime = diminuto_realtime_is_supported();
    diminuto_contract(Realtime >= 0);
    DIMINUTO_LOG_INFORMATION("Real-Time kernel %d\n", Realtime);
    DIMINUTO_LOG_INFORMATION("Real-Time scheduler %d\n", scheduler);
    DIMINUTO_LOG_INFORMATION("Real-Time priority %d\n", priority);

    DIMINUTO_LOG_NOTICE("Start");

    if (daemon) {
        size_t commandlength = 0;
        char * commandline = (char *)0;
        size_t commandresult = 0;

        commandlength = diminuto_command_length(argc, (const char **)argv);
        diminuto_contract(commandlength > 0);
        commandline = (char *)malloc(commandlength);
        diminuto_contract(commandline != (char *)0);
        commandresult = diminuto_command_line(argc, (const char **)argv, commandline, commandlength);
        diminuto_contract(commandresult == commandlength);
        DIMINUTO_LOG_NOTICE("Command \"%s\"\n", commandline);
        free(commandline);
    }

    if (test != 0) {
        DIMINUTO_LOG_NOTICE("Testing 0x%x\n", test);
    }

    if (gethostname(Hostname, sizeof(Hostname)) < 0) {
        diminuto_perror("gethostbyname");
        strncpy(Hostname, "localhost", sizeof(Hostname));
    }
    Hostname[sizeof(Hostname) - 1] = '\0';
    DIMINUTO_LOG_INFORMATION("Hostname \"%s\"\n", Hostname);

    /*
     * Set the Line consumer to our name. Only matters if we are
     * using Line to control GPIO lines.
     */

    if ((pps != (const char *)0) || (strobe != (const char *)0)) {
        (void)diminuto_line_consumer(Program);
    }

    /*
     * Necessary to get stuff like wchar_t and the "%lc" format to work,
     * which we use to display stuff like the degree sign.
     */

    rc = setenv("LC_ALL", "en_US.UTF-8", 0);
    if (rc < 0) {
        diminuto_perror("setenv");
    }
    if ((locale = setlocale(LC_ALL, "")) == (char *)0) {
        diminuto_perror("setlocale");
    } else {
        DIMINUTO_LOG_INFORMATION("Locale \"%s\"", locale);
    }

    if (identity != (const char *)0) {
        rc = diminuto_lock_file(identity);
        diminuto_contract(rc >= 0);
    }

    if (process) {
        DIMINUTO_LOG_NOTICE("Processing");
    }

    /*
     * Are we listing every valid sentence or packet to an output file?
     */

    if (listing == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(listing, "-") == 0) {
        listing_fp = stderr;
    } else if ((listing_fp = fopen(listing, "ab")) != (FILE *)0) {
        /* Do nothing. */
    } else {
        diminuto_perror(listing);
        diminuto_contract(listing_fp != (FILE *)0);
    }

    if (listing_fp != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Listing File (%d) \"%s\"\n", fileno(listing_fp), listing);
    }

    /*
     * Are we queueing every valid sentence or packet to an output file?
     */

    if (queue_option == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(queue_option, "-") == 0) {
        queue_fp = stdout;
    } else if ((queue_fp = fopen(queue_option, "ab")) != (FILE *)0) {
        /* Do nothing. */
    } else {
        diminuto_perror(queue_option);
        diminuto_contract(queue_fp != (FILE *)0);
    }

    if (queue_fp != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Queue File (%d) \"%s\"\n", fileno(queue_fp), queue_option);
        DIMINUTO_LOG_INFORMATION("Queue Mask 0x%lx\n", queue_mask);
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
    } else if ((protocol = endpoint_choose_protocol(&remote_endpoint, preference)) == IPV6) {

        remote_protocol = IPV6;

        remote_fd = diminuto_ipc6_datagram_peer(0);
        diminuto_contract(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        diminuto_contract(rc >= 0);

        role = PRODUCER;

    } else if (protocol == IPV4) {

        remote_protocol = IPV4;

        remote_fd = diminuto_ipc4_datagram_peer(0);
        diminuto_contract(remote_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(remote_fd, !0);
        diminuto_contract(rc >= 0);

        role = PRODUCER;

    } else if (preference == IPV4) {

        Source = remote_option;

        remote_protocol = IPV4;

        remote_fd = diminuto_ipc4_datagram_peer(remote_endpoint.udp);
        diminuto_contract(remote_fd >= 0);
        source_fd = remote_fd;

        rc = diminuto_mux_register_read(&mux, remote_fd);
        diminuto_contract(rc >= 0);

        role = CONSUMER;

    } else {

        Source = remote_option;

        remote_protocol = IPV6;

        remote_fd = diminuto_ipc6_datagram_peer(remote_endpoint.udp);
        diminuto_contract(remote_fd >= 0);
        source_fd = remote_fd;

        rc = diminuto_mux_register_read(&mux, remote_fd);
        diminuto_contract(rc >= 0);

        role = CONSUMER;

    }

    if (remote_fd >= 0) {
        endpoint_show_connection("Remote", remote_option, remote_fd, remote_protocol, &remote_endpoint.ipv6, &remote_endpoint.ipv4, remote_endpoint.udp);
        DIMINUTO_LOG_INFORMATION("Remote Protocol '%c'\n", remote_protocol);
        DIMINUTO_LOG_INFORMATION("Remote Role '%c'\n", role);
        DIMINUTO_LOG_INFORMATION("Remote Mask 0x%lx\n", remote_mask);
    }

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
    } else if ((protocol = endpoint_choose_protocol(&surveyor_endpoint, preference)) == IPV6) {

        /*
         * Sending keepalives and receiving updates via IPv6.
         */

        surveyor_protocol = IPV6;

        surveyor_fd = diminuto_ipc6_datagram_peer(0);
        diminuto_contract(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        diminuto_contract(rc >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_contract(rc >= 0);

    } else if (protocol == IPV4) {

        /*
         * Sending keepalives and receiving updates via IPv4.
         */

        surveyor_protocol = IPV4;

        surveyor_fd = diminuto_ipc4_datagram_peer(0);
        diminuto_contract(surveyor_fd >= 0);

        rc = diminuto_ipc_set_nonblocking(surveyor_fd, !0);
        diminuto_contract(rc >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_contract(rc >= 0);

    } else if (preference == IPV4) {

        /*
         * Receiving updates passively via IPv4 with keepalives disabled.
         */

        surveyor_protocol = IPV4;

        surveyor_fd = diminuto_ipc4_datagram_peer(surveyor_endpoint.udp);
        diminuto_contract(surveyor_fd >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_contract(rc >= 0);

        keepalive = -1;

    } else {

        /*
         * Receiving updates passively via IPv6 with keepalives disabled.
         */

        surveyor_protocol = IPV6;

        surveyor_fd = diminuto_ipc6_datagram_peer(surveyor_endpoint.udp);
        diminuto_contract(surveyor_fd >= 0);

        rc = diminuto_mux_register_read(&mux, surveyor_fd);
        diminuto_contract(rc >= 0);

        keepalive = -1;

    }

    if (surveyor_fd >= 0) {
        endpoint_show_connection("Surveyor", surveyor_option, surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv6, &surveyor_endpoint.ipv4, surveyor_endpoint.udp);
        DIMINUTO_LOG_INFORMATION("Surveyor Protocol '%c'\n", surveyor_protocol);
    }

    /*
     * Are we strobing a GPIO pin with the one pulse per second (1PPS)
     * indication we receive via either another GPIO pin or Data Carrier
     * Detect (DCD) on the serial line? This is useful for passing 1PPS
     * along to another application or device.
     */

    if (strobe != (const char *)0) {
        diminuto_line_bits_t flags = 0;

        flags |= DIMINUTO_LINE_FLAG_OUTPUT;
        if (strobeinverted) {
            flags |= DIMINUTO_LINE_FLAG_ACTIVE_LOW;
        }

        strobe_fd = diminuto_line_open(strobedevice, strobeline, flags);
        diminuto_contract(strobe_fd >= 0);

        DIMINUTO_LOG_INFORMATION("Strobe Line (%d) \"%s\" \"%s\" %d\n", strobe_fd, strobe, strobedevice, strobeline);

        rc = diminuto_line_clear(strobe_fd);
        diminuto_contract(rc >= 0);
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
        diminuto_line_bits_t flags = 0;

        flags |= DIMINUTO_LINE_FLAG_INPUT;
        flags |= DIMINUTO_LINE_FLAG_EDGE_RISING;
        flags |= DIMINUTO_LINE_FLAG_EDGE_FALLING;
        if (ppsinverted) {
            flags |= DIMINUTO_LINE_FLAG_ACTIVE_LOW;
        }

        /*
         * This is a digital signal, not a mechanical button, so no
         * debouncing to add latency.
         */

        pps_fd = diminuto_line_open_read(ppsdevice, ppsline, flags, 0);
        diminuto_contract(pps_fd >= 0);

        DIMINUTO_LOG_INFORMATION("1PPS Line (%d) \"%s\" \"%s\" %d\n", pps_fd, pps, ppsdevice, ppsline);

        rc = diminuto_line_get(pps_fd);
        diminuto_contract(rc >= 0);

        poller.ppsfd = pps_fd;
        poller.strobefd = strobe_fd;
        poller.onepps = 0;
        poller.onehz = TOLERANCE;
        poller.done = 0;

        threadp = diminuto_thread_init_base(&thread, gpiopoller, scheduler, priority);
        diminuto_contract(threadp == &thread);

        threadrc = diminuto_thread_start(threadp, &poller);
        diminuto_contract(threadrc == 0);
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
     * diminuto_contract(3) will fire, and the application will dump core. I do
     * this routinely, alas. Maybe in the future I'll add a check, a delay, and
     * a retry.
     */

    if (device == (const char *)0) {

        /* Do nothing. */

    } else if (strcmp(device, "-") == 0) {

        Source = "stdin";
        in_fp = stdin;

    } else {

        Source = strrchr(device, '/');
        if (Source != (const char *)0) {
            Source += 1;
        } else {
            Source = device;
        }

        dev_fd = open(device, readonly ? O_RDONLY : O_RDWR);
        if (dev_fd < 0) {
            diminuto_perror(device);
        }
        diminuto_contract(dev_fd >= 0);

        serial = diminuto_serial_valid(dev_fd);
        if (serial) {

            DIMINUTO_LOG_INFORMATION("Serial Port (%d) \"%s\" %d %d%c%d%s%s%s\n", dev_fd, device, bitspersecond, databits, (paritybit == 0) ? 'N' : ((paritybit % 2) == 0) ? 'E' : 'O', stopbits, modemcontrol ? " modem" : " local", xonxoff ? " xonoff" : "", rtscts ? " rtscts" : "");

            rc = diminuto_serial_set(dev_fd, bitspersecond, databits, paritybit, stopbits, modemcontrol, xonxoff, rtscts);
            diminuto_contract(rc == 0);

            rc = diminuto_serial_raw(dev_fd);
            diminuto_contract(rc == 0);

        }

        /*
         * Remarkably, below, some USB receivers will work with a mode of "w+"
         * and some will return a fatal I/O error and require "a+". "a+" seems
         * to work in either case. Weird.
         */

        dev_fp = fdopen(dev_fd, readonly ? "r" : "a+");
        if (dev_fp == (FILE *)0) {
            diminuto_perror(device);
        }
        diminuto_contract(dev_fp != (FILE *)0);

        DIMINUTO_LOG_INFORMATION("Device File (%d) \"%s\" %s \"%s\"\n", dev_fd, device, readonly ? "ro" : "rw", Source);
        DIMINUTO_LOG_INFORMATION("Device Mask 0x%lx\n", device_mask);

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

        Source = "stdin";
        in_fp = stdin;

    } else {

        Source = strrchr(source, '/');
        if (Source != (const char *)0) {
            Source += 1;
        } else {
            Source = source;
        }

        if ((in_fp = fopen(source, "r")) == (FILE *)0) {
            diminuto_perror(source);
            diminuto_contract(in_fp != (FILE *)0);
        }

    }

    /*
     * If we have absolute no input source, we fall back onto standard
     * input. Note that the input file pointer can legitimately be null
     * if we are reading datagrams from a forwarding instance of gpstool,
     * but in that case Source will not be null.
     */

    if (Source == (const char *)0) {

        Source = "stdin";
        in_fp = stdin;
    }

    /*
     * If in_fp now points to anything (a file, a FIFO, a DEVICE), get its
     * file descriptor so we can multiplex on it, and mess with the standard
     * I/O buffer.
     */

    if (in_fp != (FILE *)0) {

        in_fd = fileno(in_fp);
        source_fd = in_fd;

        DIMINUTO_LOG_INFORMATION("Buffer Default [%zu]\n", (size_t)BUFSIZ);
        if (io_size > BUFSIZ) {
            io_buffer = malloc(io_size);
            diminuto_contract(io_buffer != (void *)0);
            rc = setvbuf(in_fp, io_buffer, _IOFBF, io_size);
            diminuto_contract(rc == 0);
            DIMINUTO_LOG_INFORMATION("Buffer Read [%zu]\n", io_size);
        }

        rc = diminuto_mux_register_read(&mux, in_fd);
        diminuto_contract(rc >= 0);

    }

    DIMINUTO_LOG_INFORMATION("Buffer Sync [%zu]\n", SYNC_SIZE);
    DIMINUTO_LOG_INFORMATION("Buffer Datagram [%zu]\n", DATAGRAM_SIZE);

    /*
     * This is our source of input data, which at this point can be UDP socket,
     * a file, a serial-ish device, a FIFO, standard input, or maybe something
     * I haven't thought of but which can be abstracted as a path in the file
     * system.
     */

    DIMINUTO_LOG_INFORMATION("Source File (%d) \"%s\" %s\n", source_fd, Source, readonly ? "ro" : "rw");

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
        diminuto_contract(sink_fp != (FILE *)0);
    }

    if (sink_fp != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Sink File (%d) \"%s\"\n", fileno(sink_fp), sink);
    }

    /*
     * If we are running headless, create our temporary output file using the
     * provided prefix.
     */

    if (headless != (const char *)0) {
        out_fp = diminuto_observation_create(headless, &temporary);
        diminuto_contract(out_fp != (FILE *)0);
        DIMINUTO_LOG_INFORMATION("Observation File (%d) \"%s\"\n", fileno(out_fp), headless);
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
    } else if (!serial) {
        /* Do nothing. */
    } else if (!modemcontrol) {
        /* Do nothing. */
    } else if (!carrierdetect) {
        /* Do nothing. */
    } else {

        poller.ppsfd = fileno(dev_fp);
        poller.strobefd = strobe_fd;
        poller.onepps = 0;
        poller.onehz = TOLERANCE;
        poller.done = 0;

        threadp = diminuto_thread_init_base(&thread, dcdpoller, scheduler, priority);
        diminuto_contract(threadp == &thread);

        threadrc = diminuto_thread_start(threadp, &poller);
        diminuto_contract(threadrc == 0);
    }

    /*
     * If we are handling the 1PPS signal, either via a GPIO pin
     * or via the serial DCD signal, start a one hertz periodic timer.
     */

    if (threadp != (diminuto_thread_t *)0) {

        timerp = diminuto_timer_init_periodic(&timer, timerservice);
        diminuto_contract(timerp == &timer);

        timerticks = diminuto_frequency();
        diminuto_contract(timerticks > 0);

        timerticks = diminuto_timer_start(timerp, timerticks, &poller);
        diminuto_contract(timerticks >= (diminuto_sticks_t)0);
    }

    /*
     * If we are saving the track, open the track file.
     */

    if (tracing == (const char *)0) {
        /* Do nothing. */
    } else if (strcmp(tracing, "-") == 0) {
        trace_fp = stdout;
    } else if ((trace_fp = fopen(tracing, "a")) != (FILE *)0) {
        /* Do nothing. */
    } else {
        diminuto_perror(tracing);
        diminuto_contract(trace_fp != (FILE *)0);
    }

    if (trace_fp != (FILE *)0) {
        DIMINUTO_LOG_INFORMATION("Trace File (%d) \"%s\"\n", fileno(trace_fp), tracing);
    }

    /*
     * Miscellaneous other stuff to report at startup.
     */

    DIMINUTO_LOG_INFORMATION("Converged Threshold %uccm\n", (unsigned int)threshold);

    /*
     * Install our signal handlers.
     */

    rc = diminuto_terminator_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_pipe_install(0);
    diminuto_contract(rc >= 0);

    rc = diminuto_interrupter_install(!0);
    diminuto_contract(rc >= 0);

    rc = diminuto_hangup_install(!0);
    diminuto_contract(rc >= 0);

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
    diminuto_contract(rc == 0);

    rc = yodel_initialize();
    diminuto_contract(rc == 0);

    rc = tumbleweed_initialize();
    diminuto_contract(rc == 0);

    rc = calico_initialize();
    diminuto_contract(rc == 0);

    if (debug) {
        hazer_debug(stderr);
        yodel_debug(stderr);
        tumbleweed_debug(stderr);
        calico_debug(stderr);
    }

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

    (void)diminuto_time_timezone();

    /*
     * Start the clock.
     */

    Frequency = diminuto_frequency();
    diminuto_contract(Frequency > 0);

    Clock = diminuto_time_clock();
    diminuto_contract(Clock >= 0);

    Epoch = diminuto_time_elapsed();
    diminuto_contract(Epoch >= 0);

    Now = Epoch;

    Event = Epoch;

    delay = Frequency;

    /*
     * For some time intervals (e.g. display) we want to
     * delay initially; for others (e.g. keepalive) we do not.
     */

    expiration_now = expiration_was =
        slow_last =
            frequency_last =
                bypass_last =
                    postpone_last = Now / Frequency;

    keepalive_last = (Now / Frequency) - keepalive;

    /*
     * Initialize all state machines to attempt synchronization with the
     * input stream.
     */

    machine_start_all(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

    sync = 0;
    frame = 0;

    Synchronization = 0;

    maximum = HAZER_SYSTEM_GNSS;

    io_maximum = 0;
    io_total = 0;
    io_waiting = 0;

    /*
     * Initialize screen iff we're doing full-screen stuff with
     * ANSI escape sequences.
     */

    if (escape) {
        fputs(ANSI_INI, out_fp);
        if (report) {
            fprintf(out_fp, "INP [  0]\n");
            fprintf(out_fp, "OUT [  0]\n");
            print_local(out_fp);
        }
        fflush(out_fp);
    }

#if defined(TEST_ERROR)
#   warning TEST_ERROR enabled!

    /*
     * This code tests the log_error macro and its underlying
     * log_error_f function.
     */

    if ((test & TEST_ERROR) != 0) {
        extern void log_error_t1(void);
        extern void log_error_t2(void);

        log_error_t1();
        log_error_t2();
    }

#endif

    /**
     ** BEGIN
     **/

    DIMINUTO_LOG_NOTICE("Begin");

    while (!0) {

        /*
         * We keep working until out input goes away (end of file), or until
         * we are interrupted by a SIGINT or terminated by a SIGTERM. We
         * also check for SIGHUP, which checkpoints the headless output.
         */

        if (diminuto_terminator_check()) {
            DIMINUTO_LOG_NOTICE("Signal Terminate");
            break;
        }

        if (diminuto_pipe_check()) {
            DIMINUTO_LOG_NOTICE("Signal Pipe");
            break;
        }

        if (diminuto_interrupter_check()) {
            DIMINUTO_LOG_NOTICE("Signal Interrupt");
            break;
        }

        if (diminuto_hangup_check()) {
            /*
             * Using SIGHUP is actually a little problematic, since I
             * routinely start gpstool interactively, switch it to the
             * background, and later disconnect my terminal session and
             * let it run, causing a SIGHUP.
             */
            (void)diminuto_log_importmask(LOG_MASK_PATH);
            DIMINUTO_LOG_NOTICE("Signal Hangup");
            checkpoint = !0;
            hangup = !0;
        }

        /**
         ** TOP
         **/

        DIMINUTO_LOG_DEBUG("Top\n");

        /*
         * We keep looking for input from one of our sources until one of them
         * tells us we have a buffer to process. It could be a NMEA sentence,
         * a UBX packet, or an RTCM message. It is also possible that the
         * select(2) timed out, and no file descriptor will be returned, in
         * which case we have other work to do further below. Or it may be
         * that the select(2) was interrupted, so we need to interrogate our
         * signal handlers. Note that the code below may block.
         */

        available = 0;
        ready = 0;
        fd = -1;

        if ((in_fp != (FILE *)0) && ((available = diminuto_file_ready(in_fp)) > 0)) {

            fd = in_fd;
            if (available > io_maximum) {
                io_maximum = available;
            }

        } else if (serial && (in_fd >= 0) && ((available = diminuto_serial_available(in_fd)) > 0)) {

            fd = in_fd;
            if (available > io_maximum) {
                io_maximum = available;
            }

        } else if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {

            /* Do nothing. */

        } else if ((ready = diminuto_mux_wait(&mux, delay /* BLOCK */)) == 0) {

            /* Do nothing. */

        } else if (ready > 0) {

            fd = diminuto_mux_ready_read(&mux);
            diminuto_contract(fd >= 0);

        } else if (errno == EINTR) {

            continue;

        } else {

            diminuto_panic();

        }

consume:

        DIMINUTO_LOG_DEBUG("Consume [%d] (%d) [%zd]\n", ready, fd, available);

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

            /*
             * Consume bytes of NMEA, UBX, or RTCM from the input stream until
             * the current input stream buffer is empty or until a complete
             * buffer is assembled.
             */

            do {

                ch = fgetc(in_fp);
                if (ch != EOF) {
                    /*
                     * Note that this counter is the number if bytes
                     * consumed (one-based), not the displacement into
                     * the input stream, dump file, etc. (zero-based).
                     * Subtract one for the those values.
                     */
                    io_total += 1;
                } else if (ferror(in_fp)) {
                    DIMINUTO_LOG_WARNING("ERROR");
                    clearerr(in_fp);
                    xc = 1;
                    eof = !0;
                    break;
                } else if (feof(in_fp)) {
                    DIMINUTO_LOG_NOTICE("EOF");
                    eof = !0;
                    break;
                } else {
                    DIMINUTO_LOG_ERROR("FAILURE");
                    xc = 1;
                    eof = !0;
                    break;
                }

                if (!debug) {
                    /* Do nothing. */
                } else if (isprint(ch)) {
                    fprintf(stderr, "Datum [%zu] 0x%02x '%c'\n", io_total, ch, ch);
                } else {
                    fprintf(stderr, "Datum [%zu] 0x%02x\n", io_total, ch);
                }

                /*
                 * We put the single byte to the Catenate file sink to insure we
                 * capture even invalid characters from the input source before
                 * we check for frame synchronization.
                 */

                if (sink_fp != (FILE *)0) {
                    rc = fputc(ch, sink_fp);
                    diminuto_contract(rc != EOF);
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
                 * This all assumes that every GNSS device output format we
                 * support has a unique beginning. So far this is true. When
                 * it isn't, this logic will have to change. If the input
                 * stream isn't reliable, we might make the wrong choice
                 * because the octet happens to look like the sync character
                 * at the start of the frame; we'll lose data as the
                 * subsequent CRC or checksum fails, and we'll have to resync.
                 * Note that some U-blox devices can't keep up with the serial
                 * output stream and output partial frames (typically the last
                 * few characters). This is especially true when mixing NMEA
                 * and UBX output. This causes us to lose sync regularly.
                 */

                if (!sync) {

                    io_waiting += 1;
                    if ((io_waiting % DATAGRAM_SIZE) == 0) {
                        DIMINUTO_LOG_INFORMATION("Sync Waiting [%zu] 0x%02x %c %c %c %c\n", io_waiting, ch, nmea_state, ubx_state, rtcm_state, cpo_state);
                    }

                    if (verbose) {
                        sync_out(ch);
                    }

                } else if (!frame) {

                    /* Do nothing. */

                } else if (hazer_is_nmea((uint8_t)ch)) {

                    machine_start_nmea(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                } else if (yodel_is_ubx((uint8_t)ch)) {

                    machine_start_ubx(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                } else if (tumbleweed_is_rtcm((uint8_t)ch)) {

                    machine_start_rtcm(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                } else if (calico_is_cpo((uint8_t)ch)) {

                    machine_start_cpo(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                } else {

                    sync = 0;
                    io_waiting += 1;

                    /*
                     * Normally I'd log this at WARNING or NOTICE. But
                     * some devices with USB interfaces flood the log
                     * because of lost data every output cycle. (I'm
                     * looking at you, u-blox, which exhibits lost data
                     * on the USB interface.) I thought this was a bug in
                     * my code, but it occurs even using socat, screen, etc.
                     * Then I thought it was a bug in the Linux USB driver,
                     * but it shows up using my USB hardware analyzer. So the
                     * data is lost before we see it on the wire.
                     */

                    DIMINUTO_LOG_INFORMATION("Sync Lost [%zu] 0x%02x\n", io_total, ch);

                    if (verbose) {
                        sync_out(ch);
                    }

                    if (syncquit) {
                        goto stop;
                    }

                    /*
                     * Restart all of the state machines and try to sync again.
                     */

                    machine_start_all(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                }

                /*
                 * Run all of the state machines in parallel. Some (or even
                 * most) of them may be in a terminal state having given up.
                 */

                frame = 0;

                /*
                 * NMEA STATE MACHINE
                 */ 

                if (nmea_state != HAZER_STATE_STOP) {

                    nmea_state = hazer_machine(nmea_state, ch, nmea_buffer.payload.buffers.nmea, sizeof(nmea_buffer.payload.buffers.nmea), &nmea_context);
                    if (nmea_state == HAZER_STATE_END) {

                        buffer = (uint8_t *)nmea_buffer.payload.buffers.nmea;
                        size = hazer_size(&nmea_context);
                        length = size - 1;
                        format = NMEA;

                        if (!sync) {

                            DIMINUTO_LOG_INFORMATION("Sync Start [%zu] 0x%02x NMEA\n", io_total, ch);
                            sync = !0;
                            io_waiting = 0;

                            if (verbose) {
                                sync_in(length);
                            }

                            if (Synchronization < (countof(SYNCHRONIZATION) - 2)) {
                                Synchronization += 1;
                            }

                        }

                        frame = !0;
                        DIMINUTO_LOG_DEBUG("Input NMEA [%zd] [%zd] \"%-5.5s\"", size, length, (buffer + 1));

                        /*
                         * Do not feed any other state machines.
                         */
                        break;

                    }

                }

                /*
                 * UBX STATE MACHINE
                 */ 

                if (ubx_state != YODEL_STATE_STOP) {

                    ubx_state = yodel_machine(ubx_state, ch, ubx_buffer.payload.buffers.ubx, sizeof(ubx_buffer.payload.buffers.ubx), &ubx_context);
                    if (ubx_state == YODEL_STATE_END) {

                        buffer = ubx_buffer.payload.buffers.ubx;
                        size = yodel_size(&ubx_context);
                        length = size - 1;
                        format = UBX;

                        if (!sync) {

                            DIMINUTO_LOG_INFORMATION("Sync Start [%zu] 0x%02x UBX\n", io_total, ch);

                            sync = !0;
                            io_waiting = 0;

                            if (verbose) {
                                sync_in(length);
                            }

                            if (Synchronization < (countof(SYNCHRONIZATION) - 2)) {
                                Synchronization += 1;
                            }

                        }

                        frame = !0;
                        DIMINUTO_LOG_DEBUG("Input UBX [%zd] [%zd] 0x%02x 0x%02x", size, length, *(buffer + 2), *(buffer + 3));

                        /*
                         * Do not feed any other state machines.
                         */
                        break;

                    }

                }

                /*
                 * RTCM STATE MACHINE
                 */ 

                if (rtcm_state != TUMBLEWEED_STATE_STOP) {

                    rtcm_state = tumbleweed_machine(rtcm_state, ch, rtcm_buffer.payload.buffers.rtcm, sizeof(rtcm_buffer.payload.buffers.rtcm), &rtcm_context);
                    if (rtcm_state == TUMBLEWEED_STATE_END) {

                        buffer = rtcm_buffer.payload.buffers.rtcm;
                        size = tumbleweed_size(&rtcm_context);
                        length = size - 1;
                        format = RTCM;

                        if (!sync) {

                            DIMINUTO_LOG_INFORMATION("Sync Start [%zu] 0x%02x RTCM\n", io_total, ch);

                            sync = !0;
                            io_waiting = 0;

                            if (verbose) {
                                sync_in(length);
                            }

                            if (Synchronization < (countof(SYNCHRONIZATION) - 2)) {
                                Synchronization += 1;
                            }

                        }

                        frame = !0;

                        DIMINUTO_LOG_DEBUG("Input RTCM [%zd] [%zd] %d", size, length, tumbleweed_message(buffer, length));

                        /*
                         * Do not feed any other state machines.
                         */
                        break;

                    }

                }

                /*
                 * CPO STATE MACHINE
                 */ 

                if (cpo_state != CALICO_STATE_STOP) {

                    cpo_state = calico_machine(cpo_state, ch, cpo_buffer.payload.buffers.cpo, sizeof(cpo_buffer.payload.buffers.cpo), &cpo_context);
                    if (cpo_state == CALICO_STATE_END) {

                        buffer = cpo_buffer.payload.buffers.cpo;
                        size = calico_size(&cpo_context);
                        length = size - 1;
                        format = CPO;

                        if (!sync) {

                            DIMINUTO_LOG_INFORMATION("Sync Start [%zu] 0x%02x CPO\n", io_total, ch);

                            sync = !0;
                            io_waiting = 0;

                            if (verbose) {
                                sync_in(length);
                            }

                            if (Synchronization < (countof(SYNCHRONIZATION) - 2)) {
                                Synchronization += 1;
                            }

                        }

                        frame = !0;

                        DIMINUTO_LOG_DEBUG("Input CPO [%zd] [%zd] 0x%02x 0x%02x", size, length, *(buffer + 2), *(buffer + 3));

                        /*
                         * Do not feed any other state machines.
                         */
                        break;

                    }
                }

                /*
                 * If all the state machines have stopped, or at least one has
                 * stopped while the rest are still in their start state, then
                 * either we have never had synchronization, or we lost
                 * synchronization. Restart all of them. We print an error
                 * message if any of the state machines failed on a CRC or
                 * checksum check. (It's impossible that more than one of
                 * them can have gotten as far as to fail on a CRC or
                 * checksum, but I didn't code it that way.)
                 */

                if (machine_is_stalled(nmea_state, ubx_state, rtcm_state, cpo_state)) {

                    if (sync) {

                        DIMINUTO_LOG_INFORMATION("Sync Stop [%zu] 0x%02x\n", io_total, ch);

                        if (nmea_context.error) {
                            errno = EIO;
                            log_error(nmea_buffer.payload.buffers.nmea, nmea_context.bp - nmea_buffer.payload.buffers.nmea - 1);
                        }

                        if (ubx_context.error) {
                            errno = EIO;
                            log_error(ubx_buffer.payload.buffers.ubx, ubx_context.bp - ubx_buffer.payload.buffers.ubx - 1);
                        }

                        if (rtcm_context.error) {
                            errno = EIO;
                            log_error(rtcm_buffer.payload.buffers.rtcm, rtcm_context.bp - rtcm_buffer.payload.buffers.rtcm - 1);
                        }

                        if (cpo_context.error) {
                            errno = EIO;
                            log_error(cpo_buffer.payload.buffers.cpo, cpo_context.bp - cpo_buffer.payload.buffers.cpo - 1);
                        }

                        if (verbose) {
                            sync_out(ch);
                        }

                        if (syncquit) {
                            goto stop;
                        }

                        sync = 0;

                    }

                    frame = 0;

                    machine_start_all(&nmea_state, &ubx_state, &rtcm_state, &cpo_state);

                }

            } while ((in_fp != (FILE *)0) && (diminuto_file_ready(in_fp) > 0));

            /*
             * At this point, either we ran out of data in the input
             * stream buffer, or we assembled a complete NMEA sentence,
             * UBX packet, or NMEA message to process, or we hit end of file.
             */

        } else if ((role == CONSUMER) && (fd == remote_fd)) {

            /*
             * Receive a NMEA, UBX, or RTCM datagram from a remote gpstool.
             * We make a rule that the datagram must be a complete NMEA
             * sentence, UBX packet, or RTCM message, complete with a valid
             * checksum or cyclic redundancy check, with no extra leading or
             * trailing bytes. If we do receive an invalid datagram, that
             * is a serious bug either in this software or in the transport.
             */

            remote_total = endpoint_receive_datagram(remote_fd, &remote_buffer, sizeof(remote_buffer));
            if (remote_total > 0) {
                network_total += remote_total;
            }

            if (remote_total < sizeof(remote_buffer.header)) {

                /*
                 * Too short.
                 */

                DIMINUTO_LOG_WARNING("Datagram Length [%zd]\n", remote_total);

            } else if ((remote_size = datagram_validate(&remote_sequence, &remote_buffer.header, remote_total, &outoforder_counter, &missing_counter)) < 0) {

                DIMINUTO_LOG_NOTICE("Datagram Order [%zd] {%lu} {%lu}\n", remote_total, (unsigned long)remote_sequence, (unsigned long)ntohl(remote_buffer.header.sequence));

            } else if (hazer_is_nmea(remote_buffer.payload.buffers.nmea[0]) && ((remote_length = hazer_validate(remote_buffer.payload.buffers.nmea, remote_size)) > 0)) {

                /*
                 * NMEA sentence.
                 */

                buffer = remote_buffer.payload.buffers.nmea;
                size = remote_size;
                length = remote_length;
                format = NMEA;

                DIMINUTO_LOG_DEBUG("Datagram NMEA [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else if (yodel_is_ubx(remote_buffer.payload.buffers.ubx[0]) && ((remote_length = yodel_validate(remote_buffer.payload.buffers.ubx, remote_size)) > 0)) {

                /*
                 * UBX packet.
                 */

                buffer = remote_buffer.payload.buffers.ubx;
                size = remote_size;
                length = remote_length;
                format = UBX;

                DIMINUTO_LOG_DEBUG("Datagram UBX [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else if (tumbleweed_is_rtcm(remote_buffer.payload.buffers.rtcm[0]) && ((remote_length = tumbleweed_validate(remote_buffer.payload.buffers.rtcm, remote_size)) > 0)) {

                /*
                 * RTCM message.
                 */

                buffer = remote_buffer.payload.buffers.rtcm;
                size = remote_size;
                length = remote_length;
                format = RTCM;

                DIMINUTO_LOG_DEBUG("Datagram RTCM [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else if (calico_is_cpo(remote_buffer.payload.buffers.cpo[0]) && ((remote_length = calico_validate(remote_buffer.payload.buffers.cpo, remote_size)) > 0)) {

                /*
                 * CPO packet.
                 */

                buffer = remote_buffer.payload.buffers.cpo;
                size = remote_size;
                length = remote_length;
                format = CPO;

                DIMINUTO_LOG_DEBUG("Datagram CPO [%zd] [%zd] [%zd]", remote_total, remote_size, remote_length);

            } else {

                /*
                 * Other.
                 */

                DIMINUTO_LOG_ERROR("Datagram Other [%zd] [%zd] [%zd] 0x%02x\n", remote_total, remote_size, remote_length, remote_buffer.payload.data[0]);

            }

            /*
             * Write the datagram to the Cantenate file if it exists.
             */

            if (sink_fp != (FILE *)0) {
                written = fwrite(buffer, 1, size - 1 /* Minus trailing NUL. */, sink_fp);
                diminuto_contract(written == (size - 1));
            }

        } else if (fd == surveyor_fd) {

            /*
             * Receive an RTCM datagram from a remote gpstool doing a survey.
             */

            surveyor_total = endpoint_receive_datagram(surveyor_fd, &surveyor_buffer, sizeof(surveyor_buffer));
            if (surveyor_total > 0) {
                network_total += surveyor_total;
            }

            if (surveyor_total < sizeof(surveyor_buffer.header)) {

                DIMINUTO_LOG_WARNING("Surveyor Length [%zd]\n", surveyor_total);

            } else if ((surveyor_size = datagram_validate(&surveyor_sequence, &surveyor_buffer.header, surveyor_total, &outoforder_counter, &missing_counter)) < 0) {

                DIMINUTO_LOG_NOTICE("Surveyor Order [%zd] {%lu} {%lu}\n", surveyor_total, (unsigned long)surveyor_sequence, (unsigned long)ntohl(surveyor_buffer.header.sequence));

            } else if ((surveyor_length = tumbleweed_validate(surveyor_buffer.payload.buffers.rtcm, surveyor_size)) < TUMBLEWEED_RTCM_SHORTEST) {

                DIMINUTO_LOG_ERROR("Surveyor Data [%zd] [%zd] [%zd] 0x%02x\n", surveyor_total, surveyor_size, surveyor_length, surveyor_buffer.payload.data[0]);

            } else if (surveyor_length == TUMBLEWEED_RTCM_SHORTEST) {

                DIMINUTO_LOG_DEBUG("Surveyor Keepalive received");

            } else if (dev_fp == (FILE *)0) {

                /* Do nothing. */

            } else {

                kinematics.source = NETWORK;

                kinematics.number = tumbleweed_message(surveyor_buffer.payload.buffers.rtcm, surveyor_length);
                if (kinematics.number < 0) {
                    kinematics.number = 9999;
                }
                helper_collect(kinematics.number, &updates);

                kinematics.length = surveyor_length;

                kinematics.timeout = timeout;
                refresh = !0;

                DIMINUTO_LOG_DEBUG("Surveyor RTCM [%zd] [%zd] [%zd] <%d>\n", surveyor_total, surveyor_size, surveyor_length, kinematics.number);

                if (verbose) {
                    fputs("Datagram:\n", stderr);
                    diminuto_dump(stderr, &surveyor_buffer, surveyor_total);
                }
                buffer_write(dev_fp, surveyor_buffer.payload.buffers.rtcm, surveyor_length);

            }

        } else {

            /*
             * The multiplexor returned a file descriptor which was not one we
             * recognize; that should be impossible.
             */

            DIMINUTO_LOG_ERROR("Multiplexor Fail [%d] (%d) <%d %d %d>\n", ready, fd, dev_fd, remote_fd, surveyor_fd);
            diminuto_panic();

        }

        /*
         * If one of the input sources indicated end of file, we're done.
         * (This may be the only time I've found a legitimate use for a goto.)
         */

        if (eof) {
            goto render;
        }

        /*
         * At this point, either we have a buffer with a complete and validated
         * NMEA sentence, UBX packet, RTCM message, or CPO packet that is ready
         * to process, acquired either from a state machine or a socket, or
         * there is no input pending and maybe this is a good time to update
         * the display. It is also a good time to make a note of the current
         * system time.
         */

        Clock = diminuto_time_clock();
        diminuto_contract(Clock >= 0);

        Now = diminuto_time_elapsed();
        diminuto_contract(Now >= 0);

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
        } else if (acknakpending) {
            /* Do nothing. */
        } else if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (!time_expired(&keepalive_last, keepalive)) {
            /* Do nothing. */
        } else {

            datagram_stamp(&keepalive_buffer.header, &keepalive_sequence);
            surveyor_total = endpoint_send_datagram(surveyor_fd, surveyor_protocol, &surveyor_endpoint.ipv4, &surveyor_endpoint.ipv6, surveyor_endpoint.udp, &keepalive_buffer, sizeof(keepalive_buffer));
            if (surveyor_total > 0) {
                network_total += surveyor_total;
            }

            DIMINUTO_LOG_DEBUG("Surveyor Keepalive sent");

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
        } else if (acknakpending) {
            /* Do nothing. */
        } else if (diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (!time_expired(&postpone_last, postpone)) {
            /* Do nothing. */
        } else {

            command_node = diminuto_list_dequeue(&command_list);
            diminuto_contract(command_node != (diminuto_list_t *)0);

            command = diminuto_containerof(command_t, link, command_node);
            command_string = diminuto_list_data(command_node);
            diminuto_contract(command_string != (uint8_t *)0);

            if (command_string[0] == '\0') {

                free(command_node);
                DIMINUTO_LOG_NOTICE("Done");
                eof = !0;

            } else {

                command_size = strlen((const char *)command_string) + 1;
                DIMINUTO_LOG_NOTICE("Out [%zd] \"%s\"\n", command_size, command_string);
                command_buffer = (uint8_t *)malloc(command_size + 8 /* e.g. *, CHECKSUMA, CHECKSUMB, CR, LF, NUL. */);
                diminuto_contract(command_buffer != (uint8_t *)0);
                command_length = diminuto_escape_collapse((char *)command_buffer, (const char *)command_string, command_size);

                /*
                 * Since collapse() always includes a terminating NUL, the
                 * length will always be at least one. But if it is short,
                 * wackiness ensues below, so we check it anyway.  */

                diminuto_contract(command_length > 1);

                switch (command->emission) {
                case OPT_A:
                    /*
                     * -A STRING: UBX output to which, after collapsing, end
                     * matter must be applied, and for which an UBX-ACK-ACK or
                     * UBX-ACK-NAK is expected.
                     */
                    command_total = emit_packet(dev_fp, command_buffer, command_length);
                    if (command_total > 0) {
                        acknakpending = !0;
                        DIMINUTO_LOG_NOTICE("Pending");
                    }
                    break;
                case OPT_U:
                    /*
                     * -U STRING: UBX output to which, after collapsing, end
                     * matter must be applied.
                     */
                    command_total = emit_packet(dev_fp, command_buffer, command_length);
                    break;
                case OPT_W:
                    /*
                     * -W STRING: NMEA output to which, after collapsing, end
                     * matter must be applied.
                     */
                    command_total = emit_sentence(dev_fp, (const char *)command_buffer, command_length);
                    break;
                case OPT_Z:
                    /*
                     * -Z STRING: any output sent, after collapsing, exactly
                     * as is.
                     */
                    command_total = emit_data(dev_fp, command_buffer, command_length);
                    break;
                default:
                    command_total = -1;
                    break;
                }

                diminuto_contract(command_total > 1);

                 if (verbose) {
                    fputs("Output:\n", stderr);
                    diminuto_dump(stderr, command_buffer, ((command_total > command_length) ? command_total : command_length) - 1 /* Minus terminating nul. */);
                }

                if (escape) {
                    fputs(ANSI_OUT, out_fp);
                }

                if (report) {
                    fprintf(out_fp, "OUT [%3zd] ", command_total - 1);
                    buffer_print(out_fp, command_buffer, command_total - 1 /* Minus terminating nul. */, limitation);
                    fflush(out_fp);
                }

                free(command_buffer);
                free(command_node);

            }

        }

        if (!diminuto_list_isempty(&command_list)) {
            /* Do nothing. */
        } else if (acknakpending) {
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
         * format indicates NMEA, UBX, RTCM, or CPO;
         *
         * buffer points to a buffer containing an NMEA sentence, a UBX packet,
         * an RTCM message, or a CPO packet, with a valid checksum or CRC;
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

        /*
         * QUEUE
         */

        /*
         * We write anything whose format is enabled in the queueing mask.
         * We don't write the terminating NUL. This is refered to as queueing
         * mostly because the related options are 'Q' and 'q'. But since the
         * output file can be a named pipe, queueing isn't completely wrong
         * either.
         */

        if (queue_fp == (FILE *)0) {
            /* Do nothing. */
        } else if ((queue_mask & format) == 0) {
            /* Do nothing. */
        } else {
            queued = fwrite(buffer, 1, size - 1 /* Minus trailing NUL. */, queue_fp);
            diminuto_contract(queued == (size - 1));
            fflush(queue_fp);
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
            remote_total = endpoint_send_datagram(remote_fd, remote_protocol, &remote_endpoint.ipv4, &remote_endpoint.ipv6, remote_endpoint.udp, dp, sizeof(dp->header) + length);
            if (remote_total > 0) {
                network_total += remote_total;
                DIMINUTO_LOG_DEBUG("Datagram Sent 0x%x [%zd] [%zd]", format, remote_total, network_total);
            }
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
        } else if (!time_valid) {
            /* Do nothing. */
        } else {
            buffer_write(dev_fp, buffer, length);
        }

        /**
         ** LOG
         **/

        if (listing_fp != (FILE *)0) {
            buffer_print(listing_fp, buffer, length, UNLIMITED);
        }

        if (verbose) {
            fputs("Input:\n", stderr); diminuto_dump(stderr, buffer, length);
        }

        if (escape) {
            fputs(ANSI_INP, out_fp);
        }
        if (report) {
            fprintf(out_fp, "INP [%3zd] ", length); buffer_print(out_fp, buffer, length, limitation);
            fflush(out_fp);
        }

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
        expiration_now = Now / Frequency;
        elapsed = (expiration_now > expiration_was) ? expiration_now - expiration_was : 0;

        if (elapsed > 0) {

            for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                if (ii > maximum) { break; }
                time_countdown(&positions[ii].timeout, elapsed);
                time_countdown(&actives[ii].timeout, elapsed);
                for (jj = 0; jj < HAZER_GNSS_SIGNALS; ++jj) {
                    if (jj >= views[ii].signals) { break; }
                    time_countdown(&views[ii].sig[jj].timeout, elapsed);
                }
            }

            time_countdown(&solution.timeout, elapsed);
            time_countdown(&hardware.timeout, elapsed);
            time_countdown(&status.timeout, elapsed);
            time_countdown(&base.timeout, elapsed);
            time_countdown(&rover.timeout, elapsed);
            time_countdown(&attitude.timeout, elapsed);
            time_countdown(&odometer.timeout, elapsed);
            time_countdown(&posveltim.timeout, elapsed);
            time_countdown(&kinematics.timeout, elapsed);

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

            strncpy((char *)tokenized, (const char *)buffer, sizeof(tokenized));
            tokenized[sizeof(tokenized) - 1] = '\0';
            count = hazer_tokenize(vector, diminuto_countof(vector), tokenized, length);
            diminuto_contract(count > 0);
            diminuto_contract(vector[count - 1] == (char *)0);
            diminuto_contract(count <= diminuto_countof(vector));

            DIMINUTO_LOG_DEBUG("Tokenize NMEA [%zd]", count);

            /*
             * Make sure it's a talker and a GNSS that we care about.
             * As a special case, if we receive an update on active satellites
             * or satellites in view from something we don't recognize, then
             * we have a new GNSS that isn't supported. That's worth noting.
             * Other special cases: PUBX (u-blox), PMTK (Gtop/MTK), PSRF
             * (SiRF), and PAIR (Quectel) proprietary messages that are
             * encoded like NMEA sentences.
             */

            if ((talker = hazer_parse_talker(buffer, length)) >= HAZER_TALKER_TOTAL) {

                DIMINUTO_LOG_INFORMATION("Received NMEA Talker Other \"%*s\"", HAZER_NMEA_NAMEEND, buffer);
                continue;

            } else if (talker == HAZER_TALKER_PUBX) {

                system = HAZER_SYSTEM_GNSS;

            } else if ((talker == HAZER_TALKER_PMTK) || (talker == HAZER_TALKER_PSRF) || (talker == HAZER_TALKER_PAIR) || (talker == HAZER_TALKER_PGRM)) {

                DIMINUTO_LOG_INFORMATION("Received Proprietary Sentence Other %s \"%.*s\"", HAZER_TALKER_NAME[talker], (length > 2) ? (int)(length - 2) : (int)length, buffer);
                continue;

            } else if ((system = hazer_map_talker_to_system(talker)) >= HAZER_SYSTEM_TOTAL) {

                DIMINUTO_LOG_INFORMATION("Received NMEA System Other \"%*s\"\n", HAZER_NMEA_NAMEEND, buffer);
                continue;

            } else if (system > maximum) { 

                maximum = system;

            } else {

                /* Nominal. */

            }

            if (!systems[system]) {
                DIMINUTO_LOG_NOTICE("System NMEA Any [%d] %s\n", system, HAZER_SYSTEM_NAME[system]);
                systems[system] = true;
            }

            /*
             * Parse the sentences we care about and update our state to
             * reflect the new data. As we go along we do some reality checks
             * to decide if this sentence is valid in the sense at we want
             * to output it to an application like Google Earth Pro, that
             * gets confused is time runs backwards (which can happen if
             * we got this sentence via a UDP datagram).
             */

            if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_GGA)) {

                /*
                 * NMEA GGA
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA GGA\n");

                rc = hazer_parse_gga(&positions[system], vector, count);
                if (rc == 0) {

                    positions[system].timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("NMEA GGA");

                } else if (errno == 0) {

                    fix_relinquished("NMEA GGA");

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_RMC)) {

                /*
                 * NMEA RMC
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA RMC\n");

                rc = hazer_parse_rmc(&positions[system], vector, count);
                if (rc == 0) {

                    positions[system].timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("NMEA RMC");

                } else if (errno == 0) {

                    fix_relinquished("NMEA RMC");

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_GLL)) {

                /*
                 * NMEA GLL
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA GLL\n");

                rc = hazer_parse_gll(&positions[system], vector, count);
                if (rc == 0) {

                    positions[system].timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("NMEA GLL");

                } else if (errno == 0) {

                    fix_relinquished("NMEA GLL");

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_VTG)) {

                /*
                 * NMEA VTG
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA VTG\n");

                rc = hazer_parse_vtg(&positions[system], vector, count);
                if (rc == 0) {

                    positions[system].timeout = timeout;
                    refresh = !0;

                } else if (errno == 0) {

                    fix_relinquished("NMEA VTG");

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_GSA)) {

                /*
                 * NMEA GSA
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA GSA\n");

                rc = hazer_parse_gsa(&active_cache, vector, count);
                if (rc == 0) {

                    /*
                     * Below is a special case for the Ublox 8 used in
                     * devices like the GN-803G. It emits multiple GSA
                     * sentences all under the GN (GNSS) talker, but the
                     * satellites are either GPS or GLONASS *plus* WAAS.
                     * We'd like to classify them as either GPS or GLONASS.
                     * Later NMEA standards (2.10+) have a field in the GSA
                     * sentence that contains a GNSS System ID. The GSA parser
                     * function uses this ID if it exists, and the map function
                     * below understands it. Also note that apparently the DOP
                     * values are computed across all the satellites in
                     * whatever constellations were used for a navigation
                     * solution; this means the DOP values for GPS and
                     * GLONASS will be identical in the Ublox 8.
                     */

                    if (system == HAZER_SYSTEM_GNSS) {
                        candidate = hazer_map_active_to_system(&active_cache);
                        if (candidate < HAZER_SYSTEM_TOTAL) {
                            system = candidate;
                        }
                    }

                    if (system > maximum) {
                        maximum = system;
                    }

                    if (!systems[system]) {
                        DIMINUTO_LOG_NOTICE("System NMEA GSA [%d] %s\n", system, HAZER_SYSTEM_NAME[system]);
                        systems[system] = true;
                    }

                    actives[system] = active_cache;
                    actives[system].timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_GSV)) {

                /*
                 * NMEA GSV
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA GSV\n");

                rc = hazer_parse_gsv(&views[system], vector, count);
                if  (rc >= 0) {

                    views[system].sig[rc].timeout = timeout;

                    if (views[system].pending == 0) {
                        refresh = !0;
                        DIMINUTO_LOG_DEBUG("Received NMEA GSV complete\n");
                    } else {
                        DIMINUTO_LOG_DEBUG("Received NMEA GSV partial\n");
                    }

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_ZDA)) {

                /*
                 * NMEA ZDA
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA ZDA\n");

                rc = hazer_parse_zda(&positions[system], vector, count);
                if (rc == 0) {

                    positions[system].timeout = timeout;
                    refresh = !0;

                    /*
                     * Apparently some devices can maintain and report the
                     * current time, perhaps by using their own real-time
                     * clocks, even after a fix has been lost. Hence, receiving
                     * the current time via an NMEA ZDA message does not
                     * necessarily indicate that a fix has been acquired. So
                     * we refresh the display with the current time, but do
                     * not (re)activate the trace, nor to we log that the
                     * fix has been (re)acquired.
                     */

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_GBS)) {
                hazer_fault_t fault = HAZER_FAULT_INITIALIZER;

                /*
                 * NMEA GBS
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA GBS\n");

                rc = hazer_parse_gbs(&fault, vector, count);
                if (rc == 0) {

                    log_fault(&fault);

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_nmea_name(buffer, length, HAZER_NMEA_SENTENCE_TXT)) {

                /*
                 * NMEA TXT
                 */

                DIMINUTO_LOG_DEBUG("Parse NMEA TXT\n");

                if  (hazer_parse_txt(vector, count) == 0) {

                    DIMINUTO_LOG_INFORMATION("Received NMEA TXT \"%.*s\"", (int)(length - 2) /* Exclude CR and LF. */, buffer);

                } else {

                    log_error(buffer, length);

                }

            } else if (talker != HAZER_TALKER_PUBX) {

                /*
                 * NMEA Other
                 */

                DIMINUTO_LOG_INFORMATION("Received NMEA Other \"%.*s\"", HAZER_NMEA_NAMEEND, buffer);

            } else if (hazer_is_pubx_id(buffer, length, HAZER_PROPRIETARY_SENTENCE_PUBX_POSITION)) {

                /*
                 * PUBX POSITION
                 */

                DIMINUTO_LOG_DEBUG("Parse PUBX POSITION\n");

                rc = hazer_parse_pubx_position(&positions[system], &actives[system], vector, count);
                if  (rc == 0) {

                    positions[system].timeout = timeout;
                    actives[system].timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("PUBX POSITION");

                } else if (errno == 0) {

                    fix_relinquished("PUBX POSITION");

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_pubx_id(buffer, length, HAZER_PROPRIETARY_SENTENCE_PUBX_SVSTATUS)) {

                /*
                 * PUBX SVSTATUS
                 */

                DIMINUTO_LOG_DEBUG("Parse PUBX SVSTATUS\n");

                rc = hazer_parse_pubx_svstatus(views, actives, vector, count);
                if (rc != 0) {

                    for (system = HAZER_SYSTEM_GNSS; system < HAZER_SYSTEM_TOTAL; ++system) {
                        if ((rc & (1 << system)) != 0) {

                            if (system > maximum) {
                                maximum = system;
                            }

                            if (!systems[system]) {
                                DIMINUTO_LOG_NOTICE("System PUBX SVSTATUS [%d] %s\n", system, HAZER_SYSTEM_NAME[system]);
                                systems[system] = true;
                            }

                            views[system].sig[0].timeout = timeout;

                            if (system == HAZER_SYSTEM_GNSS) {

                                /* Do nothing. */

                            } else if (actives[HAZER_SYSTEM_GNSS].timeout == 0) {

                                /* Do nothing. */

                            } else {

                                actives[system].mode = actives[HAZER_SYSTEM_GNSS].mode;
                                actives[system].pdop = actives[HAZER_SYSTEM_GNSS].pdop;
                                actives[system].hdop = actives[HAZER_SYSTEM_GNSS].hdop;
                                actives[system].vdop = actives[HAZER_SYSTEM_GNSS].vdop;
                                actives[system].tdop = actives[HAZER_SYSTEM_GNSS].tdop;

                            }

                            actives[system].timeout = timeout;
                            refresh = !0;
                            DIMINUTO_LOG_DEBUG("Received PUBX SVSTATUS (%s)\n", HAZER_SYSTEM_NAME[system]);
                        }
                    }

                } else {

                    log_error(buffer, length);

                }

            } else if (hazer_is_pubx_id(buffer, length, HAZER_PROPRIETARY_SENTENCE_PUBX_TIME)) {

                /*
                 * PUBX TIME
                 */

                DIMINUTO_LOG_DEBUG("Parse PUBX TIME\n");

                rc = hazer_parse_pubx_time(&positions[system], vector, count);
                if (rc == 0) {

                    /*
                     * The CAM-M8Q can report time in the PUBX,04 sentence
                     * without having a valid fix, apparently based on a prior
                     * fix and its own internal clock. This PUBX sentence also
                     * does not indicate the constellation(s) that contributed
                     * to the solution. Because this time may be purely a value
                     * synthesized by the CAM-M8Q (or any generation 8 U-blox
                     * receiver), we don't reset the position timer or indicate
                     * a refresh. We'll depend on a valid position fix (perhaps
                     * from the PUBX,00 sentence) to indicate a position
                     * refresh. We still update the time in the structure -
                     * which is why we even bother with PUBX,04.
                     */

                } else {

                    log_error(buffer, length);

                }

            } else {

                /*
                 * PUBX Other
                 */

                DIMINUTO_LOG_INFORMATION("Received PUBX Other \"%*s\"\n", HAZER_PUBX_IDEND, buffer);

            }

            break;

        case UBX:

            /*
             * UBX PACKETS
             */

            if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_HPPOSLLH_Class, YODEL_UBX_NAV_HPPOSLLH_Id)) {

                /*
                 * UBX UBX-NAV-HPPOSLLH
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-HPPOSLLH\n");

                rc = yodel_ubx_nav_hpposllh(&(solution.payload), buffer, length);
                if (rc == 0) {

                    solution.timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("UBX-NAV-HPPOSLLH");

                    if (horizontal) {
                        /* Do nothing. */
                    } else if (solution.payload.hAcc > threshold) {
                        /* Do nothing. */
                    } else {
                        DIMINUTO_LOG_NOTICE("Converged Horizontal %uccm", (unsigned int)solution.payload.hAcc);
                        horizontal = !0;
                    }

                    if (vertical) {
                        /* Do nothing. */
                    } else if (solution.payload.vAcc > threshold) {
                        /* Do nothing. */
                    } else {
                        DIMINUTO_LOG_NOTICE("Converged Vertical %uccm", (unsigned int)solution.payload.vAcc);
                        vertical = !0;
                    }

                } else if (errno == 0) {

                    fix_relinquished("UBX-NAV-HPPOSLLH");

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_MON_HW_Class, YODEL_UBX_MON_HW_Id)) {

                /*
                 * UBX UBX-MON_HW
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-MON-HW\n");

                rc = yodel_ubx_mon_hw(&(hardware.payload), buffer, length);
                if (rc == 0) {

                    hardware.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_STATUS_Class, YODEL_UBX_NAV_STATUS_Id)) {

                /*
                 * UBX UBX-NAV-STATUS
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-STATUS\n");

                rc = yodel_ubx_nav_status(&(status.payload), buffer, length);
                if (rc == 0) {

                    status.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_ACK_Class, YODEL_UBX_ACK_NAK_Id) || yodel_is_ubx_class_id(buffer, length, YODEL_UBX_ACK_Class, YODEL_UBX_ACK_ACK_Id)) {

                /*
                 * UBX UBX-ACK-ACK
                 * UBX UBX-ACK-NAK
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-ACK-ACK/NAK\n");

                rc = yodel_ubx_ack(&acknak, buffer, length);
                if (rc == 0) {

                    if (acknak.state) {
                        DIMINUTO_LOG_NOTICE("Received UBX UBX-ACK-ACK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                    } else if (!nakquit) {
                        DIMINUTO_LOG_NOTICE("Received UBX UBX-ACK-NAK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                    } else {
                        DIMINUTO_LOG_WARNING("Received UBX UBX-ACK-NAK 0x%02x 0x%02x (%d)\n", acknak.clsID, acknak.msgID, acknakpending);
                        xc = 1;
                        eof = !0;
                    }

                    acknakpending = 0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_CFG_VALGET_Class, YODEL_UBX_CFG_VALGET_Id)) {
                yodel_buffer_t valget;

                /*
                 * UBX UBX-CFG-VALGET
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-CFG-VALGET\n");

                rc = yodel_ubx_cfg_valget(valget, sizeof(valget), buffer, length);
                if (rc == 0) {

                    process_ubx_cfg_valget(valget, length);

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_MON_VER_Class, YODEL_UBX_MON_VER_Id)) {

                /*
                 * UBX UBX-MON-VER
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-MON-VER\n");

                process_ubx_mon_ver(buffer, length);

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_SVIN_Class, YODEL_UBX_NAV_SVIN_Id)) {

                /*
                 * UBX UBX-NAV-SVIN
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-SVIN\n");

                rc = yodel_ubx_nav_svin(&base.payload, buffer, length);
                if (rc == 0) {

                    base.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_ATT_Class, YODEL_UBX_NAV_ATT_Id)) {

                /*
                 * UBX UBX-NAV-ATT
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-ATT\n");

                rc = yodel_ubx_nav_att(&(attitude.payload), buffer, length);
                if (rc == 0) {

                    attitude.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_ODO_Class, YODEL_UBX_NAV_ODO_Id)) {

                /*
                 * UBX UBX-NAV-ODO
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-ODO\n");

                rc = yodel_ubx_nav_odo(&(odometer.payload), buffer, length);
                if (rc == 0) {

                    odometer.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_PVT_Class, YODEL_UBX_NAV_PVT_Id)) {

                /*
                 * UBX UBX-NAV-PVT
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-PVT\n");

                rc = yodel_ubx_nav_pvt(&(posveltim.payload), buffer, length);
                if (rc == 0) {

                    posveltim.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_RXM_RTCM_Class, YODEL_UBX_RXM_RTCM_Id)) {

                /*
                 * UBX UBX-RXM-RTCM
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-RXM-RTCM\n");

                rc = yodel_ubx_rxm_rtcm(&rover.payload, buffer, length);
                if (rc == 0) {

                    rover.timeout = timeout;
                    refresh = !0;

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_MON_COMMS_Class, YODEL_UBX_MON_COMMS_Id)) {
                yodel_buffer_t comms;

                /*
                 * UBX UBX-MON-COMMS
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-MON-COMMS\n");

                rc = yodel_ubx_mon_comms(comms, sizeof(comms), buffer, length);
                if (rc == 0) {

                    process_ubx_mon_comms(comms, length);

                } else {

                    log_error(buffer, length);

                }

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_MON_TXBUF_Class , YODEL_UBX_MON_TXBUF_Id)) {

                /*
                 * UBX UBX-MON-TXBUF
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-MON-TXBUF\n");

                /* TODO */

                fprintf(stderr, "%s: UBX-MON-TXBUF [%zd] ", Program, length);
                buffer_dump(stderr, buffer, length);

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_TIMEGPS_Class, YODEL_UBX_NAV_TIMEGPS_Id)) {

                /*
                 * UBX UBX-NAV-TIMEGPS
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-TIMEGPS\n");

                /* TODO */

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_TIMEUTC_Class, YODEL_UBX_NAV_TIMEUTC_Id)) {

                /*
                 * UBX UBX-NAV-TIMEUTC
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-TIMEUTC\n");

                /* TODO */

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_NAV_CLOCK_Class, YODEL_UBX_NAV_CLOCK_Id)) {

                /*
                 * UBX UBX-NAV-CLOCK
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-NAV-CLOCK\n");

                /* TODO */

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_TIM_TP_Class, YODEL_UBX_TIM_TP_Id)) {

                /*
                 * UBX UBX-TIM-TP
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-TIM-TP\n");

                /* TODO */

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_RXM_RAWX_Class , YODEL_UBX_RXM_RAWX_Id)) {

                /*
                 * UBX UBX-RXM-RAWX
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-RXM-RAWX\n");

                /* TODO */

            } else if (yodel_is_ubx_class_id(buffer, length, YODEL_UBX_RXM_SPARTNKEY_Class , YODEL_UBX_RXM_SPARTNKEY_Id)) {

                /*
                 * UBX UBX-RXM-SPARTNKEY
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX UBX-RXM-SPARTNKEY\n");

                /*
                 * There is a bit of a security concern here. The SPARTN
                 * encryption keys are probably sensitive. We want to look
                 * at them, but we don't want to log them to the system log
                 * where they might be more visible than we want them to be.
                 * So we write them to standard error, in the hopes that
                 * it has been redirected to somewhere more secure.
                 */

                fprintf(stderr, "%s: UBX-RXM-SPARTNKEY [%zd] ", Program, length);
                buffer_dump(stderr, buffer, length);

            } else {

                /*
                 * UBX Other
                 */

                DIMINUTO_LOG_DEBUG("Parse UBX Other 0x%02x 0x%02x\n", buffer[YODEL_UBX_CLASS], buffer[YODEL_UBX_ID]);

            }

            break;

        case RTCM:

            /*
             * RTCM MESSAGES
             */

            kinematics.source = DEVICE;

            kinematics.number = tumbleweed_message(buffer, length);
            if (kinematics.number < 0) {
                kinematics.number = 9999;
            }
            helper_collect(kinematics.number, &updates);

            kinematics.length = length;

            kinematics.timeout = timeout;
            refresh = !0;

            DIMINUTO_LOG_DEBUG("Received RTCM (%d) [%lld]\n", kinematics.number, (long long int)kinematics.length);

            break;

        case CPO:

            /*
             * CPO PACKETS
             */

            if (calico_is_cpo_id_length(buffer, length, CALICO_CPO_PVT_Id, CALICO_CPO_PVT_Length)) {

                /*
                 * CPO PVT
                 */

                DIMINUTO_LOG_DEBUG("Parse CPO PVT\n");

                system = HAZER_SYSTEM_GPS;

                rc = calico_cpo_position_record(&positions[system], buffer, length);
                if (rc == 0) {

                    if (system > maximum) { 
                        maximum = system;
                    }

                    if (!systems[system]) {
                        DIMINUTO_LOG_NOTICE("System CPO PVT [%d] %s\n", system, HAZER_SYSTEM_NAME[system]);
                        systems[system] = true;
                    }

                    positions[system].timeout = timeout;
                    refresh = !0;
                    trace = !0;

                    fix_acquired("CPO PVT");

                } else if (errno == 0) {

                    fix_relinquished("CPO PVT");

                } else {

                    log_error(buffer, length);

                }

            } else if (calico_is_cpo_id_length(buffer, length, CALICO_CPO_SDR_Id, CALICO_CPO_SDR_Length)) {

                /*
                 * CPO SDR
                 */

                DIMINUTO_LOG_DEBUG("Parse CPO SDR\n");

                rc = calico_cpo_satellite_data_record(views, actives, buffer, length);
                if (rc != 0) {
                    static const hazer_system_t SYSTEMS[] = {
                        HAZER_SYSTEM_GNSS,
                        HAZER_SYSTEM_GPS,
                        HAZER_SYSTEM_SBAS,
                    };

                    for (ii = 0; ii < diminuto_countof(SYSTEMS); ++ii) {

                        system = SYSTEMS[ii];
                        if ((rc & (1 << system)) != 0) {

                            if (system > maximum) { 
                                maximum = system;
                            }

                            if (!systems[system]) {
                                DIMINUTO_LOG_NOTICE("System CPO SDR [%d] %s\n", system, HAZER_SYSTEM_NAME[system]);
                                systems[system] = true;
                            }

                            views[system].sig[HAZER_SIGNAL_ANY].timeout = timeout;
                            actives[system].timeout = timeout;
                            refresh = !0;
                            trace = !0;

                        }

                    }

                } else if (errno == 0) {

                    /* Do nothing. */

                } else {

                    log_error(buffer, length);

                }

            } else {

                /*
                 * CPO Other
                 */

                DIMINUTO_LOG_INFORMATION("Parse CPO Other 0x%02x [%u]\n", buffer[CALICO_CPO_ID], buffer[CALICO_CPO_SIZE]);

            }

            break;

        default:

            /*
             * OTHER
             */

            DIMINUTO_LOG_WARNING("Received Unknown 0x%x\n", buffer[0]);

            break;

        }

        /*
         * If we received an EOF (or anything else that say we should
         * quit), render the output screen one last time.
         */

        if (eof) {
            goto render;
        }

        /*
         * Determine if any constellation has a valid time, date, and clock.
         * We check this after parsing the input, but use it on the next
         * iteration to decide whether to forward subsequent sentences etc.
         */

        time_valid_prior = time_valid;
        time_valid = hazer_has_valid_time(positions, maximum);
        if (time_valid == time_valid_prior) {
            /* Do nothing. */
        } else if (time_valid) {
            DIMINUTO_LOG_NOTICE("Time Valid\n");
        } else {
            DIMINUTO_LOG_NOTICE("Time Invalid\n");
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
        } else if (acknakpending) {
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
        } else if (!time_expired(&frequency_last, frequency)) {
            /* Do nothing. */
        } else {
            emit_trace(trace_fp, positions, &solution, &attitude, &posveltim, &base, hangup);
            trace = 0;
            hangup = 0;
        }

        /*
         * If tracing is enabled and we transitioned from an active
         * survey to a valid fix, disable tracing. This allows us to
         * trace until the fix is established and no longer changing.
         */

        if (trace_fp == (FILE *)0) {
            /* Do nothing. */
        } else if (base.timeout == 0) {
            /* Do nothing. */
        } else if (base.payload.active) {
            /* Do nothing. */
        } else if (!base.payload.valid) {
            /* Do nothing. */
        } else if (trace_fp == stdout) {
            trace_fp = (FILE *)0;
            DIMINUTO_LOG_NOTICE("Trace disabled\n");
        } else if ((rc = fclose(trace_fp)) != EOF) {
            trace_fp = (FILE *)0;
            DIMINUTO_LOG_NOTICE("Trace disabled\n");
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
         * and never render a report. That's what the check timeout below
         * is all about. Note that the code below is non-blocking.
         */

        DIMINUTO_LOG_DEBUG("Bottom %d\n", ((Now / Frequency) >= (bypass_last + bypass)));

        available = 0;
        ready = 0;
        fd = -1;

        if (time_expired(&bypass_last, bypass)) {

            /* Do nothing. */

        } else if (hazer_has_pending_gsv(views, maximum)) {

            fd = in_fd;
            goto consume;

        } else if ((in_fp != (FILE *)0) && ((available = diminuto_file_ready(in_fp)) > 0)) {

            fd = in_fd;
            if (available > io_maximum) {
                io_maximum = available;
            }
            goto consume;

        } else if (serial && (in_fd >= 0) && ((available = diminuto_serial_available(in_fd)) > 0)) {

            fd = in_fd;
            if (available > io_maximum) {
                io_maximum = available;
            }
            goto consume;

        } else if ((fd = diminuto_mux_ready_read(&mux)) >= 0) {

            goto consume;

        } else if ((ready = diminuto_mux_wait(&mux, 0 /* POLL */)) == 0) {

            /* Do nothing. */

        } else if (ready > 0) {

            fd = diminuto_mux_ready_read(&mux);
            diminuto_contract(fd >= 0);
            goto consume;

        } else if (errno == EINTR) {

            continue; /* Interrupt. */

        } else {

            diminuto_panic();

        }

        /**
         ** RENDER
         **/

render:

        DIMINUTO_LOG_DEBUG("Render %d %d %d\n", ((Now / Frequency) >= (slow_last + slow)), refresh, report);

        if (sink_fp != (FILE *)0) {
            fflush(sink_fp);
        }

#if defined(TEST_EXPIRATION)
#   warning TEST_EXPIRATION enabled!

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

        if ((test & TEST_EXPIRATION) == 0) {

            /* Do nothing. */

        } else if (!refresh) {

            /* Do nothing. */

        } else {
            static int crowbar = 1000;

            if (crowbar <= 0) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    positions[ii].timeout = 0;
                }
            }

            if (crowbar <= 100) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    actives[ii].timeout = 0;
                 }
            }

            if (crowbar <= 200) {
                for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
                    for (jj = 0; jj < HAZER_GNSS_SIGNALS; ++jj) {
                        views[ii].sig[jj].timeout = 0;
                    }
                }
            }

            if (crowbar <= 300) {
                hardware.timeout = 0;
            }

            if (crowbar <= 400) {
                status.timeout = 0;
            }

            if (crowbar <= 500) {
                base.timeout = 0;
            }

            if (crowbar <= 600) {
                rover.timeout = 0;
            }

            if (crowbar <= 700) {
                kinematics.timeout = 0;
            }

            if (crowbar > 0) {
                crowbar -= 1;
            }
        }

#endif

        /*
         * Generate the display if necessary and sufficient reasons exist.
         */

        if (!time_expired(&slow_last, slow)) {

            /* Do nothing. */

        } else if (refresh) {

            /*
             * If we're monitoring 1PPS, either via a GPIO pin or via DCD
             * on the device, update our copy of its status now.
             */

            if (threadp != (diminuto_thread_t *)0) {

                DIMINUTO_CRITICAL_SECTION_BEGIN(&Mutex);
                    onepps = poller.onepps;
                    onehz = poller.onehz;
                DIMINUTO_CRITICAL_SECTION_END;

                if ((pulsing) && (onehz >= TOLERANCE)) {
                    DIMINUTO_LOG_NOTICE("1PPS Lost\n");
                    pulsing = false;
                } else if ((!pulsing) && (onehz <= 0)) {
                    DIMINUTO_LOG_NOTICE("1PPS Acquired\n");
                    pulsing = true;
                } else {
                    /* Do nothing. */
                }

            }

            /*
             * UPDATE
             */

            if (escape) {
                fputs(ANSI_LOC, out_fp);
            }

            if (report) {
                print_local(out_fp);
                print_positions(out_fp, positions, maximum, onepps, pulsing, network_total);
                print_hardware(out_fp, &hardware);
                print_status(out_fp, &status);
                print_solution(out_fp, &solution);
                print_attitude(out_fp, &attitude);
                print_odometer(out_fp, &odometer);
                print_posveltim(out_fp, &posveltim);
                print_corrections(out_fp, &base, &rover, &kinematics, &updates);
                print_actives(out_fp, actives, maximum);
                if (activefirst) {
                    print_views(out_fp, views, actives, maximum, ACTIVE);
                    print_views(out_fp, views, actives, maximum, INACTIVE);
                } else {
                    print_views(out_fp, views, actives, maximum, DONTCARE);
                }
            }

            if (escape) {
                fputs(ANSI_END, out_fp);
            }

            if (report) {
                fflush(out_fp);
            }

            /*
             * If we're running headless, commit this observation to the
             * file system and start a new observation in a temporary file.
             */

            if (headless != (const char *)0) {
                if (checkpoint) {
                    out_fp = diminuto_observation_checkpoint(out_fp, &temporary);
                    diminuto_contract(out_fp != (FILE *)0);
                    checkpoint = 0;
                }
                out_fp = diminuto_observation_commit(out_fp, &temporary);
                diminuto_contract(out_fp == (FILE *)0);
                out_fp = diminuto_observation_create(headless, &temporary);
                diminuto_contract(out_fp != (FILE *)0);
            }

            refresh = 0;

        } else if (headless == (const char *)0) {

            if (escape) {
                fputs(ANSI_LOC, out_fp);
            }

            if (report) {
                print_local(out_fp);
            }

        } else {

            /*
             * The output display isn't running in real-time
             * so there's no point in updating the LOCal time.
             */

        }

        if (eof) {
            DIMINUTO_LOG_NOTICE("End");
            break;
        }

    }

    /**
     ** STOP
     **/

stop:

    DIMINUTO_LOG_NOTICE("Stop");

    if (verbose) {
        sync_end();
    }

    DIMINUTO_LOG_INFORMATION("Counters Remote=%lu Surveyor=%lu Keepalive=%lu OutOfOrder=%u Missing=%u", (unsigned long)remote_sequence, (unsigned long)surveyor_sequence, (unsigned long)keepalive_sequence, outoforder_counter, missing_counter);

    rc = calico_finalize();
    diminuto_contract(rc == 0);

    rc = tumbleweed_finalize();
    diminuto_contract(rc == 0);

    rc = yodel_finalize();
    diminuto_contract(rc == 0);

    rc = hazer_finalize();
    diminuto_contract(rc == 0);

    diminuto_mux_fini(&mux);

    if (timerp != (diminuto_timer_t *)0) {
        timerticks = diminuto_timer_stop(timerp);
        diminuto_contract(timerticks >= 0);
        timerp = diminuto_timer_fini(timerp);
        diminuto_contract(timerp == (diminuto_timer_t *)0);
    }

    if (threadp != (diminuto_thread_t *)0) {
        DIMINUTO_COHERENT_SECTION_BEGIN;
            poller.done = !0;
        DIMINUTO_COHERENT_SECTION_END;
        DIMINUTO_THREAD_BEGIN(threadp);
            threadrc = diminuto_thread_notify(threadp);
        DIMINUTO_THREAD_END;
        threadrc = diminuto_thread_join(threadp, &result);
        /* Do nothing with result. */
        diminuto_contract(threadrc == 0);
        threadp = diminuto_thread_fini(threadp);
        diminuto_contract(threadp == (diminuto_thread_t *)0);
    }

    if (pps_fd >= 0) {
        pps_fd = diminuto_line_close(pps_fd);
        diminuto_contract(pps_fd < 0);
    }

    if (strobe_fd >= 0) {
        strobe_fd = diminuto_line_close(strobe_fd);
        diminuto_contract(strobe_fd < 0);
    }

    if (remote_fd >= 0) {
        rc = diminuto_ipc_close(remote_fd);
        diminuto_contract(rc >= 0);
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

    if (queue_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (queue_fp == stdout) {
        /* Do nothing. */
    } else if ((rc = fclose(queue_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(queue_fp)");
    }

    if (listing_fp == (FILE *)0) {
        /* Do nothing. */
    } else if (listing_fp == stderr) {
        /* Do nothing. */
    } else if ((rc = fclose(listing_fp)) != EOF) {
        /* Do nothing. */
    } else {
        diminuto_perror("fclose(listing_fp)");
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

    Now = diminuto_time_elapsed();
    diminuto_contract(Now >= 0);
    if (Now > Epoch) {
        DIMINUTO_LOG_INFORMATION("Bandwidth size=%zuB maximum=%zuB total=%zuB sustained=%zuBPS\n", io_size, io_maximum, io_total, (ssize_t)((io_total * Frequency) / (Now - Epoch)));
    }

    free(io_buffer);

    if (ppspath != (const char *)0) { free(ppspath); }
    if (strobepath != (const char *)0) { free(strobepath); }

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
        diminuto_contract(out_fp == (FILE *)0);
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
        diminuto_contract(command_node != (diminuto_list_t *)0);
        free(command_node);
    }

    DIMINUTO_LOG_NOTICE("Exit");

    fflush(stderr);

    return xc;
}
