/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
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
            state = HAZER_STATE_TALKER_1;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else if (ch == HAZER_STIMULUS_ENCAPSULATION) {
            DEBUG("ENCAPSULATE '%c'.\n", ch);
            state = HAZER_STATE_CHECKSUM;
            action = HAZER_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
        } else {
            /* Do nothing. */
        }
        break;

    case HAZER_STATE_TALKER_1:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_TALKER_2;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_TALKER_2:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_1;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_1:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_2;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_2:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_MESSAGE_3;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_MESSAGE_3:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            DEBUG("STARTING '%c'!\n", ch);
            state = HAZER_STATE_START;
        } else {
            state = HAZER_STATE_DELIMITER;
            action = HAZER_ACTION_SAVE;
        }
        break;

    case HAZER_STATE_DELIMITER:
        if (ch == HAZER_STIMULUS_DELIMITER) {
            state = HAZER_STATE_CHECKSUM;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CHECKSUM:
        if (ch == HAZER_STIMULUS_CHECKSUM) {
            state = HAZER_STATE_CHECKSUM_1;
        }
        action = HAZER_ACTION_SAVE;
        break;

    case HAZER_STATE_CHECKSUM_1:
        if ((HAZER_STIMULUS_DECMIN <= ch) && (ch <= HAZER_STIMULUS_DECMAX)) {
            state = HAZER_STATE_CHECKSUM_2;
            action = HAZER_ACTION_SAVE;
        } else if ((HAZER_STIMULUS_HEXMIN <= ch) && (ch <= HAZER_STIMULUS_HEXMAX)) {
            state = HAZER_STATE_CHECKSUM_2;
            action = HAZER_ACTION_SAVE;
        } else {
            DEBUG("STARTING 0x%x!\n", ch);
            state = HAZER_STATE_START;
        }
        break;

    case HAZER_STATE_CHECKSUM_2:
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

    if ((count--) > 0) {
        *vv = bb;
        while ((size--) > 0) {
            if (*bb == ',') {
                *(bb++) = '\0';
                DEBUG("TOK \"%s\".\n", *vv);
                if ((count--) <= 0) {
                    break;
                }
                *(++vv) = bb;
            } else if (*bb == '*') {
                *(bb++) = '\0';
                DEBUG("TOK \"%s\".\n", *vv);
                if ((count--) <= 0) {
                    break;
                }
                *(++vv) = (char *)0;
                DEBUG("TOK 0x0.\n");
                break;
            } else {
                ++bb;
            }
        }
    }

    return (vv - vector);
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

double hazer_parse_latlon(const char * string, char direction, uint8_t * digitsp)
{
    double latlon = 0.0;
    double fraction = 0.0;
    uint64_t numerator = 0;
    uint64_t denominator = 1;
    unsigned long dddmm = 0;
    char * end = (char *)0;
    uint8_t digits = 0;

    digits = strlen(string);

    dddmm = strtoul(string, &end, 10);
    latlon = dddmm / 100;
    fraction = dddmm % 100;
    fraction /= 60;
    latlon += fraction;
   
    if (*end == HAZER_STIMULUS_DECIMAL) {
        numerator = hazer_parse_fraction(end + 1, &denominator);
        fraction = numerator;
        fraction /= 60;
        fraction /= denominator;
        latlon += fraction;
        --digits;
    }

    switch (direction) {
    case HAZER_STIMULUS_NORTH:
    case HAZER_STIMULUS_EAST:
        break;
    case HAZER_STIMULUS_SOUTH:
    case HAZER_STIMULUS_WEST:
        latlon = -latlon;
        break;
    default:
        break;
    }

    *digitsp = digits;

    return latlon; 
}

double hazer_parse_number(const char * string)
{
    double number = 0.0;
    double fraction = 0;
    uint64_t numerator = 0;
    uint64_t denominator = 0;
    char * end = (char *)0;

    number = strtoul(string, &end, 10);

    if (*end == HAZER_STIMULUS_DECIMAL) {
        numerator = hazer_parse_fraction(end + 1, &denominator);
        fraction = numerator;
        fraction /= denominator;
        number += fraction;
    }

    return number;
}



double hazer_parse_alt(const char * string, char units)
{
    return hazer_parse_number(string);
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

void hazer_format_degrees2position(double degrees, int * degreesp, int * minutesp, int * secondsp, int * hundredthsp, int * directionp)
{
    char direction = '\0';
    double integral = 0.0;
    double fraction = 0.0;

    if (degrees < 0) {
        degrees = -degrees;
        *directionp = -1;
    } else {
        degrees = degrees;
        *directionp = 1;
    }

    fraction = modf(degrees, &integral);
    *degreesp = integral;
    *minutesp = fraction * 60.0;
    fraction -= *minutesp / 60.0;
    *secondsp = fraction * 3600.0;
    fraction -= *secondsp / 3600.0;
    *hundredthsp = fraction * 360000.0;
}

const char * hazer_format_degrees2compass(double degrees)
{
    static const char * COMPASS[] = {
        "N", "NbE", "NNE", "NEbN", "NE", "NEbE", "ENE", "EbN",
        "E", "EbS", "ESE", "SEbE", "SE", "SEbS", "SSE", "SbE",
        "S", "SbW", "SSW", "SWbS", "SW", "SWbW", "WSW", "WbS",
        "W", "WbN", "WNW", "NWbW", "NW", "NWbN", "NNW", "NbW",
    };
    static const int INCREMENT = 360000 / (sizeof(COMPASS) / sizeof(COMPASS[0]));
    unsigned long index = 0;

    index = degrees * 1000.0;
    index += INCREMENT / 2;
    index %= 360000;
    if (index < 0) { index += 360000; }
    index /= INCREMENT;

    return COMPASS[index];
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_gga(hazer_position_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char GGA[] = HAZER_NMEA_SENTENCE_START HAZER_NMEA_GPS_TALKER HAZER_NMEA_GPS_MESSAGE_GGA;
    
    if (count < 11) { 
        /* Do nothing. */
    } else if (strncmp(vector[0], GGA, sizeof(GGA) - 1) != 0) {
        /* Do nothing. */
    } else if (*vector[6] == '0') {
        /* Do nothing. */
    } else {
        datap->utc_nanoseconds = hazer_parse_utc(vector[1]);
        datap->lat_degrees = hazer_parse_latlon(vector[2], *(vector[3]), &datap->lat_digits);
        datap->lon_degrees = hazer_parse_latlon(vector[4], *(vector[5]), &datap->lon_digits);
        datap->alt_meters = hazer_parse_alt(vector[9], *(vector[10]));
        rc = 0;
    }

    return rc;
}

int hazer_parse_rmc(hazer_position_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char RMC[] = HAZER_NMEA_SENTENCE_START HAZER_NMEA_GPS_TALKER HAZER_NMEA_GPS_MESSAGE_RMC;
    
    if (count < 10) { 
        /* Do nothing. */
    } else if (strncmp(vector[0], RMC, sizeof(RMC) - 1) != 0) {
        /* Do nothing. */
    } else if (*vector[2] != 'A') {
        /* Do nothing. */
    } else {
        datap->utc_nanoseconds = hazer_parse_utc(vector[1]);
        datap->lat_degrees = hazer_parse_latlon(vector[3], *(vector[4]), &datap->lat_digits);
        datap->lon_degrees = hazer_parse_latlon(vector[5], *(vector[6]), &datap->lon_digits);
        datap->sog_knots = hazer_parse_number(vector[7]);
        datap->cog_degrees = hazer_parse_number(vector[8]);
        datap->dmy_nanoseconds = hazer_parse_dmy(vector[9]);
        rc = 0;
    }

    return rc;
}

/******************************************************************************
 *
 ******************************************************************************/

int hazer_parse_gsv(hazer_constellation_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    static const char GSV[] = HAZER_NMEA_SENTENCE_START HAZER_NMEA_GPS_TALKER HAZER_NMEA_GPS_MESSAGE_GSV;
    int messages = 0;
    int message = 0;
    int start = 0;
    int index = 4;
    int slot = 0;
    int channel = 0;
    int satellites = 0;
    int limit = sizeof(datap->sat) / sizeof(datap->sat[0]);
    unsigned int id = 0;
    
    if (count < 11) { 
        /* Do nothing. */
    } else if (strncmp(vector[0], GSV, sizeof(GSV) - 1) != 0) {
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
                datap->channels = channel;
                rc = 1;
            }
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

int hazer_parse_gsa(hazer_constellation_t * datap, char * vector[], size_t count)
{
    int rc = -1;
    int index = 3;
    int slot = 0;
    int id = 0;
    int satellites = 0;
    static const char GSA[] = HAZER_NMEA_SENTENCE_START HAZER_NMEA_GPS_TALKER HAZER_NMEA_GPS_MESSAGE_GSA;
    int limit = sizeof(datap->id) / sizeof(datap->id[0]);

    if (count < 18) {
    } else if (strncmp(vector[0], GSA, sizeof(GSA) - 1) != 0) {
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
        datap->satellites = satellites;
        datap->pdop = hazer_parse_number(vector[15]);
        datap->hdop = hazer_parse_number(vector[16]);
        datap->vdop = hazer_parse_number(vector[17]);
        rc = 0;
    }

    return rc;
}
