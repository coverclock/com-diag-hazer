/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/diminuto/diminuto_escape.h"
#include "com/diag/diminuto/diminuto_phex.h"

static const size_t UNLIMITED = ~(size_t)0;

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
 * Emit an NMEA sentences to the specified stream after adding the ending
 * matter consisting of the checksum delimiter, the two checksum characters,
 * a carriage return, and a line feed.
 * @param fp points to the FILE stream.
 * @param string points to the NUL-terminated sentence minus the ending matter.
 * @param size is the size of the NMEA sentence.
 * @return 0 for success, <0 if an error occurred.
 */
static int print_sentence(FILE * fp, const char * string, size_t size)
{
    int rc = -1;
    const char * bp = (const char *)0;
    uint8_t cs = 0;
    char msn = '\0';
    char lsn = '\0';
    size_t length = 0;
    char * buffer = (char *)0;

    do {
        bp = hazer_checksum(string, size, &cs);
        if (bp == (char *)0) { break; }
        if (hazer_checksum2characters(cs, &msn, &lsn) < 0) { break; }
        length = (const char *)bp - (const char *)string;
        buffer = malloc(length + 6);
        if (buffer == (char *)0) { break; }
        strncpy(buffer, string, length);
        buffer[length + 0] = '*';
        buffer[length + 1] = msn;
        buffer[length + 2] = lsn;
        buffer[length + 3] = '\r';
        buffer[length + 4] = '\n';
        buffer[length + 5] = '\0';
        print_buffer(stdout, buffer, length + 5, UNLIMITED);
        rc = 0;
    } while (0);
    if (buffer != (char *)0) { free(buffer); }

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
static int print_message(FILE * fp, const void * packet, size_t size)
{
    int rc = -1;
    const void * bp = (const void *)0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    size_t length = 0;
    char * buffer = (char *)0;

    do {
        bp = yodel_checksum(packet, size, &ck_a, &ck_b);
        if (bp == (void *)0) { break; }
        length = (const char *)bp - (const char *)packet;
        buffer = malloc(length + 3);
        if (buffer == (char *)0) { break; }
        strncpy(buffer, packet, length);
        buffer[length + 0] = ck_a;
        buffer[length + 1] = ck_b;
        buffer[length + 2] = '\0';
        print_buffer(stdout, buffer, length + 2, UNLIMITED);
        rc = 0;
    } while (0);
    if (buffer != (char *)0) { free(buffer); }

    return rc;
}

/**
 *
 */
int main(int argc, char * argv[])
{
    int index = 0;
    char * buffer = (char *)0;
    size_t length = 0;
    ssize_t size = 0;
    int rc = 0;

    for (index = 1; index < argc; index += 1) {
        buffer = argv[index++];
        if (buffer[0] == '\0') {
            fputc('\n', stdout);
            continue;
        }
        length = strlen(buffer) + 1;
        size = diminuto_escape_collapse(buffer, buffer, length);
        if (buffer[0] == '\0') {
            fputc('\n', stdout);
            continue;
        }
        if (size < length) {
            rc = print_message(stdout, buffer, size - 1);
        } else {
            rc = print_sentence(stdout, buffer, size - 1);
        }
        if (rc < 0) {
            fputc('\n', stdout);
            continue;
        }
    }

    return 0;
}
