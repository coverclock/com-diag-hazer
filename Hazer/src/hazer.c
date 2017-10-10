/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "com/diag/hazer/hazer.h"
#include "../src/hazer.h"

/******************************************************************************
 *
 ******************************************************************************/

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

const char * HAZER_TALKER_NAME[] = {
    "GPS",
    "GLONASS",
    "GALILEO",
    "GNSS",
    "RADIO",
};

/******************************************************************************
 *
 ******************************************************************************/

int hazer_initialize(void)
{
    tzset(); /* In my glibc this is an expensive operation the first time. */
    return 0;
}

int hazer_finalize(void)
{
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/

hazer_state_t hazer_machine(hazer_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp)
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

    case HAZER_STIMULUS_NUL:
        DEBUG("STARTING '%c'?\n", ch);
        state = HAZER_STATE_START;
        break;

    case HAZER_STIMULUS_START:
        DEBUG("STARTING '%c'?\n", ch);
        state = HAZER_STATE_START;
        break;

    case HAZER_STIMULUS_ENCAPSULATION:
        DEBUG("STARTING '%c'?\n", ch);
        state = HAZER_STATE_START;
        break;

    case HAZER_STIMULUS_CR:
        /* Do nothing. */
        break;

    case HAZER_STIMULUS_LF:
        /* Do nothing. */
        break;

    default:
        if (!((HAZER_STIMULUS_MINIMUM <= ch) && (ch <= HAZER_STIMULUS_MAXIMUM))) {
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
        if (ch == HAZER_STIMULUS_START) {
            DEBUG("START '%c'.\n", ch);
            state = HAZER_STATE_BODY;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else if (ch == HAZER_STIMULUS_ENCAPSULATION) {
            DEBUG("ENCAPSULATE '%c'.\n", ch);
            state = HAZER_STATE_BODY;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else {
            /* Do nothing. */
        }
        break;

    case HAZER_STATE_BODY:
        if (ch == HAZER_STIMULUS_CHECKSUM) {
            state = HAZER_STATE_MSN;
        }
        action = HAZER_ACTION_SAVE;
        break;

    case HAZER_STATE_MSN:
        if ((HAZER_STIMULUS_DECMIN <= ch) && (ch <= HAZER_STIMULUS_DECMAX)) {
            state = HAZER_STATE_LSN;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_STIMULUS_HEXMIN <= ch) && (ch <= HAZER_STIMULUS_HEXMAX)) {
            state = HAZER_STATE_LSN;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_LSN:
        if ((HAZER_STIMULUS_DECMIN <= ch) && (ch <= HAZER_STIMULUS_DECMAX)) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_STIMULUS_HEXMIN <= ch) && (ch <= HAZER_STIMULUS_HEXMAX)) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CR:
        if (ch == HAZER_STIMULUS_CR) {
            state = HAZER_STATE_LF;
            action = HAZER_ACTION_SAVESPECIAL;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_LF:
        if (ch == HAZER_STIMULUS_LF) {
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

uint8_t hazer_checksum(const void * buffer, size_t size)
{
    uint8_t cs = 0;
    const char * bb = (const char *)buffer;
    size_t ss = size;
    uint8_t ch = '\0';

    do {

        if (ss == 0) {
            break;
        }

        ++bb;
        --ss;

        if (ss == 0) {
            break;
        }

        ch = *(bb++);
        cs = ch;
        --ss;

        while ((ss > 0) && (*bb != HAZER_STIMULUS_CHECKSUM) && (*bb != '\0')) {
            ch = *(bb++);
            cs ^= ch;
            --ss;
        }

    } while (0);

    return cs;
}

int hazer_characters2checksum(char msn, char lsn, uint8_t * ckp)
{
    int rc = 0;

    if ((HAZER_STIMULUS_DECMIN <= msn) && (msn <= HAZER_STIMULUS_DECMAX)) {
        *ckp = (msn - HAZER_STIMULUS_DECMIN + 0) << 4;
    } else if ((HAZER_STIMULUS_HEXMIN <= msn) && (msn <= HAZER_STIMULUS_HEXMAX)) {
        *ckp = (msn - HAZER_STIMULUS_HEXMIN + 10) << 4;
    } else { 
        rc = -1;
    }

    if ((HAZER_STIMULUS_DECMIN <= lsn) && (lsn <= HAZER_STIMULUS_DECMAX)) {
        *ckp |= (lsn - HAZER_STIMULUS_DECMIN + 0);
    } else if ((HAZER_STIMULUS_HEXMIN <= lsn) && (lsn <= HAZER_STIMULUS_HEXMAX)) {
        *ckp |= (lsn - HAZER_STIMULUS_HEXMIN + 10);
    } else { 
        rc = -1;
    }

    return rc;
}

int hazer_checksum2characters(uint8_t ck, char * msnp, char * lsnp)
{
    int rc = 0;
    uint8_t msn = 0;
    uint8_t lsn = 0;

    msn = ck >> 4;

    if ((0x0 <= msn) && (msn <= 0x9)) {
        *msnp = '0' + msn;
    } else if ((0xa <= msn) && (msn <= 0xf)) {
        *msnp = 'A' + msn - 10;
    } else {
        rc = -1; /* Impossible. */
    }

    lsn = ck & 0xf;

    if ((0x0 <= lsn) && (lsn <= 0x9)) {
        *lsnp = '0' + lsn;
    } else if ((0xa <= lsn) && (lsn <= 0xf)) {
        *lsnp = 'A' + lsn - 10;
    } else {
        rc = -1; /* Impossible. */
    }

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

ssize_t hazer_tokenize(char * vector[], size_t count, void * buffer, size_t size)
{
    char ** vv = vector;
    char * bb = buffer;

    if (count > 1) {
        *(vv++) = bb;
        --count;
        while ((size--) > 0) {
            if (*bb == ',') {
                *(bb++) = '\0';
                DEBUG("TOK \"%s\".\n", *vv);
                if (count <= 1) {
                    break;
                }
                *(vv++) = bb;
                --count;
            } else if (*bb == '*') {
                *(bb++) = '\0';
                DEBUG("TOK \"%s\".\n", *vv);
                break;
            } else {
                ++bb;
            }
        }
    }

    if (count > 0) {
        *(vv++) = (char *)0;
        DEBUG("TOK 0x0.\n");
        --count;
    }

    return (vv - vector);
}

ssize_t hazer_serialize(void * buffer, size_t size, char * vector[], size_t count)
{
    char * bb = buffer;
    char ** vv = vector;
    ssize_t ss = 0;

    while ((count > 1) && (*vv != (char *)0)) {
        ss = strlen(*vv);
        if (size < (ss + 2)) {
            break;
        }
        strcpy(bb, *vv);
        DEBUG("STR \"%s\".\n", *vv);
        bb += ss;
        size -= ss;
        if (size < 2) {
            break;
        }
        if (count > 2) {
            *(bb++) = HAZER_STIMULUS_DELIMITER;
            DEBUG("CHR \"%c\".\n", HAZER_STIMULUS_DELIMITER);
        } else {
            *(bb++) = HAZER_STIMULUS_CHECKSUM;
            DEBUG("CHR \"%c\".\n", HAZER_STIMULUS_CHECKSUM);
        }
        --count;
        --size;
        ++vv;
    }

    if (size > 0) {
        *(bb++) = '\0';
        DEBUG("CHR 0x0.\n");
        --size;
    }

    return (bb - (char *)buffer);
}

/******************************************************************************
 *
 ******************************************************************************/

uint64_t hazer_parse_fraction(const char * string, uint64_t * denominatorp)
{
    unsigned long long numerator = 0;
    unsigned long long denominator = 1;
    char * end = (char *)0;
    size_t length = 0;

    numerator = strtoull(string, &end, 10);
    length = end - string;
    while ((length--) > 0) {
        denominator = denominator * 10;
    }
    *denominatorp = denominator;

    return numerator;
}

uint64_t hazer_parse_utc(const char * string)
{
    uint64_t nanoseconds = 0;
    uint64_t numerator = 0;
    uint64_t denominator = 1;
    unsigned long hhmmss = 0;
    char * end = (char *)0;

    hhmmss = strtoul(string, &end, 10);
    nanoseconds = hhmmss / 10000;
    nanoseconds *= 60;
    hhmmss %= 10000;
    nanoseconds += hhmmss / 100;
    nanoseconds *= 60;
    hhmmss %= 100;
    nanoseconds += hhmmss;
    nanoseconds *= 1000000000ULL;

    if (*end == HAZER_STIMULUS_DECIMAL) {
        numerator = hazer_parse_fraction(end + 1, &denominator);
        numerator *= 1000000000ULL;
        numerator /= denominator;
        nanoseconds += numerator;
    }

    return nanoseconds;
}

uint64_t hazer_parse_dmy(const char * string)
{
    uint64_t nanoseconds = 0;
    unsigned long ddmmyy = 0;
    struct tm datetime = { 0 };
    extern long timezone;

    ddmmyy = strtoul(string, (char **)0, 10);

    datetime.tm_year = ddmmyy % 100;
    if (datetime.tm_year < 93) {  datetime.tm_year += 100; }
    datetime.tm_mon = ((ddmmyy % 10000) / 100) - 1;
    datetime.tm_mday = ddmmyy / 10000;

    nanoseconds = mktime(&datetime); 

    nanoseconds -= timezone;

    nanoseconds *= 1000000000ULL;

    return nanoseconds;
}

int64_t hazer_parse_latlon(const char * string, char direction, uint8_t * digitsp)
{
    int64_t nanodegrees = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    unsigned long dddmm = 0;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    dddmm = strtoul(string, &end, 10);
    nanodegrees = dddmm / 100;
    nanodegrees *= 1000000000LL;
    fraction = dddmm % 100;
    fraction *= 1000000000LL;
    fraction /= 60;
    nanodegrees += fraction;
   
    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000000000LL;
        fraction /= 60;
        fraction /= denominator;
        nanodegrees += fraction;
        --digits;
    }

    switch (direction) {
    case HAZER_STIMULUS_NORTH:
    case HAZER_STIMULUS_EAST:
        break;
    case HAZER_STIMULUS_SOUTH:
    case HAZER_STIMULUS_WEST:
        nanodegrees = -nanodegrees;
        break;
    default:
        break;
    }

    *digitsp = digits;

    return nanodegrees; 
}

int64_t hazer_parse_cog(const char * string, uint8_t * digitsp)
{
    int64_t nanodegrees = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    nanodegrees = strtol(string, &end, 10);
    nanodegrees *= 1000000000LL;

    if (nanodegrees < 0) {
        --digits;
    }

    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000000000LL;
        fraction /= denominator;
        if (nanodegrees < 0) {
            nanodegrees -= fraction;
        } else {
            nanodegrees += fraction;
        }
        --digits;
    }

    *digitsp = digits;

    return nanodegrees;
}

int64_t hazer_parse_sog(const char * string, uint8_t * digitsp)
{
    int64_t microknots = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    microknots = strtol(string, &end, 10);
    microknots *= 1000000LL;

    if (microknots < 0) {
        --digits;
    }

    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000000;
        fraction /= denominator;
        if (microknots < 0) {
            microknots -= fraction;
        } else {
            microknots += fraction;
        }
        --digits;
    }

    *digitsp = digits;

    return microknots;
}

int64_t hazer_parse_alt(const char * string, char units, uint8_t * digitsp)
{
    int64_t millimeters = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    millimeters = strtol(string, &end, 10);
    millimeters *= 1000LL;

    if (millimeters < 0) {
        --digits;
    }

    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000;
        fraction /= denominator;
        if (millimeters < 0) {
            millimeters -= fraction;
        } else {
            millimeters += fraction;
        }
        --digits;
    }

    *digitsp = digits;

    return millimeters;
}

double hazer_parse_num(const char * string)
{
    double number = 0.0;
    double fraction = 0;
    uint64_t numerator = 0;
    uint64_t denominator = 0;
    char * end = (char *)0;

    number = strtol(string, &end, 10);

    if (*end == HAZER_STIMULUS_DECIMAL) {
        numerator = hazer_parse_fraction(end + 1, &denominator);
        fraction = numerator;
        fraction /= denominator;
        if (number < 0) {
            number -= fraction;
        } else {
            number += fraction;
        }
    }

    return number;
}

/******************************************************************************
 *
 ******************************************************************************/

void hazer_format_nanoseconds2timestamp(uint64_t nanoseconds, int * yearp, int * monthp, int * dayp, int * hourp, int * minutep, int * secondp, uint64_t * nanosecondsp)
{
    struct tm datetime = { 0 };
    struct tm * dtp = (struct tm *)0;
    time_t zulu = 0;

    zulu = nanoseconds / 1000000000ULL;

    dtp = gmtime_r(&zulu, &datetime);

    *yearp = dtp->tm_year + 1900;
    *monthp = dtp->tm_mon + 1;
    *dayp = dtp->tm_mday;
    *hourp = dtp->tm_hour;
    *minutep = dtp->tm_min;
    *secondp = dtp->tm_sec;
    *nanosecondsp = nanoseconds % 1000000000ULL;
}

void hazer_format_nanodegrees2position(int64_t nanodegrees, int * degreesp, int * minutesp, int * secondsp, int * hundredthsp, int * directionp)
{
    int hundredths = 0;
    char direction = '\0';

    if (nanodegrees < 0) {
        nanodegrees = -nanodegrees;
        *directionp = -1;
    } else {
        *directionp = 1;
    }

    *degreesp = nanodegrees / 1000000000LL;
    nanodegrees %= 1000000000LL;
    *minutesp = (nanodegrees * 60LL) / 1000000000LL;
    nanodegrees %= 1000000000LL / 60LL;
    *secondsp = (nanodegrees * 3600LL) / 1000000000LL;
    nanodegrees %= 1000000000LL / 3600LL;
    *hundredthsp = (nanodegrees * 360000LL) / 1000000000LL;
}

const char * hazer_format_nanodegrees2compass32(int64_t nanodegrees)
{
    static const char * COMPASS[] = {
        "N", "NbE", "NNE", "NEbN", "NE", "NEbE", "ENE", "EbN",
        "E", "EbS", "ESE", "SEbE", "SE", "SEbS", "SSE", "SbE",
        "S", "SbW", "SSW", "SWbS", "SW", "SWbW", "WSW", "WbS",
        "W", "WbN", "WNW", "NWbW", "NW", "NWbN", "NNW", "NbW",
    };
    static const int DIVISION = 360000 / (sizeof(COMPASS) / sizeof(COMPASS[0]));
    unsigned long index = 0;

    index = nanodegrees / 1000000LL;
    index += DIVISION / 2;
    index %= 360000;
    if (index < 0) { index += 360000; }
    index /= DIVISION;

    return COMPASS[index];
}

const char * hazer_format_nanodegrees2compass8(int64_t nanodegrees)
{
    static const char * COMPASS[] = {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW"
    };
    static const int DIVISION = 360000 / (sizeof(COMPASS) / sizeof(COMPASS[0]));
    unsigned long index = 0;

    index = nanodegrees / 1000000LL;
    index += DIVISION / 2;
    index %= 360000;
    if (index < 0) { index += 360000; }
    index /= DIVISION;

    return COMPASS[index];
}

/******************************************************************************
 *
 ******************************************************************************/

hazer_talker_t hazer_parse_talker(char * vector[], size_t count)
{
    hazer_talker_t talker = HAZER_TALKER_NA;

    if (count < 1) { 
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XX")) < (sizeof("$XX") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + 1, HAZER_NMEA_GNSS_TALKER, sizeof(HAZER_NMEA_GNSS_TALKER) - 1) == 0) {
        talker = HAZER_TALKER_GNSS;
    } else if (strncmp(vector[0] + 1, HAZER_NMEA_GPS_TALKER, sizeof(HAZER_NMEA_GPS_TALKER) - 1) == 0) {
        talker = HAZER_TALKER_GPS;
    } else if (strncmp(vector[0] + 1, HAZER_NMEA_GLONASS_TALKER, sizeof(HAZER_NMEA_GLONASS_TALKER) - 1) == 0) {
        talker = HAZER_TALKER_GLONASS;
    } else if (strncmp(vector[0] + 1, HAZER_NMEA_GALILEO_TALKER, sizeof(HAZER_NMEA_GALILEO_TALKER) - 1) == 0) {
        talker = HAZER_TALKER_GALILEO;
    } else if (strncmp(vector[0] + 1, HAZER_NMEA_RADIO_TALKER, sizeof(HAZER_NMEA_RADIO_TALKER) - 1) == 0) {
        talker = HAZER_TALKER_RADIO;
    } else {
        /* Do nothing. */
    }

    return talker;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_gga(hazer_position_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char GGA[] = HAZER_NMEA_GPS_MESSAGE_GGA;
    uint64_t utc_nanoseconds = 0;
    uint64_t tot_nanoseconds = 0;
    
    if (count < 1) { 
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, GGA, sizeof(GGA) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 11) { 
        /* Do nothing. */
    } else if (*vector[6] == '0') {
        /* Do nothing. */
    } else {
        utc_nanoseconds = hazer_parse_utc(vector[1]);
        tot_nanoseconds = utc_nanoseconds + datap->dmy_nanoseconds;
        if (tot_nanoseconds >= datap->tot_nanoseconds) {
            datap->tot_nanoseconds = tot_nanoseconds;
            datap->utc_nanoseconds = utc_nanoseconds;
            datap->lat_nanodegrees = hazer_parse_latlon(vector[2], *(vector[3]), &datap->lat_digits);
            datap->lon_nanodegrees = hazer_parse_latlon(vector[4], *(vector[5]), &datap->lon_digits);
            datap->sat_used = strtol(vector[7], (char **)0, 10);
            datap->alt_millimeters = hazer_parse_alt(vector[9], *(vector[10]), &datap->alt_digits);
            rc = 0;
        } else {
            DEBUG("TIME?\n");
        }
    }

    return rc;
}

int hazer_parse_rmc(hazer_position_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char RMC[] = HAZER_NMEA_GPS_MESSAGE_RMC;
    uint64_t utc_nanoseconds = 0;
    uint64_t dmy_nanoseconds = 0;
    uint64_t tot_nanoseconds = 0;

    if (count < 1) { 
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXRMC")) != (sizeof("$XXRMC") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, RMC, sizeof(RMC) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 10) { 
        /* Do nothing. */
    } else if (*vector[2] != 'A') {
        /* Do nothing. */
    } else {
        utc_nanoseconds = hazer_parse_utc(vector[1]);
        dmy_nanoseconds = hazer_parse_dmy(vector[9]);
        tot_nanoseconds = utc_nanoseconds + dmy_nanoseconds;
        if (tot_nanoseconds >= datap->tot_nanoseconds) {
            datap->tot_nanoseconds = tot_nanoseconds;
            datap->utc_nanoseconds = utc_nanoseconds;
            datap->dmy_nanoseconds = dmy_nanoseconds;
            datap->lat_nanodegrees = hazer_parse_latlon(vector[3], *(vector[4]), &datap->lat_digits);
            datap->lon_nanodegrees = hazer_parse_latlon(vector[5], *(vector[6]), &datap->lon_digits);
            datap->sog_microknots = hazer_parse_sog(vector[7], &datap->sog_digits);
            datap->cog_nanodegrees = hazer_parse_cog(vector[8], &datap->cog_digits);
            rc = 0;
        } else {
            DEBUG("TIME?\n");
        }
    }

    return rc;
}

int hazer_parse_gsv(hazer_constellation_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char GSV[] = HAZER_NMEA_GPS_MESSAGE_GSV;
    int messages = 0;
    int message = 0;
    int start = 0;
    int index = 4;
    int slot = 0;
    int channel = 0;
    int satellites = 0;
    int limit = sizeof(datap->sat) / sizeof(datap->sat[0]);
    unsigned int id = 0;
    
    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXGSV")) != (sizeof("$XXGSV") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, GSV, sizeof(GSV) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 5) {
        /* Do nothing. */
    } else {
        messages = strtol(vector[1], (char **)0, 10);
        message = strtol(vector[2], (char **)0, 10);
        if (message <= 0) {
            /* Do nothing. */
        } else if (message > messages) {
            /* Do nothing. */
        } else {
            channel = (message - 1) * HAZER_CONSTANT_GPS_VIEWS;
            satellites = strtol(vector[3], (char **)0, 10);
            for (slot = 0; slot < HAZER_CONSTANT_GPS_VIEWS; ++slot) {
                if (channel >= satellites) { break; }
                if (channel > limit) { break; }
                id = strtol(vector[index++], (char **)0, 10);
                if (id <= 0) { break; }
                datap->sat[channel].id = id;
                datap->sat[channel].elv_degrees = strtoul(vector[index++], (char **)0, 10);
                datap->sat[channel].azm_degrees = strtoul(vector[index++], (char **)0, 10);
                datap->sat[channel].snr_dbhz = strtoul(vector[index++], (char **)0, 10);
                ++channel;
                rc = 1;
            }
            datap->channels = channel;
            datap->view = satellites;
            if (rc < 0) {
                /* Do nothing. */
            } else if (message < messages) {
                /* Do nothing. */
            } else {
                rc = 0;
            }
        }
    }

    return rc;
}

int hazer_parse_gsa(hazer_solution_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    int index = 3;
    int slot = 0;
    int id = 0;
    int satellites = 0;
    static const char GSA[] = HAZER_NMEA_GPS_MESSAGE_GSA;
    int limit = sizeof(datap->id) / sizeof(datap->id[0]);

    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXGSA")) != (sizeof("$XXGSA") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, GSA, sizeof(GSA) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 18) {
        /* Do nothing. */
    } else if (*vector[2] == '1') {
        /* Do nothing. */
    } else {
        for (slot = 0; slot < limit; ++slot) {
            id = strtol(vector[index++], (char **)0, 10);
            if (id <= 0) { break; }
            datap->id[slot] = id;
            ++satellites;
        }
        datap->active = satellites;
        datap->pdop = hazer_parse_num(vector[15]);
        datap->hdop = hazer_parse_num(vector[16]);
        datap->vdop = hazer_parse_num(vector[17]);
        rc = 0;
    }

    return rc;
}
