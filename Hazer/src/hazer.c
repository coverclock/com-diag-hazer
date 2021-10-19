/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the Hazer module.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/common.h"
#include "../src/hazer.h"

/******************************************************************************
 *
 ******************************************************************************/

static FILE * debug  = (FILE *)0;

const char * HAZER_TALKER_NAME[] = HAZER_TALKER_NAME_INITIALIZER;

const char * HAZER_SYSTEM_NAME[] = HAZER_SYSTEM_NAME_INITIALIZER;

const char * HAZER_MODE_NAME[] = HAZER_MODE_NAME_INITIALIZER;

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
            pp->msn = HAZER_NMEA_UNSET;
            pp->lsn = HAZER_NMEA_UNSET;
            pp->error = 0;
            state = HAZER_STATE_PAYLOAD;
            action = HAZER_ACTION_SAVE;
        } else if (ch == HAZER_STIMULUS_ENCAPSULATION) {
            pp->bp = (uint8_t *)buffer;
            pp->sz = size;
            pp->cs = 0;
            pp->msn = HAZER_NMEA_UNSET;
            pp->lsn = HAZER_NMEA_UNSET;
            pp->error = 0;
            state = HAZER_STATE_PAYLOAD;
            action = HAZER_ACTION_SAVE;
        } else {
            /* Do nothing. */
        }
        break;

    case HAZER_STATE_PAYLOAD:
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
            pp->error = !0;
            state = HAZER_STATE_STOP;
        }
        break;

    case HAZER_STATE_LSN:
        if (ch == pp->lsn) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else {
            pp->error = !0;
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
    } else if (isprint(ch)) {
        fprintf(debug, "Machine NMEA %c %c %c *%c%c 0x%02x '%c'\n", old, state, action, pp->msn, pp->lsn, ch, ch);
    } else {
        fprintf(debug, "Machine NMEA %c %c %c *%c%c 0x%02x\n", old, state, action, pp->msn, pp->lsn, ch);
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
    uint8_t msn = HAZER_NMEA_UNSET;
    uint8_t lsn = HAZER_NMEA_UNSET;

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
    ssize_t result = 0;
    char ** vv = vector;
    char * bb = (char *)buffer;
    char ** tt = (char **)0;

    if (count > 1) {
        tt = vv;
        *(vv++) = bb;
        --count;
        while ((size--) > 0) {
            if (*bb == ',') {
                *(bb++) = '\0';
                if (debug != (FILE *)0) {
                    fprintf(debug, "Token [%zd] \"%s\"\n", result, *tt);
                }
                ++result;
                if (count <= 1) {
                    break;
                }
                tt = vv;
                *(vv++) = bb;
                --count;
            } else if (*bb == '*') {
                *(bb++) = '\0';
                if (debug != (FILE *)0) {
                    fprintf(debug, "Token [%zd] \"%s\"\n", result, *tt);
                }
                ++result;
                break;
            } else {
                ++bb;
            }
        }
    }

    if (count > 0) {
        *(vv++) = (char *)0;
        if (debug != (FILE *)0) {
            fprintf(debug, "Token [%zd] NULL\n", result);
        }
        ++result;
    }

    if (debug != (FILE *)0) {
        fprintf(debug, "Tokens [%zd]\n", result);
    }

    return result;
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

uint64_t hazer_parse_fraction(const char * string, uint64_t * denominatorp, char ** endp)
{
    unsigned long long numerator = 0;
    unsigned long long denominator = 1;
    size_t length = 0;

    numerator = strtoull(string, endp, 10);
    length = *endp - string;
    while ((length--) > 0) {
        denominator = denominator * 10;
    }
    *denominatorp = denominator;

    return numerator;
}

uint64_t hazer_parse_utc(const char * string, char ** endp)
{
    uint64_t nanoseconds = 0;
    uint64_t numerator = 0;
    uint64_t denominator = 1;
    unsigned long hhmmss = 0;

    hhmmss = strtoul(string, endp, 10);
    nanoseconds = hhmmss / 10000;
    nanoseconds *= 60;
    hhmmss %= 10000;
    nanoseconds += hhmmss / 100;
    nanoseconds *= 60;
    hhmmss %= 100;
    nanoseconds += hhmmss;
    nanoseconds *= 1000000000ULL;

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        numerator = hazer_parse_fraction(*endp + 1, &denominator, endp);
        numerator *= 1000000000ULL;
        numerator /= denominator;
        nanoseconds += numerator;
    }

    return nanoseconds;
}

uint64_t hazer_parse_dmy(const char * string, char ** endp)
{
    uint64_t nanoseconds = 0;
    unsigned long ddmmyy = 0;
    struct tm datetime = { 0, };
    extern long timezone;

    ddmmyy = strtoul(string, endp, 10);

    datetime.tm_year = ddmmyy % 100;
    if (datetime.tm_year < 93) {  datetime.tm_year += 100; }
    datetime.tm_mon = ((ddmmyy % 10000) / 100) - 1;
    datetime.tm_mday = ddmmyy / 10000;

    nanoseconds = mktime(&datetime); 
    nanoseconds -= timezone;
    nanoseconds *= 1000000000ULL;

    return nanoseconds;
}

int64_t hazer_parse_latlon(const char * string, char direction, uint8_t * digitsp, char ** endp)
{
    int64_t nanominutes = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    unsigned long dddmm = 0;
    uint8_t digits = 0;
    static char invalid = '?';

    digits = strlen(string);

    dddmm = strtoul(string, endp, 10);
    nanominutes = dddmm / 100;
    nanominutes *= 60000000000LL;
    fraction = dddmm % 100;
    fraction *= 1000000000LL;
    nanominutes += fraction;

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
        fraction *= 1000000000LL;
        fraction /= denominator;
        nanominutes += fraction;
        --digits;
    }

    if (**endp == '\0') {
        switch (direction) {
        case HAZER_STIMULUS_NORTH:
        case HAZER_STIMULUS_EAST:
            break;
        case HAZER_STIMULUS_SOUTH:
        case HAZER_STIMULUS_WEST:
            nanominutes = -nanominutes;
            break;
        default:
            /*
             * NOTE: not thread safe but we need to indicate an error.
             * In the event of a race condition, the error will still
             * be indicated, but the indicated character may not
             * be correct.
             */
            invalid = direction;
            *endp = &invalid;
            break;
        }
    }

    *digitsp = digits;

    return nanominutes;
}

int64_t hazer_parse_cog(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t nanodegrees = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    uint8_t digits = 0;

    digits = strlen(string);

    nanodegrees = strtol(string, endp, 10);
    nanodegrees *= 1000000000LL;

    if (nanodegrees < 0) {
        --digits;
    }

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
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

int64_t hazer_parse_sog(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t microknots = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    uint8_t digits = 0;

    digits = strlen(string);

    microknots = strtol(string, endp, 10);
    microknots *= 1000000LL;

    if (microknots < 0) {
        --digits;
    }

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
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

int64_t hazer_parse_smm(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t millimetersperhour = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    uint8_t digits = 0;

    digits = strlen(string);

    millimetersperhour = strtol(string, endp, 10);
    millimetersperhour *= 1000000LL;

    if (millimetersperhour < 0) {
        --digits;
    }

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
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

int64_t hazer_parse_alt(const char * string, char units, uint8_t * digitsp, char ** endp)
{
    int64_t millimeters = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    uint8_t digits = 0;

    digits = strlen(string);

    millimeters = strtol(string, endp, 10);
    millimeters *= 1000LL;

    if (millimeters < 0) {
        --digits;
    }

    if (**endp == HAZER_STIMULUS_DECIMAL) {
        fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
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

uint16_t hazer_parse_dop(const char * string, char ** endp)
{
    uint16_t dop = HAZER_GNSS_DOP;
    unsigned long number = 0;
    int64_t fraction = 0;
    uint64_t denominator = 0;

    if (*string != '\0') {

        number = strtoul(string, endp, 10);
        if (number <= (HAZER_GNSS_DOP / 100)) {

            number *= 100;

            if (**endp == HAZER_STIMULUS_DECIMAL) {

                fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
                fraction *= 100;
                fraction /= denominator;

                number += fraction;

            }

            dop = number;

        }

    } else  {

        /*
         * NOTE: dropping const qualifier. Dangerous, but consistent
         * with strtoul(3) et al. API which does the same.
         */
        *endp = (char *)string;

    }

    return dop;
}

/******************************************************************************
 *
 ******************************************************************************/

void hazer_format_nanoseconds2timestamp(uint64_t nanoseconds, int * yearp, int * monthp, int * dayp, int * hourp, int * minutep, int * secondp, uint64_t * nanosecondsp)
{
    struct tm datetime = { 0, };
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

/**
 * Return the name of a compass point given a bearing in billionths of a
 * degree, an array of compass point names, and the dimension of the array.
 * @param nanodegrees is the bearing.
 * @param compass is an array of compass point names.
 * @param count is the dimension of the array.
 * @return a compass point name.
 */
static inline const char * hazer_format_nanodegrees2compass(int64_t nanodegrees, const char * compass[], size_t count)
{
    size_t division = 0;
    size_t index = 0;

    while (nanodegrees < 0) { nanodegrees += 360000000000LL; }
    nanodegrees %= 360000000000LL;
    index = nanodegrees / 1000000LL;
    division = 360000 / count;
    index += division / 2;
    index %= 360000;
    index /= division;

    return compass[index];
}

const char * hazer_format_nanodegrees2compass32(int64_t nanodegrees)
{
    static const char * COMPASS[] = {
        "N", "NbE", "NNE", "NEbN", "NE", "NEbE", "ENE", "EbN",
        "E", "EbS", "ESE", "SEbE", "SE", "SEbS", "SSE", "SbE",
        "S", "SbW", "SSW", "SWbS", "SW", "SWbW", "WSW", "WbS",
        "W", "WbN", "WNW", "NWbW", "NW", "NWbN", "NNW", "NbW",
    };
    return hazer_format_nanodegrees2compass(nanodegrees, COMPASS, (sizeof(COMPASS) / sizeof(COMPASS[0])));
}

const char * hazer_format_nanodegrees2compass16(int64_t nanodegrees)
{
    static const char * COMPASS[] = {
        "N", "NNE", "NE", "ENE",
        "E", "ESE", "SE", "SSE", 
        "S", "SSW", "SW", "WSW",
        "W", "WNW", "NW", "NNW",
    };
    return hazer_format_nanodegrees2compass(nanodegrees, COMPASS, (sizeof(COMPASS) / sizeof(COMPASS[0])));
}

const char * hazer_format_nanodegrees2compass8(int64_t nanodegrees)
{
    static const char * COMPASS[] = {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW"
    };
    return hazer_format_nanodegrees2compass(nanodegrees, COMPASS, (sizeof(COMPASS) / sizeof(COMPASS[0])));
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
            rc = strncmp(id, name, strlen(name)); /* Compare just the prefix. */
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

/*
 * NMEA 0183 4.10 table 20 p. 94-95.
 */
hazer_system_t hazer_map_nmea_to_system(uint8_t constellation)
{
    hazer_system_t system = HAZER_SYSTEM_TOTAL;

    switch (constellation) {
    case HAZER_NMEA_GPS:
        system = HAZER_SYSTEM_GPS;
        break;
    case HAZER_NMEA_GLONASS:
        system = HAZER_SYSTEM_GLONASS;
        break;
    case HAZER_NMEA_GALILEO:
        system = HAZER_SYSTEM_GALILEO;
        break;
    case HAZER_NMEA_BEIDOU:
        system = HAZER_SYSTEM_BEIDOU;
        break;
    case HAZER_NMEA_SBAS:
        system = HAZER_SYSTEM_SBAS;
        break;
    case HAZER_NMEA_IMES:
        system = HAZER_SYSTEM_IMES;
        break;
    case HAZER_NMEA_QZSS:
        system = HAZER_SYSTEM_QZSS;
        break;
    default:
        break;
    }

    return system;
}

/*
 * NMEA 0183 4.10 p. 94.
 * UBLOX8 R15 p. 373.
 * UBLOX8 R19 Appendix A p. 402.
 */
hazer_system_t hazer_map_nmeaid_to_system(uint16_t id)
{
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;

    if (id == 0) {
        /* Do nothing. */
    } else if ((HAZER_NMEA_GPS_FIRST <= id) && (id <= HAZER_NMEA_GPS_LAST)) {
        candidate = HAZER_SYSTEM_GPS;
    } else if ((HAZER_NMEA_SBAS_FIRST <= id) && (id <= HAZER_NMEA_SBAS_LAST)) {
        candidate = HAZER_SYSTEM_SBAS;
    } else if ((HAZER_NMEA_GLONASS_FIRST <= id) && (id <= HAZER_NMEA_GLONASS_LAST)) {
        candidate = HAZER_SYSTEM_GLONASS;
    } else if ((HAZER_NMEA_BEIDOU1_FIRST <= id) && (id <= HAZER_NMEA_BEIDOU1_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else if ((HAZER_NMEA_IMES_FIRST <= id) && (id <= HAZER_NMEA_IMES_LAST)) {
        candidate = HAZER_SYSTEM_IMES;
    } else if ((HAZER_NMEA_QZSS_FIRST <= id) && (id <= HAZER_NMEA_QZSS_LAST)) {
        candidate = HAZER_SYSTEM_QZSS;
    } else if ((HAZER_NMEA_GALILEO_FIRST <= id) && (id <= HAZER_NMEA_GALILEO_LAST)) {
        candidate = HAZER_SYSTEM_GALILEO;
    } else if ((HAZER_NMEA_BEIDOU2_FIRST <= id) && (id <= HAZER_NMEA_BEIDOU2_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else {
        /* Do nothing. */
    }

    return candidate;
}

/*
 * UBLOX8 R24 p. 446.
 */
hazer_system_t hazer_map_pubxid_to_system(uint16_t id)
{
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;

    if (id == 0) {
        /* Do nothing. */
    } else if ((HAZER_PUBX_GPS_FIRST <= id) && (id <= HAZER_PUBX_GPS_LAST)) {
        candidate = HAZER_SYSTEM_GPS;
    } else if ((HAZER_PUBX_BEIDOU1_FIRST <= id) && (id <= HAZER_PUBX_BEIDOU1_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else if ((HAZER_PUBX_GLONASS1_FIRST <= id) && (id <= HAZER_PUBX_GLONASS1_LAST)) {
        candidate = HAZER_SYSTEM_GLONASS;
    } else if ((HAZER_PUBX_SBAS_FIRST <= id) && (id <= HAZER_PUBX_SBAS_LAST)) {
        candidate = HAZER_SYSTEM_SBAS;
    } else if ((HAZER_PUBX_GALILEO_FIRST <= id) && (id <= HAZER_PUBX_GALILEO_LAST)) {
        candidate = HAZER_SYSTEM_GALILEO;
    } else if ((HAZER_PUBX_BEIDOU2_FIRST <= id) && (id <= HAZER_PUBX_BEIDOU2_LAST)) {
        candidate = HAZER_SYSTEM_BEIDOU;
    } else if ((HAZER_PUBX_IMES_FIRST <= id) && (id <= HAZER_PUBX_IMES_LAST)) {
        candidate = HAZER_SYSTEM_IMES;
    } else if ((HAZER_PUBX_QZSS_FIRST <= id) && (id <= HAZER_PUBX_QZSS_LAST)) {
        candidate = HAZER_SYSTEM_QZSS;
    } else if ((HAZER_PUBX_GLONASS2_FIRST <= id) && (id <= HAZER_PUBX_GLONASS2_LAST)) {
        candidate = HAZER_SYSTEM_GLONASS;
    } else {
        /* Do nothing. */
    }

    return candidate;
}

/*
 * NMEA 0183 4.10 p. 94-95.
 * UBLOX8 R15 p. 373.
 */
hazer_system_t hazer_map_active_to_system(const hazer_active_t * activep)
{
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    hazer_system_t candidate = HAZER_SYSTEM_TOTAL;
    int slot = 0;
    static const int IDENTIFIERS = sizeof(activep->id) / sizeof(activep->id[0]);

    if ((HAZER_SYSTEM_GNSS <= activep->system) && (activep->system < HAZER_SYSTEM_TOTAL)) {
        system = (hazer_system_t)activep->system;
    } else {
        for (slot = 0; slot < IDENTIFIERS; ++slot) {
            if (slot >= activep->active) {
                break;
            } else if (activep->id[slot] == 0) {
                break;
            } else if ((candidate = hazer_map_nmeaid_to_system(activep->id[slot])) == HAZER_SYSTEM_TOTAL) {
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

static void update_time(hazer_position_t * positionp)
{
    uint64_t total = 0;

    if (positionp->dmy_nanoseconds != HAZER_NANOSECONDS_UNSET) {
        total = positionp->utc_nanoseconds + positionp->dmy_nanoseconds;
        if (positionp->tot_nanoseconds == HAZER_NANOSECONDS_UNSET) {
            positionp->old_nanoseconds = total;
            positionp->tot_nanoseconds = total;
        } else if (total != positionp->tot_nanoseconds) {
            positionp->old_nanoseconds = positionp->tot_nanoseconds;
            positionp->tot_nanoseconds = total;
        } else {
            /* Do nothing. */
        }
    }
}

/******************************************************************************
 *
 ******************************************************************************/

/*
 * Note that the count passed to the terminating parsers include the
 * terminating null pointer in the token vector. So a token vector
 * containing a single token would have a count of two: [ "token", NULL ].
 */

/*
 * I am frequently tempted to replace the numerical constants used as counts
 * and indices below with symbolic constants, but I find they actually make
 * the code a lot harder to read, to debug, and to compare against the NMEA
 * spec.
 */

/*
 * Do not be tempted to do a struture assignment (copy) in the code below.
 * That would update ALL of the fields, and that is not necessarily what is
 * intended.
 */

/*
 * The most recent copy of the NMEA 0183 standard I have is 4.10. There is a
 * 4.11 version now, but NMEA wants an (IMO) astronomical $2000 for a copy of
 * it. It's time for GPS receiver manufacturers to form an industry association
 * to fork the NMEA standard and produces a non-proprietary (or at least
 * reasonably priced) standard. Or maybe ISO to do so (although that wouldn't
 * be cheap either.)
 */

int hazer_parse_gga(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char GGA[] = HAZER_NMEA_SENTENCE_GGA;

    do {

        /*
         * IDENTIFY
         */
    
        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GGA) != 0) {
            break;
        }

        /*
         * VALIDATE
         */
    
        if (count < 16) {
            enodata = vector[0];
            break;
        }

        position.sat_used = strtol(vector[7], &end, 10);
        if (*end != '\0') {
            einval = vector[7];
            break;
        }

        if (position.sat_used == 0) {
            positionp->sat_used = 0;
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            einval = vector[1];
            break;
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[2], *(vector[3]), &position.lat_digits, &end);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[4], *(vector[5]), &position.lon_digits, &end);
        if (*end != '\0') {
            einval = vector[4];
            break;
        }

        position.alt_millimeters = hazer_parse_alt(vector[9], *(vector[10]), &position.alt_digits, &end);
        if (*end != '\0') {
            einval = vector[9];
            break;
        }

        position.sep_millimeters = hazer_parse_alt(vector[11], *(vector[12]), &position.sep_digits, &end);
        if (*end != '\0') {
            einval = vector[11];
            break;
        }

        /*
         * APPLY
         */

        positionp->utc_nanoseconds = position.utc_nanoseconds;
        update_time(positionp);

        positionp->lat_nanominutes = position.lat_nanominutes;
        positionp->lat_digits = position.lat_digits;

        positionp->lon_nanominutes = position.lon_nanominutes;
        positionp->lon_digits = position.lon_digits;

        positionp->sat_used = position.sat_used;

        positionp->alt_millimeters = position.alt_millimeters;
        positionp->alt_digits = position.alt_digits;

        positionp->sep_millimeters = position.sep_millimeters;
        positionp->sep_digits = position.sep_digits;

        positionp->label = GGA;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_gsa(hazer_active_t * activep, char * vector[], size_t count)
{
    int rc = -1;
    int index = 3;
    int slot = 0;
    int id = 0;
    int satellites = 0;
    int system = 0;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
    static const int IDENTIFIERS = sizeof(activep->id) / sizeof(activep->id[0]);
    static const char GSA[] = HAZER_NMEA_SENTENCE_GSA;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGSA")) != (sizeof("$XXGSA") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GSA) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 19) {
            enodata = vector[0];
            break;
        }

        active.mode = strtoul(vector[2], &end, 10);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }
        if (!((HAZER_MODE <= active.mode) && (active.mode < HAZER_MODE_TOTAL))) {
            active.mode = HAZER_MODE_TOTAL;
        }

        for (slot = 0; slot < IDENTIFIERS; ++slot) {
            id = strtol(vector[index], &end, 10);
            if (*end != '\0') {
                einval = vector[index];
                break;
            }
            if (id <= 0) {
                break;
            }
            active.id[slot] = id;
            ++satellites;
            ++index;
        }
        if (*end != '\0') {
            break;
        }

        /*
         * Unlike the GSV sentence, the GSA sentence isn't variable length.
         * Unused slots in the active list are denoted by empty fields.
         */

        active.active = satellites;

        active.pdop = hazer_parse_dop(vector[15], &end);
        if (*end != '\0') {
            einval = vector[15];
            break;
        }

        active.hdop = hazer_parse_dop(vector[16], &end);
        if (*end != '\0') {
            einval = vector[16];
            break;
        }

        active.vdop = hazer_parse_dop(vector[17], &end);
        if (*end != '\0') {
            einval = vector[17];
            break;
        }

        active.tdop = HAZER_GNSS_DOP;

        /*
         * NMEA 0183 4.10 2012 has an additional 19th field containing
         * the GNSS System ID to identify GPS, GLONASS, GALILEO, etc.
         */

        if (count > 19) {

            system = strtol(vector[18], &end, 10);
            if (*end != '\0') {
                einval = vector[18];
                break;
            }

            active.system = hazer_map_nmea_to_system(system);

        } else {

            active.system = HAZER_SYSTEM_TOTAL;

        }

        /*
         * APPLY
         */

        activep->mode = active.mode;

        for (index = 0; index < slot; ++index) {
            activep->id[index] = active.id[index];
        }

        activep->active = active.active;

        activep->pdop = active.pdop;
        activep->hdop = active.hdop;
        activep->vdop = active.vdop;
        activep->tdop = active.tdop;

        activep->system = active.system;

        activep->label = GSA;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_gsv(hazer_view_t * viewp, char * vector[], size_t count)
{
    int rc = -1;
    int messages = 0;
    int message = 0;
    int start = 0;
    int index = 4;
    int slot = 0;
    int sequence = 0;
    int channel = 0;
    int first = 0;
    int past = 0;
    int satellites = 0;
    int signal = 0;
    unsigned int id = 0;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_view_t view = HAZER_VIEW_INITIALIZER;
    static const int SATELLITES = sizeof(viewp->sat) / sizeof(viewp->sat[0]);
    static const char GSV[] = HAZER_NMEA_SENTENCE_GSV;

    do {

        /*
         * IDENTIFY
         */
    
        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGSV")) != (sizeof("$XXGSV") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GSV) != 0) {
            break;
        }

        /*
         * VALIDATE
         */
    
        if (count < 5) {
            enodata = vector[0];
            break;
        }

        messages = strtol(vector[1], &end, 10);
        if (*end != '\0') {
            einval = vector[1];
            break;
        }

        message = strtol(vector[2], &end, 10);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }

        if (message <= 0) {
            break;
        }

        if (message > messages) {
            break;
        }

        sequence = message - 1;
        channel = sequence * HAZER_GNSS_VIEWS;
        first = channel;
        past = channel;
        satellites = strtol(vector[3], &end, 10);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        /*
         * "Null fields are not required for unused sets when less
         * than four sets are transmitted." [NMEA 0183 v4.10 2012 p. 96]
         * Unlike the GSA sentence, the GSV sentence can have a variable
         * number of fields. So from here on all indices are effectively
         * relative.
         */

        for (slot = 0; slot < HAZER_GNSS_VIEWS; ++slot) {

            if (channel >= satellites) {
                break;
            }

            if (channel >= SATELLITES) {
                break;
            }

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

            if ((index + 4) >= count) {
                break;
            }

            id = strtol(vector[index], &end, 10);
            if (*end != '\0') {
                einval = vector[index];
                break;
            }

            ++index;

            if (id <= 0) {
                break;
            }
            view.sat[channel].id = id;

            /*
             * "For efficiency it is recommended that null fields be used
             * in the additional sentences when the data is unchanged from
             * the first sentence." [NMEA 0183 v4.10 2012 p. 96]
             * Does this mean that the same satellite ID can appear more
             * than once in the same tuple of GSV sentences? Or does this
             * just apply to the (newish) signal ID in the last field?
             */

            view.sat[channel].phantom = 0;

            if (strlen(vector[index]) == 0) {
                view.sat[channel].phantom = !0;
                view.sat[channel].elv_degrees = 0;
            } else {
                view.sat[channel].elv_degrees = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    einval = vector[index];
                    break;
                }
            }

            ++index;

            if (strlen(vector[index]) == 0) {
                view.sat[channel].phantom = !0;
                view.sat[channel].azm_degrees = 0;
            } else {
                view.sat[channel].azm_degrees = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    einval = vector[index];
                    break;
                }
            }

            ++index;

            if (strlen(vector[index]) == 0) {
                view.sat[channel].untracked = !0;
                view.sat[channel].snr_dbhz = 0;
            } else {
                view.sat[channel].untracked = 0;
                view.sat[channel].snr_dbhz = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    einval = vector[index];
                    break;
                }
            }

            ++index;

            past = ++channel;

        }
        if (*end != '\0') {
            break;
        }

        view.channels = channel;
        view.view = satellites;
        view.pending = messages - message;
        view.label = GSV;

        /*
         * NMEA 0183 4.10 2012 has an additional field containing the
         * signal identifier. This is constellation specific, but
         * indicates what frequency band was used, e.g. for GPS: L1C/A,
         * L2, etc. It complicates things by applying to all of the
         * satellites in this particular message, but we don't know
         * if the field exists until we have processed all the
         * satellites in this message. If it is present, it applies
         * to all the satellites in this message.
         */

        if (index > (count - 2)) {
            signal = 0;
        } else if (strlen(vector[count - 2]) > 0) {
            signal = strtol(vector[count - 2], &end, 10);
            if (*end != '\0') {
                einval = vector[count - 2];
                break;
            }
        } else if (first > 0) {
            /*
             * We have to reference the actual structure, not the
             * working structure.
             */
            signal = viewp->sat[first - 1].signal;
        } else {
            signal = 0;
        }

        for (channel = first; channel < past; ++channel) {
            if (channel >= satellites) {
                break;
            }
            if (channel >= SATELLITES) {
                break;
            }
            view.sat[channel].signal = signal;
        }

        /*
         * APPLY
         */

        for (channel = first; channel < past; ++channel) {
            if (channel >= satellites) {
                break;
            }
            if (channel >= SATELLITES) {
                break;
            }
            viewp->sat[channel].id = view.sat[channel].id;
            viewp->sat[channel].phantom = view.sat[channel].phantom;
            viewp->sat[channel].elv_degrees = view.sat[channel].elv_degrees;
            viewp->sat[channel].azm_degrees = view.sat[channel].azm_degrees;
            viewp->sat[channel].untracked = view.sat[channel].untracked;
            viewp->sat[channel].snr_dbhz = view.sat[channel].snr_dbhz;
            viewp->sat[channel].signal = view.sat[channel].signal;
        }

        viewp->channels = view.channels;
        viewp->view = view.view;
        viewp->pending = view.pending;
        viewp->label = GSV;

        /*
         * Only if this is the last message in the GSV tuple do we
         * emit a zero return code. That lets the application decide
         * when it wants to peruse its view database. (We could have
         * treated this expression like a boolean, but the return code
         * is genuinely tri-state).
         */

        rc = (viewp->pending > 0) ? 1 : 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_rmc(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char RMC[] = HAZER_NMEA_SENTENCE_RMC;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXRMC")) != (sizeof("$XXRMC") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, RMC) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 13) {
            enodata = vector[0];
            break;
        }

        if (strcmp(vector[2], "A") != 0) {
            break;
        }

#if 0
        if ((count > 13) && (strcmp(vector[12], "N") == 0)) { /* NMEA 2.30+ */
            break;
        }
#endif

#if 0
        if ((count > 14) && (strcmp(vector[13], "V") == 0)) { /* NMEA 4.10+ */
            /* Not clear what this means on the u-blox UBX-F9P. */
            break;
        }
#endif

        position.utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            einval = vector[1];
            break;
        }
        position.lat_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lat_digits, &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[5], *(vector[6]), &position.lon_digits, &end);
        if (*end != '\0') {
            einval = vector[5];
            break;
        }

        position.sog_microknots = hazer_parse_sog(vector[7], &position.sog_digits, &end);
        if (*end != '\0') {
            einval = vector[7];
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[8], &position.cog_digits, &end);
        if (*end != '\0') {
            einval = vector[8];
            break;
        }

        position.dmy_nanoseconds = hazer_parse_dmy(vector[9], &end);
        if (*end != '\0') {
            einval = vector[9];
            break;
        }

        /*
         * APPLY
         */

        positionp->utc_nanoseconds = position.utc_nanoseconds;

        positionp->lat_nanominutes = position.lat_nanominutes;
        positionp->lat_digits = position.lat_digits;

        positionp->lon_nanominutes = position.lon_nanominutes;
        positionp->lon_digits = position.lon_digits;

        positionp->sog_microknots = position.sog_microknots;
        positionp->sog_digits = position.sog_digits;

        positionp->cog_nanodegrees = position.cog_nanodegrees;
        positionp->cog_digits = position.cog_digits;

        positionp->dmy_nanoseconds = position.dmy_nanoseconds;
        update_time(positionp);

        positionp->label = RMC;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_gll(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char GLL[] = HAZER_NMEA_SENTENCE_GLL;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GLL) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 8) {
            enodata = vector[0];
            break;
        }

        if (strcmp(vector[6], "A") != 0) {
            break;
        }

#if 0
        if ((count >= 9) && (strcmp(vector[7], "N") == 0)) {
            break;
        }
#endif

        position.lat_nanominutes = hazer_parse_latlon(vector[1], *(vector[2]), &position.lat_digits, &end);
        if (*end != '\0') {
            einval = vector[1];
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lon_digits, &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[5], &end);
        if (*end != '\0') {
            einval = vector[5];
            break;
        }

        /*
         * APPLY
         */

        positionp->lat_nanominutes = position.lat_nanominutes;
        positionp->lat_digits = position.lat_digits;

        positionp->lon_nanominutes = position.lon_nanominutes;
        positionp->lon_digits = position.lon_digits;

        positionp->utc_nanoseconds = position.utc_nanoseconds;
        update_time(positionp);

        positionp->label = GLL;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_vtg(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char VTG[] = HAZER_NMEA_SENTENCE_VTG;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            break;
        }

        if (strnlen(vector[0], sizeof("$XXVTG")) != (sizeof("$XXVTG") - 1)) {
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, VTG) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 10) {
            enodata = vector[0];
            break;
        }

        if (strcmp(vector[9], "N") == 0) {
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[1], &position.cog_digits, &end);
        if (*end != '\0') {
            einval = vector[1];
            break;
        }

        position.mag_nanodegrees = hazer_parse_cog(vector[3], &position.mag_digits, &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        position.sog_microknots = hazer_parse_sog(vector[5], &position.sog_digits, &end);
        if (*end != '\0') {
            einval = vector[5];
            break;
        }

        position.sog_millimetersperhour = hazer_parse_smm(vector[7], &position.smm_digits, &end);
        if (*end != '\0') {
            einval = vector[7];
            break;
        }

        /*
         * APPLY
         */

        positionp->cog_nanodegrees = position.cog_nanodegrees;
        positionp->cog_digits = position.cog_digits;

        positionp->mag_nanodegrees = position.mag_nanodegrees;
        positionp->mag_digits = position.mag_digits;

        positionp->sog_microknots = position.sog_microknots;
        positionp->sog_digits = position.sog_digits;

        positionp->sog_millimetersperhour = position.sog_millimetersperhour;
        positionp->smm_digits = position.smm_digits;

        positionp->label = VTG;

        rc = 0;

   } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_txt(char * vector[], size_t count)
{
    int rc = -1;
    static const char TXT[] = HAZER_NMEA_SENTENCE_TXT;

    if (count < 2) {
        /* Do nothing. */
    } else if (strnlen(vector[0], sizeof("$XXTXT")) != (sizeof("$XXTXT") - 1)) {
        /* Do nothing. */
    } else if (*vector[0] != HAZER_STIMULUS_START) {
        /* Do nothing. */
    } else if (strcmp(vector[0] + sizeof("$XX") - 1, TXT) != 0) {
        /* Do nothing. */
    } else {
        rc = 0;
    }

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_pubx_position(hazer_position_t * positionp, hazer_active_t * activep, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_POSITION;

    do {

        /*
         * IDENTIFY
         */

        if (count < 3) {
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 22) {
            enodata = vector[1];
            break;
        }

        if (strcmp(vector[8], "NF") == 0) {
            activep->mode = HAZER_MODE_NOFIX;
            break;
        }

        if (strcmp(vector[18], "0") == 0) {
            activep->mode = HAZER_MODE_ZERO;
            break;
        }

        if (strcmp(vector[8], "TT") == 0) {

            position.utc_nanoseconds = hazer_parse_utc(vector[2], &end);
            if (*end != '\0') {
                einval = vector[2];
                break;
            }

            position.sat_used = strtol(vector[18], &end, 10);
            if (*end != '\0') {
                einval = vector[18];
                break;
            }

            active.mode = HAZER_MODE_TIME;

            active.hdop = hazer_parse_dop(vector[15], &end);
            if (*end != '\0') {
                einval = vector[15];
                break;
            }

            active.vdop = hazer_parse_dop(vector[16], &end);
            if (*end != '\0') {
                einval = vector[16];
                break;
            }

            active.tdop = hazer_parse_dop(vector[17], &end);
            if (*end != '\0') {
                einval = vector[17];
                break;
            }

            /*
             * APPLY
             */

            positionp->utc_nanoseconds = position.utc_nanoseconds;
            update_time(positionp);

            positionp->sat_used = position.sat_used;

            positionp->label = PUBX;

            activep->mode = active.mode;

            activep->hdop = active.hdop;
            activep->vdop = active.vdop;
            activep->tdop = active.tdop;

            activep->label = PUBX;

            rc = 0;

            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[2], &end);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lat_digits, &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[5], *(vector[6]), &position.lon_digits, &end);
        if (*end != '\0') {
            einval = vector[5];
            break;
        }

        position.sep_millimeters = hazer_parse_alt(vector[7], *(vector[12]), &position.sep_digits, &end);
        if (*end != '\0') {
            einval = vector[7];
            break;
        }

        if (strcmp(vector[8], "DR") == 0) {
            active.mode = HAZER_MODE_IMU;
        } else if (strcmp(vector[8], "G2") == 0) {
            active.mode = HAZER_MODE_2D;
        } else if (strcmp(vector[8], "G3") == 0) {
            active.mode = HAZER_MODE_3D;
        } else if (strcmp(vector[8], "RK") == 0) {
            active.mode = HAZER_MODE_COMBINED;
        } else if (strcmp(vector[8], "D2") == 0) {
            active.mode = HAZER_MODE_DGNSS2D;
        } else if (strcmp(vector[8], "D3") == 0) {
            active.mode = HAZER_MODE_DGNSS3D;
        } else {
            active.mode = HAZER_MODE_TOTAL;
        }

        position.sog_millimetersperhour = hazer_parse_smm(vector[11], &position.sog_digits, &end);
        if (*end != '\0') {
            einval = vector[11];
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[12], &position.cog_digits, &end);
        if (*end != '\0') {
            einval = vector[12];
            break;
        }

        position.sat_used = strtol(vector[18], &end, 10);
        if (*end != '\0') {
            einval = vector[18];
            break;
        }

        active.hdop = hazer_parse_dop(vector[15], &end);
        if (*end != '\0') {
            einval = vector[15];
            break;
        }

        active.vdop = hazer_parse_dop(vector[16], &end);
        if (*end != '\0') {
            einval = vector[16];
            break;
        }

        active.tdop = hazer_parse_dop(vector[17], &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        /*
         * APPLY
         */

        positionp->utc_nanoseconds = position.utc_nanoseconds;
        update_time(positionp);

        positionp->lat_nanominutes = position.lat_nanominutes;
        positionp->lat_digits = position.lat_digits;

        positionp->lon_nanominutes = position.lon_nanominutes;
        positionp->lon_digits = position.lon_digits;

        positionp->sep_millimeters = position.sep_millimeters;
        positionp->sep_digits = position.sep_digits;

        positionp->sog_millimetersperhour = position.sog_millimetersperhour;
        positionp->sog_digits = position.sog_digits;

        positionp->cog_nanodegrees = position.cog_nanodegrees;
        positionp->cog_digits = position.cog_digits;

        positionp->sat_used = position.sat_used;

        positionp->label = PUBX;

        activep->mode = active.mode;

        activep->hdop = active.hdop;
        activep->vdop = active.vdop;
        activep->tdop = active.tdop;

        activep->label = PUBX;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_pubx_svstatus(hazer_view_t viewa[], hazer_active_t activea[], char * vector[], size_t count)
{
    int rc = -1;
    int result = 0;
    int satellites = 0;
    int satellite = 0;
    int channel = 0;
    int channels[HAZER_SYSTEM_TOTAL] = { 0, };
    int ranger = 0;
    int rangers[HAZER_SYSTEM_TOTAL] = { 0, };
    int index = 3;
    int id = 0;
    int system = 0;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_view_t views[HAZER_SYSTEM_TOTAL] = HAZER_VIEWS_INITIALIZER;
    hazer_active_t actives[HAZER_SYSTEM_TOTAL] = HAZER_ACTIVES_INITIALIZER;
    static const int SATELLITES = sizeof(views[0].sat) / sizeof(views[0].sat[0]);
    static const int RANGERS = sizeof(actives[0].id) / sizeof(actives[0].id[0]);
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_SVSTATUS;

    do {

        /* IDENTIFY */

        if (count < 4) {
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        satellites = strtol(vector[2], &end, 10);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }

        if (count < (4 + (satellites * 6))) {
            enodata = vector[1];
            break;
        }

        for (satellite = 0; satellite < satellites; ++satellite) {

            id = strtol(vector[index + 0], &end, 10);
            if (*end != '\0') {
                einval = vector[2];
                break;
            }

            system = hazer_map_pubxid_to_system(id);
            if (system >= HAZER_SYSTEM_TOTAL) {
                system = HAZER_SYSTEM_GNSS;
            }

            channel = channels[system];
            if (channel >= SATELLITES) {
                continue;
            }

            views[system].sat[channel].id = id;
            views[system].sat[channel].phantom = 0;
            views[system].sat[channel].untracked = 0;
            views[system].sat[channel].unused = 0;

            if (strcmp(vector[index + 1], "e") == 0) {
                /* Do nothing. */
            } else if (strcmp(vector[index + 1], "U") == 0) {
                ranger = rangers[system];
                if (ranger < RANGERS) {
                    actives[system].id[ranger] = id;
                    ranger += 1;
                    rangers[system] = ranger;
                    actives[system].active = ranger;
                }
                actives[system].system = system;
                actives[system].label = PUBX;
            } else if (strcmp(vector[index + 1], "-") == 0) {
                views[system].sat[channel].unused = !0;
            } else {
                /* Should never happen, and not clear what it means if it does. */
                views[system].sat[channel].phantom = !0;
                views[system].sat[channel].untracked = !0;
                views[system].sat[channel].unused = !0;
            }

            if (strlen(vector[index + 2]) == 0) {
                views[system].sat[channel].phantom = !0;
                views[system].sat[channel].azm_degrees = 0;
            } else {
                views[system].sat[channel].azm_degrees = strtol(vector[index + 2], &end, 10);
                if (*end != '\0') {
                    einval = vector[index + 2];
                    break;
                }
            }

            if (strlen(vector[index + 3]) == 0) {
                views[system].sat[channel].phantom = !0;
                views[system].sat[channel].elv_degrees = 0;
            } else {
                views[system].sat[channel].elv_degrees = strtol(vector[index + 3], &end, 10);
                if (*end != '\0') {
                    einval = vector[index + 3];
                    break;
                }
            }

            if (strlen(vector[index + 4]) == 0) {
                views[system].sat[channel].untracked = !0;
                views[system].sat[channel].snr_dbhz = 0;
            } else {
                views[system].sat[channel].snr_dbhz = strtol(vector[index + 4], &end, 10);
                if (*end != '\0') {
                    einval = vector[index + 4];
                    break;
                }
            }

            views[system].sat[channel].signal = 0;

            channel += 1;

            channels[system] = channel;

            views[system].channels = channel;
            views[system].view = satellites;

            result |= (1 << system);

            index += 6;

        }
        if (*end != '\0') {
            break;
        }

        /*
         * APPLY
         */

        for (system = HAZER_SYSTEM_GNSS; system < HAZER_SYSTEM_TOTAL; ++system) {
            if ((result & (1 << system)) != 0) {

                viewa[system].label = PUBX;

                for (channel = 0; channel < views[system].channels; ++channel) {
                    viewa[system].sat[channel] = views[system].sat[channel]; /* Structure copy. */
                }

                viewa[system].view = views[system].view;
                viewa[system].channels = views[system].channels;
                viewa[system].pending = 0;

                activea[system].label = PUBX;

                for (ranger = 0; ranger < actives[system].active; ++ranger) {
                    activea[system].id[ranger] = actives[system].id[ranger];
                }

                activea[system].system = actives[system].system;
                activea[system].active = actives[system].active;
                activea[system].mode = actives[system].mode;

            }
        }

        rc = result;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

int hazer_parse_pubx_time(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    char * enodata = (char *)0;
    char * einval = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_TIME;

    do {

        /*
         * IDENTIFY
         */

        if (count < 3) {
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 11) {
            enodata = vector[1];
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[2], &end);
        if (*end != '\0') {
            einval = vector[2];
            break;
        }

        position.dmy_nanoseconds = hazer_parse_dmy(vector[3], &end);
        if (*end != '\0') {
            einval = vector[3];
            break;
        }

        /*
         * APPLY
         */

        positionp->utc_nanoseconds = position.utc_nanoseconds;
        positionp->dmy_nanoseconds = position.dmy_nanoseconds;
        update_time(positionp);

        positionp->label = PUBX;

        rc = 0;

    } while (0);

    if (enodata != (char *)0) {
        errno = ENODATA;
        perror(enodata);
    }

    if (einval != (char *)0) {
        errno = EINVAL;
        perror(einval);
    }

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_has_pending_gsv(const hazer_view_t va[], size_t count)
{
    size_t ii = 0;

    for (ii = 0; ii < count; ++ii) {
        if (va[ii].ticks == 0) {
            /* Do nothing. */
        } else if (va[ii].pending == 0) {
            /* Do nothing. */
        } else {
            return !0;
        }
    }

    return 0;
}

int hazer_has_valid_time(const hazer_position_t pa[], size_t count)
{
    size_t ii = 0;

    for (ii = 0; ii < count; ++ii) {
        if (hazer_is_valid_time(&(pa[ii]))) {
            return !0;
        }
    }

    return 0;
}
