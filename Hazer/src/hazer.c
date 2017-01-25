/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hazer.h"
#include "com/diag/hazer/hazer.h"

static FILE * debug  = (FILE *)0;

#if 1
#   define DEBUG(...)   ((debug != (FILE *)0) ? fprintf(debug, __VA_ARGS__) : 0)
#else
#   define DEBUG(...)   ((void)0)
#endif

FILE * hazer_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

ssize_t hazer_sentence_read(FILE *fp, void * buffer, size_t size)
{
    char * bb;
    size_t ss;
    int ch;

    while (!0) {

        /*
         * Initialize.
         */

        bb = (char *)buffer;
        ss = size;

        DEBUG("BEGIN.\n");

        /*
         * Find '$' or '!' start character.
         */

        while (!0) {

            if ((ch = fgetc(fp)) == EOF) {
                DEBUG("EOF 0x%x!\n", ch);
                return 0;
            }

            if (ch == HAZER_NMEA_SENTENCE_START) {
                DEBUG("START '%c.'\n", ch);
                break;
            }

            if (ch == HAZER_NMEA_SENTENCE_ENCAPSULATE) {
                DEBUG("ENCAPSULATE '%c'.\n", ch);
                break;
            }

            DEBUG("SKIP 0x%x.\n", ch);

        }

        if (ss == 0) {
            DEBUG("LONG?\n");
            continue;
        }

        *(bb++) = ch;
        --ss;

        /*
         * Find '\r' penultimate character.
         */

        while (!0) {

            if ((ch = fgetc(fp)) == EOF) {
                DEBUG("EOF 0x%x!\n", ch);
                return 0;
            }

            if (ss == 0) {
                DEBUG("LONG?\n");
                break;
            }

            *(bb++) = ch;
            --ss;

            if (ch == HAZER_NMEA_SENTENCE_CR) {
                DEBUG("CR 0x%x.\n", ch);
                break;
            }

            DEBUG("SAVE '%c'.\n", ch);

        }

        if (ss == 0) {
            continue;
        }

        /*
         * Check for '\n' final character.
         */

        if ((ch = fgetc(fp)) == EOF) {
            DEBUG("EOF 0x%x!\n", ch);
            return 0;
        }

        if (ch != HAZER_NMEA_SENTENCE_LF) {
            DEBUG("LF 0x%x?\n", ch);
            continue;
        }

        DEBUG("LF 0x%x.\n", ch);

        if (ss == 0) {
            DEBUG("LONG?\n");
            continue;
        }

        *(bb++) = ch;
        --ss;

        /*
         * Provide '\0' terminator.
         */

        if (ss == 0) {
            DEBUG("LONG?\n");
            continue;
        }

        DEBUG("NUL.\n");

        *(bb++) = '\0';
        --ss;

        /*
         * Done.
         */

        DEBUG("END.\n");
        break;

    }

    return (size - ss);
}

ssize_t hazer_sentence_check(const void * buffer, size_t size)
{
    const char * bb = (const char *)buffer;
    size_t eff = size;
    size_t ss = 0;
    uint8_t ch = 0;
    uint8_t cs = 0;
    uint8_t ck = 0;

    do {

        /*
         * Ignore the terminating NUL if it exists.
         */

        if (eff == 0) {
            DEBUG("ZERO?\n");
            break;
        }

        ss = eff - 1;
        if (bb[ss] == '\0') {
            eff -= 1;
        }

        /*
         * Check length.
         */

        if (eff < HAZER_NMEA_LENGTH_MINIMUM) {
            DEBUG("SHORT?\n");
            break;
        }

        if (eff > (sizeof(hazer_buffer_t) - 1)) {
            DEBUG("LONG?\n");
            break;
        }

        /*
         * Check for '$' or '!'.
         */

        ss = 0;
        if ((bb[ss] != HAZER_NMEA_SENTENCE_START) && (bb[ss] != HAZER_NMEA_SENTENCE_ENCAPSULATE)) {
            DEBUG("START 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for 'G' and 'P' talker and ','.
         */

        ss = 1;
        if (bb[ss] != HAZER_NMEA_TALKER_GPS[0]) {
            DEBUG("TALKER 0x%x?\n", bb[ss]);
            break;
        }

        ss = 2;
        if (bb[ss] != HAZER_NMEA_TALKER_GPS[1]) {
            DEBUG("TALKER 0x%x?\n", bb[ss]);
            break;
        }

        ss = 6;
        if (bb[ss] != HAZER_NMEA_SENTENCE_DELIMITER) {
            DEBUG("DELIM 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for '*'.
         */

        ss = eff - 5;
        if (bb[ss] != HAZER_NMEA_SENTENCE_CHECKSUM) {
            DEBUG("STAR 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for [0-9A-F][0-9A-F].
         */

        ss = eff - 4;
        if (('0' <= bb[ss]) && (bb[ss] <= '9')) {
            DEBUG("MOST '%c'.\n", bb[ss]);
            ck = (bb[ss] - '0' + 0) << 4;
        } else if (('A' <= bb[ss]) && (bb[ss] <= 'F')) {
            DEBUG("MOST '%c'.\n", bb[ss]);
            ck = (bb[ss] - 'A' + 10) << 4;
        } else { 
            DEBUG("MOST 0x%x?\n", bb[ss]);
            break;
        }

        ss = eff - 3; 
        if (('0' <= bb[ss]) && (bb[ss] <= '9')) {
            DEBUG("LEAST '%c'.\n", bb[ss]);
            ck |= (bb[ss] - '0' + 0);
        } else if (('A' <= bb[ss]) && (bb[ss] <= 'F')) {
            DEBUG("LEAST '%c'.\n", bb[ss]);
            ck |= (bb[ss]- 'A' + 10);
        } else { 
            DEBUG("LEAST 0x%x?\n", bb[ss]);
            break;
        }

        DEBUG("CK 0x%x.\n", ck);

        /*
         * Compute checksum.
         */

        ss = 1;
        ch = bb[ss];
        cs = ch;
        ++ss;
        while (bb[ss] != HAZER_NMEA_SENTENCE_CHECKSUM) {
            if (!((HAZER_NMEA_SENTENCE_MINIMUM <= bb[ss]) && (bb[ss] <= HAZER_NMEA_SENTENCE_MAXIMUM))) {
                DEBUG("BAD 0x%x?\n", bb[ss]);
                break;
            }
            ch = bb[ss];
            cs ^= ch;
            ++ss;
        }

        DEBUG("CS 0x%x.\n", cs);

        if (cs != ck) {
            break;
        }

        /*
         * Check for penultimate '\r'.
         */

        ss = eff - 2;
        if (bb[ss] != HAZER_NMEA_SENTENCE_CR) {
            DEBUG("CR 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for final '\n'.
         */

        ss = eff - 1;
        if (bb[ss] != HAZER_NMEA_SENTENCE_LF) {
            DEBUG("LF 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Okay.
         */

        ss = size;

    } while (0);

    return ss;
}
