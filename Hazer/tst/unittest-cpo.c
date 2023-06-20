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

        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_PVT_Id, CALICO_CPO_PVT_Length);
        assert(!rc);
        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_SDR_Id, CALICO_CPO_SDR_Length);
        assert(rc);

        rc = calico_cpo_satellite_data_record(views, actives, message, size);
        assert(rc == ((1 << HAZER_SYSTEM_GPS) | (1 << HAZER_SYSTEM_SBAS)));

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

        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_PVT_Id, CALICO_CPO_PVT_Length);
        assert(rc);
        rc = calico_is_cpo_id_length(message, bytes, CALICO_CPO_SDR_Id, CALICO_CPO_SDR_Length);
        assert(!rc);

        rc = calico_cpo_position_record(&position, message, bytes);
        assert(rc == 0);

    END;

    /**************************************************************************/

    return 0;
}
