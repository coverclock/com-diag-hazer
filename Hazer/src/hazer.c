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

hazer_state_t hazer_nmea_machine(hazer_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp)
{
    int done = !0;
    hazer_action_t action = HAZER_ACTION_SKIP;

    /*
     * Short circuit state machine for certain characters.
     */

    switch (ch) {

    case EOF:
        DEBUG("EOF %d!\n", ch);
        state = HAZER_STATE_EOF;
        break;

    case HAZER_NMEA_CHARACTER_START:
        DEBUG("STARTING '%c'?\n", ch);
        state = HAZER_STATE_START;
        break;

    case HAZER_NMEA_CHARACTER_ENCAPSULATION:
        DEBUG("STARTING '%c'?\n", ch);
        state = HAZER_STATE_START;
        break;

    case HAZER_NMEA_CHARACTER_CR:
        /* Do nothing. */
        break;

    case HAZER_NMEA_CHARACTER_LF:
        /* Do nothing. */
        break;

    default:
        if (!((HAZER_NMEA_CHARACTER_MINIMUM <= ch) && (ch <= HAZER_NMEA_CHARACTER_MAXIMUM))) {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    }

    /*
     * Advance state machine based on stimulus.
     */

    switch (state) {

    case HAZER_STATE_EOF:
        *bp = (char *)buffer;
        *sp = 0;
        break;

    case HAZER_STATE_START:
        if (ch == HAZER_NMEA_CHARACTER_START) {
            DEBUG("START '%c'.\n", ch);
            state = HAZER_STATE_TALKER_1;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else if (ch == HAZER_NMEA_CHARACTER_ENCAPSULATION) {
            DEBUG("ENCAPSULATE '%c'.\n", ch);
            state = HAZER_STATE_CHECKSUM;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else {
            /* Do nothing. */
        }
        break;

    case HAZER_STATE_TALKER_1:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_TALKER_2;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_TALKER_2:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_1;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_1:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_2;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_2:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_3;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_3:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_DELIMITER;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_DELIMITER:
        if (ch == HAZER_NMEA_CHARACTER_DELIMITER) {
            state = HAZER_STATE_CHECKSUM;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CHECKSUM:
        if (ch == HAZER_NMEA_CHARACTER_CHECKSUM) {
            state = HAZER_STATE_CHECKSUM_1;
        }
        action = HAZER_ACTION_SAVE;
        break;

    case HAZER_STATE_CHECKSUM_1:
        if ((HAZER_NMEA_CHARACTER_DECMIN <= ch) && (ch <= HAZER_NMEA_CHARACTER_DECMAX)) {
            state = HAZER_STATE_CHECKSUM_2;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_NMEA_CHARACTER_HEXMIN <= ch) && (ch <= HAZER_NMEA_CHARACTER_HEXMAX)) {
            state = HAZER_STATE_CHECKSUM_2;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CHECKSUM_2:
        if ((HAZER_NMEA_CHARACTER_DECMIN <= ch) && (ch <= HAZER_NMEA_CHARACTER_DECMAX)) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_NMEA_CHARACTER_HEXMIN <= ch) && (ch <= HAZER_NMEA_CHARACTER_HEXMAX)) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CR:
        if (ch == HAZER_NMEA_CHARACTER_CR) {
            state = HAZER_STATE_LF;
            action = HAZER_ACTION_SAVESPECIAL;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_LF:
        if (ch == HAZER_NMEA_CHARACTER_LF) {
            state = HAZER_STATE_END;
            action = HAZER_ACTION_TERMINATE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_END:
        DEBUG("END 0x%x!\n", ch);
        break;

    }

    /*
     * Perform associated action.
     */

    switch (action) {

    case HAZER_ACTION_SKIP:
        DEBUG("SKIP 0x%x?\n", ch);
        break;

    case HAZER_ACTION_SAVE:
        if ((*sp) > 0) {
            *((*bp)++) = ch;
            (*sp) -= 1;
            DEBUG("SAVE '%c'.\n", ch);
        } else {
            state = HAZER_STATE_START;
            DEBUG("LONG!\n");
        }
        break;

    case HAZER_ACTION_SAVESPECIAL:
        if ((*sp) > 0) {
            *((*bp)++) = ch;
            (*sp) -= 1;
            DEBUG("SAVE 0x%x.\n", ch);
        } else {
            state = HAZER_STATE_START;
            DEBUG("LONG!\n");
        }
        break;

    case HAZER_ACTION_TERMINATE:
        if ((*sp) > 1) {
            *((*bp)++) = ch;
            (*sp) -= 1;
            DEBUG("SAVE 0x%x.\n", ch);
            *((*bp)++) = '\0';
            (*sp) -= 1;
            DEBUG("SAVE 0x%x.\n", '\0');
            (*sp) = size - (*sp);
        } else {
            state = HAZER_STATE_START;
            DEBUG("LONG!\n");
        }
        break;

    }

    /*
     * Done.
     */

    return state;
}

ssize_t hazer_nmea_read(FILE *fp, void * buffer, size_t size)
{
    hazer_state_t state = HAZER_STATE_START;
    char * bb = (char *)0;
    size_t ss = 0;
    int ch = EOF;

    do {
        ch = fgetc(fp);
        state = hazer_nmea_machine(state, ch, buffer, size, &bb, &ss);
    } while ((state != HAZER_STATE_END) && (HAZER_STATE_EOF));

    return ss;
}

ssize_t hazer_nmea_check(const void * buffer, size_t size)
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

        if (eff < HAZER_NMEA_CONSTANT_SHORTEST) {
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
        if ((bb[ss] != HAZER_NMEA_CHARACTER_START) && (bb[ss] != HAZER_NMEA_CHARACTER_ENCAPSULATION)) {
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
        if (bb[ss] != HAZER_NMEA_CHARACTER_DELIMITER) {
            DEBUG("DELIM 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for '*'.
         */

        ss = eff - 5;
        if (bb[ss] != HAZER_NMEA_CHARACTER_CHECKSUM) {
            DEBUG("STAR 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for [0-9A-F][0-9A-F].
         */

        ss = eff - 4;
        if ((HAZER_NMEA_CHARACTER_DECMIN <= bb[ss]) && (bb[ss] <= HAZER_NMEA_CHARACTER_DECMAX)) {
            DEBUG("MOST '%c'.\n", bb[ss]);
            ck = (bb[ss] - HAZER_NMEA_CHARACTER_DECMIN + 0) << 4;
        } else if ((HAZER_NMEA_CHARACTER_HEXMIN <= bb[ss]) && (bb[ss] <= HAZER_NMEA_CHARACTER_HEXMAX)) {
            DEBUG("MOST '%c'.\n", bb[ss]);
            ck = (bb[ss] - HAZER_NMEA_CHARACTER_HEXMIN + 10) << 4;
        } else { 
            DEBUG("MOST 0x%x?\n", bb[ss]);
            break;
        }

        ss = eff - 3; 
        if ((HAZER_NMEA_CHARACTER_DECMIN <= bb[ss]) && (bb[ss] <= HAZER_NMEA_CHARACTER_DECMAX)) {
            DEBUG("LEAST '%c'.\n", bb[ss]);
            ck |= (bb[ss] - HAZER_NMEA_CHARACTER_DECMIN + 0);
        } else if ((HAZER_NMEA_CHARACTER_HEXMIN <= bb[ss]) && (bb[ss] <= HAZER_NMEA_CHARACTER_HEXMAX)) {
            DEBUG("LEAST '%c'.\n", bb[ss]);
            ck |= (bb[ss]- HAZER_NMEA_CHARACTER_HEXMIN + 10);
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
        while (bb[ss] != HAZER_NMEA_CHARACTER_CHECKSUM) {
            if (!((HAZER_NMEA_CHARACTER_MINIMUM <= bb[ss]) && (bb[ss] <= HAZER_NMEA_CHARACTER_MAXIMUM))) {
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
        if (bb[ss] != HAZER_NMEA_CHARACTER_CR) {
            DEBUG("CR 0x%x?\n", bb[ss]);
            break;
        }

        /*
         * Check for final '\n'.
         */

        ss = eff - 1;
        if (bb[ss] != HAZER_NMEA_CHARACTER_LF) {
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

size_t hazer_sentence_tokenize(hazer_vector_t vector, void * buffer, size_t size)
{
    char * bb = buffer;
    char ** vv = vector;
    size_t nn = 0;

}
