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
    int64_t millimetersperhour = 0;
    uint16_t dop = 0;
    uint64_t number = 0;
    uint8_t digits = 0;
    char * end = (char *)0;

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

    assert(!hazer_is_valid_latitude(-5400000000001LL));
    assert(hazer_is_valid_latitude(-5400000000000LL));
    assert(hazer_is_valid_latitude(0));
    assert(hazer_is_valid_latitude(5400000000000LL));
    assert(!hazer_is_valid_latitude(5400000000001LL));

    assert(!hazer_is_valid_longitude(-10800000000001LL));
    assert(hazer_is_valid_longitude(-10800000000000LL));
    assert(hazer_is_valid_longitude(0));
    assert(hazer_is_valid_longitude(10800000000000LL));
    assert(!hazer_is_valid_longitude(10800000000001LL));

    assert(!hazer_is_valid_courseoverground(-360000000001LL));
    assert(hazer_is_valid_courseoverground(-360000000000LL));
    assert(hazer_is_valid_courseoverground(0));
    assert(hazer_is_valid_courseoverground(360000000000LL));
    assert(!hazer_is_valid_courseoverground(360000000001LL));

    assert(hazer_is_valid_dilutionofprecision(0));
    assert(hazer_is_valid_dilutionofprecision(9999));
    assert(!hazer_is_valid_dilutionofprecision(10000));

    assert(!hazer_is_valid_elevation(-91));
    assert(hazer_is_valid_elevation(-90));
    assert(hazer_is_valid_elevation(0));
    assert(hazer_is_valid_elevation(90));
    assert(!hazer_is_valid_elevation(91));

    assert(!hazer_is_valid_azimuth(-361));
    assert(hazer_is_valid_azimuth(-360));
    assert(hazer_is_valid_azimuth(0));
    assert(hazer_is_valid_azimuth(360));
    assert(!hazer_is_valid_azimuth(361));

    assert(hazer_is_valid_signaltonoiseratio(0));
    assert(hazer_is_valid_signaltonoiseratio(99));
    assert(!hazer_is_valid_signaltonoiseratio(100));

    /**************************************************************************/

    end = (char *)0;
    numerator = hazer_parse_fraction("", &denominator, &end);
    assert(numerator == 0);
    assert(denominator == 1);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("1", &denominator, &end);
    assert(numerator == 1);
    assert(denominator == 10);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("12", &denominator, &end);
    assert(numerator == 12);
    assert(denominator == 100);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("123", &denominator, &end);
    assert(numerator == 123);
    assert(denominator == 1000);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("1234", &denominator, &end);
    assert(numerator == 1234);
    assert(denominator == 10000);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("0000", &denominator, &end);
    assert(numerator == 0);
    assert(denominator == 10000);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("0009", &denominator, &end);
    assert(numerator == 9);
    assert(denominator == 10000);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("9000", &denominator, &end);
    assert(numerator == 9000);
    assert(denominator == 10000);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("1.5", &denominator, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    numerator = hazer_parse_fraction("1a5", &denominator, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    number = hazer_parse_utc("000000", &end);
    assert(number == 0LL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959", &end);
    assert(number == 86399000000000ULL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959.", &end);
    assert(number == 86399000000000ULL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959.0", &end);
    assert(number == 86399000000000ULL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959.125", &end);
    assert(number == 86399125000000ULL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959c125", &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    number = hazer_parse_utc("235959.125d", &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    number = hazer_parse_dmy("310117", &end);
    assert(number == 1485820800000000000ULL);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    number = hazer_parse_dmy("310117.", &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    number = hazer_parse_dmy("310117d", &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    nanominutes = hazer_parse_latlon("00000", 'E', &digits, &end);
    assert(nanominutes == 0LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("00000", 'W', &digits, &end);
    assert(nanominutes == 0LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("0000", 'N', &digits, &end);
    assert(nanominutes == 0LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("0000", 'S', &digits, &end);
    assert(nanominutes == 0LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("18000", 'E', &digits, &end);
    assert(nanominutes == 10800000000000LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("18000", 'W', &digits, &end);
    assert(nanominutes == -10800000000000LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("9000", 'N', &digits, &end);
    assert(nanominutes == 5400000000000LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("9000", 'S', &digits, &end);
    assert(nanominutes == -5400000000000LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("17959.99999", 'E', &digits, &end);
    assert(nanominutes == 10799999990000LL);
    assert(digits == 10);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("17959.99999", 'W', &digits, &end);
    assert(nanominutes == -10799999990000LL);
    assert(digits == 10);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_longitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("8959.99999", 'N', &digits, &end);
    assert(nanominutes == 5399999990000LL);
    assert(digits == 9);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("8959.99999", 'S', &digits, &end);
    assert(nanominutes == -5399999990000LL);
    assert(digits == 9);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_latitude(nanominutes));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("8959;99999", 'S', &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("8959.99999,", 'S', &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    nanominutes = hazer_parse_latlon("8959.99999", 'X', &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    nanodegrees = hazer_parse_cog("0", &digits, &end);
    assert(nanodegrees == 0LL);
    assert(digits == 1);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("360", &digits, &end);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("360.", &digits, &end);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("360.0", &digits, &end);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("360.00", &digits, &end);
    assert(nanodegrees == 360000000000LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("90.5", &digits, &end);
    assert(nanodegrees == 90500000000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("90.25", &digits, &end);
    assert(nanodegrees == 90250000000LL);
    assert(digits == 4);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("90.125", &digits, &end);
    assert(nanodegrees == 90125000000LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("-90.125", &digits, &end);
    assert(nanodegrees == -90125000000LL);
    assert(digits == 5);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_courseoverground(nanodegrees));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("-90,125", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    nanodegrees = hazer_parse_cog("-90.12;", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    microknots = hazer_parse_sog("15.5", &digits, &end);
    assert(microknots == 15500000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    microknots = hazer_parse_sog("-15.5", &digits, &end);
    assert(microknots == -15500000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    microknots = hazer_parse_sog("-15;5", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    microknots = hazer_parse_sog("-15.5?", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    millimetersperhour = hazer_parse_smm("15.5", &digits, &end);
    assert(millimetersperhour == 15500000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimetersperhour = hazer_parse_smm("-15.5", &digits, &end);
    assert(millimetersperhour == -15500000LL);
    assert(digits == 3);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimetersperhour = hazer_parse_smm("-15;5", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    millimetersperhour = hazer_parse_smm("-15.5?", &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    millimeters = hazer_parse_alt("", 'M', &digits, &end);
    assert(millimeters == 0ULL);
    assert(digits == 0);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("0", 'M', &digits, &end);
    assert(millimeters == 0ULL);
    assert(digits == 1);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("0.", 'M', &digits, &end);
    assert(millimeters == 0ULL);
    assert(digits == 1);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("0.0", 'M', &digits, &end);
    assert(millimeters == 0ULL);
    assert(digits == 2);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("521.125", 'M', &digits, &end);
    assert(millimeters == 521125ULL);
    assert(digits == 6);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("-521.125", 'M', &digits, &end);
    assert(millimeters == -521125ULL);
    assert(digits == 6);
    assert((end != (char *)0) && (*end == '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("-521/125", 'M', &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    millimeters = hazer_parse_alt("-521.125;", 'M', &digits, &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    end = (char *)0;
    dop = hazer_parse_dop("", &end);
    assert(dop == 9999);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    dop = hazer_parse_dop("0", &end);
    assert(dop == 0);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("1", &end);
    assert(dop == 100);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("2.", &end);
    assert(dop == 200);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("3.4", &end);
    assert(dop == 340);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("56.78", &end);
    assert(dop == 5678);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("99.99", &end);
    assert(dop == 9999);
    assert((end != (char *)0) && (*end == '\0'));
    assert(hazer_is_valid_dilutionofprecision(dop));

    end = (char *)0;
    dop = hazer_parse_dop("99:99", &end);
    assert((end != (char *)0) && (*end != '\0'));

    end = (char *)0;
    dop = hazer_parse_dop("99.9!9", &end);
    assert((end != (char *)0) && (*end != '\0'));

    /**************************************************************************/

    return 0;
}
