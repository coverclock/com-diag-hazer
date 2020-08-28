/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include <stdint.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

int main(void)
{

    {
        /**
         * Derived from "Points of the compass" in Wikipedia.
         */
        static struct {
            const char name[8];
            unsigned int minimum;
            unsigned int maximum;
        } POINT[] = {
            { "N", 337500, 22500 },
            { "NE", 22500, 67500 },
            { "E", 67500, 112500 },
            { "SE", 112500, 157500 },
            { "S", 157500, 202500 },
            { "SW", 202500, 247500 },
            { "W", 247500, 292500 },
            { "NW", 292500, 337500 },
        };
        static const int DIVISIONS = (sizeof(POINT) / sizeof(POINT[0]));
        double degrees = 0.0;
        const char * name = (const char *)0;
        int index = 0;
        uint64_t nanodegrees = 0;
        uint32_t millidegrees = 0;

        for (degrees = 0.0; degrees < 360.0; degrees += 0.001) {
            nanodegrees = degrees * 1000000000.0;
            name = hazer_format_nanodegrees2compass8(nanodegrees);
            millidegrees = degrees * 1000.0;
            for (index = 0; index < DIVISIONS; ++index) {
                if (index == 0) {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < 360000)) {
                        break;
                    }
                    if ((0 <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                } else {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                }
            }
            assert(index < DIVISIONS);
            assert(name != (const char *)0);
            if ((millidegrees % 1000) == 0) {
                fprintf(stderr, "COMPASS %2d %7.3lf %2d %s %s\n", DIVISIONS, ((double)millidegrees) / 1000.0, index, POINT[index].name, name);
            }
            assert(strcmp(name, POINT[index].name) == 0);
        }

        assert(strcmp(hazer_format_nanodegrees2compass8(-720000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass8(-360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass8(360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass8(720000000000LL), "N") == 0);
    }

    {
        /**
         * Derived from "Points of the compass" in Wikipedia.
         */
        static struct {
            const char name[8];
            unsigned int minimum;
            unsigned int maximum;
        } POINT[] = {
            { "N", 348750, 11250 },
            { "NNE", 11250, 33750 },
            { "NE", 33750, 56250 },
            { "ENE", 56250, 78750 },
            { "E", 78750, 101250 },
            { "ESE", 101250, 123750 },
            { "SE", 123750, 146250 },
            { "SSE", 146250, 168750 },
            { "S", 168750, 191250 },
            { "SSW", 191250, 213750 },
            { "SW", 213750, 236250 },
            { "WSW", 236250, 258750 },
            { "W", 258750, 281250 },
            { "WNW", 281250, 303750 },
            { "NW", 303750, 326250 },
            { "NNW", 326250, 348750 },
        };
        static const int DIVISIONS = (sizeof(POINT) / sizeof(POINT[0]));
        double degrees = 0.0;
        const char * name = (const char *)0;
        int index = 0;
        uint64_t nanodegrees = 0;
        uint32_t millidegrees = 0;

        for (degrees = 0.0; degrees < 360.0; degrees += 0.001) {
            nanodegrees = degrees * 1000000000.0;
            name = hazer_format_nanodegrees2compass16(nanodegrees);
            millidegrees = degrees * 1000.0;
            for (index = 0; index < DIVISIONS; ++index) {
                if (index == 0) {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < 360000)) {
                        break;
                    }
                    if ((0 <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                } else {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                }
            }
            assert(index < DIVISIONS);
            assert(name != (const char *)0);
            if ((millidegrees % 1000) == 0) {
                fprintf(stderr, "COMPASS %2d %7.3lf %2d %s %s\n", DIVISIONS, ((double)millidegrees) / 1000.0, index, POINT[index].name, name);
            }
            assert(strcmp(name, POINT[index].name) == 0);
        }

        assert(strcmp(hazer_format_nanodegrees2compass16(-720000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass16(-360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass16(360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass16(720000000000LL), "N") == 0);
    }

    {
        /**
         * Derived from "Points of the compass" in Wikipedia except that, remarkably, I don't think
         * it's correct. The "32 cardinal points" table needs to have three fractional digits to be
         * accurate.
         */
        static struct {
            const char name[8];
            unsigned int minimum;
            unsigned int maximum;
        } POINT[] = {
            { "N", 354375, 5625 },
            { "NbE", 5625, 16875 },
            { "NNE", 16875, 28125 },
            { "NEbN", 28125, 39375 },
            { "NE", 39375, 50625 },
            { "NEbE", 50625, 61875 },
            { "ENE", 61875, 73125 },
            { "EbN", 73125, 84375 },
            { "E", 84375, 95625 },
            { "EbS", 95625, 106875 },
            { "ESE", 106875, 118125 },
            { "SEbE", 118125, 129375 },
            { "SE", 129375, 140625 },
            { "SEbS", 140625, 151875 },
            { "SSE", 151875, 163125 },
            { "SbE", 163125, 174375 },
            { "S", 174375, 185625 },
            { "SbW", 185625, 196875 },
            { "SSW", 196875, 208125 },
            { "SWbS", 208125, 219375 },
            { "SW", 219375, 230625 },
            { "SWbW", 230625, 241875 },
            { "WSW", 241875, 253125 },
            { "WbS", 253125, 264375 },
            { "W", 264375, 275625 },
            { "WbN", 275625, 286875 },
            { "WNW", 286875, 298125 },
            { "NWbW", 298125, 309375 },
            { "NW", 309375, 320625 },
            { "NWbN", 320625, 331875 },
            { "NNW", 331875, 343125 },
            { "NbW", 343125, 354375 },
        };
        static const int DIVISIONS = (sizeof(POINT) / sizeof(POINT[0]));
        double degrees = 0.0;
        const char * name = (const char *)0;
        int index = 0;
        uint64_t nanodegrees = 0;
        uint32_t millidegrees = 0;

       for (degrees = 0.0; degrees < 360.0; degrees += 0.001) {
            nanodegrees = degrees * 1000000000.0;
            name = hazer_format_nanodegrees2compass32(nanodegrees);
            millidegrees = degrees * 1000.0;
            for (index = 0; index < DIVISIONS; ++index) {
                if (index == 0) {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < 360000)) {
                        break;
                    }
                    if ((0 <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                } else {
                    if ((POINT[index].minimum <= millidegrees) && (millidegrees < POINT[index].maximum)) {
                        break;
                    }
                }
            }
            assert(index < DIVISIONS);
            assert(name != (const char *)0);
            if ((millidegrees % 1000) == 0) {
                fprintf(stderr, "COMPASS %2d %7.3lf %2d %s %s\n", DIVISIONS, ((double)millidegrees) / 1000.0, index, POINT[index].name, name);
            }
            assert(strcmp(name, POINT[index].name) == 0);
        }

        assert(strcmp(hazer_format_nanodegrees2compass32(-720000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass32(-360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass32(360000000000LL), "N") == 0);
        assert(strcmp(hazer_format_nanodegrees2compass32(720000000000LL), "N") == 0);
    }

    {
        int year = ~0;
        int month = ~0;
        int day = ~0;
        int hour = ~0;
        int minute = ~0;
        int second = ~0;
        uint64_t nanosecond = ~0ULL;
        hazer_format_nanoseconds2timestamp(1563285269123456789ULL, &year, &month, &day, &hour, &minute, &second, &nanosecond);
        assert(year == 2019);
        assert(month == 7);
        assert(day == 16);
        assert(hour == 13);
        assert(minute == 54);
        assert(second == 29);
        assert(nanosecond == 123456789ULL);
    }

    {
        int degrees = ~0;
        int minutes = ~0;
        int seconds = ~0;
        int thousandths = ~0;
        int direction = 0;
        hazer_format_nanominutes2position(2387652807660LL, &degrees, &minutes, &seconds, &thousandths, &direction);
        assert(degrees == 39);
        assert(minutes == 47);
        assert(seconds == 39);
        assert(thousandths == 168);
        assert(direction == 1);
    }

    {
        int degrees = 0;
        uint64_t tenmillionths = 0;
        hazer_format_nanominutes2degrees(2387652807660LL, &degrees, &tenmillionths);
        assert(degrees == 39);
        assert(tenmillionths == 7942134ULL);
    }

    {
        int degrees = ~0;
        int minutes = ~0;
        int seconds = ~0;
        int thousandths = ~0;
        int direction = 0;
        hazer_format_nanominutes2position(-6309202937220LL, &degrees, &minutes, &seconds, &thousandths, &direction);
        assert(degrees == 105);
        assert(minutes == 9);
        assert(seconds == 12);
        assert(thousandths == 176);
        assert(direction == -1);
    }

    {
        int degrees = 0;
        uint64_t tenmillionths = 0;
        hazer_format_nanominutes2degrees(-6309202937220LL, &degrees, &tenmillionths);
        assert(degrees == -105);
        assert(tenmillionths == 1533822ULL);
    }

    {
        int32_t degrees = 0;
        uint64_t billionths = 0;
        yodel_format_hppos2degrees(397942134, 61, &degrees, &billionths);
        assert(degrees == 39);
        assert(billionths == 794213461ULL);
    }

    {
        int32_t degrees = 0;
        uint64_t billionths = 0;
        yodel_format_hppos2degrees(-1051533822, -87, &degrees, &billionths);
        assert(degrees == -105);
        assert(billionths == 153382287ULL);
    }

    {
        uint32_t degrees = ~0;
        uint32_t minutes = ~0;
        uint32_t seconds = ~0;
        uint32_t onehundredthousandsth = ~0;
        int direction = 0;
        yodel_format_hppos2position(397942134, 61, &degrees, &minutes, &seconds, &onehundredthousandsth, &direction);
        assert(degrees == 39);
        assert(minutes == 47);
        assert(seconds == 39);
        assert(onehundredthousandsth == 16845);
        assert(direction == 1);
    }

    {
        uint32_t degrees = ~0;
        uint32_t minutes = ~0;
        uint32_t seconds = ~0;
        uint32_t onehundredthousandsth = ~0;
        int direction = 0;
        yodel_format_hppos2position(-1051533822, -87, &degrees, &minutes, &seconds, &onehundredthousandsth, &direction);
        assert(degrees == 105);
        assert(minutes == 9);
        assert(seconds == 12);
        assert(onehundredthousandsth == 17623);
        assert(direction == -1);
        /*
         * It pays to unit test: this uncovered a day-one bug in this function!
         */
    }

    {
        int32_t meters = ~0;
        uint32_t tenthousandths = ~0;
        yodel_format_hpalt2aaltitude(2345, 6, &meters, &tenthousandths);
        assert(meters == 2);
        assert(tenthousandths == 3456);
    }


    {
        int32_t meters = ~0;
        uint32_t tenthousandths = ~0;
        yodel_format_hpalt2aaltitude(-2345, -6, &meters, &tenthousandths);
        assert(meters == -2);
        assert(tenthousandths == 3456);
    }

    {
        int32_t meters = ~0;
        uint32_t tenthousandths = ~0;
        yodel_format_hpacc2accuracy(23456, &meters, &tenthousandths);
        assert(meters == 2);
        assert(tenthousandths == 3456);
    }


    {
        int32_t meters = ~0;
        uint32_t tenthousandths = ~0;
        yodel_format_hpacc2accuracy(-23456, &meters, &tenthousandths);
        assert(meters == -2);
        assert(tenthousandths == 3456);
    }
}
