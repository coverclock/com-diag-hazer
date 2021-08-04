/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implemention of the gpstool Emit API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include <errno.h>
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_absolute.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "com/diag/diminuto/diminuto_observation.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "types.h"
#include "globals.h"
#include "constants.h"
#include "buffer.h"
#include "emit.h"

ssize_t emit_sentence(FILE * fp, char * sentence, size_t size)
{
    ssize_t rc = -1;
    uint8_t * bp = (uint8_t *)0;
    uint8_t msn = '\0';
    uint8_t lsn = '\0';

    if ((bp = (uint8_t *)hazer_checksum_buffer(sentence, size, &msn, &lsn)) == (uint8_t *)0) {
        errno = EIO;
        diminuto_perror("emit_sentence: checksum");
    } else if (fprintf(fp, "%s%c%c%c\r\n", sentence, HAZER_STIMULUS_CHECKSUM, msn, lsn) < 0) {
        errno = EIO;
        diminuto_perror("emit_sentence: fprintf");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("emit_sentence: fflush");
   } else {
        *(bp++) = HAZER_STIMULUS_CHECKSUM;
        *(bp++) = msn;
        *(bp++) = lsn;
        *(bp++) = '\r';
        *(bp++) = '\n';
        *(bp++) = '\0';
        rc = bp - (uint8_t *)sentence;
   }

    return rc;
}

ssize_t emit_packet(FILE * fp, void * packet, size_t size)
{
    ssize_t rc = -1;
    uint8_t * bp = (uint8_t *)0;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;

    if ((bp = (uint8_t *)yodel_checksum_buffer(packet, size, &ck_a, &ck_b)) == (uint8_t *)0) {
        errno = EIO;
        diminuto_perror("emit_packet: checksum");
    } else if (fwrite(packet, bp - (uint8_t *)packet, 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 1");
    } else if (fwrite(&ck_a, sizeof(ck_a), 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 2");
    } else if (fwrite(&ck_b, sizeof(ck_b), 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_packet: fwrite 3");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("emit_packet: fflush");
    } else {
        *(bp++) = ck_a;
        *(bp++) = ck_b;
        *(bp++) = '\0';
        rc = bp - (uint8_t *)packet;
    }

    return rc;
}

ssize_t emit_data(FILE * fp, void * data, size_t size)
{
    ssize_t rc = -1;

    if (size < 2) {
        rc = 0;
    } else if (fwrite(data, size - 1 /* Ignore terminating nul. */, 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("emit_data: fwrite");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("emit_data: fflush");
    } else {
        rc = size;
    }

    return rc;
}

void emit_trace(FILE * fp, const hazer_position_t pa[], const yodel_solution_t * sp, const yodel_attitude_t * ap, const yodel_posveltim_t * pp, const yodel_base_t * bp)
{
    static uint64_t sn = 0;
    diminuto_ticks_t ticks = 0;
    int system = HAZER_SYSTEM_GNSS;
    uint64_t seconds = 0;
    uint64_t nanoseconds = 0;
    int32_t degrees = 0;
    uint64_t nanodegrees = 0; /* UBX */
    uint64_t decimicrodegrees = 0; /* NMEA */
    uint32_t centimillidegrees = 0; /* IMU */
    int32_t meters = 0;
    uint32_t decimillimeters = 0; /* UBX */
    uint32_t millimeters = 0; /* NMEA */
    int32_t totalmillimeters = 0;
    int32_t knots = 0;
    uint32_t microknots = 0;
    int fix = YODEL_UBX_NAV_PVT_fixType_noFix;
    int ii = 0;
    static const int64_t NANO = 1000000000;
    static const int64_t MICRO = 1000000;
    static const int64_t MILLI = 1000;
    static const int32_t CENTIMILLI = 100000;

    /*
     * HEADINGS
     */

    if (sn == 0) {
        for (ii = 0; ii < countof(HEADINGS); ++ii) {
            if (ii > 0) { fputc(' ', fp); }
            fputs(HEADINGS[ii], fp);
            if (ii < (countof(HEADINGS) - 1)) { fputc(',', fp); } else { fputc('\n', fp); }
        }
        sn++;
    }

    /*
     * Find a GNSS solution.
     */

    for (ii = HAZER_SYSTEM_GNSS; ii <= HAZER_SYSTEM_BEIDOU; ++ii) {
        if (pa[ii].ticks == 0) {
            /* Do nothing. */
        } else if (pa[ii].utc_nanoseconds == 0) {
            /* Do nothing. */
        } else if (pa[ii].dmy_nanoseconds == 0) {
            /* Do nothing. */
        } else {
            system = ii;
            break;
        }
    }

    /* NAM */

    fprintf(fp, "\"%s\"", Hostname);

    /* NUM */

    fprintf(fp, ", %llu", (long long unsigned int)(sn++));

    /* FIX */

    if (pp->ticks > 0) {
        fix = pp->payload.fixType;
    } else if ((pa[system].lat_digits == 0) || (pa[system].lon_digits == 0)) {
        fix = YODEL_UBX_NAV_PVT_fixType_noFix;
    } else if ((pa[system].alt_digits == 0) || (pa[system].sep_digits == 0)) {
        fix = YODEL_UBX_NAV_PVT_fixType_2D;
    } else {
        fix = YODEL_UBX_NAV_PVT_fixType_3D;
    }

    fprintf(fp, ", %d", fix);

    /* SYS */

    fprintf(fp, ", %d", system);

    /* SAT */

    if (pa[system].ticks > 0) {

        fprintf(fp, ", %d", pa[system].sat_used);

    } else {

        fputs(", 0", fp); /* missing sat used */

    }

    /* CLK */

    ticks = diminuto_frequency_ticks2units(Now, NANO);

    seconds = ticks / NANO;
    nanoseconds = ticks % NANO;
    fprintf(fp, ", %llu.%09llu", (long long unsigned int)seconds, (long long unsigned int)nanoseconds);

    /* TIM */

    if ((pa[system].ticks > 0) && (pa[system].utc_nanoseconds > 0) && (pa[system].dmy_nanoseconds > 0)) {

        seconds = pa[system].tot_nanoseconds / NANO;
        nanoseconds = pa[system].tot_nanoseconds % NANO;
        fprintf(fp, ", %llu.%09llu", (long long unsigned int)seconds, (long long unsigned int)nanoseconds);

    } else {

        fputs(EMPTY, fp); /* missing GNSS time */

    }

    /* LAT, LON, HAC, MSL, GEO, VAC */

    /*
     * We use the high precision fix if it is available.
     */

    if (sp->ticks > 0) {

        yodel_format_hppos2degrees(sp->payload.lat, sp->payload.latHp, &degrees, &nanodegrees);
        fprintf(fp, ", %d.%09llu", degrees, (long long unsigned int)nanodegrees);

        yodel_format_hppos2degrees(sp->payload.lon, sp->payload.lonHp, &degrees, &nanodegrees);
        fprintf(fp, ", %d.%09llu", degrees, (long long unsigned int)nanodegrees);

        yodel_format_hpacc2accuracy(sp->payload.hAcc, &meters, &decimillimeters);
        fprintf(fp, ", %lld.%04llu", (long long signed int)meters, (long long unsigned int)decimillimeters);

        yodel_format_hpalt2aaltitude(sp->payload.hMSL, sp->payload.hMSLHp, &meters, &decimillimeters);
        fprintf(fp, ", %lld.%04llu", (long long signed int)meters, (long long unsigned int)decimillimeters);

        yodel_format_hpalt2aaltitude(sp->payload.height, sp->payload.heightHp, &meters, &decimillimeters);
        fprintf(fp, ", %lld.%04llu", (long long signed int)meters, (long long unsigned int)decimillimeters);

        yodel_format_hpacc2accuracy(sp->payload.vAcc, &meters, &decimillimeters);
        fprintf(fp, ", %lld.%04llu", (long long signed int)meters, (long long unsigned int)decimillimeters);

    } else if (pa[system].ticks > 0) {

        if (pa[system].lat_digits > 0) {

            hazer_format_nanominutes2degrees(pa[system].lat_nanominutes, &degrees, &decimicrodegrees);
            fprintf(fp, ", %d.%07llu", degrees, (long long unsigned int)decimicrodegrees);

        } else {

            fputs(EMPTY, fp); /* missing latitude */

        }

        if (pa[system].lon_digits > 0) {

            hazer_format_nanominutes2degrees(pa[system].lon_nanominutes, &degrees, &decimicrodegrees);
            fprintf(fp, ", %d.%07llu", degrees, (long long unsigned int)decimicrodegrees);

        } else {

            fputs(EMPTY, fp); /* missing longitude */
        }

        fputs(EMPTY, fp); /* missing horizontal accuracy */

        if (pa[system].alt_digits > 0) {

            totalmillimeters = pa[system].alt_millimeters; /* MSL */

            meters = totalmillimeters / MILLI;
            millimeters = abs64(totalmillimeters) % MILLI;
            fprintf(fp, ", %lld.%03llu", (long long signed int)meters, (long long unsigned int)millimeters);

        } else {

            fputs(EMPTY, fp); /* missing MSL */

        }

        if (pa[system].sep_digits > 0) {

            totalmillimeters += pa[system].sep_millimeters; /* SEP */

            meters = totalmillimeters / MILLI;
            millimeters = abs64(totalmillimeters) % MILLI;
            fprintf(fp, ", %lld.%03llu", (long long signed int)meters, (long long unsigned int)millimeters);

        } else {

            fputs(EMPTY, fp); /* missing SEP */

        }

        fputs(EMPTY, fp); /* missing vertical accuracy */

    } else {

        fputs(EMPTY, fp); /* missing latitude */
        fputs(EMPTY, fp); /* missing longitude */
        fputs(EMPTY, fp); /* missing horizontal accuracy */
        fputs(EMPTY, fp); /* missing MSL */
        fputs(EMPTY, fp); /* missing SEP */
        fputs(EMPTY, fp); /* missing vertical accuracy */

    }

    /* SOG, COG */

    /*
     * I was tempted to convert knots into meters/second, but knots or
     * nautical miles per hour are actually the typical units used in
     * meterology, aviation, and maritime; that's probably why they are
     * the standard units of speed for NMEA, even though the knot is not
     * an SI unit.
     */

    if (pa[system].ticks > 0) {

        if (pa[system].sog_digits > 0) {

            knots = pa[system].sog_microknots / MICRO;
            microknots = abs64(pa[system].sog_microknots) % MICRO;
            fprintf(fp, ", %ld.%06lu", (long signed int)knots, (long unsigned int)microknots);

        } else {

            fputs(EMPTY, fp); /* missing SOG */

        }

        if (pa[system].cog_digits > 0) {

            degrees = pa[system].cog_nanodegrees / NANO;
            nanodegrees = abs64(pa[system].cog_nanodegrees) % NANO;
            fprintf(fp, ", %ld.%09llu", (long signed int)degrees, (long long unsigned int)nanodegrees);

        } else {

            fputs(EMPTY, fp); /* missing COG */

        }

    } else {

        fputs(EMPTY, fp); /* missing SOG */
        fputs(EMPTY, fp); /* missing COG */

    }

    /* ROL, PIT, YAW, RAC, PAC, YAC */

    if (ap->ticks > 0) {

        degrees = ap->payload.roll / CENTIMILLI;
        centimillidegrees = abs32(ap->payload.roll) % CENTIMILLI;
        fprintf(fp, ", %ld.%05lu", (long signed int)degrees, (long unsigned int)centimillidegrees);

        degrees = ap->payload.pitch / CENTIMILLI;
        centimillidegrees = abs32(ap->payload.pitch) % CENTIMILLI;
        fprintf(fp, ", %ld.%05lu", (long signed int)degrees, (long unsigned int)centimillidegrees);

        degrees = ap->payload.heading / CENTIMILLI;
        centimillidegrees = abs32(ap->payload.heading) % CENTIMILLI;
        fprintf(fp, ", %ld.%05lu", (long signed int)degrees, (long unsigned int)centimillidegrees);

        degrees = ap->payload.accRoll / CENTIMILLI;
        centimillidegrees = ap->payload.accRoll % CENTIMILLI;
        fprintf(fp, ", %lu.%05lu", (long unsigned int)degrees, (long unsigned int)centimillidegrees);

        degrees = ap->payload.accPitch / CENTIMILLI;
        centimillidegrees = ap->payload.accPitch % CENTIMILLI;
        fprintf(fp, ", %lu.%05lu", (long unsigned int)degrees, (long unsigned int)centimillidegrees);

        degrees = ap->payload.accHeading / CENTIMILLI;
        centimillidegrees = ap->payload.accHeading % CENTIMILLI;
        fprintf(fp, ", %lu.%05lu", (long unsigned int)degrees, (long unsigned int)centimillidegrees);

    } else {

        fputs(EMPTY, fp); /* missing roll */
        fputs(EMPTY, fp); /* missing pitch */
        fputs(EMPTY, fp); /* missing heading */
        fputs(EMPTY, fp); /* missing roll accuracy */
        fputs(EMPTY, fp); /* missing pitch accurady */
        fputs(EMPTY, fp); /* missing heading accuracy */

    }

    /* OBS, MAC */

    if (bp->ticks > 0) {

        fprintf(fp, ", %d", bp->payload.obs);

        yodel_format_hpacc2accuracy(bp->payload.meanAcc, &meters, &decimillimeters);
        fprintf(fp, ", %lld.%04llu", (long long signed int)meters, (long long unsigned int)decimillimeters);

    } else {

        fputs(", 0", fp); /* missing survey observations */
        fputs(EMPTY, fp); /* missing survey mean accuracy */

    }

    /* END */

    fputs("\n", fp);

    fflush(fp);
}

int emit_solution(const char * arp, const yodel_base_t * bp, const yodel_solution_t * sp)
{
    int rc = 0;
    FILE * fp = (FILE *)0;
    char * temporary = (char *)0;
    int64_t value = 0;
    uint32_t acc = 0;
    int32_t lat = 0;
    int8_t latHp = 0;
    int32_t lon = 0;
    int8_t lonHp = 0;
    int32_t height = 0;
    int8_t heightHp = 0;

    if (bp->ticks == 0) {
        /* Do nothing. */
    } else if (sp->ticks == 0) {
        /* Do nothing. */
    } else if (bp->payload.active) {
        /* Do nothing. */
    } else if (!bp->payload.valid) {
        /* Do nothing. */
    } else if ((fp = diminuto_observation_create(arp, &temporary)) == (FILE *)0) {
        /* Do nothing. */
    } else {

        acc = bp->payload.meanAcc;
        lat = sp->payload.lat;
        latHp = sp->payload.latHp;
        lon = sp->payload.lon;
        lonHp = sp->payload.lonHp;

        /*
         * Remarkably, the documented output format for the high precision
         * height in SURVEY-IN mode [UBX ZED-F9P Interface, p. 145] is in
         * different units (mm and 0.1mm, which yields plausible results)
         * than the documented input format in FIXED mode [UBX ZED-F9P
         * Interface, pp. 226..227] (cm and 0.1mm).
         */

        height  = sp->payload.height; /* mm == 10^-3m */
        heightHp = sp->payload.heightHp; /* 0.1mm == 10^-4m (-9..+9) */
        value = (height * 10) + heightHp; /* 0.1mm == 10^-4m */
        height = value / 100; /* 10^-4m / 10^2 == 10^-2m == cm */
        heightHp = value % 100; /* 0.1mm == 10^-4m (-99..+99) */

        DIMINUTO_LOG_INFORMATION("Fix Emit acc 0x%8.8x lat 0x%8.8x 0x%2.2x lon 0x%8.8x 0x%2.2x alt 0x%8.8x 0x%2.2x\n", acc, (uint32_t)lat, (uint8_t)latHp, (uint32_t)lon, (uint8_t)lonHp, (uint32_t)height, (uint8_t)heightHp);

        COM_DIAG_YODEL_HTOLE(acc);
        COM_DIAG_YODEL_HTOLE(lat);
        COM_DIAG_YODEL_HTOLE(latHp);
        COM_DIAG_YODEL_HTOLE(lon);
        COM_DIAG_YODEL_HTOLE(lonHp);
        COM_DIAG_YODEL_HTOLE(height);
        COM_DIAG_YODEL_HTOLE(heightHp);

        dump_buffer(fp, &acc, sizeof(acc));
        dump_buffer(fp, &lat, sizeof(lat));
        dump_buffer(fp, &latHp, sizeof(latHp));
        dump_buffer(fp, &lon, sizeof(lon));
        dump_buffer(fp, &lonHp, sizeof(lonHp));
        dump_buffer(fp, &height, sizeof(height));
        dump_buffer(fp, &heightHp, sizeof(heightHp));

        rc = (diminuto_observation_commit(fp, &temporary) == (FILE *)0);

    }

    return rc;
}
