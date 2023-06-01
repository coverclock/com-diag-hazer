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
#include "print.h"
#include "test.h"
#include "types.h"

void print_actives(FILE * fp, const hazer_actives_t aa, hazer_system_t ss)
{
    static const unsigned int IDENTIFIERS = diminuto_countof(aa[0].id);
    unsigned int system = 0;
    unsigned int satellite = 0;
    unsigned int count = 0;
    unsigned int total = 0;
    static unsigned int maximum = 0;

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (system > ss) { break; }
        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }

        total += aa[system].active;

    }

    if (total > maximum) { maximum = total; }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (system > ss) { break; }
        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }

        fprintf(fp, "%s {", "ACT [1]");

        count = 0;
        for (satellite = 0; satellite < (IDENTIFIERS / 2); ++satellite) {
            if ((satellite < aa[system].active) && (aa[system].id[satellite] != 0)) {
                fprintf(fp, " %5u", aa[system].id[satellite]);
                count += 1;
            } else {
                fputs("      ", fp);
            }
        }

        fprintf(fp, " } [%2u] [%2u] [%2u] [%2u]", count, aa[system].active, total, maximum);

        fprintf(fp, " %2.2s", HAZER_MODE_NAME[aa[system].mode]);

        fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

        if (aa[system].active <= (IDENTIFIERS / 2)) { continue; }

        fprintf(fp, "%s {", "ACT [2]");

        count = 0;
        for (satellite = (IDENTIFIERS / 2); satellite < IDENTIFIERS; ++satellite) {
            if ((satellite < aa[system].active) && (aa[system].id[satellite] != 0)) {
                fprintf(fp, " %5u", aa[system].id[satellite]);
                count += 1;
            } else {
                fputs("      ", fp);
            }
        }

        fprintf(fp, " } [%2u] [%2u] [%2u] [%2u]", count, aa[system].active, total, maximum);

        fprintf(fp, " %2.2s", HAZER_MODE_NAME[aa[system].mode]);

        fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (system > ss) { break; }
        if (aa[system].ticks == 0) { continue; }
        if (aa[system].active == 0) { continue; }

        fprintf(fp, "%s", "DOP");

        fprintf(fp, " %6.2lfpdop %6.2lfhdop %6.2lfvdop %6.2lftdop", (double)aa[system].pdop / 100.0, (double)aa[system].hdop / 100.0, (double)aa[system].vdop / 100.0, (double)aa[system].tdop / 100.0);

        fprintf(fp, "%23s", "");

        fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

        fputc('\n', fp);

    }

}

void print_views(FILE *fp, const hazer_views_t va, const hazer_actives_t aa, hazer_system_t ss)
{
    static const unsigned int SATELLITES = diminuto_countof(va[0].sig[0].sat);
    static const unsigned int SIGNALS = diminuto_countof(va[0].sig);
    static const unsigned int IDENTIFIERS = diminuto_countof(aa[0].id);
    unsigned int system = 0;
    unsigned int signal = 0;
    unsigned int channel = 0;
    unsigned int satellite = 0;
    unsigned int active = 0;
    unsigned int limit = 0;
    marker_t ranged = MARKER;
    marker_t phantom = MARKER;
    marker_t untracked = MARKER;
    marker_t unused = MARKER;
    char source = '\0';

    for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

        if (system > ss) { break; }

        for (signal = 0; signal < SIGNALS; ++signal) {

            if (signal >= va[system].signals) { break; }

            if (va[system].sig[signal].ticks == 0) { continue; }

            limit = va[system].sig[signal].channels;
            if (limit > SATELLITES) { limit = SATELLITES; }

            for (satellite = 0; satellite < limit; ++satellite) {

                if (va[system].sig[signal].sat[satellite].id == 0) { continue; }

                ranged = INACTIVE;
                if (aa[system].ticks == 0) {
                    /* Do nothing. */
                } else if (aa[system].active == 0) {
                    /* Do nothing. */
                } else {
                    for (active = 0; active < IDENTIFIERS; ++active) {

                        if (active >= aa[system].active) { break; }
                        if (aa[system].id[active] == 0) { break; }
                        if (aa[system].id[active] == va[system].sig[signal].sat[satellite].id) { ranged = ACTIVE; }

                    }
                }

                phantom = va[system].sig[signal].sat[satellite].phantom ? PHANTOM : INACTIVE;
                untracked = va[system].sig[signal].sat[satellite].untracked ? UNTRACKED : INACTIVE;
                unused = va[system].sig[signal].sat[satellite].unused ? UNUSED : INACTIVE;
                source = (va[system].label == (const char *)0) ? '?' : (va[system].label[0] == 'P') ? va[system].label[1] : va[system].label[0];

                fputs("SAT", fp);

                fprintf(fp, " [%3u] %5uid %3d%lcelv %4d%lcazm %4ddBHz  %-8.8s %c %c %c %c %c", ++channel, va[system].sig[signal].sat[satellite].id, va[system].sig[signal].sat[satellite].elv_degrees, (wint_t)COMMON_DEGREE, va[system].sig[signal].sat[satellite].azm_degrees, (wint_t)COMMON_DEGREE, va[system].sig[signal].sat[satellite].snr_dbhz, HAZER_SIGNAL_NAME[system][signal], source, ranged, phantom, untracked, unused);

                fprintf(fp, "%7s", "");

                fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

                fputc('\n', fp);

            }

        }

    }

}

void print_local(FILE * fp)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    diminuto_sticks_t offset = 0;
    diminuto_ticks_t fraction = 0;
    diminuto_sticks_t milliseconds = 0;
    int rc = 0;

    fputs("LOC", fp);

    /*
     * LOCAL CLOCK TIME
     */

    rc = diminuto_time_juliet(Clock, &year, &month, &day, &hour, &minute, &second, &fraction);
    diminuto_contract(rc == 0);
    diminuto_contract((1 <= month) && (month <= 12));
    diminuto_contract((1 <= day) && (day <= 31));
    diminuto_contract((0 <= hour) && (hour <= 23));
    diminuto_contract((0 <= minute) && (minute <= 59));
    diminuto_contract((0 <= second) && (second <= 59));

    /*
     * I arbitrarily decided to render the fractional part in milliseconds.
     */

    milliseconds = diminuto_frequency_ticks2units(fraction, 1000LL);
    diminuto_contract((0 <= milliseconds) && (milliseconds < 1000LL));

    fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02d.%03lu", year, month, day, hour, minute, second, (long unsigned int)milliseconds);

    /*
     * There are time zones whose offset are not in whole hours. That's why
     * ISO 8601 permits HH:MM as a format. Go figure. Why do we call this
     * every time rather than cache the offset ourselves? Because it is
     * conceivable that it might be manually changed by a misguided systems
     * administrator while the application is running, and the underlying
     * tzset(3) might actually notice that the zone information in the file
     * system has been altered. Not only will this change the local time,
     * but it will add some latency and jitter to the GPS display. (Yet
     * another reason to admin your embedded system to UTC.)
     */

    offset = diminuto_time_timezone();

    offset = diminuto_frequency_ticks2wholeseconds(offset);
    hour = offset / 3600;
    minute = (offset % 3600) / 60;
    if (minute < 0) { minute = -minute; }
    fprintf(fp, "%+2.2d:%02d", hour, minute);

    /*
     * The abomination that is Daylight Saving Time has an offset that
     * depends upon the current date and time. We express this separately,
     * in a mild extension of ISO 8601, so that we don't confuse the DST
     * offset (which changes seasonally) with the time zone offset (which is,
     * typically, fixed).
     */

    offset = diminuto_time_daylightsaving(Clock);
    offset = diminuto_frequency_ticks2wholeseconds(offset);
    hour = offset / 3600;
    fprintf(fp, "%+2.2d", hour);

    /*
     * ELAPSED MONOTONIC TIME
     */

    rc = diminuto_time_duration(Now - Epoch, &day, &hour, &minute, &second, &fraction);
    diminuto_contract(rc >= 0);
    diminuto_contract(day >= 0);
    diminuto_contract((0 <= hour) && (hour <= 23));
    diminuto_contract((0 <= minute) && (minute <= 59));
    diminuto_contract((0 <= second) && (second <= 59));

    /*
     * I limited the resolution to seconds just to save space on a
     * crowded output line, especially when (for example) a DGNSS base
     * station runs for a long time. The monotonic time has nothing to
     * do with the higher precision time displayed above that is
     * probably adjusted via NTP. It's more like uptime(1).
     */

    fprintf(fp, " %03d/%02d:%02d:%02d", day, hour, minute, second);

    fprintf(fp, " %-8.8s", COM_DIAG_HAZER_RELEASE_VALUE);

    fprintf(fp, " %10d", Process);

    fputs(" ", fp);

    fprintf(fp, " %-8.8s", Hostname);

    fputc('\n', fp);
}

void print_hardware(FILE * fp, const yodel_hardware_t * hp)
{
    /*
     * Indicate detection of broadband or continuous wave (cw) jamming.
     * Relies on support from later versions of Ublox 8 firmware, and must be
     * explicitly enabled by sending appropriate messages to the Ublox device.
     */

    if (hp->ticks > 0) {
        uint8_t value;
        char jamming;
        static char jamming_prior = STATUS;
        static char jamming_history = STATUS;
        static uint8_t jamInd_maximum = 0;

        value = (hp->payload.flags >> YODEL_UBX_MON_HW_flags_jammingState_SHIFT) & YODEL_UBX_MON_HW_flags_jammingState_MASK;
        switch (value) {
        case YODEL_UBX_MON_HW_flags_jammingState_unknown:
            jamming = UNKNOWN;
            if (jamming_history == STATUS) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_none:
            jamming = NONE;
            if ((jamming_history == STATUS) || (jamming_history == UNKNOWN)) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_warning:
            jamming = WARNING;
            if (jamming_history != CRITICAL) { jamming_history = jamming; }
            break;
        case YODEL_UBX_MON_HW_flags_jammingState_critical:
            jamming = CRITICAL;
            jamming_history = jamming;
            break;
        default:
            jamming = INVALID;
            if ((jamming_history == STATUS) || (jamming_history == UNKNOWN)) { jamming_history = jamming; }
            break;
        }

        if (jamming != jamming_prior) {
            DIMINUTO_LOG_NOTICE("Indicate UBX-MON-HW jamming %u indicator %u\n", value, hp->payload.jamInd);
            jamming_prior = jamming;
        }

        if (hp->payload.jamInd > jamInd_maximum) { jamInd_maximum = hp->payload.jamInd; }

        fputs("MON", fp);

        fprintf(fp, " %cjamming  %chistory %3uindicator %3umaximum", jamming, jamming_history, hp->payload.jamInd, jamInd_maximum);

        fprintf(fp, "%24s", ""); /* This is actually important. */

        fprintf(fp, " %-8.8s", Source);

        fputc('\n', fp);
    }
}

void print_status(FILE * fp, const yodel_status_t * sp)
{
    static uint32_t msss_prior = 0;
    static uint16_t msss_epoch = 0;

    /*
     * Indicate detection of spoofing by comparing solutions from multiple
     * GNSSes if (and only if) available. Relies on support from later versions
     * of Ublox 8 firmware, and must be explicitly enabled by sending
     * appropriate messages to the UBlox device.
     */

    if (sp->ticks > 0) {
        static char spoofing_prior = STATUS;
        static char spoofing_history = STATUS;
        uint8_t value;
        char spoofing;

        value = (sp->payload.flags2 >> YODEL_UBX_NAV_STATUS_flags2_spoofDetState_SHIFT) & YODEL_UBX_NAV_STATUS_flags2_spoofDetState_MASK;

        switch (value) {
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_unknown:
            spoofing = UNKNOWN;
            if (spoofing_history == STATUS) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_none:
            spoofing = NONE;
            if ((spoofing_history == STATUS) || (spoofing_history == UNKNOWN)) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_one:
            spoofing = WARNING;
            if (spoofing_history != CRITICAL) { spoofing_history = spoofing; }
            break;
        case YODEL_UBX_NAV_STATUS_flags2_spoofDetState_many:
            spoofing = CRITICAL;
            spoofing_history = spoofing;
            break;
        default:
            spoofing = INVALID;
            if ((spoofing_history == STATUS) || (spoofing_history == UNKNOWN)) { spoofing_history = spoofing; }
            break;
        }

        if (spoofing != spoofing_prior) {
            DIMINUTO_LOG_NOTICE("Indicate UBX-NAV-STATUS spoofing %u\n", value);
            spoofing_prior = spoofing;
        }

        if (sp->payload.msss < msss_prior) {
            msss_epoch += 1;
        }

        fputs("STA", fp);

        fprintf(fp, " %cspoofing %chistory %10ums %10ums %5uepoch", spoofing, spoofing_history, sp->payload.ttff, sp->payload.msss, msss_epoch);

        fprintf(fp, "%11s", ""); /* This is actually important. */

        fprintf(fp, " %-8.8s", Source);

        fputc('\n', fp);
    }

    msss_prior = sp->payload.msss;
}

void print_positions(FILE * fp, const hazer_positions_t pa, hazer_system_t ss, int pps, uint64_t bytes)
{
    unsigned int system = 0;

    {
        int year = 0;
        int month = 0;
        int day = 0;
        int hour = 0;
        int minute = 0;
        int second = 0;
        uint64_t nanoseconds = 0;
        int rc = 0;
        diminuto_ticks_t fraction = 0;
        diminuto_sticks_t milliseconds = 0;
        static int timeonce = 0;
        static int fixonce = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }
            if (pa[system].dmy_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }

            fputs("TIM", fp);

            /*
             * GPS TIME
             */

            hazer_format_nanoseconds2timestamp(pa[system].tot_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);
            diminuto_contract((1 <= month) && (month <= 12));
            diminuto_contract((1 <= day) && (day <= 31));
            diminuto_contract((0 <= hour) && (hour <= 23));
            diminuto_contract((0 <= minute) && (minute <= 59));
            diminuto_contract((0 <= second) && (second <= 59));
            diminuto_contract((0 <= nanoseconds) && (nanoseconds < 1000000000LLU));
            fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02d.000-00:00+00", year, month, day, hour, minute, second);

            if (!timeonce) {
                DIMINUTO_LOG_NOTICE("Time Stamp %04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);
                timeonce = !0;
            }

            /*
             * TIME TO FIRST FIX (TTFF)
             */

            /*
             * This is where we calculate time to first fix. We display dashes
             * if it is negative, asterisks if it is a day or more, the actual
             * values otherwise. Note that although the TTFF is displayed for
             * each GNSS there is only one global TTFF.
             */

            if (Fix < 0) {
                /* Do nothing. */
            } else if (First >= 0) {
                /* Do nothing. */
            } else {
                First = Fix;
            }

            if (First < 0) {

                fputs(" --:--:--.---", fp);

            } else {

                rc = diminuto_time_duration(First - Epoch, &day, &hour, &minute, &second, &fraction);
                diminuto_contract(rc >= 0);
                diminuto_contract(day >= 0);
                diminuto_contract((0 <= hour) && (hour <= 23));
                diminuto_contract((0 <= minute) && (minute <= 59));
                diminuto_contract((0 <= second) && (second <= 59));
                milliseconds = diminuto_frequency_ticks2units(fraction, 1000LL);
                diminuto_contract((0 <= milliseconds) && (milliseconds < 1000LL));

                if (day > 0) {
                    fputs(" **:**:**.***", fp);
                } else {
                    fprintf(fp, " %02d:%02d:%02d.%03lu", hour, minute, second, (long unsigned int)milliseconds);
                }

                if (!fixonce) {
                    DIMINUTO_LOG_NOTICE("Fix First %d/%02d:%02d:%02d.%03lu", day, hour, minute, second, (long unsigned int)milliseconds);
                    fixonce = !0;
                }

            }

            fprintf(fp, " %s", pps ? "PPS" : "pps");

            fprintf(fp, "%17s", "");

            fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int degrees = 0;
        int minutes = 0;
        int seconds = 0;
        int thousandths = 0;
        int direction = 0;
        uint64_t tenmillionths = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }

            fputs("POS", fp);

            hazer_format_nanominutes2position(pa[system].lat_nanominutes, &degrees, &minutes, &seconds, &thousandths, &direction);
            diminuto_contract((0 <= degrees) && (degrees <= 90));
            diminuto_contract((0 <= minutes) && (minutes <= 59));
            diminuto_contract((0 <= seconds) && (seconds <= 59));
            diminuto_contract((0 <= thousandths) && (thousandths <= 999));
            fprintf(fp, " %2d%lc%02d'%02d.%03d\"%c,", degrees, (wint_t)COMMON_DEGREE, minutes, seconds, thousandths, (direction < 0) ? 'S' : 'N');

            hazer_format_nanominutes2position(pa[system].lon_nanominutes, &degrees, &minutes, &seconds, &thousandths, &direction);
            diminuto_contract((0 <= degrees) && (degrees <= 180));
            diminuto_contract((0 <= minutes) && (minutes <= 59));
            diminuto_contract((0 <= seconds) && (seconds <= 59));
            diminuto_contract((0 <= thousandths) && (thousandths <= 999));
            fprintf(fp, " %3d%lc%02d'%02d.%03d\"%c", degrees, (wint_t)COMMON_DEGREE, minutes, seconds, thousandths, (direction < 0) ? 'W' : 'E');

            fputc(' ', fp);

            hazer_format_nanominutes2degrees(pa[system].lat_nanominutes, &degrees, &tenmillionths);
            diminuto_contract((-90 <= degrees) && (degrees <= 90));
            diminuto_contract((0 <= tenmillionths) && (tenmillionths <= 9999999));
            fprintf(fp, " %4d.%07llu,", degrees, (diminuto_llu_t)tenmillionths);

            hazer_format_nanominutes2degrees(pa[system].lon_nanominutes, &degrees, &tenmillionths);
            diminuto_contract((-180 <= degrees) && (degrees <= 180));
            fprintf(fp, " %4d.%07llu", degrees, (diminuto_llu_t)tenmillionths);
            diminuto_contract((0 <= tenmillionths) && (tenmillionths <= 9999999));

            fprintf(fp, " %cq", HAZER_QUALITY_NAME[pa[system].quality]);

            fprintf(fp, " %cs", HAZER_SAFETY_NAME[pa[system].safety]);

            fprintf(fp, "%1s", "");

            fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int64_t millimeters = 0;
        int64_t meters = 0;
        uint64_t thousandths = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }

            fputs("ALT", fp);

            millimeters = pa[system].alt_millimeters;

            fprintf(fp, " %10.2lf'", millimeters * 3.2808 / 1000.0);

            meters = millimeters / 1000LL;
            thousandths = abs64(millimeters) % 1000LLU;
            fprintf(fp, " %6lld.%03llum MSL", (diminuto_lld_t)meters, (diminuto_llu_t)thousandths);

            millimeters += pa[system].sep_millimeters;

            fprintf(fp, " %10.2lf'", millimeters * 3.2808 / 1000.0);

            meters = millimeters / 1000LL;
            thousandths = abs64(millimeters) % 1000LLU;
            fprintf(fp, " %6lld.%03llum GEO", (diminuto_lld_t)meters, (diminuto_llu_t)thousandths);

            fprintf(fp, "%11s", "");

            fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int64_t degrees = 0;
        uint64_t billionths = 0;
        const char * compass = (const char *)0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }

            fputs("COG", fp);

            diminuto_contract((0LL <= pa[system].cog_nanodegrees) && (pa[system].cog_nanodegrees <= 360000000000LL));

            compass = hazer_format_nanodegrees2compass16(pa[system].cog_nanodegrees);
            diminuto_contract(compass != (const char *)0);
            diminuto_contract(strlen(compass) <= 4);
            fprintf(fp, " %-3s", compass);

            degrees = pa[system].cog_nanodegrees / 1000000000LL;
            billionths = abs64(pa[system].cog_nanodegrees) % 1000000000LLU;
            fprintf(fp, " %4lld.%09llu%lcT", (diminuto_lld_t)degrees, (diminuto_llu_t)billionths, (wint_t)COMMON_DEGREE);

            degrees = pa[system].mag_nanodegrees / 1000000000LL;
            billionths = abs64(pa[system].mag_nanodegrees) % 1000000000LLU;
            fprintf(fp, " %4lld.%09llu%lcM", (diminuto_lld_t)degrees, (diminuto_llu_t)billionths, (wint_t)COMMON_DEGREE);

            fprintf(fp, "%29s", "");

            fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        double milesperhour = 0.0;
        int64_t knots = 0;
        int64_t kilometersperhour = 0;
        uint64_t thousandths = 0;
        double meterspersecond = 0.0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }
            if (pa[system].utc_nanoseconds == HAZER_NANOSECONDS_UNSET) { continue; }

            fputs("SOG", fp);

            milesperhour = pa[system].sog_microknots;
            milesperhour *= 1.150779;
            milesperhour /= 1000000.0;
            fprintf(fp, " %11.3lfmph", milesperhour);

            knots = pa[system].sog_microknots / 1000000LL;
            thousandths = (abs64(pa[system].sog_microknots) % 1000000LLU) / 1000LLU;
            fprintf(fp, " %7lld.%03lluknots", (diminuto_lld_t)knots, (diminuto_llu_t)thousandths);

            kilometersperhour = pa[system].sog_millimetersperhour / 1000000LL;
            thousandths = (abs64(pa[system].sog_millimetersperhour) % 1000000LLU) / 1000LLU;
            fprintf(fp, " %7lld.%03llukph", (diminuto_lld_t)kilometersperhour, (diminuto_llu_t)thousandths);

            meterspersecond = pa[system].sog_millimetersperhour;
            meterspersecond /= 1000.0;
            meterspersecond /= 3600.0;
            fprintf(fp, " %11.3lfm/s", meterspersecond);

            fprintf(fp, "%5s", "");

            fprintf(fp, " %-8.8s", HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

        }
    }

    {
        int atleastone = 0;
        int dmyokay = 0;
        int totokay = 0;
        static int totokay_prior = 0;

        for (system = 0; system < HAZER_SYSTEM_TOTAL; ++system) {

            if (system > ss) { break; }
            if (pa[system].ticks == 0) { continue; }

            dmyokay = (pa[system].dmy_nanoseconds != HAZER_NANOSECONDS_UNSET);
            totokay = (pa[system].tot_nanoseconds != HAZER_NANOSECONDS_UNSET) &&
                      (pa[system].old_nanoseconds != HAZER_NANOSECONDS_UNSET) &&
                      (pa[system].tot_nanoseconds >= pa[system].old_nanoseconds);
            if (totokay != totokay_prior) {
                DIMINUTO_LOG_NOTICE("Clock %s\n", totokay ? "Monotonic" : "Retrograde");
                totokay_prior = totokay;
            }

            fputs("INT", fp);

            fprintf(fp, " %3.3s", (pa[system].label == (const char *)0) ? "???" : (pa[system].label[0] == 'P') ? &(pa[system].label[1]) : pa[system].label);
            fprintf(fp, " [%2u]", pa[system].sat_used);
            fprintf(fp, " %3s", dmyokay ? "DMY" : "dmy");
            fprintf(fp, " %3s", totokay ? "TOT" : "tot");
            fprintf(fp, " ( %2d %2d %2d %2d %2d %2d %2d %2d )", pa[system].lat_digits, pa[system].lon_digits, pa[system].alt_digits, pa[system].sep_digits, pa[system].cog_digits, pa[system].mag_digits, pa[system].sog_digits, pa[system].smm_digits);
            fprintf(fp, " %20lluB", (unsigned long long)bytes); /* (2^64)-1 == 0xFFFFFFFFFFFFFFFF == 18,446,744,073,709,551,615. */

            fprintf(fp, " %-8.8s", (system == 0) ? Source : HAZER_SYSTEM_NAME[system]);

            fputc('\n', fp);

            atleastone = !0;

        }

        if (!atleastone) {

            fputs("INT", fp);

            fputs(" ---", fp);
            fputs(" [ 0]", fp);
            fputs(" dmy", fp);
            fputs(" tot", fp);
            fputs(" (  0  0  0  0  0  0  0  0 )", fp);
            fputs("                    0B", fp);
            fprintf(fp, " %-8.8s", Source);

            fputc('\n', fp);

        }
    }

}

void print_corrections(FILE * fp, const yodel_base_t * bp, const yodel_rover_t * rp, const tumbleweed_message_t * kp, const tumbleweed_updates_t * up)
{

    if (bp->ticks != 0) {

        fputs("BAS", fp);
        fprintf(fp, " %dactive %dvalid %10usec %10uobs %12.4lfm", !!bp->payload.active, !!bp->payload.valid, bp->payload.dur, bp->payload.obs, (double)bp->payload.meanAcc / 10000.0);
        fprintf(fp, "%10s", "");
        fprintf(fp, " %-8.8s", "DGNSS");
        fputc('\n', fp);

    }

     if (rp->ticks != 0) {

        fputs("ROV", fp);
        fprintf(fp, " %5u: %5u (%5u)", rp->payload.refStation, rp->payload.msgType, rp->payload.subType);
        fprintf(fp, "%46s", "");
        fprintf(fp, " %-8.8s", "DGNSS");
        fputc('\n', fp);

     }

     if (kp->ticks != 0) {

         fputs("RTK", fp);
         fprintf(fp, " %4u [%4zu] %-8.8s <%8.8s>", kp->number, kp->length, (kp->source == DEVICE) ? "base" : (kp->source == NETWORK) ? "rover" : "unknown", up->bytes);
         fprintf(fp, "%36s", "");
         fprintf(fp, "%-8.8s", "DGNSS");
         fputc('\n', fp);

     }

}

void print_solution(FILE * fp, const yodel_solution_t * sp)
{
    int32_t decimaldegrees = 0;
    uint32_t degrees = 0;
    uint32_t minutes = 0;
    uint32_t seconds = 0;
    uint64_t billionths = 0;
    int direction = 0;
    int32_t meters = 0;
    uint32_t tenthousandths = 0;

    if (sp->ticks != 0) {

        fputs("HPP", fp);

        yodel_format_hppos2degrees(sp->payload.lat, sp->payload.latHp, &decimaldegrees, &billionths);
        fprintf(fp, " %4d.%09llu,", decimaldegrees, (diminuto_llu_t)billionths);

        yodel_format_hppos2degrees(sp->payload.lon, sp->payload.lonHp, &decimaldegrees, &billionths);
        fprintf(fp, " %4d.%09llu", decimaldegrees, (diminuto_llu_t)billionths);

        yodel_format_hpacc2accuracy(sp->payload.hAcc, &meters, &tenthousandths);
        fprintf(fp, " %lc%6lld.%04llum", (wint_t)COMMON_PLUSMINUS, (diminuto_lld_t)meters, (diminuto_llu_t)tenthousandths);

        fprintf(fp, "%22s", "");

        fprintf(fp, " %-8.8s", "GNSS");

        fputc('\n', fp);

        fputs("HPA", fp);

        yodel_format_hpalt2aaltitude(sp->payload.hMSL, sp->payload.hMSLHp, &meters, &tenthousandths);
        fprintf(fp, " %6lld.%04llum MSL", (diminuto_lld_t)meters, (diminuto_llu_t)tenthousandths);

        yodel_format_hpalt2aaltitude(sp->payload.height, sp->payload.heightHp, &meters, &tenthousandths);
        fprintf(fp, " %6lld.%04llum GEO", (diminuto_lld_t)meters, (diminuto_llu_t)tenthousandths);

        yodel_format_hpacc2accuracy(sp->payload.vAcc, &meters, &tenthousandths);
        fprintf(fp, " %lc%6lld.%04llum", (wint_t)COMMON_PLUSMINUS, (diminuto_lld_t)meters, (diminuto_llu_t)tenthousandths);

        fprintf(fp, "%19s", "");

        fprintf(fp, " %-8.8s", "GNSS");

        fputc('\n', fp);

        fputs("NGS", fp);

        yodel_format_hppos2position(sp->payload.lat, sp->payload.latHp, &degrees, &minutes, &seconds, &tenthousandths, &direction);
        fprintf(fp, " %3u %02u %02u.%05u(%c)", degrees, minutes, seconds, tenthousandths, (direction < 0) ? 'S' : 'N');

        yodel_format_hppos2position(sp->payload.lon, sp->payload.lonHp, &degrees, &minutes, &seconds, &tenthousandths, &direction);
        fprintf(fp, " %3u %02u %02u.%05u(%c)", degrees, minutes, seconds, tenthousandths, (direction < 0) ? 'W' : 'E');

        fprintf(fp, "%29s", "");

        fprintf(fp, " %-8.8s", "GNSS");

        fputc('\n', fp);

    }
}

void print_attitude(FILE * fp, const yodel_attitude_t * sp)
{
    static const int32_t CENTIMILLI = 100000;

    if (sp->ticks != 0) {

        fputs("ATT", fp);

        fprintf(fp, " %4d.%01u%lc roll %lc%4d.%01u%lc",
            sp->payload.roll / CENTIMILLI,
            abs32(sp->payload.roll) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE,
            (wint_t)COMMON_PLUSMINUS,
            sp->payload.accRoll / CENTIMILLI,
            abs32(sp->payload.accRoll) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE);

        fprintf(fp, " %4d.%01u%lc pitch %lc%4d.%01u%lc",
            sp->payload.pitch / CENTIMILLI,
            abs32(sp->payload.pitch) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE,
            (wint_t)COMMON_PLUSMINUS,
            sp->payload.accPitch / CENTIMILLI,
            abs32(sp->payload.accPitch) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE);

        fprintf(fp, " %4d.%01u%lc yaw %lc%4d.%01u%lc",
            sp->payload.heading / CENTIMILLI,
            abs32(sp->payload.heading) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE,
            (wint_t)COMMON_PLUSMINUS,
            sp->payload.accHeading / CENTIMILLI,
            abs32(sp->payload.accHeading) % CENTIMILLI / (CENTIMILLI / 10),
            (wint_t)COMMON_DEGREE);

        fprintf(fp, "%1s", "");

        fprintf(fp, " %-8.8s", "IMU");

        fputc('\n', fp);

    }
}

void print_odometer(FILE * fp, const yodel_odometer_t * sp)
{
    double miles = 0.0;

    if (sp->ticks != 0) {

        fputs("ODO", fp);

        miles = sp->payload.distance;
        miles /= 1000.0;
        miles *= 0.621371;

        fprintf(fp, " %10.3lfmi", miles);
        fprintf(fp, " %6u.%03ukm", sp->payload.distance / 1000, sp->payload.distance % 1000);

        miles = sp->payload.totalDistance;
        miles /= 1000.0;
        miles *= 0.621371;

        fputs(" (", fp);
        fprintf(fp, " %10.3lfmi", miles);
        fprintf(fp, " %6u.%03ukm", sp->payload.totalDistance / 1000, sp->payload.totalDistance % 1000);
        fputs(" )", fp);

        fprintf(fp, " %lc%8um", (wint_t)COMMON_PLUSMINUS,  sp->payload.distanceStd);

        fprintf(fp, " %-8.8s", "IMU");

        fputc('\n', fp);

    }
}

void print_posveltim(FILE * fp, const yodel_posveltim_t * sp)
{
    if (sp->ticks != 0) {

        fputs("NED", fp);

        fprintf(fp, " %10dmm/s north %10dmm/s east %10dmm/s down", sp->payload.velN, sp->payload.velE, sp->payload.velD);

        fprintf(fp, " (%c)", (sp->payload.fixType < countof(FIXES)) ? FIXES[sp->payload.fixType] : FIXES[countof(FIXES) - 1]);

        fprintf(fp, "%2s", "");

        fprintf(fp, " %-8.8s", "IMU");

        fputc('\n', fp);

    }
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
void print_error_f(const char * file, int line, const void * buffer, ssize_t length)
{
    int error = errno;
    const uint8_t * bp = (const uint8_t *)buffer;
    size_t ll = length;
    unsigned char * expanded = (unsigned char *)0;
    unsigned char * ep = (unsigned char *)0;
    static unsigned char HEX[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    if (buffer == (const void *)0) {
        errno = EINVAL;
        diminuto_perror("print_error_f: buffer");
        return;
    }

    if (length <= 0) {
        errno = EINVAL;
        diminuto_perror("print_error_f: length");
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

void print_error_t1(void)
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
    print_error(BUFFER, sizeof(BUFFER));
}

void print_error_t2(void)
{
    /* This data was cut and pasted from the output of print_error_t1(). */
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
