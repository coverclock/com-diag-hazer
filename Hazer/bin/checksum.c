/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief Computes and displays the checksum of an NMEA, UBX, or RTCM message.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 *
 * ABSTRACT
 *
 * Computes and displays the checksum of an NMEA, UBX, or RTCM message.
 *
 * USAGE
 * 
 * checksum [ ESCAPEDSEQUENCE ... ]
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/diminuto/diminuto_log.h"

static const size_t UNLIMITED = ~(size_t)0;

/**
 * Print an NMEA sentence or UBX message to a stream, expanding non-printable
 * characters into escape sequences.
 * @param fp points to the FILE stream.
 * @param buffer points to the sentence or packet.
 * @param size is the size of the sentence or packet.
 * @param canonical is true to print standard special characters in hex.
 * @return 0 for success, <0 if an error occurred.
 */
static int print_buffer(FILE * fp, const void * buffer, size_t size, int canonical)
{
    const uint8_t * bb = (const uint8_t *)0;
    size_t current = 0;
    int end = 0;

    for (bb = buffer; size > 0; --size) {
        diminuto_phex_emit(fp, *(bb++), UNLIMITED, 0, 0, !!canonical, &current, &end, 0);
    }
    fputc('\n', fp);

    return 0;
}

/**
 * Emit an NMEA sentences to the specified stream after adding the ending
 * matter consisting of the checksum delimiter, the two checksum characters,
 * a carriage return, and a line feed.
 * @param fp points to the FILE stream.
 * @param sentence points to the NUL-terminated sentence.
 * @param size is the size of the NMEA sentence.
 * @return 0 for success, <0 if an error occurred.
 */
static int print_sentence(FILE * fp, const char * sentence, size_t size)
{
    int rc = -1;
    const uint8_t * bp = (const uint8_t *)0;
    char msn = '\0';
    char lsn = '\0';
    size_t length = 0;
    ssize_t validated = 0;
    uint8_t * buffer = (char *)0;

    do {
        bp = hazer_checksum_buffer(sentence, size, &msn, &lsn);
        if (bp == (uint8_t *)0) {
            errno = EINVAL;
            diminuto_perror("hazer_checksum_buffer");
            break;
        }
        length = (const uint8_t *)bp - (const uint8_t *)sentence;
        buffer = (uint8_t *)malloc(length + 5);
        if (buffer == (uint8_t *)0) {
            diminuto_perror("malloc");
            break;
        }
        strncpy(buffer, sentence, length);
        buffer[length++] = HAZER_STIMULUS_CHECKSUM;
        buffer[length++] = msn;
        buffer[length++] = lsn;
        buffer[length++] = HAZER_STIMULUS_CR;
        buffer[length++] = HAZER_STIMULUS_LF;
        validated = hazer_validate(buffer, length);
        if (validated < 0) {
            errno = EINVAL;
            diminuto_perror("hazer_validate");
        }
        DIMINUTO_LOG_DEBUG("NMEA: size=%zu length=%zu validated=%zd\n", size, length, validated);
        rc = print_buffer(stdout, buffer, length, 0);
    } while (0);
    if (buffer != (uint8_t *)0) {
        free(buffer);
    }

    return rc;
}

/**
 * Emit a UBX packet to the specified stream after adding the ending matter
 * consisting of the two Fletcher checksum bytes.
 * @param fp points to the FILE stream.
 * @param packet points to the message.
 * @param size is the size of the UBX packet.
 * @return 0 for success, <0 if an error occurred.
 */
static int print_packet(FILE * fp, const void * packet, size_t size)
{
    int rc = -1;
    const void * bp = (const void *)0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    size_t length = 0;
    ssize_t validated = 0;
    uint8_t * buffer = (uint8_t *)0;

    do {
        bp = yodel_checksum_buffer(packet, size, &ck_a, &ck_b);
        if (bp == (void *)0) {
            errno = EINVAL;
            diminuto_perror("yodel_checksum_buffer");
            break;
        }
        length = (const uint8_t *)bp - (const uint8_t *)packet;
        buffer = (uint8_t *)malloc(length + 2);
        if (buffer == (uint8_t *)0) {
            diminuto_perror("malloc");
            break;
        }
        memcpy(buffer, packet, length);
        buffer[length++] = ck_a;
        buffer[length++] = ck_b;
        validated = yodel_validate(buffer, length);
        if (validated < 0) {
            errno = EINVAL;
            diminuto_perror("yodel_validate");
            break;
        }
        DIMINUTO_LOG_DEBUG("UBX: size=%zu length=%zu validated=%zd\n", size, length, validated);
        rc = print_buffer(stdout, buffer, length, !0);
    } while (0);
    if (buffer != (uint8_t *)0) {
        free(buffer);
    }

    return rc;
}

/**
 * Emit a RTCM message to the specified stream after adding the ending matter
 * consisting of the three CRC24Q cyclic redundancy check bytes.
 * @param fp points to the FILE stream.
 * @param message points to the message.
 * @param size is the size of the UBX packet.
 * @return 0 for success, <0 if an error occurred.
 */
static int print_message(FILE * fp, const void * message, size_t size)
{
    int rc = -1;
    const void * bp = (const void *)0;
    uint8_t crc1 = 0;
    uint8_t crc2 = 0;
    uint8_t crc3 = 0;
    size_t length = 0;
    ssize_t validated = 0;
    uint8_t * buffer = (char *)0;

    do {
        bp = tumbleweed_checksum_buffer(message, size, &crc1, &crc2, &crc3);
        if (bp == (void *)0) {
            errno = EINVAL;
            diminuto_perror("tumbleweed_checksum_buffer");
            break;
        }
        length = (const uint8_t *)bp - (const uint8_t *)message;
        buffer = (uint8_t *)malloc(length + 3);
        if (buffer == (uint8_t *)0) {
            diminuto_perror("malloc");
            break;
        }
        memcpy(buffer, message, length);
        buffer[length++] = crc1;
        buffer[length++] = crc2;
        buffer[length++] = crc3;
        validated = tumbleweed_validate(buffer, length);
        if (validated < 0) {
            errno = EINVAL;
            diminuto_perror("tumblweed_validate");
        }
        DIMINUTO_LOG_DEBUG("RTCM: size=%zu length=%zu validated=%zd\n", size, length, validated);
        rc = print_buffer(stdout, buffer, length, !0);
    } while (0);
    if (buffer != (uint8_t *)0) {
        free(buffer);
    }

    return rc;
}

/**
 *
 */
int main(int argc, char * argv[])
{
    int xc = 0;
    int index = 0;
    uint8_t * buffer = (char *)0;
    size_t length = 0;
    ssize_t size = 0;
    int rc = 0;

    diminuto_log_setmask();

    for (index = 1; index < argc; index += 1) {
        buffer = argv[index];
        if (buffer[0] == '\0') {
            fputc('\n', stdout);
            DIMINUTO_LOG_WARNING("expanded: empty?");
            continue;
        }
        length = strlen(buffer) + 1;
        size = diminuto_escape_collapse(buffer, buffer, length);
        if (buffer[0] == '\0') {
            fputc('\n', stdout);
            DIMINUTO_LOG_WARNING("collapsed: empty?");
            continue;
        }
        if (buffer[0] == HAZER_STIMULUS_START) {
            rc = print_sentence(stdout, buffer, size - 1);
        } else if ((buffer[0] == YODEL_STIMULUS_SYNC_1) && (buffer[1] == YODEL_STIMULUS_SYNC_2)) {
            rc = print_packet(stdout, buffer, size - 1);
        } else if (buffer[0] == TUMBLEWEED_STIMULUS_PREAMBLE) {
            rc = print_message(stdout, buffer, size - 1);
        } else {
            rc = print_buffer(stdout, buffer, size - 1, 0);
        }
        if (rc < 0) {
            fputc('\n', stdout);
            xc = 1;
            continue;
        }
    }

    return xc;
}
