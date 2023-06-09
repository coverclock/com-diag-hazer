/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Calico module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * THIS IS A WORK IN PROGRESS
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include "com/diag/hazer/calico.h"
#include "com/diag/hazer/common.h"
#include "../src/calico.h"

/******************************************************************************
 *
 ******************************************************************************/

static FILE * debug  = (FILE *)0;

FILE * calico_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

/******************************************************************************
 *
 ******************************************************************************/

int calico_initialize(void)
{
    return 0;
}

int calico_finalize(void)
{
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/

calico_state_t calico_machine(calico_state_t state, uint8_t ch, void * buffer, size_t size, calico_context_t * pp)
{
    calico_action_t action = CALICO_ACTION_SKIP;
    calico_state_t old = state;

    /*
     * Advance state machine based on stimulus.
     */

    switch (state) {

    case CALICO_STATE_STOP:
        /* Do nothing. */
        break;

    case CALICO_STATE_START:
        if (ch == CALICO_STIMULUS_DLE) {
            pp->bp = (uint8_t *)buffer;
            pp->sz = size;
            pp->tot = 0;
            pp->ln = 0;
            pp->cc = 0;
            pp->cs = 0;
            pp->error = 0;
            state = CALICO_STATE_ID;
            action = CALICO_ACTION_SAVE;
        }
        break;

    case CALICO_STATE_ID:
        calico_checksum(ch, &(pp->cc), &(pp->cs));
        state = CALICO_STATE_SIZE;
        action = CALICO_ACTION_SAVE;
        break;

    case CALICO_STATE_SIZE:
        if (ch == CALICO_STIMULUS_DLE) {
            state = CALICO_STATE_SIZE_DLE;
            action = CALICO_ACTION_SKIP;
        } else {
            calico_checksum(ch, &(pp->cc), &(pp->cs));
            pp->ln = ch;
            state = CALICO_STATE_PAYLOAD;
            action = CALICO_ACTION_SAVE;
        }
        break;

    case CALICO_STATE_SIZE_DLE:
        calico_checksum(ch, &(pp->cc), &(pp->cs));
        pp->ln = ch;
        state = CALICO_STATE_PAYLOAD;
        action = CALICO_ACTION_SAVE;
        break;

    case CALICO_STATE_PAYLOAD:
        if (ch == CALICO_STIMULUS_DLE) {
            state = CALICO_STATE_PAYLOAD_DLE;
            action = CALICO_ACTION_SKIP;
        } else {
            calico_checksum(ch, &(pp->cc), &(pp->cs));
            if ((pp->ln--) > 1) {
                state = CALICO_STATE_PAYLOAD;
            } else {
                state = CALICO_STATE_CS;
            }
            action = CALICO_ACTION_SAVE;
        }
        break;

    case CALICO_STATE_PAYLOAD_DLE:
        calico_checksum(ch, &(pp->cc), &(pp->cs));
        if ((pp->ln--) > 1) {
            state = CALICO_STATE_PAYLOAD;
        } else {
            state = CALICO_STATE_CS;
        }
        action = CALICO_ACTION_SAVE;
        break;

    case CALICO_STATE_CS:
        if (ch == CALICO_STIMULUS_DLE) {
            state = CALICO_STATE_CS_DLE;
            action = CALICO_ACTION_SKIP;
        } else if (ch == pp->cs) {
            state = CALICO_STATE_DLE;
            action = CALICO_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = CALICO_STATE_STOP;
            action = CALICO_ACTION_TERMINATE;
        }
        break;

    case CALICO_STATE_CS_DLE:
        if (ch == pp->cs) {
            state = CALICO_STATE_DLE;
            action = CALICO_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = CALICO_STATE_STOP;
            action = CALICO_ACTION_TERMINATE;
        }
        break;

    case CALICO_STATE_DLE:
        if (ch == CALICO_STIMULUS_DLE) {
            state = CALICO_STATE_ETX;
            action = CALICO_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = CALICO_STATE_STOP;
            action = CALICO_ACTION_TERMINATE;
        }
        break;

    case CALICO_STATE_ETX:
        if (ch == CALICO_STIMULUS_ETX) {
            state = CALICO_STATE_END;
            action = CALICO_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = CALICO_STATE_STOP;
            action = CALICO_ACTION_TERMINATE;
        }
        break;

    case CALICO_STATE_END:
        break;

    /*
     * No default: must handle all cases.
     */

    }

    /*
     * Perform associated action.
     */

    switch (action) {

    case CALICO_ACTION_SKIP:
        break;

    case CALICO_ACTION_SAVE:
        if (pp->sz > 0) {
            *(pp->bp++) = ch;
            pp->sz -= 1;
        } else {
            state = CALICO_STATE_STOP;
        }
        break;

    case CALICO_ACTION_TERMINATE:
        /*
         * It seems like it's not really meaningful to NUL-terminate a binary
         * DIS packet, but it is. Doing so simplifies user code that doesn't
         * know yet the format of the data in the buffer, e.g. in the case of
         * IP datagrams. And it guarantees that we don't run off the end of
         * a DIS message.
         */
        if (pp->sz > 1) {
            *(pp->bp++) = ch;
            pp->sz -= 1;
            *(pp->bp++) = '\0';
            pp->sz -= 1;
            pp->tot = size - pp->sz;
        } else {
            state = CALICO_STATE_STOP;
        }
        break;

    /*
     * No default: must handle all cases.
     */

    }

    /*
     * Done.
     */

    if (debug == (FILE *)0) {
        /* Do nothing. */
    } else if (old == CALICO_STATE_STOP) {
        /* Do nothing. */
    } else if (isprint(ch)) {
        fprintf(debug, "Machine DIS  %c %c %c 0x%02x,0x%02x '\\x%02x' '%c'\n", old, state, action, pp->cc, pp->cs, ch, ch);
    } else {
        fprintf(debug, "Machine DIS  %c %c %c 0x%02x,0x%02x '\\x%02x'\n", old, state, action, pp->cc, pp->cs, ch);
    }

    return state;
}

/******************************************************************************
 *
 ******************************************************************************/

/*
 * The portion of the buffer being summed includes the length, but we have
 * to compute the length first to do the checksum. Seems chicken-and-egg,
 * but I've seen the same thing in TCP headers.
 */
const void * calico_checksum_buffer(const void * buffer, size_t size, int8_t * ccp, int8_t * csp)
{
    const void * result = (void *)0;
    const int8_t * bp = (const int8_t *)buffer;
    int8_t cc = 0;
    int8_t cs = 0;
    size_t length = 0;

    length = (uint8_t)bp[CALICO_DIS_SIZE];
    length += CALICO_DIS_SUMMED;

    if ((length + CALICO_DIS_UNSUMMED) <= size) {

        for (bp += CALICO_DIS_ID; length > 0; --length) {
            calico_checksum(*(bp++), &cc, &cs);
        }

        *ccp = cc;
        *csp = cs;

        result = bp;

    }

    return result;
}

ssize_t calico_length(const void * buffer, size_t size)
{
   ssize_t result = -1;
   size_t length = 0;
   const uint8_t * packet = (const uint8_t *)0;

   packet = (const uint8_t *)buffer;

   if (size < CALICO_DIS_SHORTEST) {
       /* Do nothing. */
   } else if (packet[CALICO_DIS_SYNC] != CALICO_STIMULUS_DLE) {
       /* Do nothing. */
   } else {
        length = packet[CALICO_DIS_SIZE];
        if (length <= (size - CALICO_DIS_SHORTEST)) {
            result = length;
            result += CALICO_DIS_SHORTEST;
        }
    }

   return result;
}

ssize_t calico_validate(const void * buffer, size_t size)
{
    ssize_t result = -1;
    ssize_t length = 0;
    const uint8_t * bp = (uint8_t *)0;
    int8_t cc = 0;
    int8_t cs = 0;

    if ((length = calico_length(buffer, size)) <= 0) {
        /* Do nothing. */
    } else if ((bp = (uint8_t *)calico_checksum_buffer(buffer, length, &cc, &cs)) == (unsigned char *)0) {
        /* Do nothing. */
    } else if (bp[0] != (uint8_t)cs) {
        /* Do nothing. */
    } else if (bp[1] != CALICO_STIMULUS_DLE) {
        /* Do nothing. */
    } else if (bp[2] != CALICO_STIMULUS_ETX) {
        /* Do nothing. */
    } else {
        result = length;
    }

    return result;
}
