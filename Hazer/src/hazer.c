/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2023 Digital Aggregates Corporation, Colorado, USA.
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

const char HAZER_QUALITY_NAME[] = HAZER_QUALITY_NAME_INITIALIZER;

const char HAZER_SAFETY_NAME[] = HAZER_SAFETY_NAME_INITIALIZER;

const char * HAZER_SIGNAL_NAME[HAZER_SYSTEM_TOTAL][HAZER_GNSS_SIGNALS] = HAZER_SIGNAL_NAME_INITIALIZER;

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
            /* Not an error since could be UBX or RTCM data. */
            state = HAZER_STATE_STOP;
            action = HAZER_ACTION_TERMINATE;
        }
        break;

    case HAZER_STATE_MSN:
        if (ch == pp->msn) {
            state = HAZER_STATE_LSN;
            action = HAZER_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = HAZER_STATE_STOP;
            action = HAZER_ACTION_TERMINATE;
        }
        break;

    case HAZER_STATE_LSN:
        if (ch == pp->lsn) {
            state = HAZER_STATE_CR;
            action = HAZER_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = HAZER_STATE_STOP;
            action = HAZER_ACTION_TERMINATE;
        }
        break;

    case HAZER_STATE_CR:
        if (ch == HAZER_STIMULUS_CR) {
            state = HAZER_STATE_LF;
            action = HAZER_ACTION_SAVE;
        } else {
            pp->error = !0;
            state = HAZER_STATE_STOP;
            action = HAZER_ACTION_TERMINATE;
        }
        break;

    case HAZER_STATE_LF:
        if (ch == HAZER_STIMULUS_LF) {
            state = HAZER_STATE_END;
            action = HAZER_ACTION_TERMINATE;
        } else {
            pp->error = !0;
            state = HAZER_STATE_STOP;
            action = HAZER_ACTION_TERMINATE;
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
        fprintf(debug, "Machine NMEA %c %c %c 0x%02x%02x '\\x%02x' '%c'\n", old, state, action, pp->msn, pp->lsn, ch, ch);
    } else {
        fprintf(debug, "Machine NMEA %c %c %c 0x%02x%02x '\\x%02x'\n", old, state, action, pp->msn, pp->lsn, ch);
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

/**
 * Fixup the value if it has a negative sign. This happens because when
 * strtol(3) sees "-0.123" it ignores the minus sign and returns zero for
 * the non-fractional part.
 * @param value is the numeric value.
 * @param sign is its leading sign if any.
 * @param the new value possibly unchagened.
 */
static inline int64_t fixup(int64_t value, char sign)
{
    if (value <= 0) {
        /* Do nothing. */
    } else if (sign != HAZER_STIMULUS_NEGATIVE) {
        /* Do nothing. */
    } else {
        value = -value;
    }

    return value;
}

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
    uint64_t field = 0;
    uint64_t nanoseconds = 0;
    uint64_t numerator = 0;
    uint64_t denominator = 1;
    unsigned long hhmmss = 0;

    do {

        hhmmss = strtoul(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        field = hhmmss / 10000;
        if(!((0 <= field) && (field < 24))) {
            *endp = (char *)string; /* Questionable. */
            break;
        }
        hhmmss %= 10000;

        nanoseconds = field;
        nanoseconds *= 60;

        field = hhmmss / 100;
        if(!((0 <= field) && (field < 60))) {
            *endp = (char *)string; /* Dangerous. */
            break;
        }
        hhmmss %= 100;

        nanoseconds += field;
        nanoseconds *= 60;

        field = hhmmss;
        if(!((0 <= field) && (field < 60))) {
            *endp = (char *)string; /* Really? */
            break;
        }

        nanoseconds += field;
        nanoseconds *= 1000000000ULL;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            numerator = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (!(**endp == '\0')) {
                break;
            }

            numerator *= 1000000000ULL;
            numerator /= denominator;
            nanoseconds += numerator;

        }

    } while (0);

    return nanoseconds;
}

uint64_t hazer_parse_dmy(const char * string, char ** endp)
{
    uint64_t nanoseconds = 0;
    time_t seconds = 0;
    unsigned long ddmmyy = 0;
    struct tm datetime = { 0, };
    extern long timezone;

    do {

        ddmmyy = strtoul(string, endp, 10);
        if (**endp != '\0') {
            break;
        }

        /*
         * We let mktime check the fields since it is
         * more complicated with a variable number of
         * days per months and depending on the year.
         */

        datetime.tm_year = ddmmyy % 100;
        if (datetime.tm_year < 93) {  datetime.tm_year += 100; }
        datetime.tm_mon = ((ddmmyy % 10000) / 100) - 1;
        datetime.tm_mday = ddmmyy / 10000;

        seconds = mktime(&datetime); 
        if (seconds == (time_t)-1) {
            *endp = (char *)string; /* No, I really mean it. */
            break;
        }

        nanoseconds = seconds;
        nanoseconds -= timezone;
        nanoseconds *= 1000000000ULL;

    } while (0);

    return nanoseconds;
}

uint64_t hazer_parse_d_m_y(const char * stringd, const char * stringm, const char * stringy, char ** endp)
{
    uint64_t nanoseconds = 0;
    time_t seconds = 0;
    unsigned int dd = 0;
    unsigned int mm = 0;
    unsigned int yyyy = 0;
    struct tm datetime = { 0, };
    extern long timezone;

    do {

        dd = strtoul(stringd, endp, 10);
        if (**endp != '\0') {
            break;
        }

        mm = strtoul(stringm, endp, 10);
        if (**endp != '\0') {
            break;
        }

        yyyy = strtoul(stringy, endp, 10);
        if (**endp != '\0') {
            break;
        }

        /*
         * We let mktime check the fields since it is
         * more complicated with a variable number of
         * days per months and depending on the year.
         */

        datetime.tm_mday = dd;
        datetime.tm_mon = mm - 1;
        datetime.tm_year = yyyy - 1900;

        seconds = mktime(&datetime); 
        if (seconds == (time_t)-1) {
            *endp = (char *)stringy; /* No, I really mean it. */
            break;
        }

        nanoseconds = seconds;
        nanoseconds -= timezone;
        nanoseconds *= 1000000000ULL;

    } while (0);

    return nanoseconds;
}

int64_t hazer_parse_latlon(const char * string, char direction, uint8_t * digitsp, char ** endp)
{
    int64_t nanominutes = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    unsigned long dddmm = 0;
    size_t digits = 0;

    do {

        digits = strlen(string);
        if (digits == 0) {
            *digitsp = 0;
            *endp = (char *)string; /* Again? */
            break;
        }

        dddmm = strtoul(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        nanominutes = dddmm / 100;

        nanominutes *= 60000000000LL;
        fraction = dddmm % 100;
        fraction *= 1000000000LL;
        nanominutes += fraction;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

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
                *endp = (char *)string; /* Oh, fer pete's sake! */
                break;
            }
        }
        if (**endp != '\0') {
            break;
        }

        *digitsp = digits;

    } while (0);

    return nanominutes;
}

int64_t hazer_parse_cog(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t nanodegrees = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    size_t digits = 0;

    do {

        digits = strlen(string);
        if (digits == 0) {
            *digitsp = 0;
            *endp = (char *)string; /* Here we are again. */
            break;
        }

        nanodegrees = strtol(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        nanodegrees *= 1000000000LL;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

            fraction *= 1000000000LL;
            fraction /= denominator;
            if (nanodegrees < 0) {
                nanodegrees -= fraction;
            } else {
                nanodegrees += fraction;
            }

            --digits;
        }

        nanodegrees = fixup(nanodegrees, string[0]);

        if (nanodegrees < 0) {
            --digits;
        }

        *digitsp = digits;

    } while (0);

    return nanodegrees;
}

int64_t hazer_parse_sog(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t microknots = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    size_t digits = 0;

    do {

        digits = strlen(string);
        if (digits == 0) {
            *digitsp = 0;
            *endp = (char *)string; /* Have we learning nothing? */
            break;
        }

        microknots = strtol(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        microknots *= 1000000LL;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

            fraction *= 1000000;
            fraction /= denominator;
            if (microknots < 0) {
                microknots -= fraction;
            } else {
                microknots += fraction;
            }

            --digits;

        }

        microknots = fixup(microknots, string[0]);

        if (microknots < 0) {
            --digits;
        }

        *digitsp = digits;

    } while (0);

    return microknots;
}

int64_t hazer_parse_smm(const char * string, uint8_t * digitsp, char ** endp)
{
    int64_t millimetersperhour = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    size_t digits = 0;

    do {

        digits = strlen(string);
        if (digits == 0) {
            *digitsp = 0;
            *endp = (char *)string; /* Apparently not. */
            break;
        }

        millimetersperhour = strtol(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        millimetersperhour *= 1000000LL;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

            fraction *= 1000000;
            fraction /= denominator;
            if (millimetersperhour < 0) {
                millimetersperhour -= fraction;
            } else {
                millimetersperhour += fraction;
            }

            --digits;
        }

        millimetersperhour = fixup(millimetersperhour, string[0]);

        if (millimetersperhour < 0) {
            --digits;
        }

        *digitsp = digits;

    } while (0);

    return millimetersperhour;
}

int64_t hazer_parse_alt(const char * string, char units, uint8_t * digitsp, char ** endp)
{
    int64_t millimeters = 0;
    int64_t fraction = 0;
    uint64_t denominator = 1;
    size_t digits = 0;

    do {

        digits = strlen(string);
        if (digits == 0) {
            *digitsp = 0;
            *endp = (char *)string; /* Don't make me come back there! */
            break;
        }

        millimeters = strtol(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        millimeters *= 1000LL;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

            fraction *= 1000;
            fraction /= denominator;
            if (millimeters < 0) {
                millimeters -= fraction;
            } else {
                millimeters += fraction;
            }

            --digits;

        }

        millimeters = fixup(millimeters, string[0]);

        if (millimeters < 0) {
            --digits;
        }

        *digitsp = digits;

    } while (0);

    return millimeters;
}

uint16_t hazer_parse_dop(const char * string, char ** endp)
{
    uint16_t dop = HAZER_GNSS_DOP;
    unsigned long number = 0;
    int64_t fraction = 0;
    uint64_t denominator = 0;

    do {

        if (*string == '\0') {
            *endp = (char *)string; /* The strtoul(3) API does this too! */
            break;
        }

        number = strtoul(string, endp, 10);
        if (!((**endp == '\0') || (**endp == HAZER_STIMULUS_DECIMAL))) {
            break;
        }

        number *= 100;

        if (**endp == HAZER_STIMULUS_DECIMAL) {

            fraction = hazer_parse_fraction(*endp + 1, &denominator, endp);
            if (**endp != '\0') {
                break;
            }

            fraction *= 100;
            fraction /= denominator;
            number += fraction;

        }

        dop = number;

    } while (0);

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

hazer_talker_t hazer_parse_talker(const void * buffer, ssize_t length)
{
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    const char * sentence = (const char *)0;
    const char * id = (const char *)0;
    const char * name = (const char *)0;
    int ii = 0;
    int rc = -1;

    sentence = (const char *)buffer;
    id = &(sentence[HAZER_NMEA_TALKER]);

    if (length < HAZER_NMEA_NAME) {
        /* Do nothing. */
    } else if (sentence[0] != HAZER_STIMULUS_START) {
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
                continue;
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
    case HAZER_TALKER_BEIDOU2:
        system = HAZER_SYSTEM_BEIDOU;
        break;

    case HAZER_TALKER_QZSS:
    case HAZER_TALKER_QZSS2:
        system = HAZER_SYSTEM_QZSS;
        break;

    case HAZER_TALKER_NAVIC:
        system = HAZER_SYSTEM_NAVIC;
        break;

    default:
        /* Do nothing. */
        break;

    }

    return system;
}

/*
 * NMEA 0183 4.10 Table 20 pp. 94-95.
 * NMEA 0183 4.11 Table 19 pp. 83-84
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

    case HAZER_NMEA_QZSS:
#if !0 /* DEPRECATED */
    case HAZER_NMEA_QZSS2:
#endif
        system = HAZER_SYSTEM_QZSS;
        break;

    case HAZER_NMEA_NAVIC:
        system = HAZER_SYSTEM_NAVIC;
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
    } else if ((HAZER_NMEA_SBAS1_FIRST <= id) && (id <= HAZER_NMEA_SBAS1_LAST)) {
        candidate = HAZER_SYSTEM_SBAS;
    } else if ((HAZER_NMEA_GLONASS_FIRST <= id) && (id <= HAZER_NMEA_GLONASS_LAST)) {
        candidate = HAZER_SYSTEM_GLONASS;
    } else if ((HAZER_NMEA_SBAS2_FIRST <= id) && (id <= HAZER_NMEA_SBAS2_LAST)) {
        candidate = HAZER_SYSTEM_SBAS;
    } else if ((HAZER_NMEA_GLONASS_FIRST <= id) && (id <= HAZER_NMEA_GLONASS_LAST)) {
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
 * Why all the syntax and validity checking of NMEA(-ish) sentences? Because
 * after five years I finally tested a GPS device - a Bad Elf GPS Pro+ that I
 * bought used off eBay - that required it. The device reads NMEA output from a
 * MediaTek (MTK) GPS receiver, makes changes to it to make it palatable to the
 * iPhone/iPad/iEtc, regenerates the checksum, and then emits it. I found that
 * it was dropping characters in the NMEA sentences - typically the comma field
 * separator - and then recomputeing the checksum of the malformed sentence.
 * Wackiness ensued. (See the ISSUES.md file for more detail.)
 *
 * I am frequently tempted to replace the numerical constants used as counts
 * and indices below with symbolic constants, but I find in this particular
 * case they actually make the code a lot harder to read, to debug, and to
 * compare against the NMEA spec. The constants are not part of the Hazer
 * public API.
 *
 * Note that the count passed to the terminating parsers include the
 * terminating null pointer in the token vector. So a token vector
 * containing a single token would have a count of two: [ "token", NULL ].
 *
 * Do not be tempted to do a struture assignment (copy) in the code below.
 * That would update ALL of the fields, and that is not necessarily what is
 * intended. Parsing functions only update the fields for which they have
 * new data from the sentence that they are parsing. The provinence of
 * the fields in the structures may originate from several different NMEA
 * sentences.
 *
 * The semantics of errno are more complex than they might appear. For one, the
 * variable is actually thread safe: if you include the pthread header, it uses
 * a macro to redefine errno to be a function that uses a per-thread variable.
 * Secondly - and more subtly - you shouldn't change errno unless you actually
 * have something to communicate. That's why I don't, for example, initialize
 * errno to zero and only change it if parse function actually returns <0. To
 * do otherwise is to possibly introduce bugs in the calling application that
 * can be very difficult to find, particularly those applications that do a
 * good job checking for and reporting errors.
 */

int hazer_parse_gga(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char GGA[] = HAZER_NMEA_SENTENCE_GGA;

    do {

        /*
         * IDENTIFY
         */
    
        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GGA) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */
    
        if (count < 16) {
            errno = ENODATA;
            break;
        }

        position.quality = strtol(vector[6], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (position.quality == HAZER_QUALITY_NOFIX) {
            errno = 0;
            break;
        } else if (!((HAZER_QUALITY_MINIMUM <= position.quality) && (position.quality <= HAZER_QUALITY_MAXIMUM))) {
            position.quality = HAZER_QUALITY_INVALID;
        } else {
            /* Do nothing. */
        }

        position.sat_used = strtol(vector[7], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (position.sat_used == 0) {
            errno = 0;
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[2], *(vector[3]), &position.lat_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_latitude(position.lat_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[4], *(vector[5]), &position.lon_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_longitude(position.lon_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.alt_millimeters = hazer_parse_alt(vector[9], *(vector[10]), &position.alt_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.sep_millimeters = hazer_parse_alt(vector[11], *(vector[12]), &position.sep_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
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

        positionp->quality = position.quality;

        positionp->sat_used = position.sat_used;

        positionp->alt_millimeters = position.alt_millimeters;
        positionp->alt_digits = position.alt_digits;

        positionp->sep_millimeters = position.sep_millimeters;
        positionp->sep_digits = position.sep_digits;

        positionp->label = GGA;

        rc = 0;

    } while (0);

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
    hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
    static const int IDENTIFIERS = sizeof(activep->id) / sizeof(activep->id[0]);
    static const char GSA[] = HAZER_NMEA_SENTENCE_GSA;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGSA")) != (sizeof("$XXGSA") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GSA) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 19) {
            errno = ENODATA;
            break;
        }

        active.mode = strtoul(vector[2], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!((HAZER_MODE_MINIMUM <= active.mode) && (active.mode <= HAZER_MODE_MAXIMUM))) {
            active.mode = HAZER_MODE_INVALID;
        }

        /*
         * All of the satellite ID fields may be null (empty) if the GSA
         * sentence is merely reporting the DOP values.
         */

        for (slot = 0; slot < IDENTIFIERS; ++slot) {
            if (strlen(vector[index]) > 0) {
                id = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                active.id[slot] = id;
                ++satellites;
                ++index;
            }
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
            errno = EINVAL;
            break;
        }

        active.hdop = hazer_parse_dop(vector[16], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        active.vdop = hazer_parse_dop(vector[17], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        active.tdop = HAZER_GNSS_DOP;

        /*
         * NMEA 0183 4.10 has an additional 19th field containing
         * the GNSS System ID to identify GPS, GLONASS, GALILEO, etc.
         * NMEA 0183 4.11 suggests that this field is hexadecimal (1..F).
         */

        if (count > 19) {

            system = strtol(vector[18], &end, 16);
            if (*end != '\0') {
                errno = EINVAL;
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

    return rc;
}

int hazer_parse_gsv(hazer_view_t * viewp, char * vector[], size_t count)
{
    int rc = -1;
    int messages = 0;
    int message = 0;
    int channels = 0;
    int channel = 0;
    int signal = 0;
    int satellites = 0;
    int index = 4;
    int offset = 0;
    int slot = 0;
    unsigned int id = 0;
    char * end = (char *)0;
    hazer_band_t band = HAZER_BAND_INITIALIZER;
    static const int SATELLITES = sizeof(viewp->sig[0].sat) / sizeof(viewp->sig[0].sat[0]);
    static const char GSV[] = HAZER_NMEA_SENTENCE_GSV;

    do {

        /*
         * IDENTIFY
         */
    
        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGSV")) != (sizeof("$XXGSV") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GSV) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */
    
        if (count < 5) {
            errno = ENODATA;
            break;
        }

        messages = strtol(vector[1], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        message = strtol(vector[2], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        if (message <= 0) {
            errno = EINVAL;
            break;
        }

        if (message > messages) {
            errno = ERANGE;
            break;
        }

        /*
         * These fields are in decimal even though they may have a
         * leading zero (which in C/C++ conventions would make them
         * octal).
         */

        satellites = strtol(vector[3], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        /*
         * "Null fields are not required for unused sets when less
         * than four sets are transmitted." [NMEA 0183 v4.10 2012 p. 96]
         * Unlike the GSA sentence, the GSV sentence can have a variable
         * number of fields. So from here on all indices are effectively
         * relative. Also, while null fields are not required, they sometimes
         * are present.
         */

        for (slot = 0; slot < HAZER_GNSS_VIEWS; ++slot) {

            /*
             * Remember, the last slot is the NULL pointer.
             */
            if ((index + 4) >= count) {
                break;
            }

            if (strlen(vector[index]) == 0) {
                /*
                 * We can't break because we don't know how many sets of
                 * four null fields [ id, elevation, azimuth, SNR ] there
                 * will be, and there may still be a band number at the end.
                 * So we have to index through them. I'm looking at you,
                 * BU-353W10.
                 */
                index += 4;
                errno = 0;
                continue;
            }

            id = strtol(vector[index], &end, 10);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            band.sat[channel].id = id;

            ++index;

            /*
             * "For efficiency it is recommended that null fields be used
             * in the additional sentences when the data is unchanged from
             * the first sentence." [NMEA 0183 v4.10 2012 p. 96]
             * Does this mean that the same satellite ID can appear more
             * than once in the same tuple of GSV sentences? Or does this
             * just apply to the (newish) signal ID in the last field?
             */

            band.sat[channel].phantom = 0;

            if (strlen(vector[index]) == 0) {
                band.sat[channel].phantom = !0;
                band.sat[channel].elv_degrees = 0;
            } else {
                band.sat[channel].elv_degrees = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_elevation(band.sat[channel].elv_degrees)) {
                    errno = ERANGE;
                    break;
                }
            }

            ++index;

            if (strlen(vector[index]) == 0) {
                band.sat[channel].phantom = !0;
                band.sat[channel].azm_degrees = 0;
            } else {
                band.sat[channel].azm_degrees = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_azimuth(band.sat[channel].azm_degrees)) {
                    errno = ERANGE;
                    break;
                }
            }

            ++index;

            if (strlen(vector[index]) == 0) {
                band.sat[channel].untracked = !0;
                band.sat[channel].snr_dbhz = 0;
            } else {
                band.sat[channel].untracked = 0;
                band.sat[channel].snr_dbhz = strtol(vector[index], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_signaltonoiseratio(band.sat[channel].snr_dbhz)) {
                    errno = ERANGE;
                    break;
                }
            }

            ++index;
            ++channel;

            errno = 0;

        }
        if (errno != 0) {
            break;
        }

        /*
         * NMEA 0183 4.10 2012 has an additional field containing the
         * Signal IDentifier. This is constellation specific, but
         * indicates what frequency band was used, e.g. for GPS: L1C/A,
         * L2, etc. NMEA 0183 4.10 implies this field is in hex: 0..F.
         *
         * NMEA 0183 4.10 Note 4 p. 96: "This field shall not be null."
         * But if it happens to be null (missing), we have to handle it in
         * a reasonable way, since earlier NMEA implementations may not
         * include it.
         *
         * NMEA 0183 4.11 p. 98: "When more than one ranging signal is
         * used per satellite, separate GSV sentences with a System ID
         * corresponding to the ranging signals shall be required." Note
         * that this does not imply that the GSV sentences can't be in
         * a single grouping of consecutive sentences. Merely that all
         * of the reported satellites in a single GSV sentence are in
         * the same system and have the same signal (band).
         *
         * See also: Errata #0183 20190507, and Errata #0183 20190515,
         * both of which fix my issues with the 4.11 definition of the
         * GSV sentence.
         */

        if ((index + 1) >= count) {
            /* Do nothing. */
        } else if (strlen(vector[index]) == 0) {
            /* Do nothing. */
        } else {
            signal = strtol(vector[index], &end, 16);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }
        }

        if (!((0 <= signal) && (signal < HAZER_GNSS_SIGNALS))) {
            errno = E2BIG;
            break;
        }

        /*
         * Now we have to figure out how to map the local structure
         * into the structure passed by the caller.
         */

        offset = (message == 1) ? 0 : viewp->sig[signal].channels;
        channels = offset + channel;

        /*
         * APPLY
         */

        viewp->label = GSV;

        index = 0;
        for (channel = offset; channel < channels; ++channel) {
            if (channel >= satellites) {
                break;
            }
            if (channel >= SATELLITES) {
                break;
            }
            viewp->sig[signal].sat[channel] = band.sat[index++]; /* Structure copy. */
        }

        viewp->sig[signal].channels = channel;
        viewp->sig[signal].visible = satellites; /* Ambiguous. */

        if (signal >= viewp->signals) {
            viewp->signals = signal + 1; /* Never decreases. */
        }
        viewp->signal = signal;
        viewp->pending = messages - message;

        rc = signal; /* <0..0xF> */

    } while (0);

    return rc;
}

int hazer_parse_rmc(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char RMC[] = HAZER_NMEA_SENTENCE_RMC;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXRMC")) != (sizeof("$XXRMC") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, RMC) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 13) {
            errno = ENODATA;
            break;
        }

        /* NMEA 0183 4.11 p. 117 Note 3. (I find the spec very ambiguous.) */
        if (strcmp(vector[2], "V") != 0) {
            /* Do nothing. */
        } else if (count <= 13) {
            /* Do nothing. */
        } else if (strcmp(vector[12], "A") == 0) {
            /* Do nothing. */
        } else if (strcmp(vector[12], "D") == 0) {
            /* Do nothing. */
        } else {
            errno = 0;
            break;
        }

        /* NMEA 0183 4.11 p. 116 Note 2. */
        if (count <= 13) {
            position.quality = HAZER_QUALITY_UNKNOWN;
        } else if (strcmp(vector[12], "A") == 0) {
            position.quality = HAZER_QUALITY_AUTONOMOUS;
        } else if (strcmp(vector[12], "D") == 0) {
            position.quality = HAZER_QUALITY_DIFFERENTIAL;
        } else if (strcmp(vector[12], "E") == 0) {
            position.quality = HAZER_QUALITY_ESTIMATED;
        } else if (strcmp(vector[12], "F") == 0) {
            position.quality = HAZER_QUALITY_RTKFLOAT;
        } else if (strcmp(vector[12], "M") == 0) {
            position.quality = HAZER_QUALITY_MANUAL;
        } else if (strcmp(vector[12], "N") == 0) {
            errno = 0;
            break;
        } else if (strcmp(vector[12], "P") == 0) {
            position.quality = HAZER_QUALITY_PRECISE;
        } else if (strcmp(vector[12], "R") == 0) {
            position.quality = HAZER_QUALITY_RTK;
        } else if (strcmp(vector[12], "S") == 0) {
            position.quality = HAZER_QUALITY_SIMULATOR;
        } else {
            position.quality = HAZER_QUALITY_INVALID;
        }

        /* NMEA 0183 4.11 p. 116 Note 4. */
        if (count <= 14) {
            position.safety = HAZER_SAFETY_UNKNOWN;
        } else if (strcmp(vector[13], "S") == 0) {
            position.safety = HAZER_SAFETY_SAFE;
        } else if (strcmp(vector[13], "C") == 0) {
            position.safety = HAZER_SAFETY_CAUTION;
        } else if (strcmp(vector[13], "U") == 0) {
            position.safety = HAZER_SAFETY_UNSAFE;
        } else if (strcmp(vector[13], "V") == 0) {
            position.safety = HAZER_SAFETY_NOSTATUS;
        } else {
            position.safety = HAZER_SAFETY_INVALID;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lat_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_latitude(position.lat_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[5], *(vector[6]), &position.lon_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_longitude(position.lon_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.sog_microknots = hazer_parse_sog(vector[7], &position.sog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[8], &position.cog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_courseoverground(position.cog_nanodegrees)) {
            errno = ERANGE;
            break;
        }

        position.dmy_nanoseconds = hazer_parse_dmy(vector[9], &end);
        if (*end != '\0') {
            errno = EINVAL;
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

        positionp->quality = position.quality;

        positionp->safety = position.safety;

        positionp->label = RMC;

        rc = 0;

    } while (0);

    return rc;
}

int hazer_parse_gll(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char GLL[] = HAZER_NMEA_SENTENCE_GLL;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXGGA")) != (sizeof("$XXGGA") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GLL) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 8) {
            errno = ENODATA;
            break;
        }

        if (strcmp(vector[6], "A") != 0) {
            errno = 0;
            break;
        }

        if (count < 9) {
            position.quality = HAZER_QUALITY_UNKNOWN;
        } else {
            switch (vector[7][0]) {
            case 'A':
                position.quality = HAZER_QUALITY_AUTONOMOUS;
                break;
            case 'D':
                position.quality = HAZER_QUALITY_DIFFERENTIAL;
                break;
            case 'E':
                position.quality = HAZER_QUALITY_ESTIMATED;
                break;
            case 'M':
                position.quality = HAZER_QUALITY_MANUAL;
                break;
            case 'S':
                position.quality = HAZER_QUALITY_SIMULATOR;
                break;
            case 'N':
                position.quality = HAZER_QUALITY_INVALID;
                break;
            default:
                position.quality = HAZER_QUALITY_INVALID; 
                break;
            }
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[1], *(vector[2]), &position.lat_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_latitude(position.lat_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lon_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_longitude(position.lon_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[5], &end);
        if (*end != '\0') {
            errno = EINVAL;
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

        positionp->quality = position.quality;

        positionp->label = GLL;

        rc = 0;

    } while (0);

    return rc;
}

int hazer_parse_vtg(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char VTG[] = HAZER_NMEA_SENTENCE_VTG;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXVTG")) != (sizeof("$XXVTG") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, VTG) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 10) {
            errno = ENODATA;
            break;
        }

        if (strcmp(vector[9], "N") == 0) {
            errno = 0;
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[1], &position.cog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_courseoverground(position.cog_nanodegrees)) {
            errno = ERANGE;
            break;
        }

        position.mag_nanodegrees = hazer_parse_cog(vector[3], &position.mag_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.sog_microknots = hazer_parse_sog(vector[5], &position.sog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.sog_millimetersperhour = hazer_parse_smm(vector[7], &position.smm_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
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

int hazer_parse_zda(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    int sign = 1;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char ZDA[] = HAZER_NMEA_SENTENCE_ZDA;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof("$XXZDA")) != (sizeof("$XXZDA") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, ZDA) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 8) {
            errno = ENODATA;
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.dmy_nanoseconds = hazer_parse_d_m_y(vector[2], vector[3], vector[4], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        /*
         * NMEA 0183 4.10 p. 132 Note 1
         */

        position.tz_nanoseconds = strtol(vector[5], &end, 10) * 60LL * 60LL * 1000000000LL;
        if (*end != '\0') {
            break;
        }

        if (position.tz_nanoseconds < 0) {
            position.tz_nanoseconds *= -1;
            sign = -1;
        }

        position.tz_nanoseconds += strtoul(vector[6], &end, 10) * 60LL * 1000000000LL;
        if (*end != '\0') {
            break;
        }

        if (sign < 0) {
            position.tz_nanoseconds *= sign;
        }

        /*
         * APPLY
         */

        positionp->utc_nanoseconds = position.utc_nanoseconds;
        positionp->dmy_nanoseconds = position.dmy_nanoseconds;
        positionp->tz_nanoseconds = position.tz_nanoseconds;
        /*
         * We update the time even though it's not completely
         * clear that is the right thing to do. Some devices,
         * e.g. the UBX-NEO-F10T, appear to continue to report
         * the time via NMEA ZDA, presumably using their own
         * internal real-time clock, even though a fix has been
         * lost.
         */
        update_time(positionp);

        positionp->label = ZDA;

        rc = 0;

    } while (0);

    return rc;
}

int hazer_parse_gbs(hazer_fault_t * faultp, char * vector[], size_t count)
{
    int rc = -1;
    size_t length = 0;
    char * end = (char *)0;
    uint8_t digits = 0;
    int system = HAZER_SYSTEM_TOTAL;
    static const hazer_fault_t FAULT = HAZER_FAULT_INITIALIZER;
    static const char GBS[] = HAZER_NMEA_SENTENCE_GBS;

    do {

        /*
         * IDENTIFY
         */

        if (count < 2) {
            errno = ENOMSG;
            break;
        }

        length = strnlen(vector[0], sizeof("$XXGBS"));
        if (length != (sizeof("$XXGBS") - 1)) {
            errno = ENOMSG;
            break;
        }

        if (*vector[0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[0] + sizeof("$XX") - 1, GBS) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 10) {
            errno = ENODATA;
            break;
        }

        memcpy(faultp, &FAULT, sizeof(*faultp));

        faultp->talker = hazer_parse_talker(vector[0], length);

        faultp->utc_nanoseconds = hazer_parse_utc(vector[1], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->lat_millimeters = hazer_parse_alt(vector[2], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->lon_millimeters = hazer_parse_alt(vector[3], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->alt_millimeters = hazer_parse_alt(vector[4], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->id = strtol(vector[5], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->probability = hazer_parse_alt(vector[6], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->est_millimeters = hazer_parse_alt(vector[7], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        faultp->std_deviation = hazer_parse_alt(vector[8], 'M', &digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        if (count < 12) {
            rc = 0;
            break;
        }

        system = strtol(vector[9], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        faultp->system = hazer_map_nmea_to_system(system);

        faultp->signal = strtol(vector[10], &end, 16);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        rc = 0;

    } while (0);

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_pubx_position(hazer_position_t * positionp, hazer_active_t * activep, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_POSITION;

    do {

        /*
         * IDENTIFY
         */

        if (count < 3) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            errno = ENOMSG;
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 22) {
            errno = ENODATA;
            break;
        }

        if (strcmp(vector[8], "NF") == 0) {
            errno = 0;
            break;
        }

        if (strcmp(vector[18], "0") == 0) {
            errno = 0;
            break;
        }

        if (strcmp(vector[8], "TT") == 0) {

            position.utc_nanoseconds = hazer_parse_utc(vector[2], &end);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            position.sat_used = strtol(vector[18], &end, 10);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            active.mode = HAZER_MODE_TIME;

            active.hdop = hazer_parse_dop(vector[15], &end);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            active.vdop = hazer_parse_dop(vector[16], &end);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            active.tdop = hazer_parse_dop(vector[17], &end);
            if (*end != '\0') {
                errno = EINVAL;
                break;
            }

            /*
             * APPLY
             */

            /* Position. */

            positionp->utc_nanoseconds = position.utc_nanoseconds;
            update_time(positionp);

            positionp->sat_used = position.sat_used;

            positionp->label = PUBX;

            /* Active. */

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
            errno = EINVAL;
            break;
        }

        position.lat_nanominutes = hazer_parse_latlon(vector[3], *(vector[4]), &position.lat_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_latitude(position.lat_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.lon_nanominutes = hazer_parse_latlon(vector[5], *(vector[6]), &position.lon_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_longitude(position.lon_nanominutes)) {
            errno = ERANGE;
            break;
        }

        position.sep_millimeters = hazer_parse_alt(vector[7], *(vector[12]), &position.sep_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        if (strcmp(vector[8], "DR") == 0) {
            active.mode = HAZER_MODE_IMU;
            position.quality = HAZER_QUALITY_ESTIMATED;
        } else if (strcmp(vector[8], "G2") == 0) {
            active.mode = HAZER_MODE_2D;
            position.quality = HAZER_QUALITY_AUTONOMOUS;
        } else if (strcmp(vector[8], "G3") == 0) {
            active.mode = HAZER_MODE_3D;
            position.quality = HAZER_QUALITY_AUTONOMOUS;
        } else if (strcmp(vector[8], "RK") == 0) {
            active.mode = HAZER_MODE_COMBINED;
            position.quality = HAZER_QUALITY_ESTIMATED;
        } else if (strcmp(vector[8], "D2") == 0) {
            active.mode = HAZER_MODE_DGNSS2D;
            position.quality = HAZER_QUALITY_DIFFERENTIAL;
        } else if (strcmp(vector[8], "D3") == 0) {
            active.mode = HAZER_MODE_DGNSS3D;
            position.quality = HAZER_QUALITY_DIFFERENTIAL;
        } else {
            active.mode = HAZER_MODE_INVALID;
            position.quality = HAZER_QUALITY_INVALID;
        }

        position.sog_millimetersperhour = hazer_parse_smm(vector[11], &position.sog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.cog_nanodegrees = hazer_parse_cog(vector[12], &position.cog_digits, &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }
        if (!hazer_is_valid_courseoverground(position.cog_nanodegrees)) {
            errno = ERANGE;
            break;
        }

        position.sat_used = strtol(vector[18], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        active.hdop = hazer_parse_dop(vector[15], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        active.vdop = hazer_parse_dop(vector[16], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        active.tdop = hazer_parse_dop(vector[17], &end);
        if (*end != '\0') {
            errno = EINVAL;
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

        positionp->quality = position.quality;

        positionp->label = PUBX;

        activep->mode = active.mode;

        activep->hdop = active.hdop;
        activep->vdop = active.vdop;
        activep->tdop = active.tdop;

        activep->label = PUBX;

        rc = 0;

    } while (0);

    return rc;
}

int hazer_parse_pubx_svstatus(hazer_views_t viewa, hazer_actives_t activea, char * vector[], size_t count)
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
    hazer_view_t views[HAZER_SYSTEM_TOTAL] = HAZER_VIEWS_INITIALIZER;
    hazer_active_t actives[HAZER_SYSTEM_TOTAL] = HAZER_ACTIVES_INITIALIZER;
    static const int SATELLITES = sizeof(views[0].sig[0].sat) / sizeof(views[0].sig[0].sat[0]);
    static const int RANGERS = sizeof(actives[0].id) / sizeof(actives[0].id[0]);
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_SVSTATUS;

    do {

        /* IDENTIFY */

        if (count < 4) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            errno = ENOMSG;
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        satellites = strtol(vector[2], &end, 10);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        if (count < (4 + (satellites * 6))) {
            errno = ENODATA;
            break;
        }

        for (satellite = 0; satellite < satellites; ++satellite) {

            id = strtol(vector[index + 0], &end, 10);
            if (*end != '\0') {
                errno = EINVAL;
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

            views[system].sig[HAZER_SIGNAL_ANY].sat[channel].id = id;
            views[system].sig[HAZER_SIGNAL_ANY].sat[channel].phantom = 0;
            views[system].sig[HAZER_SIGNAL_ANY].sat[channel].untracked = 0;
            views[system].sig[HAZER_SIGNAL_ANY].sat[channel].unused = 0;

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
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].unused = !0;
            } else {
                /* Should never happen, and not clear what it means if it does. */
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].phantom = !0;
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].untracked = !0;
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].unused = !0;
            }

            if (strlen(vector[index + 2]) == 0) {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].phantom = !0;
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].azm_degrees = 0;
            } else {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].azm_degrees = strtol(vector[index + 2], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_azimuth(views[system].sig[HAZER_SIGNAL_ANY].sat[channel].azm_degrees)) {
                    errno = ERANGE;
                    break;
                }
            }

            if (strlen(vector[index + 3]) == 0) {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].phantom = !0;
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].elv_degrees = 0;
            } else {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].elv_degrees = strtol(vector[index + 3], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_elevation(views[system].sig[HAZER_SIGNAL_ANY].sat[channel].elv_degrees)) {
                    errno = ERANGE;
                    break;
                }
            }

            if (strlen(vector[index + 4]) == 0) {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].untracked = !0;
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].snr_dbhz = 0;
            } else {
                views[system].sig[HAZER_SIGNAL_ANY].sat[channel].snr_dbhz = strtol(vector[index + 4], &end, 10);
                if (*end != '\0') {
                    errno = EINVAL;
                    break;
                }
                if (!hazer_is_valid_signaltonoiseratio(views[system].sig[HAZER_SIGNAL_ANY].sat[channel].snr_dbhz)) {
                    errno = ERANGE;
                    break;
                }
            }

            channel += 1;

            channels[system] = channel;

            views[system].sig[HAZER_SIGNAL_ANY].channels = channel;
            views[system].sig[HAZER_SIGNAL_ANY].visible = satellites;

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

                for (channel = 0; channel < views[system].sig[HAZER_SIGNAL_ANY].channels; ++channel) {
                    viewa[system].sig[HAZER_SIGNAL_ANY].sat[channel] = views[system].sig[HAZER_SIGNAL_ANY].sat[channel]; /* Structure copy. */
                }

                viewa[system].sig[HAZER_SIGNAL_ANY].channels = views[system].sig[HAZER_SIGNAL_ANY].channels;
                viewa[system].sig[HAZER_SIGNAL_ANY].visible = views[system].sig[HAZER_SIGNAL_ANY].visible;

                activea[system].label = PUBX;

                for (ranger = 0; ranger < actives[system].active; ++ranger) {
                    activea[system].id[ranger] = actives[system].id[ranger];
                }

                activea[system].system = actives[system].system;
                activea[system].active = actives[system].active;
                activea[system].mode = actives[system].mode;

                if (0 >= viewa[system].signals) {
                    viewa[system].signals = 0 + 1;
                }

                viewa[system].pending = 0;

            }

        }

        rc = result;

    } while (0);

    return rc;
}

int hazer_parse_pubx_time(hazer_position_t * positionp, char * vector[], size_t count)
{
    int rc = -1;
    char * end = (char *)0;
    hazer_position_t position = HAZER_POSITION_INITIALIZER;
    static const char PUBX[] = HAZER_PROPRIETARY_SENTENCE_PUBX;
    static const char ID[] = HAZER_PROPRIETARY_SENTENCE_PUBX_TIME;

    do {

        /*
         * IDENTIFY
         */

        if (count < 3) {
            errno = ENOMSG;
            break;
        }

        if (strnlen(vector[0], sizeof(PUBX) + 1) != sizeof(PUBX)) {
            errno = ENOMSG;
            break;
        }

        if (vector[0][0] != HAZER_STIMULUS_START) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(&vector[0][1], PUBX) != 0) {
            errno = ENOMSG;
            break;
        }

        if (strcmp(vector[1], ID) != 0) {
            errno = ENOMSG;
            break;
        }

        /*
         * VALIDATE
         */

        if (count < 11) {
            errno = ENODATA;
            break;
        }

        position.utc_nanoseconds = hazer_parse_utc(vector[2], &end);
        if (*end != '\0') {
            errno = EINVAL;
            break;
        }

        position.dmy_nanoseconds = hazer_parse_dmy(vector[3], &end);
        if (*end != '\0') {
            errno = EINVAL;
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

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_has_pending_gsv(const hazer_views_t va, hazer_system_t ss)
{
    size_t ii = 0;
    size_t jj = 0;

    for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
        if (ii > ss) { break; }
        if (va[ii].pending > 0) {
            for (jj = 0; jj < HAZER_GNSS_SIGNALS; ++jj) {
                if (jj >= va[ii].signals) { break; }
                if (va[ii].sig[jj].timeout > 0) {
                    return !0;
                }
            }
        }
    }

    return 0;
}

int hazer_has_valid_time(const hazer_positions_t pa, hazer_system_t ss)
{
    size_t ii = 0;

    for (ii = 0; ii < HAZER_SYSTEM_TOTAL; ++ii) {
        if (ii > ss) { break; }
        if (hazer_is_valid_time(&(pa[ii]))) {
            return !0;
        }
    }

    return 0;
}
