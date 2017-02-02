/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 *
 * EXAMPLE
 *
 * gpstool [ -d ] [ -v ]
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "com/diag/hazer/hazer.h"

static void print_constellation(FILE *fp, const char * name, const hazer_constellation_t * cp, int full)
{
    static const int SATELLITES = sizeof(cp->id) / sizeof(cp->id[0]);
    static const int CHANNELS = sizeof(cp->sat) / sizeof(cp->sat[0]);
    int channel = 0;
    int channels = 0;
    int limit = 0;

    channels = cp->satellites;
    limit = (channels > SATELLITES) ? SATELLITES : channels;

    fprintf(fp, "%s {", name);
    for (channel = 0; channel < limit; ++channel) {
        if (cp->id[channel] != 0) {
            fprintf(fp, " %u", cp->id[channel]);
        }
    }
    fprintf(fp, " } [%d/%d] %.2lf %.2lf %.2lf\n", channels, SATELLITES, cp->pdop, cp->hdop, cp->vdop);

    if (!full) {
        return;
    }

    channels = cp->channels;
    limit = (channels > CHANNELS) ? CHANNELS : channels;

    for (channel = 0; channel < limit; ++channel) {
        if (cp->sat[channel].id != 0) {
            fprintf(fp, "%s [%d/%d/%d] %u %uo %uo %udBHz\n", name, channel + 1, channels, CHANNELS, cp->sat[channel].id, cp->sat[channel].elv_degrees, cp->sat[channel].azm_degrees, cp->sat[channel].snr_dbhz);
        }
    }
}

static void print_position(FILE * fp, const char * name, const hazer_position_t * pp)
{
    uint64_t nanoseconds = 0;
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    int degrees = 0;
    int minutes = 0;
    int seconds = 0;
    int direction = 0;

    nanoseconds = pp->dmy_nanoseconds;
    if (nanoseconds == 0) { return; }

    fputs(name, fp);

    nanoseconds += pp->utc_nanoseconds;
    hazer_format_nanoseconds2timestamp(nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);
    fprintf(fp, " %04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour, minute, second);

    hazer_format_degrees2position(pp->lat_degrees, &degrees, &minutes, &seconds, &direction);
    fprintf(fp, " %do%02d'%02d\"%c", degrees, minutes, seconds, direction < 0 ? 'S' : 'N');

    hazer_format_degrees2position(pp->lon_degrees, &degrees, &minutes, &seconds, &direction);
    fprintf(fp, " %do%02d'%02d\"%c", degrees, minutes, seconds, direction < 0 ? 'W' : 'E');

    fprintf(fp, " %.2lf'", pp->alt_meters * 3.2808);

    fprintf(fp, " %.2lfo", pp->cog_degrees);

    fprintf(fp, " %.2lfmph", pp->sog_knots * 1.150779);

    fputc('\n', fp);
}

int main(int argc, char * argv[])
{
    const char * program = (const char *)0;
    int debug = 0;
    int verbose = 0;
    hazer_state_t state = HAZER_STATE_EOF;
    hazer_state_t prior = HAZER_STATE_START;
    hazer_buffer_t buffer = { 0 };
    hazer_vector_t vector = { 0 };
    hazer_position_t position = { 0 };
    hazer_constellation_t constellation = { 0 };
    FILE * fp = stdin;
    int rc = 0;
    int ch = EOF;
    char * bb = (char *)0;
    ssize_t size = 0;
    ssize_t ss = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    ssize_t count = 0;
    char ** vv = (char **)0;
    int tt = 0;
    uint8_t cs = 0;
    uint8_t ck = 0;
    char msn = '\0';
    char lsn = '\0';

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    if (argc <= 1) {
        /* Do nothing. */
    } else if (strcmp(argv[1], "-d") == 0) {
        debug = !0;
    } else if (strcmp(argv[1], "-v") == 0) {
        verbose = !0;
    } else {
        /* Do nothing. */
    }

    if (debug) {
        hazer_debug(stderr);
    }

    rc = hazer_initialize();
    assert(rc == 0);

    while (!0) {

        state = HAZER_STATE_START;

        while (!0) {
            ch = fgetc(fp);
            prior = state;
            state = hazer_machine(state, ch, buffer, sizeof(buffer), &bb, &ss);
            if (state == HAZER_STATE_END) {
                break;
            } else if  (state == HAZER_STATE_EOF) {
                break;
            } else if ((prior != HAZER_STATE_START) && (state == HAZER_STATE_START)) {
                /* State machine restarted. */
                fprintf(stderr, "%s: ERR\n", program);
            } else {
                /* Do nothing. */
            }
        }

        if (state == HAZER_STATE_EOF) {
            fprintf(stderr, "%s: EOF\n", program);
            break;
        }

        assert(state == HAZER_STATE_END);

        size = ss;
        assert(size > 0);

        assert(buffer[0] == '$');
        assert(buffer[size - 1] == '\0');
        assert(buffer[size - 2] == '\n');
        assert(buffer[size - 3] == '\r');
        assert(buffer[size - 6] == '*');

        cs = hazer_checksum(buffer, size);

        rc = hazer_characters2checksum(buffer[size - 5], buffer[size - 4], &ck);
        assert(rc >= 0);

        if (ck != cs) {
            /* Checksum failed. */
            fprintf(stderr, "%s: BAD 0x%02x 0x%02x\n", program, cs, ck);
            continue;
        }

        rc = hazer_checksum2characters(ck, &msn, &lsn);
        assert(rc >= 0);
        assert(msn == buffer[size - 5]);
        assert(lsn == buffer[size - 4]);

        rc = hazer_characters2checksum(msn, lsn, &ck);
        assert(rc >= 0);
        assert(ck == cs);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]),  buffer, size);
        assert(count >= 0);
        assert(vector[count] == (char *)0);
        assert(count < (sizeof(vector) / sizeof(vector[0])));

        if (verbose) {
            for (vv = vector, tt = 1; *vv != (char *)0; ++vv, ++tt) {
                fputs(*vv, stderr);
                fputc((tt == count) ? '\n' : ',', stderr);
            }
            fflush(stderr);
        }

        if (hazer_parse_gga(&position, vector, count) == 0) {
            print_position(stdout, "GGA",  &position);
        } else if (hazer_parse_rmc(&position, vector, count) == 0) {
            print_position(stdout, "RMC", &position);
        } else if (hazer_parse_gsa(&constellation, vector, count) == 0) {
            print_constellation(stdout, "GSA", &constellation, 0);
        } else if (hazer_parse_gsv(&constellation, vector, count) == 0) {
            print_constellation(stdout, "GSV", &constellation, !0);
        } else {
            /* Do nothing. */
        }
        fflush(stdout);

    }

    rc = hazer_finalize();
    assert(rc == 0);

    return 0;
}
