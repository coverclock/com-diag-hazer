/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
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

const char * HAZER_TALKER_NAME[] = HAZER_TALKER_NAME_INITIALIZER;

const char * HAZER_SYSTEM_NAME[] = HAZER_SYSTEM_NAME_INITIALIZER;

/******************************************************************************
 *
 ******************************************************************************/

/**
 * Return the absolute value of a signed sixty-four bit integer.
 * @param datum is a signed sixty-four bit integer.
 * @return an unsigned sixty-four bit integer.
 */
static inline uint64_t abs64(int64_t datum)
{
    return (datum >= 0) ? datum : -datum;
}

/******************************************************************************
 *
 ******************************************************************************/

FILE * hazer_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_initialize(void)
{
    /*
     * In the glibc I perused, this is a relatively expensive operation the
     * first time it is called.
     */
    tzset();
    return 0;
}

int hazer_finalize(void)
{
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/

hazer_state_t hazer_machine(hazer_state_t state, uint8_t ch, void * buffer, size_t size, hazer_context_t * pp)
{
    int done = !0;
    hazer_action_t action = HAZER_ACTION_SKIP;
    hazer_state_t old = state;

    /*
     * Advance state machine based on stimulus.
     */

    switch (state) {

    case HAZER_STATE_STOP:
        /* Do nothing. */
        break;

    case HAZER_STATE_START:
        if (ch == HAZER_STIMULUS_START) {
            pp->bp = (uint8_t *)buffer;
            pp->sz = size;
            pp->tot = 0;
            pp->cs = 0;
            pp->msn = '\0';
            pp->lsn = '\0';
            state = HAZER_STATE_BODY;
            action = HAZER_ACTION_SAVE;
        } else if (ch == HAZER_STIMULUS_ENCAPSULATION) {
            pp->bp = (uint8_t *)buffer;
            pp->sz = size;
            pp->cs = 0;
            pp->msn = '\0';
            pp->lsn = '\0';
            state = HAZER_STATE_BODY;
            action = HAZER_ACTION_SAVE;
        } else {
            /* Do nothing. */
        }
        break;

    case HAZER_STATE_BODY:
        /*
         * According to [NMEA 0183, 4.10, 2012] the checksum field is "required
         * on all sentences". According to [Wikipedia, "NMEA 0183", 2019-05-27]
         * the checksum field is optional on all but a handful of sentences.
         * I'm assuming Wikipedia is referencing an earlier version of the
         * standard. I've never tested an NMEA device that didn't provide
         * checksums on all sentences.
         */
        if (ch == HAZER_STIMULUS_CHECKSUM) {
            hazer_checksum2characters(pp->cs, &(pp->msn), &(pp->lsn));
            state = HAZER_STATE_MSN;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_STIMULUS_MINIMUM <= ch) && (ch <= HAZER_STIMULUS_MAXIMUM)) {
            hazer_checksum(ch, &(pp->cs));
            action = HAZER_ACTION_SAVE;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_MSN:
        if (ch == pp->msn) {
            state = HAZER_STATE_LSN;
            action = HAZER_ACTION_SAVE;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_LSN:
        if (ch == pp->lsn) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_CR:
        if (ch == HAZER_STIMULUS_CR) {
            state = HAZER_STATE_LF;
            action = HAZER_ACTION_SAVE;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_LF:
        if (ch == HAZER_STIMULUS_LF) {
            state = HAZER_STATE_END;
            action = HAZER_ACTION_TERMINATE;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_END:
        break;

    /*
     * No default: must handle all cases.
     */

    }

    /*
     * Perform associated action.
     */

    switch (action) {

    case HAZER_ACTION_SKIP:
        break;

    case HAZER_ACTION_SAVE:
        if (pp->sz > 0) {
            *(pp->bp++) = ch;
            pp->sz -= 1;
        } else {
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_ACTION_TERMINATE:
        if (pp->sz > 1) {
            *(pp->bp++) = ch;
            pp->sz -= 1;
            *(pp->bp++) = '\0';
            pp->sz -= 1;
            pp->tot = size - pp->sz;
        } else {
             state = HAZER_STATE_STOP;
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
    } else if (old == HAZER_STATE_STOP) {
        /* Do nothing. */
    } else if ((' ' <= ch) && (ch <= '~')) {
        fprintf(debug, "NMEA %c %c %c 0x%02x '%c'\n", old, state, action, ch, ch);
    } else {
        fprintf(debug, "NMEA %c %c %c 0x%02x\n", old, state, action, ch);
    }

    return state;
}

/******************************************************************************
 *
 ******************************************************************************/

const void * hazer_checksum_buffer(const void * buffer, size_t size, uint8_t * msnp, uint8_t * lsnp)
{
    const void * result = (void *)0;
    const unsigned char * bp = (const unsigned char *)0;
    uint8_t cs = 0;

    if (size > 0) {

        bp = (const unsigned char *)buffer;

        ++bp;
        --size;

        while ((size > 0) && (*bp != HAZER_STIMULUS_CHECKSUM) && (*bp != '\0')) {
            hazer_checksum(*(bp++), &cs);
            --size;
        }

        hazer_checksum2characters(cs, msnp, lsnp);

        result = bp;

    }

    return result;
}

int hazer_characters2checksum(uint8_t msn, uint8_t lsn, uint8_t * ckp)
{
    int rc = 0;

    if ((HAZER_STIMULUS_DECMIN <= msn) && (msn <= HAZER_STIMULUS_DECMAX)) {
        *ckp = (msn - HAZER_STIMULUS_DECMIN + 0) << 4;
    } else if ((HAZER_STIMULUS_HEXMIN_LC <= msn) && (msn <= HAZER_STIMULUS_HEXMAX_LC)) {
        *ckp = (msn - HAZER_STIMULUS_HEXMIN_LC + 10) << 4;
    } else if ((HAZER_STIMULUS_HEXMIN_UC <= msn) && (msn <= HAZER_STIMULUS_HEXMAX_UC)) {
        *ckp = (msn - HAZER_STIMULUS_HEXMIN_UC + 10) << 4;
    } else { 
        rc = -1;
    }

    if ((HAZER_STIMULUS_DECMIN <= lsn) && (lsn <= HAZER_STIMULUS_DECMAX)) {
        *ckp |= (lsn - HAZER_STIMULUS_DECMIN + 0);
    } else if ((HAZER_STIMULUS_HEXMIN_LC <= lsn) && (lsn <= HAZER_STIMULUS_HEXMAX_LC)) {
        *ckp |= (lsn - HAZER_STIMULUS_HEXMIN_LC + 10);
    } else if ((HAZER_STIMULUS_HEXMIN_UC <= lsn) && (lsn <= HAZER_STIMULUS_HEXMAX_UC)) {
        *ckp |= (lsn - HAZER_STIMULUS_HEXMIN_UC + 10);
    } else { 
        rc = -1;
    }

    return rc;
}

void hazer_checksum2characters(uint8_t ck, uint8_t * msnp, uint8_t * lsnp)
{
    uint8_t msn = 0;
    uint8_t lsn = 0;

    msn = ck >> 4;

    if ((0x0 <= msn) && (msn <= 0x9)) {
        *msnp = '0' + msn;
    } else if ((0xa <= msn) && (msn <= 0xf)) {
        *msnp = 'A' + msn - 10;
    } else {
        /* Impossible. */
    }

    lsn = ck & 0xf;

    if ((0x0 <= lsn) && (lsn <= 0x9)) {
        *lsnp = '0' + lsn;
    } else if ((0xa <= lsn) && (lsn <= 0xf)) {
        *lsnp = 'A' + lsn - 10;
    } else {
        /* Impossible. */
    }
}

ssize_t hazer_length(const void * buffer, size_t size)
{
    ssize_t result = -1;
    size_t length = 0;
    const char * bp = (const char *)0;

    bp = (const char *)buffer;

    if (size < HAZER_NMEA_SHORTEST) {
        /* Do nothing. */
    } else if (*bp != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else {
        while (size > 0) {
            if (*bp == '\0') { break; }
            size -= 1;
            length += 1;
            if (*(bp++) == HAZER_STIMULUS_LF) { break; }
        }
        if (length < HAZER_NMEA_SHORTEST) {
            /* Do nothing. */
        } else if (*(--bp) != HAZER_STIMULUS_LF) {
            /* Do nothing. */
        } else if (*(--bp) != HAZER_STIMULUS_CR) {
            /* Do nothing. */
        } else {
            result = length;
        }
    }

    return result;
}

ssize_t hazer_validate(const void * buffer, size_t size)
{
    ssize_t result = -1;
    ssize_t length = 0;
    const uint8_t * bp = (uint8_t *)0;
    uint8_t msn = 0;
    uint8_t lsn = 0;

    if ((length = hazer_length(buffer, size)) <= 0) {
        /* Do nothing. */
    } else if ((bp = (uint8_t *)hazer_checksum_buffer(buffer, length, &msn, &lsn)) == (unsigned char *)0) {
        /* Do nothing. */
    } else if ((msn != bp[1]) || (lsn != bp[2])) {
        /* Do nothing. */
    } else {
        result = length;
    }

    return result;
}

/******************************************************************************
 *
 ******************************************************************************/

ssize_t hazer_tokenize(char * vector[], size_t count, void * buffer, size_t size)
{
    char ** vv = vector;
    char ** tt = (char **)0;
    char * bb = (char *)buffer;
    ssize_t nn = 0;

    if (count > 1) {
        tt = vv;
        *(vv++) = bb;
        ++nn;
        --count;
        while ((size--) > 0) {
            if (*bb == ',') {
                *(bb++) = '\0';
                if (count <= 1) {
                    break;
                }
                tt = vv;
                *(vv++) = bb;
                ++nn;
                --count;
            } else if (*bb == '*') {
                *(bb++) = '\0';
                break;
            } else {
                ++bb;
            }
        }
    }

    if (count > 0) {
        tt = vv;
        *(vv++) = (char *)0;
        ++nn;
        --count;
    }

    return (vv - vector);
}

ssize_t hazer_serialize(void * buffer, size_t size, char * vector[], size_t count)
{
    char * bb = (char *)buffer;
    char ** vv = vector;
    ssize_t ss = 0;

    while ((count > 1) && (*vv != (char *)0)) {
        ss = strlen(*vv);
        if (size < (ss + 2)) {
            break;
        }
        strcpy(bb, *vv);
        bb += ss;
        size -= ss;
        if (size < 2) {
            break;
        }
        if (count > 2) {
            *(bb++) = HAZER_STIMULUS_DELIMITER;
        } else {
            *(bb++) = HAZER_STIMULUS_CHECKSUM;
        }
        --count;
        --size;
        ++vv;
    }

    if (size > 0) {
        *(bb++) = '\0';
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
    int64_t nanominutes = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    unsigned long dddmm = 0;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    dddmm = strtoul(string, &end, 10);
    nanominutes = dddmm / 100;
    nanominutes *= 60000000000LL;
    fraction = dddmm % 100;
    fraction *= 1000000000LL;
    nanominutes += fraction;
   
    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000000000LL;
        fraction /= denominator;
        nanominutes += fraction;
        --digits;
    }

    switch (direction) {
    case HAZER_STIMULUS_NORTH:
    case HAZER_STIMULUS_EAST:
        break;
    case HAZER_STIMULUS_SOUTH:
    case HAZER_STIMULUS_WEST:
        nanominutes = -nanominutes;
        break;
    default:
        break;
    }

    *digitsp = digits;

    return nanominutes;
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

int64_t hazer_parse_smm(const char * string, uint8_t * digitsp)
{
    int64_t millimetersperhour = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    millimetersperhour = strtol(string, &end, 10);
    millimetersperhour *= 1000000LL;

    if (millimetersperhour < 0) {
        --digits;
    }

    if (*end == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(end + 1, &denominator);
        fraction *= 1000000;
        fraction /= denominator;
        if (millimetersperhour < 0) {
            millimetersperhour -= fraction;
        } else {
            millimetersperhour += fraction;
        }
        --digits;
    }

    *digitsp = digits;

    return millimetersperhour;
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

uint16_t hazer_parse_dop(const char * string)
{
    uint16_t dop = HAZER_GNSS_DOP;
    unsigned long number = 0;
    int64_t fraction = 0;
    uint64_t denominator = 0;
    char * end = (char *)0;

    if (*string != '\0') {

        number = strtoul(string, &end, 10);
        if (end == (char *)0) {
            /* Do nothing. */
        } else if (number > (HAZER_GNSS_DOP / 100)) {
            /* Do nothing. */
        } else {

            number *= 100;

            if (*end == HAZER_STIMULUS_DECIMAL) {

                fraction = hazer_parse_fraction(end + 1, &denominator);
                fraction *= 100;
                fraction /= denominator;

                number += fraction;

            }

            dop = number;

        }
    }

    return dop;
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

void hazer_format_nanominutes2position(int64_t nanominutes, int * degreesp, int * minutesp, int * secondsp, int * thousandthsp, int * directionp)
{
    int hundredths = 0;
    char direction = '\0';

    if (nanominutes < 0) {
        nanominutes = -nanominutes;
        *directionp = -1;
    } else {
        *directionp = 1;
    }

    *degreesp = nanominutes / 60000000000LL;                /* Get integral degrees. */
    nanominutes = nanominutes % 60000000000LL;              /* Remainder. */
    *minutesp = nanominutes / 1000000000LL;                 /* Get integral minutes. */
    nanominutes = nanominutes % 1000000000LL;;              /* Remainder. */
    nanominutes *= 60LL;                                    /* Convert to nanoseconds. */
    *secondsp = nanominutes / 1000000000LL;                 /* Get integral seconds. */
    nanominutes = nanominutes % 1000000000LL;               /* Remainder. */
    *thousandthsp = (nanominutes * 1000LL) / 1000000000LL;   /* Get thousandths of a second. */
}

void hazer_format_nanominutes2degrees(int64_t nanominutes, int * degreesp, uint64_t * tenmillionthsp)
{
    *degreesp = nanominutes / 60000000000LL;                 /* Get integral degrees. */
    nanominutes = abs64(nanominutes);                        /* Fraction is unsigned. */
    nanominutes = nanominutes % 60000000000LL;               /* Remainder. */
    *tenmillionthsp = nanominutes / 6000ULL;                 /* Get ten millionths of a degree. */
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

hazer_talker_t hazer_parse_talker(const void * buffer)
{
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    const char * sentence = (const char *)0;
    const char * id = (const char *)0;
    const char * name = (const char *)0;
    int ii = 0;
    int rc = -1;

    sentence = (const char *)buffer;
    id = &(sentence[1]);

    if (sentence[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strnlen(sentence, sizeof("$XX")) < (sizeof("$XX") - 1)) {
        /* Do nothing. */
    } else {
        for (ii = 0; ii < HAZER_TALKER_TOTAL; ++ii) {
            name = HAZER_TALKER_NAME[ii];
            rc = strncmp(id, name, strlen(name));
            if (rc < 0) {
                break;
            } else if (rc == 0) {
                talker = (hazer_talker_t)ii;
                break;
            } else {
                /* Do nothing. */
            }
        }

    }

    return talker;
}

hazer_system_t hazer_map_talker_to_system(hazer_talker_t talker)
{
    hazer_system_t system = HAZER_SYSTEM_TOTAL;

    switch (talker) {

    case HAZER_TALKER_GPS:
        system = HAZER_SYSTEM_GPS;
        break;

    case HAZER_TALKER_GLONASS:
        system = HAZER_SYSTEM_GLONASS;
        break;

    case HAZER_TALKER_GALILEO:
        system = HAZER_SYSTEM_GALILEO;
        break;

    case HAZER_TALKER_GNSS:
        system = HAZER_SYSTEM_GNSS;
        break;

    /*
     * There are apparently three different BeiDou systems.
     * I haven't quite grokked how to discriminate them.
     * And there are two BeiDou talkers defined. I punt
     * and map them all to a single system until I find
     * more documentation. The only cited source is
     * "Technical Specification of Communication Protocol
     * for BDS Compatible Positioning Module" (TSCPB) which
     * I'm told is in Mandarin with no English translation.
     */

    case HAZER_TALKER_BEIDOU1:
        system = HAZER_SYSTEM_BEIDOU;
        break;

    case HAZER_TALKER_BEIDOU2:
        system = HAZER_SYSTEM_BEIDOU;
        break;

    case HAZER_TALKER_QZSS:
        system = HAZER_SYSTEM_QZSS;
        break;

    default:
        /* Do nothing. */
        break;

    }

    return system;
}

hazer_system_t hazer_map_id_to_system(uint16_t id)
{
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;

    if (id == 0) {
        /* Do nothing. */
    } else if ((HAZER_ID_GPS_FIRST <= id) && (id <= HAZER_ID_GPS_LAST)) {
        candidate = HAZER_SYSTEM_GPS;
    } else if ((HAZER_ID_SBAS_FIRST <= id) && (id <= HAZER_ID_SBAS_LAST)) {
        candidate = HAZER_SYSTEM_SBAS;
    } else if ((HAZER_ID_GLONASS_FIRST <= id) && (id <= HAZER_ID_GLONASS_LAST)) {
        candidate = HAZER_SYSTEM_GLONASS;
    } else if ((HAZER_ID_BEIDOU1_FIRST <= id) && (id <= HAZER_ID_BEIDOU1_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else if ((HAZER_ID_IMES_FIRST <= id) && (id <= HAZER_ID_IMES_LAST)) {
        candidate = HAZER_SYSTEM_IMES;
    } else if ((HAZER_ID_QZSS_FIRST <= id) && (id <= HAZER_ID_QZSS_LAST)) {
        candidate = HAZER_SYSTEM_QZSS;
    } else if ((HAZER_ID_GALILEO_FIRST <= id) && (id <= HAZER_ID_GALILEO_LAST)) {
        candidate = HAZER_SYSTEM_GALILEO;
    } else if ((HAZER_ID_BEIDOU2_FIRST <= id) && (id <= HAZER_ID_BEIDOU2_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else {
        /* Do nothing. */
    }

    return candidate;
}

/*
 * NMEA 0183 4.10 p. 94-95.
 * UBLOX8 R15 p. 373.
 */
hazer_system_t hazer_map_active_to_system(const hazer_active_t * activep) {
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;
    int slot = 0;
    static const int IDENTIFIERS = sizeof(activep->id) / sizeof(activep->id[0]);

    if ((0 <= activep->system) && (activep->system < HAZER_SYSTEM_TOTAL)) {
        system = (hazer_system_t)activep->system;
    } else {
        for (slot = 0; slot < IDENTIFIERS; ++slot) {
            if (slot >= activep->active) {
                break;
            } else if (activep->id[slot] == 0) {
                break;
            } else if ((candidate = hazer_map_id_to_system(activep->id[slot])) == HAZER_SYSTEM_TOTAL) {
                continue;
            } else {
                /* Do nothing. */
            }
            if (system == HAZER_SYSTEM_TOTAL) {
                system = candidate;
            } else if (system == candidate) {
                continue;
            } else if (candidate == HAZER_SYSTEM_SBAS) {
                continue;
            } else if (system == HAZER_SYSTEM_SBAS) {
                system = candidate;
            } else {
                system = HAZER_SYSTEM_GNSS;
            }
        }
    }

    return system;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_gga(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    static const char GGA[] = HAZER_NMEA_SENTENCE_GGA;
    
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
        positionp->utc_nanoseconds = hazer_parse_utc(vector[1]);
        positionp->old_nanoseconds = positionp->tot_nanoseconds;
        positionp->tot_nanoseconds = positionp->utc_nanoseconds + positionp->dmy_nanoseconds;
        positionp->lat_nanominutes = hazer_parse_latlon(vector[2], *(vector[3]), &positionp->lat_digits);
        positionp->lon_nanominutes = hazer_parse_latlon(vector[4], *(vector[5]), &positionp->lon_digits);
        positionp->sat_used = strtol(vector[7], (char **)0, 10);
        positionp->alt_millimeters = hazer_parse_alt(vector[9], *(vector[10]), &positionp->alt_digits);
        positionp->label = GGA;
        rc = 0;
    }

    return rc;
}

int hazer_parse_gsa(hazer_active_t * activep, char * vector[], size_t count)
{
    int rc = -1;
    static const char GSA[] = HAZER_NMEA_SENTENCE_GSA;
    int index = 3;
    int slot = 0;
    int id = 0;
    int satellites = 0;
    static const int IDENTIFIERS = sizeof(activep->id) / sizeof(activep->id[0]);

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
        for (slot = 0; slot < IDENTIFIERS; ++slot) {
            id = strtol(vector[index++], (char **)0, 10);
            if (id <= 0) { break; }
            activep->id[slot] = id;
            ++satellites;
        }
        /*
         * Unlike the GSV sentence, the GSA sentence isn't variable length.
         * Unused slots in the active list are denoted by empty fields.
         */
        activep->active = satellites;
        activep->pdop = hazer_parse_dop(vector[15]);
        activep->hdop = hazer_parse_dop(vector[16]);
        activep->vdop = hazer_parse_dop(vector[17]);
        /*
         * NMEA 0183 4.10 2012 has an additional 19th field containing
         * the GNSS System ID to identify GPS, GLONASS, GALILEO, etc.
         */
        activep->system = (count > 19) ? strtoul(vector[18], (char **)0, 10) : HAZER_SYSTEM_TOTAL;
        activep->label = GSA;
        rc = 0;
    }

    return rc;
}

int hazer_parse_gsv(hazer_view_t * viewp, char * vector[], size_t count)
{
    int rc = -1;
    static const char GSV[] = HAZER_NMEA_SENTENCE_GSV;
    int messages = 0;
    int message = 0;
    int start = 0;
    int index = 4;
    int slot = 0;
    int sequence = 0;
    int channel = 0;
    int satellites = 0;
    unsigned int id = 0;
    static const int SATELLITES = sizeof(viewp->sat) / sizeof(viewp->sat[0]);
    
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
            sequence = message - 1;
            channel = sequence * HAZER_GNSS_VIEWS;
            satellites = strtol(vector[3], (char **)0, 10);
            /*
             * "Null fields are not required for unused sets when less
             * than four sets are transmitted." [NMEA 0183 v4.10 2012 p. 96]
             * Unlike the GSA sentence, the GSV sentence can have a variable
             * number of fields. So from here on all indices are effectively
             * relative.
             */
            for (slot = 0; slot < HAZER_GNSS_VIEWS; ++slot) {
                if (channel >= satellites) { break; }
                if (channel >= SATELLITES) { break; }
                /*
                 * I'm pretty sure my U-Blox ZED-F9P-00B-01 chip has a
                 * firmware bug.  I believe this GSV sentence that it
                 * sent is incorrect.
                 *
                 * $GLGSV,3,3,11,85,26,103,25,86,02,152,29,1*75\r\n.
                 *
                 * I think either there should be a third set of four
                 * fields for the eleventh satellite, or the total count
                 * should be ten instead of eleven. So we check for that
                 * here.
                 */
                if ((index + 4) >= count) { break; }
                id = strtol(vector[index], (char **)0, 10);
                ++index;
                if (id <= 0) { break; }
                viewp->sat[channel].id = id;
                /*
                 * "For efficiency it is recommended that null fields be used
                 * in the additional sentences when the data is unchanged from
                 * the first sentence." [NMEA 0183 v4.10 2012 p. 96]
                 * Does this mean that the same satellite ID can appear more
                 * than once in the same tuple of GSV sentences? Or does this
                 * just apply to the (newish) signal ID in the last field?
                 */
                viewp->sat[channel].phantom = 0;
                if (strlen(vector[index]) == 0) {
                    viewp->sat[channel].phantom = !0;
                    viewp->sat[channel].elv_degrees = 0;
                } else {
                    viewp->sat[channel].elv_degrees = strtol(vector[index], (char **)0, 10);
                }
                ++index;
                if (strlen(vector[index]) == 0) {
                    viewp->sat[channel].phantom = !0;
                    viewp->sat[channel].azm_degrees = 0;
                } else {
                    viewp->sat[channel].azm_degrees = strtol(vector[index], (char **)0, 10);
                }
                ++index;
                viewp->sat[channel].untracked = 0;
                if (strlen(vector[index]) == 0) {
                    viewp->sat[channel].untracked = !0;
                    viewp->sat[channel].snr_dbhz = 0;
                } else {
                    viewp->sat[channel].snr_dbhz = strtol(vector[index], (char **)0, 10);
                }
                ++index;
                ++channel;
                rc = 1;
            }
            /*
             * NMEA 0183 4.10 2012 has an additional field containing the
             * signal identifier. This is constellation specific, but
             * indicates what frequency band was used, e.g. for GPS: L1C/A,
             * L2, etc.
             */
            if (index > (count - 2)) {
                viewp->signal[sequence] = 0;
            } else if (strlen(vector[count - 2]) > 0) {
                viewp->signal[sequence] = strtol(vector[count - 2], (char **)0, 10);
            } else if (sequence > 0) {
                viewp->signal[sequence] =  viewp->signal[sequence - 1];
            } else {
                viewp->signal[sequence] = 0;
            }
            viewp->channels = channel;
            viewp->view = satellites;
            viewp->pending = messages - message;
            viewp->label = GSV;
            /*
             * Only if this is the last message in the GSV tuple do we
             * emit a zero return code. That lets the application decide
             * when it wants to peruse its view database.
             */
            if (rc < 0) {
                /* Do nothing. */
            } else if (viewp->pending > 0) {
                /* Do nothing. */
            } else {
                rc = 0;
            }
        }
    }

    return rc;
}

int hazer_parse_rmc(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    static const char RMC[] = HAZER_NMEA_SENTENCE_RMC;

    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXRMC")) != (sizeof("$XXRMC") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, RMC, sizeof(RMC) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 12) {
        /* Do nothing. */
    } else if (*vector[2] != 'A') {
        /* Do nothing. */
    } else if (*vector[10] == 'N') {
        /* Do nothing. */
    } else if (*vector[11] == 'V') {
        /* Do nothing. */
    } else {
        positionp->utc_nanoseconds = hazer_parse_utc(vector[1]);
        positionp->dmy_nanoseconds = hazer_parse_dmy(vector[9]);
        positionp->old_nanoseconds = positionp->tot_nanoseconds;
        positionp->tot_nanoseconds = positionp->utc_nanoseconds + positionp->dmy_nanoseconds;
        positionp->lat_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &positionp->lat_digits);
        positionp->lon_nanominutes = hazer_parse_latlon(vector[5], *(vector[6]), &positionp->lon_digits);
        positionp->sog_microknots = hazer_parse_sog(vector[7], &positionp->sog_digits);
        positionp->cog_nanodegrees = hazer_parse_cog(vector[8], &positionp->cog_digits);
        positionp->label = RMC;
        rc = 0;
    }

    return rc;
}

int hazer_parse_gll(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    static const char GLL[] = HAZER_NMEA_SENTENCE_GLL;

    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, GLL, sizeof(GLL) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 8) {
        /* Do nothing. */
    } else if (*vector[6] == 'V') {
        /* Do nothing. */
    } else if (*vector[7] == 'N') {
        /* Do nothing. */
    } else {
        positionp->utc_nanoseconds = hazer_parse_utc(vector[5]);
        positionp->old_nanoseconds = positionp->tot_nanoseconds;
        positionp->tot_nanoseconds = positionp->utc_nanoseconds + positionp->dmy_nanoseconds;;
        positionp->lat_nanominutes = hazer_parse_latlon(vector[1], *(vector[2]), &positionp->lat_digits);
        positionp->lon_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &positionp->lon_digits);
        positionp->label = GLL;
        rc = 0;
    }

    return rc;
}

int hazer_parse_vtg(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    static const char VTG[] = HAZER_NMEA_SENTENCE_VTG;

    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXVTG")) != (sizeof("$XXVTG") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, VTG, sizeof(VTG) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 10) {
        /* Do nothing. */
    } else if (*vector[9] == 'N') {
        /* Do nothing. */
    } else {
        positionp->cog_nanodegrees = hazer_parse_cog(vector[1], &positionp->cog_digits);
        positionp->mag_nanodegrees = hazer_parse_cog(vector[3], &positionp->mag_digits);
        positionp->sog_microknots = hazer_parse_sog(vector[5], &positionp->sog_digits);
        positionp->sog_millimeters = hazer_parse_smm(vector[7], &positionp->smm_digits);
        positionp->label = VTG;
        rc = 0;
   }

    return rc;
}

int hazer_parse_txt(char * vector[], size_t count)
{
    int rc = -1;
    static const char TXT[] = HAZER_NMEA_SENTENCE_TXT;

    if (count < 1) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXTXT")) != (sizeof("$XXTXT") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strncmp(vector[0] + sizeof("$XX") - 1, TXT, sizeof(TXT) - 1) != 0) {
        /* Do nothing. */
    } else if (count < 5) {
        /* Do nothing. */
    } else {
        rc = 0;
   }

    return rc;
}
