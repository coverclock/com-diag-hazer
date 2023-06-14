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
            action = CALICO_ACTION_TERMINATE;
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
         * CPO packet, but it is. Doing so simplifies user code that doesn't
         * know yet the format of the data in the buffer, e.g. in the case of
         * IP datagrams. And it guarantees that we don't run off the end of
         * a CPO message.
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
        fprintf(debug, "Machine CPO  %c %c %c 0x%02x,0x%02x '\\x%02x' '%c'\n", old, state, action, (uint8_t)pp->cc, (uint8_t)pp->cs, ch, ch);
    } else {
        fprintf(debug, "Machine CPO  %c %c %c 0x%02x,0x%02x '\\x%02x'\n", old, state, action, (uint8_t)pp->cc, (uint8_t)pp->cs, ch);
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
const void * calico_checksum_buffer(const void * buffer, size_t size, uint8_t * ccp, uint8_t * csp)
{
    const void * result = (void *)0;
    const uint8_t * bp = (const uint8_t *)buffer;
    uint8_t cc = 0;
    uint8_t cs = 0;
    size_t length = 0;

    length = (uint8_t)bp[CALICO_CPO_SIZE];
    length += CALICO_CPO_SUMMED;

    if ((length + CALICO_CPO_UNSUMMED) <= size) {

        for (bp += CALICO_CPO_ID; length > 0; --length) {
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

   if (size < CALICO_CPO_SHORTEST) {
       /* Do nothing. */
   } else if (packet[CALICO_CPO_SYNC] != CALICO_STIMULUS_DLE) {
       /* Do nothing. */
   } else {
        length = packet[CALICO_CPO_SIZE];
        if (length <= (size - CALICO_CPO_SHORTEST)) {
            result = length;
            result += CALICO_CPO_SHORTEST;
        }
    }

   return result;
}

ssize_t calico_validate(const void * buffer, size_t size)
{
    ssize_t result = -1;
    ssize_t length = 0;
    const uint8_t * bp = (uint8_t *)0;
    uint8_t cc = 0;
    uint8_t cs = 0;

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

/******************************************************************************
 *
 ******************************************************************************/

int calico_cpo_satellite_data_record(hazer_view_t * gvp, hazer_view_t * wvp, hazer_active_t * gap, hazer_active_t * wap, const void * bp, ssize_t length)
{
    return 0;
}

int calico_cpo_position_record(hazer_position_t * gpp, const void * bp, ssize_t length)
{
    int rc= -1;
    const uint8_t * cp = (const uint8_t *)0;
    const calico_cpo_header_t * hp = (const calico_cpo_header_t *)0;
    const calico_cpo_pvt_data_packet_t * dp = (const calico_cpo_pvt_data_packet_t *)0;
    calico_cpo_pvt_data_t pvt = CALICO_CPO_PVT_DATA_INITIALIZER;

    do {

        /*
         * IDENTIFY
         */

        cp = (const uint8_t *)bp;

        if (length < sizeof(calico_cpo_header_t)) {
            errno = ENOMSG;
            break;
        }

        hp = (const calico_cpo_header_t *)&(cp[CALICO_CPO_SYNC]);

        if (hp->id != CALICO_CPO_PVT_Id) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (length < (sizeof(calico_cpo_header_t) + CALICO_CPO_PVT_Length + sizeof(calico_cpo_trailer_t))) {
            errno = ENODATA;
            break;
        }

        dp = (const calico_cpo_pvt_data_packet_t *)&(cp[CALICO_CPO_PAYLOAD]);

        COM_DIAG_CALICO_LETOH(pvt.alt, dp->alt);
        COM_DIAG_CALICO_LETOH(pvt.epe, dp->epe);
        COM_DIAG_CALICO_LETOH(pvt.eph, dp->eph);
        COM_DIAG_CALICO_LETOH(pvt.epv, dp->epv);
        COM_DIAG_CALICO_LETOH(pvt.fix, dp->fix);
        COM_DIAG_CALICO_LETOH(pvt.gps_tow, dp->gps_tow);
        COM_DIAG_CALICO_LETOH(pvt.lat, dp->lat);
        COM_DIAG_CALICO_LETOH(pvt.lon, dp->lon);
        COM_DIAG_CALICO_LETOH(pvt.lon_vel, dp->lon_vel);
        COM_DIAG_CALICO_LETOH(pvt.lat_vel, dp->lat_vel);
        COM_DIAG_CALICO_LETOH(pvt.alt_vel, dp->alt_vel);
        COM_DIAG_CALICO_LETOH(pvt.msl_hght, dp->msl_hght);
        COM_DIAG_CALICO_LETOH(pvt.leap_sec, dp->leap_sec);
        COM_DIAG_CALICO_LETOH(pvt.grmn_days, dp->grmn_days);

#if !0
        {
            int64_t lat;
            int64_t lon;
            int latd;
            uint64_t latf;
            int lond;
            uint64_t lonf;
            uint64_t ticks;

            lat = calico_format_radians2nanominutes(pvt.lat);
            hazer_format_nanominutes2degrees(lat, &latd, &latf);

            lon = calico_format_radians2nanominutes(pvt.lon);
            hazer_format_nanominutes2degrees(lon, &lond, &lonf);

            ticks = calico_format_cpo2nanoseconds(pvt.grmn_days, pvt.gps_tow, pvt.leap_sec);

            fputs("CPO PVT:\n", stderr);
            fprintf(stderr, "alt=%fm\n", pvt.alt);
            fprintf(stderr, "epe=%fm\n", pvt.epe);
            fprintf(stderr, "eph=%fm\n", pvt.eph);
            fprintf(stderr, "epv=%fm\n", pvt.epv);
            fprintf(stderr, "fix=%d\n", pvt.fix);
            fprintf(stderr, "gps_tow=%lf\n", pvt.gps_tow);
            fprintf(stderr, "lat=%lfr=%d.%07llu\n", pvt.lat, latd, (unsigned long long)latf);
            fprintf(stderr, "lon=%lfr=%d.%07llu\n", pvt.lon, lond, (unsigned long long)lonf);
            fprintf(stderr, "lon_vel=%fm/s\n", pvt.lon_vel);
            fprintf(stderr, "lat_vel=%fm/s\n", pvt.lat_vel);
            fprintf(stderr, "alt_vel=%fm/s\n", pvt.alt_vel);
            fprintf(stderr, "msl_hght=%fm:%fm\n", pvt.msl_hght, pvt.alt + pvt.msl_hght);
            fprintf(stderr, "leap_sec=%ds\n", pvt.leap_sec);
            fprintf(stderr, "grmn_days=%dd\n", pvt.grmn_days);
            fprintf(stderr, "ticks=%lluticks\n", (unsigned long long)ticks);
        }
#endif

        rc = 0;

    } while (0);

    return rc;
}
