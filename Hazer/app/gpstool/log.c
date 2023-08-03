/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Print API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_absolute.h"
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_types.h"
#include "com/diag/hazer/common.h"
#include "com/diag/hazer/hazer_version.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "constants.h"
#include "globals.h"
#include "log.h"
#include "test.h"
#include "types.h"

void log_fault(const hazer_fault_t * tp)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    uint64_t nanoseconds = 0;
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    int signal = 0;

    hazer_format_nanoseconds2timestamp(tp->utc_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);

    if (tp->talker >= HAZER_TALKER_TOTAL) {
        talker = HAZER_TALKER_GNSS;
    } else {
        talker = tp->talker;
    }

    system = hazer_map_nmea_to_system(tp->system);
    if (system >= HAZER_SYSTEM_TOTAL) {
        system = HAZER_SYSTEM_GNSS;
    }

    if (tp->signal >= HAZER_GNSS_SIGNALS) {
        signal = 0;
    } else {
        signal = tp->signal;
    }

    diminuto_log_log(DIMINUTO_LOG_PRIORITY_NOTICE, "Fault %02d:%02d:%02dZ %s %s %s %d %.3lfm %.3lfm %.3lfm %.3lf%% %.3lfm %.3lf\n",
        hour, minute, second,
        HAZER_TALKER_NAME[talker],
        HAZER_SYSTEM_NAME[system],
        HAZER_SIGNAL_NAME[system][signal],
        tp->id,
        (double)(tp->lat_millimeters) / 1000.0,
        (double)(tp->lon_millimeters) / 1000.0,
        (double)(tp->alt_millimeters) / 1000.0,
        (double)(tp->probability) / 1000.0,
        (double)(tp->est_millimeters) / 1000.0,
        (double)(tp->std_deviation) / 1000.0);
}

/*
 * This is an expensive function. But we only call it if the GPS source
 * sends us a malformed sentence/packet/message. That's a pretty serious
 * failure, which is why we log at WARNING.
 *
 * I could have used the diminuto_escape_expand() function here, but
 * mostly wanted an excuse to see if this approach worked. The idea
 * is to emit a string that not only captures the bad data but which
 * could be cut and pasted into a C program or a CLI command.
 */
void log_error_f(const char * file, int line, const void * buffer, ssize_t length)
{
    int error = errno;
    const uint8_t * bp = (const uint8_t *)buffer;
    size_t ll = length;
    unsigned char * expanded = (unsigned char *)0;
    unsigned char * ep = (unsigned char *)0;
    static unsigned char HEX[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    if (buffer == (const void *)0) {
        errno = EINVAL;
        diminuto_perror("log_error_f: buffer");
        return;
    }

    if (length <= 0) {
        errno = EINVAL;
        diminuto_perror("log_error_f: length");
        return;
    }

    expanded = (unsigned char *)alloca((length * (sizeof("\\xab") - 1)) + 1); /* No error return. */
    ep = expanded;

    while ((ll--) > 0) {
        if (*bp == '\\') {
            *(ep++) = '\\';
            *(ep++) = *(bp++);
        } else if (*bp == '"') {
            *(ep++) = '\\';
            *(ep++) = *(bp++);
        } else if (isprint(*bp)) {
            *(ep++) = *(bp++);
        } else {
            *(ep++) = '\\';
            *(ep++) = 'x';
            *(ep++) = HEX[(*bp & 0xf0) >> 4];
            *(ep++) = HEX[*(bp++) & 0x0f];
        }
    }
    *(ep++) = '\0';

    diminuto_log_log(DIMINUTO_LOG_PRIORITY_WARNING, "%s@%d: \"%s\"[%zu]: \"%s\" (%d)\n", file, line, expanded, length, strerror(error), error);

    errno = error;
}

/*

#include <stdio.h>
void main(void) {
    int data;
    for (data = 0; data < 256; ++data) {
        if (data == 0) { printf("%16s'\\x%2.2x', ", "", data); }
        else if ((data % 8) == 0) { printf("\n%16s'\\x%2.2x', ", "", data); }
        else { printf("'\\x%2.2x', ", data); }
    }
    putchar('\n');
}

*/

#if defined(TEST_ERROR)
#   warning TEST_ERROR enabled!

void log_error_t1(void)
{
    static const uint8_t BUFFER[] = {
        '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', 
        '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0f', 
        '\x10', '\x11', '\x12', '\x13', '\x14', '\x15', '\x16', '\x17', 
        '\x18', '\x19', '\x1a', '\x1b', '\x1c', '\x1d', '\x1e', '\x1f', 
        '\x20', '\x21', '\x22', '\x23', '\x24', '\x25', '\x26', '\x27', 
        '\x28', '\x29', '\x2a', '\x2b', '\x2c', '\x2d', '\x2e', '\x2f', 
        '\x30', '\x31', '\x32', '\x33', '\x34', '\x35', '\x36', '\x37', 
        '\x38', '\x39', '\x3a', '\x3b', '\x3c', '\x3d', '\x3e', '\x3f', 
        '\x40', '\x41', '\x42', '\x43', '\x44', '\x45', '\x46', '\x47', 
        '\x48', '\x49', '\x4a', '\x4b', '\x4c', '\x4d', '\x4e', '\x4f', 
        '\x50', '\x51', '\x52', '\x53', '\x54', '\x55', '\x56', '\x57', 
        '\x58', '\x59', '\x5a', '\x5b', '\x5c', '\x5d', '\x5e', '\x5f', 
        '\x60', '\x61', '\x62', '\x63', '\x64', '\x65', '\x66', '\x67', 
        '\x68', '\x69', '\x6a', '\x6b', '\x6c', '\x6d', '\x6e', '\x6f', 
        '\x70', '\x71', '\x72', '\x73', '\x74', '\x75', '\x76', '\x77', 
        '\x78', '\x79', '\x7a', '\x7b', '\x7c', '\x7d', '\x7e', '\x7f', 
        '\x80', '\x81', '\x82', '\x83', '\x84', '\x85', '\x86', '\x87', 
        '\x88', '\x89', '\x8a', '\x8b', '\x8c', '\x8d', '\x8e', '\x8f', 
        '\x90', '\x91', '\x92', '\x93', '\x94', '\x95', '\x96', '\x97', 
        '\x98', '\x99', '\x9a', '\x9b', '\x9c', '\x9d', '\x9e', '\x9f', 
        '\xa0', '\xa1', '\xa2', '\xa3', '\xa4', '\xa5', '\xa6', '\xa7', 
        '\xa8', '\xa9', '\xaa', '\xab', '\xac', '\xad', '\xae', '\xaf', 
        '\xb0', '\xb1', '\xb2', '\xb3', '\xb4', '\xb5', '\xb6', '\xb7', 
        '\xb8', '\xb9', '\xba', '\xbb', '\xbc', '\xbd', '\xbe', '\xbf', 
        '\xc0', '\xc1', '\xc2', '\xc3', '\xc4', '\xc5', '\xc6', '\xc7', 
        '\xc8', '\xc9', '\xca', '\xcb', '\xcc', '\xcd', '\xce', '\xcf', 
        '\xd0', '\xd1', '\xd2', '\xd3', '\xd4', '\xd5', '\xd6', '\xd7', 
        '\xd8', '\xd9', '\xda', '\xdb', '\xdc', '\xdd', '\xde', '\xdf', 
        '\xe0', '\xe1', '\xe2', '\xe3', '\xe4', '\xe5', '\xe6', '\xe7', 
        '\xe8', '\xe9', '\xea', '\xeb', '\xec', '\xed', '\xee', '\xef', 
        '\xf0', '\xf1', '\xf2', '\xf3', '\xf4', '\xf5', '\xf6', '\xf7', 
        '\xf8', '\xf9', '\xfa', '\xfb', '\xfc', '\xfd', '\xfe', '\xff', 
    };
    errno = ENOTSUP;
    log_error(BUFFER, sizeof(BUFFER));
}

void log_error_t2(void)
{
    /* This data was cut and pasted from the output of log_error_t1(). */
    static const uint8_t BUFFER[] = "\
\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7f\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff\
";
    static int ch;
    diminuto_contract(sizeof(BUFFER) == 257); /* Including terminating NUL. */
    for (ch = 0; ch <= 256; ++ch) {
        diminuto_contract(BUFFER[ch] == (uint8_t)ch);
    }
    diminuto_contract(BUFFER[257] == '\0');
}

#endif
