/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Dally module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * THIS IS A WORK IN PROGRESS.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "com/diag/hazer/dally.h"

static FILE * debug = (FILE *)0;

FILE * dally_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

dally_state_t dally_machine(dally_context_t * cp, int ch)
{
    dally_state_t state = DALLY_STATE_START;
    dally_byte_t byte = 0;

    state = cp->state;

    do {

        if (ch > 0xffU) {
            state = DALLY_STATE_ERROR;
            break;
        }

        byte = ch;

        switch (state) {

        case DALLY_STATE_START:
            state = DALLY_STATE_HEADING;
            /* Fall through. */

        case DALLY_STATE_HEADING:
            switch (byte) {
            case DALLY_HEADING:
                cp->packetp->d.header = byte;
                state = DALLY_STATE_FLAG;
                break;
            default:
                /* Do nothing. */
                break;
            }
            break;

        case DALLY_STATE_FLAG:
            switch (byte) {
            case DALLY_FLAG_DATA:
                cp->packetp->d.flag = byte;
                cp->wordp = &(cp->packetp->d.payload[0]);
                cp->count = DALLY_PAYLOAD_DATA_WORDS;
                state = DALLY_STATE_DATA_LOW;
                break;
            case DALLY_FLAG_REGISTER:
                cp->packetp->r.flag = byte;
                state = DALLY_STATE_REGISTER_LOW;
                break;
            case DALLY_HEADING:
                cp->packetp->d.header = byte;
                state = DALLY_STATE_FLAG;
                break;
            default:
                state = DALLY_STATE_HEADING;
                break;
            }
            break;

        case DALLY_STATE_REGISTER_LOW:
            switch (byte) {
            case DALLY_REGISTER_YEARMONTH:
            case DALLY_REGISTER_DATEHOUR:
            case DALLY_REGISTER_MINUTESECOND:
            case DALLY_REGISTER_MILLISECOND:
            case DALLY_REGISTER_MAGNETICFIELD:
            case DALLY_REGISTER_TEMPERATURE:
            case DALLY_REGISTER_QUATERNION:
                cp->word = byte;
                state = DALLY_STATE_REGISTER_HIGH;
                break;
            case DALLY_HEADING:
                cp->packetp->d.header = byte;
                state = DALLY_STATE_FLAG;
                break;
            default:
                state = DALLY_STATE_HEADING;
                break;
            }
            break;

        case DALLY_STATE_REGISTER_HIGH:
            switch (byte) {
            case 0x00:
                cp->packetp->r.reg = cp->word;
                cp->wordp = &(cp->packetp->r.payload[0]);
                cp->count = DALLY_PAYLOAD_REGISTER_WORDS;
                state = DALLY_STATE_DATA_LOW;
                break;
            case DALLY_HEADING:
                cp->packetp->d.header = byte;
                state = DALLY_STATE_FLAG;
                break;
            default:
                state = DALLY_STATE_HEADING;
                break;
            }
            break;

        case DALLY_STATE_DATA_LOW:
            cp->word = byte;
            state = DALLY_STATE_DATA_HIGH;
            break;

        case DALLY_STATE_DATA_HIGH:
            cp->word |= byte << 8;
            *((cp->wordp)++) = cp->word;
            cp->count -= 1;
            if (cp->count > 0) {
                state = DALLY_STATE_DATA_LOW;
            } else {
                state = DALLY_STATE_FINAL;
            }
            break;

        case DALLY_STATE_FINAL:
        case DALLY_STATE_ERROR:
            break;

        default:
            state = DALLY_STATE_ERROR;
            break;

        }

    } while (0);

    if (debug != (FILE *)0) {
        fprintf(debug, "dally_machine: state %c char 0x%8.8x byte 0x%2.2x state %c word 0x%4.4x count %lu\n", cp->state, ch, byte, state, (cp->word & 0xffff), cp->count);
    }

    cp->state = state;

    return state;
}
