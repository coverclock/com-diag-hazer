/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the Parse unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

int main(void)
{
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    uint8_t constellation = 0;
    uint16_t id = 0;
    int index = 0;
    char * string = (char *)0;
    uint64_t numerator = 0;
    uint64_t denominator = 0;
    int64_t nanominutes = 0;
    int64_t nanodegrees = 0;
    int64_t millimeters = 0;
    int64_t microknots = 0;
    uint16_t dop = 0;
    uint64_t number = 0;
    uint8_t digits = 0;

    /**************************************************************************/

#define UNITTEST_PARSE_TALKER_SYSTEM(_STRING_, _TALKER_, _SYSTEM_) \
    do { \
        string = "$" _STRING_; \
        talker = hazer_parse_talker(string); \
        assert(talker == _TALKER_); \
        system = hazer_map_talker_to_system(talker); \
        assert(system == _SYSTEM_); \
        assert((_TALKER_ == HAZER_TALKER_TOTAL) || (strcmp(_STRING_, HAZER_TALKER_NAME[_TALKER_]) == 0)); \
    } while (0)

    UNITTEST_PARSE_TALKER_SYSTEM("GN", HAZER_TALKER_GNSS, HAZER_SYSTEM_GNSS);
    UNITTEST_PARSE_TALKER_SYSTEM("GP", HAZER_TALKER_GPS, HAZER_SYSTEM_GPS);
    UNITTEST_PARSE_TALKER_SYSTEM("GL", HAZER_TALKER_GLONASS, HAZER_SYSTEM_GLONASS);
    UNITTEST_PARSE_TALKER_SYSTEM("GA", HAZER_TALKER_GALILEO, HAZER_SYSTEM_GALILEO);
    UNITTEST_PARSE_TALKER_SYSTEM("ZV", HAZER_TALKER_RADIO, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("GB", HAZER_TALKER_BEIDOU1, HAZER_SYSTEM_BEIDOU);
    UNITTEST_PARSE_TALKER_SYSTEM("BD", HAZER_TALKER_BEIDOU2, HAZER_SYSTEM_BEIDOU);
    UNITTEST_PARSE_TALKER_SYSTEM("CD", HAZER_TALKER_DSC, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("EC", HAZER_TALKER_ECDIS, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("PMTK", HAZER_TALKER_PMTK, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("PSRF", HAZER_TALKER_PSRF, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("PUBX", HAZER_TALKER_PUBX, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("??", HAZER_TALKER_TOTAL, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("???", HAZER_TALKER_TOTAL, HAZER_SYSTEM_TOTAL);
    UNITTEST_PARSE_TALKER_SYSTEM("", HAZER_TALKER_TOTAL, HAZER_SYSTEM_TOTAL);

    /**************************************************************************/

#define UNITTEST_PARSE_NMEA_SYSTEM(_NMEA_, _SYSTEM_) \
    if (constellation == (_NMEA_)) { assert(hazer_map_nmea_to_system(constellation) == (_SYSTEM_)); continue; }

    for (constellation = 0, index = 0; index <= 0xff; ++index, ++constellation) {
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_GPS, HAZER_SYSTEM_GPS);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_GLONASS, HAZER_SYSTEM_GLONASS);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_GALILEO, HAZER_SYSTEM_GALILEO);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_BEIDOU, HAZER_SYSTEM_BEIDOU);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_SBAS, HAZER_SYSTEM_SBAS);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_IMES, HAZER_SYSTEM_IMES);
        UNITTEST_PARSE_NMEA_SYSTEM(HAZER_NMEA_QZSS, HAZER_SYSTEM_QZSS);
        assert(hazer_map_nmea_to_system(constellation) == HAZER_SYSTEM_TOTAL);
    }

    /**************************************************************************/

#define UNITTEST_PARSE_NMEAID_SYSTEM(_MIN_, _MAX_, _SYSTEM_) \
    if (((_MIN_) <= id) && (id <= (_MAX_))) { assert(hazer_map_nmeaid_to_system(id) == (_SYSTEM_)); continue; }

    for (id = 0, index = 0; index <= 0xffff; ++index, ++id) {
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_GPS_FIRST, HAZER_NMEA_GPS_LAST, HAZER_SYSTEM_GPS);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_SBAS_FIRST, HAZER_NMEA_SBAS_LAST, HAZER_SYSTEM_SBAS);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_GLONASS_FIRST, HAZER_NMEA_GLONASS_LAST, HAZER_SYSTEM_GLONASS);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_IMES_FIRST, HAZER_NMEA_IMES_LAST, HAZER_SYSTEM_IMES);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_QZSS_FIRST, HAZER_NMEA_QZSS_LAST, HAZER_SYSTEM_QZSS);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_BEIDOU1_FIRST, HAZER_NMEA_BEIDOU1_LAST, HAZER_SYSTEM_BEIDOU);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_GALILEO_FIRST, HAZER_NMEA_GALILEO_LAST, HAZER_SYSTEM_GALILEO);
        UNITTEST_PARSE_NMEAID_SYSTEM(HAZER_NMEA_BEIDOU2_FIRST, HAZER_NMEA_BEIDOU2_LAST, HAZER_SYSTEM_BEIDOU);
        assert(hazer_map_nmeaid_to_system(id) == HAZER_SYSTEM_TOTAL);
    }

    /**************************************************************************/

#define UNITTEST_PARSE_PUBXID_SYSTEM(_MIN_, _MAX_, _SYSTEM_) \
    if (((_MIN_) <= id) && (id <= (_MAX_))) { assert(hazer_map_pubxid_to_system(id) == (_SYSTEM_)); continue; }

    for (id = 0, index = 0; index <= 0xffff; ++index, ++id) {
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_GPS_FIRST, HAZER_PUBX_GPS_LAST, HAZER_SYSTEM_GPS);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_BEIDOU1_FIRST, HAZER_PUBX_BEIDOU1_LAST, HAZER_SYSTEM_BEIDOU);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_GLONASS1_FIRST, HAZER_PUBX_GLONASS1_LAST, HAZER_SYSTEM_GLONASS);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_SBAS_FIRST, HAZER_PUBX_SBAS_LAST, HAZER_SYSTEM_SBAS);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_GALILEO_FIRST, HAZER_PUBX_GALILEO_LAST, HAZER_SYSTEM_GALILEO);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_BEIDOU2_FIRST, HAZER_PUBX_BEIDOU2_LAST, HAZER_SYSTEM_BEIDOU);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_IMES_FIRST, HAZER_PUBX_IMES_LAST, HAZER_SYSTEM_IMES);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_QZSS_FIRST, HAZER_PUBX_QZSS_LAST, HAZER_SYSTEM_QZSS);
        UNITTEST_PARSE_PUBXID_SYSTEM(HAZER_PUBX_GLONASS2_FIRST, HAZER_PUBX_GLONASS2_LAST, HAZER_SYSTEM_GLONASS);
        assert(hazer_map_pubxid_to_system(id) == HAZER_SYSTEM_TOTAL);
    }

    /**************************************************************************/

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                        HAZER_NMEA_GPS_FIRST,
                        HAZER_NMEA_GPS_FIRST + 1,
                        HAZER_NMEA_GPS_FIRST + 2,
                        HAZER_NMEA_GPS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                4,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GPS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_SBAS_FIRST,
                    HAZER_NMEA_SBAS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                2,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_SBAS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_SBAS_FIRST,
                    HAZER_NMEA_GPS_FIRST,
                    HAZER_NMEA_SBAS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GPS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_GPS_FIRST,
                    HAZER_NMEA_SBAS_FIRST,
                    HAZER_NMEA_SBAS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GPS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_GLONASS_FIRST,
                    HAZER_NMEA_GLONASS_FIRST + 1,
                    HAZER_NMEA_GLONASS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GLONASS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_BEIDOU1_FIRST,
                    HAZER_NMEA_BEIDOU1_FIRST + 1,
                    HAZER_NMEA_BEIDOU1_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_BEIDOU);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_BEIDOU2_FIRST,
                    HAZER_NMEA_BEIDOU2_FIRST + 1,
                    HAZER_NMEA_BEIDOU2_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_BEIDOU);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_QZSS_FIRST,
                    HAZER_NMEA_QZSS_FIRST + 1,
                    HAZER_NMEA_QZSS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                3,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_QZSS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_GPS_FIRST,
                    HAZER_NMEA_GPS_LAST,
                    HAZER_NMEA_GLONASS_FIRST,
                    HAZER_NMEA_GLONASS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                4,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GNSS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    HAZER_NMEA_GPS_FIRST,
                    HAZER_NMEA_GLONASS_FIRST,
                    HAZER_NMEA_GPS_LAST,
                    HAZER_NMEA_GLONASS_LAST,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                4,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GNSS);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    1,
                    2,
                    3,
                    4,
                    5,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_GALILEO,
                5,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_GALILEO);

    }

    {
        static const hazer_active_t ACTIVE = {
                NULL,
                {
                    97,
                    98,
                    99,
                    100,
                },
                HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP, HAZER_GNSS_DOP,
                HAZER_SYSTEM_TOTAL,
                4,
        };

        assert(hazer_map_active_to_system(&ACTIVE) == HAZER_SYSTEM_TOTAL);

    }

    /**************************************************************************/

#if defined(DEPRECATED)
    {
        static const hazer_view_t VIEW[HAZER_SYSTEM_TOTAL] = {
                { NULL }, /* GNSS */
                {
                    NULL,
                    {
                        { HAZER_NMEA_GPS_FIRST, 0, 0, 0 },
                        { HAZER_NMEA_GPS_FIRST + 1, 0, 0, 0 },
                        { HAZER_NMEA_GPS_FIRST + 2, 0, 0, 0 },
                        { HAZER_NMEA_GPS_LAST, 0, 0, 0 },
                    },
                    4,
                    0,
                    0,
                },
                {
                    NULL,
                    {
                        { HAZER_NMEA_GLONASS_FIRST, 0, 0, 0 },
                        { HAZER_NMEA_GLONASS_FIRST + 1, 0, 0, 0 },
                        { HAZER_NMEA_GLONASS_LAST, 0, 0, 0 },
                    },
                    3,
                    0,
                    0,
                },
                { NULL }, /* GALILEO */
                { NULL }, /* BEIDOU */
                {
                    NULL,
                    {
                        { HAZER_NMEA_SBAS_FIRST, 0, 0, 0},
                        { HAZER_NMEA_SBAS_LAST, 0, 0, 0 },
                    },
                    2,
                    0,
                    0,
                },
                { NULL }, /* IMES */
                { NULL }, /* 7 */
                { NULL }, /* 8 */
                { NULL }, /* 9 */
                { NULL }, /* 10 */
                { NULL }, /* 11 */
                { NULL }, /* 12 */
                { NULL }, /* 13 */
                { NULL }, /* 14 */
                { NULL }, /* QZSS */
        };

        assert(hazer_map_svid_to_system(HAZER_NMEA_GPS_FIRST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GPS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GPS_FIRST + 1, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GPS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GPS_FIRST + 2, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GPS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GPS_FIRST + 3, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_TOTAL);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GPS_LAST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GPS);

        assert(hazer_map_svid_to_system(HAZER_NMEA_GLONASS_FIRST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GLONASS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GLONASS_FIRST + 1, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GLONASS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GLONASS_FIRST + 2, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_TOTAL);
        assert(hazer_map_svid_to_system(HAZER_NMEA_GLONASS_LAST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_GLONASS);

        assert(hazer_map_svid_to_system(HAZER_NMEA_SBAS_FIRST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_SBAS);
        assert(hazer_map_svid_to_system(HAZER_NMEA_SBAS_FIRST + 1, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_TOTAL);
        assert(hazer_map_svid_to_system(HAZER_NMEA_SBAS_LAST, VIEW, sizeof(VIEW) / sizeof(VIEW[0])) == HAZER_SYSTEM_SBAS);

    }
#endif

    /**************************************************************************/

    numerator = hazer_parse_fraction("", &denominator);
    assert(numerator == 0);
    assert(denominator == 1);

    numerator = hazer_parse_fraction("1", &denominator);
    assert(numerator == 1);
    assert(denominator == 10);

    numerator = hazer_parse_fraction("12", &denominator);
    assert(numerator == 12);
    assert(denominator == 100);

    numerator = hazer_parse_fraction("123", &denominator);
    assert(numerator == 123);
    assert(denominator == 1000);

    numerator = hazer_parse_fraction("1234", &denominator);
    assert(numerator == 1234);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("0000", &denominator);
    assert(numerator == 0);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("0009", &denominator);
    assert(numerator == 9);
    assert(denominator == 10000);

    numerator = hazer_parse_fraction("9000", &denominator);
    assert(numerator == 9000);
    assert(denominator == 10000);

    /**************************************************************************/

    number = hazer_parse_utc("000000");
    assert(number == 0LL);

    number = hazer_parse_utc("235959");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.0");
    assert(number == 86399000000000ULL);

    number = hazer_parse_utc("235959.125");
    assert(number == 86399125000000ULL);

    /**************************************************************************/

    number = hazer_parse_dmy("310117");
    assert(number == 1485820800000000000ULL);

    /**************************************************************************/

    nanominutes = hazer_parse_latlon("00000", 'E', &digits);
    assert(nanominutes == 0LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("00000", 'S', &digits);
    assert(nanominutes == 0LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18000", 'E', &digits);
    assert(nanominutes == 10800000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18000", 'S', &digits);
    assert(nanominutes == -10800000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18030", 'E', &digits);
    assert(nanominutes == 10830000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18030", 'S', &digits);
    assert(nanominutes == -10830000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18030.", 'E', &digits);
    assert(nanominutes == 10830000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18030.", 'W', &digits);
    assert(nanominutes == -10830000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("18030.60", 'E', &digits);
    assert(nanominutes == 10830600000000LL);
    assert(digits == 7);

    nanominutes = hazer_parse_latlon("18030.60", 'W', &digits);
    assert(nanominutes == -10830600000000LL);
    assert(digits == 7);

    nanominutes = hazer_parse_latlon("0000", 'N', &digits);
    assert(nanominutes == 0LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("0000", 'S', &digits);
    assert(nanominutes == 0LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9000", 'N', &digits);
    assert(nanominutes == 5400000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9000", 'S', &digits);
    assert(nanominutes == -5400000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9030", 'N', &digits);
    assert(nanominutes == 5430000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9030", 'S', &digits);
    assert(nanominutes == -5430000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9030.", 'N', &digits);
    assert(nanominutes == 5430000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9030.", 'S', &digits);
    assert(nanominutes == -5430000000000LL);
    assert(digits == 4);

    nanominutes = hazer_parse_latlon("9030.0", 'N', &digits);
    assert(nanominutes == 5430000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("9030.0", 'S', &digits);
    assert(nanominutes == -5430000000000LL);
    assert(digits == 5);

    nanominutes = hazer_parse_latlon("9030.60", 'N', &digits);
    assert(nanominutes == 5430600000000LL);
    assert(digits == 6);

    nanominutes = hazer_parse_latlon("9030.60", 'S', &digits);
    assert(nanominutes == -5430600000000LL);
    assert(digits == 6);

    nanominutes = hazer_parse_latlon("9030.66", 'N', &digits);
    assert(nanominutes == 5430660000000LL);
    assert(digits == 6);

    /**************************************************************************/

    nanodegrees = hazer_parse_cog("0", &digits);
    assert(nanodegrees == 0LL);
    assert(digits == 1);

    nanodegrees = hazer_parse_cog("360", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("360.", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("360.0", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_cog("360.00", &digits);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_cog("90.5", &digits);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 3);

    nanodegrees = hazer_parse_cog("90.25", &digits);
    assert(nanodegrees == 90250000000LL);
    assert(digits == 4);

    nanodegrees = hazer_parse_cog("90.125", &digits);
    assert(nanodegrees == 90125000000LL);
    assert(digits == 5);

    nanodegrees = hazer_parse_cog("-90.125", &digits);
    assert(nanodegrees == -90125000000LL);
    assert(digits == 5);

    /**************************************************************************/

    microknots = hazer_parse_sog("15.5", &digits);
    assert(microknots == 15500000LL);
    assert(digits == 3);

    microknots = hazer_parse_sog("-15.5", &digits);
    assert(microknots == -15500000LL);
    assert(digits == 3);

    /**************************************************************************/

    millimeters = hazer_parse_alt("", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 0);

    millimeters = hazer_parse_alt("0", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 1);

    millimeters = hazer_parse_alt("0.", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 1);

    millimeters = hazer_parse_alt("0.0", 'M', &digits);
    assert(millimeters == 0ULL);
    assert(digits == 2);

    millimeters = hazer_parse_alt("521.125", 'M', &digits);
    assert(millimeters == 521125ULL);
    assert(digits == 6);

    millimeters = hazer_parse_alt("-521.125", 'M', &digits);
    assert(millimeters == -521125ULL);
    assert(digits == 6);

    /**************************************************************************/

    dop = hazer_parse_dop("");
    assert(dop == 9999);

    dop = hazer_parse_dop("-1");
    assert(dop == 9999);

    dop = hazer_parse_dop("1");
    assert(dop == 100);

    dop = hazer_parse_dop("2.");
    assert(dop == 200);

    dop = hazer_parse_dop("3.4");
    assert(dop == 340);

    dop = hazer_parse_dop("56.78");
    assert(dop == 5678);

    dop = hazer_parse_dop("99.99");
    assert(dop == 9999);

    dop = hazer_parse_dop("100");
    assert(dop == 9999);

    /**************************************************************************/

    return 0;
}
