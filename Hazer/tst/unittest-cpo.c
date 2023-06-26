/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the UBX unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * N.B. All of the test data in this unit test must be "cooked", as if it
 * had been processed by the CPO state machine, to have its DLE escape
 * sequences removed. See the sanity unit test for tests on raw data.
 */

#include <stdio.h>
#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <errno.h>
#include <string.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/calico.h"
#include "./unittest.h"

int main(void)
{
    calico_debug(stderr);

    /**************************************************************************/

    {
        assert(sizeof(calico_cpo_header_t) == CALICO_CPO_HEADER);
        assert(sizeof(calico_cpo_trailer_t) == CALICO_CPO_TRAILER);
        assert(sizeof(calico_cpo_pvt_packet_t) == CALICO_CPO_PVT_Length);
        assert(sizeof(calico_cpo_sdr_array_packet_t) == CALICO_CPO_SDR_Length);
        assert(sizeof(calico_cpo_sdr_packet_t) == (CALICO_CPO_SDR_Length / CALICO_CPO_SDR_Count));
        assert((sizeof(calico_cpo_header_t) + sizeof(calico_cpo_trailer_t)) == CALICO_CPO_SHORTEST);
    }

    /**************************************************************************/

    {
        calico_cpo_header_t header = { 0 };
        unsigned char * buffer;

        buffer = (unsigned char *)&header;

        buffer[CALICO_CPO_SYNC] = CALICO_STIMULUS_DLE;
        buffer[CALICO_CPO_ID] = CALICO_CPO_PVT_Id;
        buffer[CALICO_CPO_SIZE] = 0xa5;

        assert(header.sync == CALICO_STIMULUS_DLE);
        assert(header.id == CALICO_CPO_PVT_Id);
        assert(header.size == 0xa5);
    }

    /**************************************************************************/

    BEGIN("\\x10rT\\x05H\\rLT\\0\\a\\v\\xf0\\n\\x1f@\\0\\a\\f\\x8c\\n\\x17\\xb9\\0\\a\\r\\b\\a\\x0e\\x80\\0\\a\\x0f`\\t\\x0e\\xa2\\0\\a\\x14\\x80\\f23\\0\\a\\x19t\\x0e)\\xe0\\0\\a\\x1d\\xe4\\fAB\\x01\\a\\x12\\x9c\\xff\\x14\\x0e\\x01\\0\\x17\\x9c\\xff\\x01\\xd9\\0\\0\\x1a\\x9c\\xff\\tB\\x01\\0.\\xd8\\x0e%\\xd6\\0\\x10\\xaa\\x10\\x03");
        ssize_t bytes;
        int rc;
        uint8_t cc;
        uint8_t cs;
        const void * pointer;
        hazer_views_t views = HAZER_VIEWS_INITIALIZER;
        hazer_actives_t actives = HAZER_ACTIVES_INITIALIZER;

        bytes = calico_length(message, size);
        assert(bytes == (CALICO_CPO_SHORTEST + sizeof(calico_cpo_sdr_array_packet_t)));

        cc = cs = 0;
        pointer = calico_checksum_buffer(message, bytes, &cc, &cs);
        assert(pointer != (void *)0);
        assert(*((uint8_t *)pointer) == cs);

        bytes = calico_validate(message, bytes);
        assert(bytes == (CALICO_CPO_SHORTEST + sizeof(calico_cpo_sdr_array_packet_t)));

        rc = calico_is_cpo(message[0]);
        assert(rc);

        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_PVT_Id, CALICO_CPO_PVT_Length);
        assert(!rc);
        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_SDR_Id, CALICO_CPO_SDR_Length);
        assert(rc);

        rc = calico_cpo_satellite_data_record(views, actives, message, size);
        assert(rc == ((1 << HAZER_SYSTEM_GPS) | (1 << HAZER_SYSTEM_SBAS)));

/*
        CPO SDR[0]: svid=5 snr=3400 elev=76 azmth=84 status=0x7
        CPO SDR[1]: svid=11 snr=2800 elev=31 azmth=64 status=0x7
        CPO SDR[2]: svid=12 snr=2700 elev=23 azmth=185 status=0x7
        CPO SDR[3]: svid=13 snr=1800 elev=14 azmth=128 status=0x7
        CPO SDR[4]: svid=15 snr=2400 elev=14 azmth=162 status=0x7
        CPO SDR[5]: svid=20 snr=3200 elev=50 azmth=51 status=0x7
        CPO SDR[6]: svid=25 snr=3700 elev=41 azmth=224 status=0x7
        CPO SDR[7]: svid=29 snr=3300 elev=65 azmth=322 status=0x7
        CPO SDR[8]: svid=18 snr=65436 elev=20 azmth=270 status=0x0
        CPO SDR[9]: svid=23 snr=65436 elev=1 azmth=217 status=0x0
        CPO SDR[10]: svid=26 snr=65436 elev=9 azmth=322 status=0x0
        CPO SDR[11]: svid=46 snr=3800 elev=37 azmth=214 status=0x10
*/

        assert(strcmp(views[HAZER_SYSTEM_GPS].label, "CPO") == 0);
        assert(views[HAZER_SYSTEM_GPS].signals == 1);
        assert(views[HAZER_SYSTEM_GPS].signal == HAZER_SIGNAL_ANY);
        assert(views[HAZER_SYSTEM_GPS].pending == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].channels == 11);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].visible == 11);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].id == 5);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].elv_degrees == 76);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].azm_degrees == 84);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].snr_dbhz == 34);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[0].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].id == 11);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].elv_degrees == 31);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].azm_degrees == 64);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].snr_dbhz == 28);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[1].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].id == 12);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].elv_degrees == 23);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].azm_degrees == 185);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].snr_dbhz == 27);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[2].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].id == 13);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].elv_degrees == 14);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].azm_degrees == 128);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].snr_dbhz == 18);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[3].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].id == 15);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].elv_degrees == 14);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].azm_degrees == 162);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].snr_dbhz == 24);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[4].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].id == 20);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].elv_degrees == 50);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].azm_degrees == 51);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].snr_dbhz == 32);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[5].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].id == 25);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].elv_degrees == 41);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].azm_degrees == 224);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].snr_dbhz == 37);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[6].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].id == 29);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].elv_degrees == 65);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].azm_degrees == 322);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].snr_dbhz == 33);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].phantom == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].untracked == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[7].unused == 0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].id == 18);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].elv_degrees == 20);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].azm_degrees == 270);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].snr_dbhz == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].phantom == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].untracked == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[8].unused == !0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].id == 23);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].elv_degrees == 1);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].azm_degrees == 217);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].snr_dbhz == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].phantom == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].untracked == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[9].unused == !0);

        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].id == 26);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].elv_degrees == 9);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].azm_degrees == 322);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].snr_dbhz == 0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].phantom == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].untracked == !0);
        assert(views[HAZER_SYSTEM_GPS].sig[HAZER_SIGNAL_ANY].sat[10].unused == !0);

        assert(strcmp(views[HAZER_SYSTEM_SBAS].label, "CPO") == 0);
        assert(views[HAZER_SYSTEM_SBAS].signals == 1);
        assert(views[HAZER_SYSTEM_SBAS].signal == HAZER_SIGNAL_ANY);
        assert(views[HAZER_SYSTEM_SBAS].pending == 0);

        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].channels == 1);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].visible == 1);

        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].id == 46);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].elv_degrees == 37);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].azm_degrees == 214);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].snr_dbhz == 38);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].phantom == 0);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].untracked == 0);
        assert(views[HAZER_SYSTEM_SBAS].sig[HAZER_SIGNAL_ANY].sat[0].unused == 0);

        assert(strcmp(actives[HAZER_SYSTEM_GPS].label, "CPO") == 0);
        assert(actives[HAZER_SYSTEM_GPS].id[0] == 5);
        assert(actives[HAZER_SYSTEM_GPS].id[1] == 11);
        assert(actives[HAZER_SYSTEM_GPS].id[2] == 12);
        assert(actives[HAZER_SYSTEM_GPS].id[3] == 13);
        assert(actives[HAZER_SYSTEM_GPS].id[4] == 15);
        assert(actives[HAZER_SYSTEM_GPS].id[5] == 20);
        assert(actives[HAZER_SYSTEM_GPS].id[6] == 25);
        assert(actives[HAZER_SYSTEM_GPS].id[7] == 29);
        assert(actives[HAZER_SYSTEM_GPS].pdop == HAZER_GNSS_DOP);
        assert(actives[HAZER_SYSTEM_GPS].hdop == HAZER_GNSS_DOP);
        assert(actives[HAZER_SYSTEM_GPS].vdop == HAZER_GNSS_DOP);
        assert(actives[HAZER_SYSTEM_GPS].tdop == HAZER_GNSS_DOP);
        assert(actives[HAZER_SYSTEM_GPS].system == HAZER_SYSTEM_GPS);
        assert(actives[HAZER_SYSTEM_GPS].active == 8);
        assert(actives[HAZER_SYSTEM_GPS].mode == HAZER_MODE_UNKNOWN);

    END;

    /**************************************************************************/

    BEGIN("\\x103@\\x8a\\xd0\\xd3D\\xcb\\xe6\\x8dBBi\\x0eA\\xd0\\xc7\\x8cB\\x05\\0\\0\\0\\0\\0 \\b\\x03A\\xd9\\x1bu\\xf7\\xac9\\xe6\\?\\xa5o\\xf6\\x89F]\\xfd\\xbf\\xfe_*<Z\\r\\xf5;\\b)7\\xb9\\x05\\xf8\\x8fA\\x12\\0\\xbe/\\0\\0)\\x10\\x03");
        ssize_t bytes = 0;
        int rc = 0;
        uint8_t cc;
        uint8_t cs;
        const void * pointer;
        hazer_position_t position = HAZER_POSITION_INITIALIZER;

        bytes = calico_length(message, size);
        assert(bytes == (CALICO_CPO_SHORTEST + sizeof(calico_cpo_pvt_packet_t)));

        cc = cs = 0;
        pointer = calico_checksum_buffer(message, bytes, &cc, &cs);
        assert(pointer != (void *)0);
        assert(*((uint8_t *)pointer) == cs);

        bytes = calico_validate(message, bytes);
        assert(bytes == (CALICO_CPO_SHORTEST + sizeof(calico_cpo_pvt_packet_t)));

        rc = calico_is_cpo(message[0]);
        assert(rc);

        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_PVT_Id, CALICO_CPO_PVT_Length);
        assert(rc);
        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_SDR_Id, CALICO_CPO_SDR_Length);
        assert(!rc);

        rc = calico_cpo_position_record(&position, message, bytes);
        assert(rc == 0);

/*
        CPO PVT: qual=2 lat=2387654308557 lon=-6309201592241 alt=1676520 sep=-17996 dmy=1687132782000000000 utc=69508000000000 old=18446744073709551615 tot=1687202290000000000 label="CPO"
*/

        assert(strcmp(position.label, "CPO") == 0);
        assert(position.old_nanoseconds == 18446744073709551615ULL);
        assert(position.tot_nanoseconds == 1687202290000000000ULL);
        assert(position.utc_nanoseconds == 69508000000000ULL);
        assert(position.dmy_nanoseconds == 1687132782000000000ULL);
        assert(position.lat_nanominutes == 2387654308557LL);
        assert(position.lon_nanominutes == -6309201592241LL);
        assert(position.alt_millimeters == 1676520ULL);
        assert(position.sep_millimeters == -17996ULL);
        assert(position.quality == HAZER_QUALITY_DIFFERENTIAL);

    END;

    /**************************************************************************/

    return 0;
}
