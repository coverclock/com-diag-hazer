/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Calico module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * THIS IS A WORK IN PROGRESS.
 * This is the implementation of the support for the Garmin GPS-18x PC
 * binary serial output format. This format, described in Garmin's
 * "GPS 18x TECHNICAL SPECIFICATIONS", is not well documented. Much of
 * this code is the result of reverse engineering the output of the
 * device, along with some guesswork.
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include "com/diag/hazer/calico.h"
#include "../src/calico.h"

static const char * LABEL = "CPO";

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
 * to compute the length first to do the checksum. Seems chicken-and-egg.
 * Other framing algorithms use a separate checksum or CRC just for the
 * header containing the length. (The DLE framing device, implemented in
 * machine, is kinda poor, too; see HDLC for a better example, implemented
 * in Diminuto's framer feature.)
 */
const void * calico_checksum_buffer(const void * buffer, size_t size, uint8_t * ccp, uint8_t * csp)
{
    const void * result = (void *)0;
    const uint8_t * bp = (const uint8_t *)buffer;
    uint8_t cc = 0;
    uint8_t cs = 0;
    size_t length = 0;

    if (size >= CALICO_CPO_SHORTEST) {

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

int calico_cpo_satellite_data_record(hazer_views_t viewa, hazer_actives_t activea, const void * bp, ssize_t length)
{
    int rc = 0;
    const uint8_t * cp = (const uint8_t *)0;
    const calico_cpo_header_t * hp = (const calico_cpo_header_t *)0;
    const calico_cpo_sdr_array_packet_t * rp = (const calico_cpo_sdr_array_packet_t *)0;
    const calico_cpo_sdr_packet_t * dp = (const calico_cpo_sdr_packet_t *)0;
    calico_cpo_sdr_t sdr = CALICO_CPO_SDR_INITIALIZER;
    int ii = 0;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_view_t * vp = (hazer_view_t *)0;
    hazer_band_t * sp = (hazer_band_t *)0;
    hazer_satellite_t * ip = (hazer_satellite_t *)0;
    hazer_active_t * ap = (hazer_active_t *)0;
    int vi = 0;
    int ai = 0;

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

        if (hp->id != CALICO_CPO_SDR_Id) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (length != (sizeof(calico_cpo_header_t) + CALICO_CPO_SDR_Length + sizeof(calico_cpo_trailer_t))) {
            errno = ENODATA;
            break;
        }

        rp = (const calico_cpo_sdr_array_packet_t *)&(cp[CALICO_CPO_PAYLOAD]);

        /*
         * CONVERT
         */

        system = HAZER_SYSTEM_GNSS;
        viewa[system].signals = 0;
        viewa[system].signal = 0;
        viewa[system].pending = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].channels = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].visible = 0;
        activea[HAZER_SYSTEM_GNSS].active = 0;

        system = HAZER_SYSTEM_GPS;
        viewa[system].signals = 0;
        viewa[system].signal = 0;
        viewa[system].pending = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].channels = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].visible = 0;
        activea[system].active = 0;

        system = HAZER_SYSTEM_SBAS;
        viewa[system].signals = 0;
        viewa[system].signal = 0;
        viewa[system].pending = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].channels = 0;
        viewa[system].sig[HAZER_SIGNAL_ANY].visible = 0;
        activea[system].active = 0;

        for (ii = 0; ii < CALICO_CPO_SDR_Count; ++ii) {

            dp = &(rp->sat[ii]);

            COM_DIAG_CALICO_LETOH(sdr.svid, dp->svid);
            COM_DIAG_CALICO_LETOH(sdr.snr, dp->snr);
            COM_DIAG_CALICO_LETOH(sdr.elev, dp->elev);
            COM_DIAG_CALICO_LETOH(sdr.azmth, dp->azmth);
            COM_DIAG_CALICO_LETOH(sdr.status, dp->status);

#if 0
            fprintf(stderr, "CPO SDR[%d]: svid=%u snr=%u elev=%u azmth=%u status=0x%x\n", ii, sdr.svid, sdr.snr, sdr.elev, sdr.azmth, sdr.status);
#endif

            system = calico_map_cposvid_to_system(sdr.svid);
            if (system >= HAZER_SYSTEM_TOTAL) {
                continue;
            }

            vp = &(viewa[system]);
            sp = &(viewa[system].sig[HAZER_SIGNAL_ANY]);
            ap = &(activea[system]);

            vi = sp->channels;
            if (vi < HAZER_GNSS_SATELLITES) {

                ip = &(viewa[system].sig[HAZER_SIGNAL_ANY].sat[vi++]);
                ip->id = sdr.svid;
                ip->elv_degrees = sdr.elev;
                ip->azm_degrees = sdr.azmth;
                if ((sdr.status & (CALICO_CPO_SDR_STATUS_Solution | CALICO_CPO_SDR_STATUS_Augmentation)) == 0) {
                    ip->snr_dbhz = 0;
                } else {
                    ip->snr_dbhz = sdr.snr / 100; /* Guessing. */
                }
                ip->phantom = ((sdr.status & (CALICO_CPO_SDR_STATUS_Ephemeris | CALICO_CPO_SDR_STATUS_Augmentation)) == 0);
                ip->untracked = ((sdr.status & (CALICO_CPO_SDR_STATUS_Correction | CALICO_CPO_SDR_STATUS_Augmentation)) == 0);
                ip->unused = ((sdr.status & (CALICO_CPO_SDR_STATUS_Solution | CALICO_CPO_SDR_STATUS_Augmentation)) == 0);
                sp->channels = vi;
                sp->visible = vi;
                vp->signals = 1;
                vp->signal = HAZER_SIGNAL_ANY;
                vp->pending = 0;

                vp->label = LABEL;

                if ((sdr.status & CALICO_CPO_SDR_STATUS_Solution) != 0) {

                    ai = ap->active;
                    if (ai < HAZER_GNSS_ACTIVES) {

                        ap->id[ai++] = sdr.svid;
                        ap->active = ai;
                        ap->pdop = HAZER_GNSS_DOP;
                        ap->hdop = HAZER_GNSS_DOP;
                        ap->vdop = HAZER_GNSS_DOP;
                        ap->tdop = HAZER_GNSS_DOP;
                        ap->system = system;
                        ap->mode = HAZER_MODE_UNKNOWN;

                        ap->label = LABEL;
                    }

                }

            }

            rc |= (1 << system);

        }

        errno = 0;

    } while (0);

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int calico_cpo_position_record(hazer_position_t * gpp, const void * bp, ssize_t length)
{
    int rc= -1;
    const uint8_t * cp = (const uint8_t *)0;
    const calico_cpo_header_t * hp = (const calico_cpo_header_t *)0;
    const calico_cpo_pvt_packet_t * dp = (const calico_cpo_pvt_packet_t *)0;
    calico_cpo_pvt_t pvt = CALICO_CPO_PVT_INITIALIZER;
    double fvalue = 0.0;
    uint64_t tvalue = 0;
    uint64_t ivalue = 0;
    uint64_t nanoseconds = 0;
    static const uint64_t DAY = 24ULL * 60ULL * 60ULL * 1000000000ULL;

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

        if (length != (sizeof(calico_cpo_header_t) + CALICO_CPO_PVT_Length + sizeof(calico_cpo_trailer_t))) {
            errno = ENODATA;
            break;
        }

        dp = (const calico_cpo_pvt_packet_t *)&(cp[CALICO_CPO_PAYLOAD]);

        /*
         * CONVERT
         */

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

#if 0
        fputs("CPO PVT:\n", stderr);
        fprintf(stderr, "alt=%fm\n", pvt.alt);
        fprintf(stderr, "epe=%fm\n", pvt.epe);
        fprintf(stderr, "eph=%fm\n", pvt.eph);
        fprintf(stderr, "epv=%fm\n", pvt.epv);
        fprintf(stderr, "fix=%d\n", pvt.fix);
        fprintf(stderr, "gps_tow=%lf\n", pvt.gps_tow);
        fprintf(stderr, "lat=%lfr\n", pvt.lat);
        fprintf(stderr, "lon=%lfr\n", pvt.lon);
        fprintf(stderr, "lon_vel=%fm/s\n", pvt.lon_vel);
        fprintf(stderr, "lat_vel=%fm/s\n", pvt.lat_vel);
        fprintf(stderr, "alt_vel=%fm/s\n", pvt.alt_vel);
        fprintf(stderr, "msl_hght=%fm\n", pvt.msl_hght);
        fprintf(stderr, "leap_sec=%ds\n", pvt.leap_sec);
        fprintf(stderr, "grmn_days=%dd\n", pvt.grmn_days);
#endif

        /*
         * APPLY
         */

        switch (pvt.fix) {
        case CALICO_CPO_PVT_FIX_None:
        case CALICO_CPO_PVT_FIX_StillNone:
            gpp->quality = HAZER_QUALITY_NOFIX;
            break;
        case CALICO_CPO_PVT_FIX_2D:
            gpp->quality = HAZER_QUALITY_AUTONOMOUS;
            break;
        case CALICO_CPO_PVT_FIX_3D:
            gpp->quality = HAZER_QUALITY_AUTONOMOUS;
            break;
        case CALICO_CPO_PVT_FIX_2DDifferential:
            gpp->quality = HAZER_QUALITY_DIFFERENTIAL;
            break;
        case CALICO_CPO_PVT_FIX_3DDifferential:
            gpp->quality = HAZER_QUALITY_DIFFERENTIAL;
            break;
        default:
            gpp->quality = HAZER_QUALITY_TOTAL;
            break;
        }

        if (gpp->quality == HAZER_QUALITY_NOFIX) {
            errno = 0;
            break;
        }

        /*
         * Convert radians used by Garmin to nanominutes used
         * by Hazer.
         */

        fvalue = (180.0 * 60.0 * 1000000000.0) / M_PI;
        gpp->lat_nanominutes = pvt.lat * fvalue;
        gpp->lon_nanominutes = pvt.lon * fvalue;

        /*
         * Convert altitude and and Mean Sea Level height in
         * meters used by Garmin to MSL altitude and WGS84
         * ellipsoid separation in millimeters used by Hazer.
         * I'm guessing as this, since the Garmin documentation
         * isn't IMO completely clear.
         */

        gpp->alt_millimeters = (pvt.alt - pvt.msl_hght) * 1000.0;
        gpp->sep_millimeters = -pvt.msl_hght * 1000.0;

        /*
         * Useful commands:
         *
         * POSIX epoch offset in seconds: 0
         *
         *  date -u --date='1970-01-01 00:00:00 UTC' +'%s'
         *
         * POSIX epoch calendar:
         *
         *  cal 1 1970
         *
         * GPS epoch offset in seconds: 315964800
         *
         *  date -u --date='1980-01-06 00:00:00 UTC' +'%s'
         *
         * GPS epoch calendar:
         *
         *  cal 1 1980
         *
         * Garmin epoch offset in seconds: 631065600
         *
         *  date -u --date='1989-12-31 00:00:00 UTC' +'%s'
         *
         * Garmin epoch calendar:
         *
         *  cal 12 1989
         *
         * Current date and time as UTC in ISO8601.
         *
         *  date -u --iso-8601=seconds
         */

        /*
         * Start with the fixed Garmin epoch offset from the POSIX epoch.
         * Generated using the Linux/GNU date command above, this includes
         * the leap seconds between the POSIX epoch and the Garmin epoch. The
         * Garmin epoch is on a Sunday.
         */

        ivalue = 631065600ULL * 1000000000ULL;
        nanoseconds = ivalue;

        /*
         * IETF leap seconds list as of 2023-06-15 plus my notes:
         *
         *                  #  1 Jan 1970 POSIX Epoch
         * 2272060800	10	#  1 Jan 1972
         * 2287785600	11	#  1 Jul 1972
         * 2303683200	12	#  1 Jan 1973
         * 2335219200	13	#  1 Jan 1974
         * 2366755200	14	#  1 Jan 1975
         * 2398291200	15	#  1 Jan 1976
         * 2429913600	16	#  1 Jan 1977
         * 2461449600	17	#  1 Jan 1978
         * 2492985600	18	#  1 Jan 1979
         * 2524521600	19	#  1 Jan 1980
         *                  #  6 Jan 1980 GPS Epoch
         * 2571782400	20	#  1 Jul 1981
         * 2603318400	21	#  1 Jul 1982
         * 2634854400	22	#  1 Jul 1983
         * 2698012800	23	#  1 Jul 1985
         * 2776982400	24	#  1 Jan 1988
         *                  # 31 Dec 1989 Garmin Epoch
         * 2840140800	25	#  1 Jan 1990
         * 2871676800	26	#  1 Jan 1991
         * 2918937600	27	#  1 Jul 1992
         * 2950473600	28	#  1 Jul 1993
         * 2982009600	29	#  1 Jul 1994
         * 3029443200	30	#  1 Jan 1996
         * 3076704000	31	#  1 Jul 1997
         * 3124137600	32	#  1 Jan 1999
         * 3345062400	33	#  1 Jan 2006
         * 3439756800	34	#  1 Jan 2009
         * 3550089600	35	#  1 Jul 2012
         * 3644697600	36	#  1 Jul 2015
         * 3692217600	37	#  1 Jan 2017
         */

        /*
         * Subtract the leap seconds between the GPS epoch and the Garmin
         * epoch, otherwise we will count those twice below.
         */

        ivalue = 5ULL * 1000000000ULL;
        nanoseconds -= ivalue;

        /*
         * Add the days since most recent week start of the Garmin epoch.
         * We have to back up to the most recent Sunday in the Garmin
         * calendar because the GPS TOW is relative to the most recent
         * week start in the GPS calendar which was also a Sunday. Any
         * leap seconds that may have occurred between the GPS epoch and
         * now will be accounted for below.
         */

        ivalue = pvt.grmn_days;
        ivalue -= ivalue % 7ULL;
        ivalue *= DAY;
        nanoseconds += ivalue;

        /*
         * Convert the GPS Time Of Week to seconds. Apparently, empirically,
         * Garmin has already converted the 1.5s GPS TOW ticks to 1s ticks.
         */

        fvalue = pvt.gps_tow;
        fvalue *= 1000000000.0;

        /*
         * Get the D-M-Y part of the GPS TOW.
         */

        ivalue = fvalue;
        tvalue = ivalue % DAY;
        ivalue -= tvalue;
        nanoseconds += ivalue;

        /*
         * Get the leap seconds to convert GPS Time to UTC. Presumably this
         * will get incremented automatically as leap seconds are added and
         * GPS incorporates this into its own messaging.
         */

        ivalue = pvt.leap_sec;
        ivalue *= 1000000000ULL;
        nanoseconds += ivalue;

#if !0
        /*
         * Yeah, I got nothin'. This 31s correction is based on comparisons
         * with an NTP server and with a second GPS (U-blox) device.
         * It's interesting that if we ignore the 5s leap correction above,
         * our clock would be off by exactly twice the number of UTC leap
         * seconds - as if we should have subtracted them instead of adding
         * them. Can't see however how this can be the case. But it seems
         * likely to be something to do with leap seconds somehow.
         */

        nanoseconds -= 31ULL * 1000000000ULL;
#endif

        gpp->dmy_nanoseconds = nanoseconds;

        /*
         * Get the H:M:S part of the GPS TOW.
         */

        nanoseconds = tvalue;

        gpp->utc_nanoseconds = nanoseconds;

        gpp->old_nanoseconds = gpp->tot_nanoseconds;
        gpp->tot_nanoseconds = gpp->dmy_nanoseconds + gpp->utc_nanoseconds;

        gpp->label = LABEL;

#if 0
        fprintf(stderr, "CPO PVT: qual=%u lat=%lld lon=%lld alt=%lld sep=%lld dmy=%llu utc=%llu old=%llu tot=%llu label=\"%s\"\n", gpp->quality, (signed long long)(gpp->lat_nanominutes), (signed long long)(gpp->lon_nanominutes), (signed long long)(gpp->alt_millimeters), (signed long long)(gpp->sep_millimeters), (unsigned long long)(gpp->dmy_nanoseconds), (unsigned long long)(gpp->utc_nanoseconds), (unsigned long long)(gpp->old_nanoseconds), (unsigned long long)gpp->tot_nanoseconds, gpp->label);
#endif

        rc = 0;

    } while (0);

    return rc;
}
