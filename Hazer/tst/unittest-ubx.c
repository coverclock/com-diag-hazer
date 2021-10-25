/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2018-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the UBX unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>
#include <string.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

int main(void)
{
    yodel_debug(stderr);

    /**************************************************************************/

    {
        assert(sizeof(yodel_ubx_header_t) == (YODEL_UBX_UNSUMMED + YODEL_UBX_SUMMED));
        assert(sizeof(yodel_ubx_mon_hw_t) == YODEL_UBX_MON_HW_Length);
        assert(sizeof(yodel_ubx_nav_status_t) == YODEL_UBX_NAV_STATUS_Length);
        assert(sizeof(yodel_ubx_ack_t) == (YODEL_UBX_ACK_Length + sizeof(uint8_t)));
        assert(sizeof(yodel_ubx_cfg_valget_t) == YODEL_UBX_CFG_VALGET_Length);
        assert(sizeof(yodel_ubx_nav_svin_t) == YODEL_UBX_NAV_SVIN_Length);
        assert(sizeof(yodel_ubx_nav_status_t) == YODEL_UBX_NAV_STATUS_Length);
        assert(sizeof(yodel_ubx_rxm_rtcm_t) == YODEL_UBX_RXM_RTCM_Length);
        assert(sizeof(yodel_ubx_nav_att_t) == YODEL_UBX_NAV_ATT_Length);
        assert(sizeof(yodel_ubx_nav_odo_t) == YODEL_UBX_NAV_ODO_Length);
        assert(sizeof(yodel_ubx_nav_pvt_t) == YODEL_UBX_NAV_PVT_Length);
    }

    /**************************************************************************/

    {
        yodel_ubx_header_t header = { 0 };
        unsigned char * buffer;

        buffer = (unsigned char *)&header;

        buffer[YODEL_UBX_SYNC_1] = YODEL_STIMULUS_SYNC_1;
        buffer[YODEL_UBX_SYNC_2] = YODEL_STIMULUS_SYNC_2;
        buffer[YODEL_UBX_CLASS] = 0x11;
        buffer[YODEL_UBX_ID] = 0x22;
        buffer[YODEL_UBX_LENGTH_LSB] = 0x33;
        buffer[YODEL_UBX_LENGTH_MSB] = 0x44;

        assert(header.sync_1 == YODEL_STIMULUS_SYNC_1);
        assert(header.sync_2 == YODEL_STIMULUS_SYNC_2);
        assert(header.classx == 0x11);
        assert(header.id == 0x22);
        assert(le16toh(header.length) == 0x4433);
    }

    /**************************************************************************/

    {
        union { uint64_t integer; uint8_t byte[sizeof(uint64_t)]; } u64 = { 0x1122334455667788ULL };
        union { uint32_t integer; uint8_t byte[sizeof(uint32_t)]; } u32 = { 0x11223344UL };
        union { uint16_t integer; uint8_t byte[sizeof(uint16_t)]; } u16 = { 0x1122U };
        union { uint8_t  integer; uint8_t byte[sizeof(uint8_t)];  } u8 =  { 0x11U };

#if __BYTE_ORDER == __LITTLE_ENDIAN

        assert(u64.byte[0] == 0x88);
        assert(u64.byte[1] == 0x77);
        assert(u64.byte[2] == 0x66);
        assert(u64.byte[3] == 0x55);
        assert(u64.byte[4] == 0x44);
        assert(u64.byte[5] == 0x33);
        assert(u64.byte[6] == 0x22);
        assert(u64.byte[7] == 0x11);

        assert(u32.byte[0] == 0x44);
        assert(u32.byte[1] == 0x33);
        assert(u32.byte[2] == 0x22);
        assert(u32.byte[3] == 0x11);

        assert(u16.byte[0] == 0x22);
        assert(u16.byte[1] == 0x11);

        assert(u8.byte[0]  == 0x11);

#else

        assert(u64.byte[0] == 0x11);
        assert(u64.byte[1] == 0x22);
        assert(u64.byte[2] == 0x33);
        assert(u64.byte[3] == 0x44);
        assert(u64.byte[4] == 0x55);
        assert(u64.byte[5] == 0x66);
        assert(u64.byte[6] == 0x77);
        assert(u64.byte[7] == 0x88);

        assert(u32.byte[0] == 0x11);
        assert(u32.byte[1] == 0x22);
        assert(u32.byte[2] == 0x33);
        assert(u32.byte[3] == 0x44);

        assert(u16.byte[0] == 0x11);
        assert(u16.byte[1] == 0x22);

        assert(u8.byte[0]  == 0x11);

#endif

        COM_DIAG_YODEL_LETOH(u64.integer);
        COM_DIAG_YODEL_LETOH(u32.integer);
        COM_DIAG_YODEL_LETOH(u16.integer);
        COM_DIAG_YODEL_LETOH(u8.integer);

        /*
         * If the host on which this is running is also Little Endian,
         * the bytes will not have changed and so will be the same as the
         * Little Endian segment above; otherwise, they will be reversed
         * with respect to the Big Endian (not Little Endian) segment above.
         */

        assert(u64.byte[0] == 0x88);
        assert(u64.byte[1] == 0x77);
        assert(u64.byte[2] == 0x66);
        assert(u64.byte[3] == 0x55);
        assert(u64.byte[4] == 0x44);
        assert(u64.byte[5] == 0x33);
        assert(u64.byte[6] == 0x22);
        assert(u64.byte[7] == 0x11);

        assert(u32.byte[0] == 0x44);
        assert(u32.byte[1] == 0x33);
        assert(u32.byte[2] == 0x22);
        assert(u32.byte[3] == 0x11);

        assert(u16.byte[0] == 0x22);
        assert(u16.byte[1] == 0x11);

        assert(u8.byte[0]  == 0x11);

        COM_DIAG_YODEL_HTOLE(u64.integer);
        COM_DIAG_YODEL_HTOLE(u32.integer);
        COM_DIAG_YODEL_HTOLE(u16.integer);
        COM_DIAG_YODEL_HTOLE(u8.integer);

        /*
         * Now the bytes should be back in their original order.
         */

#if __BYTE_ORDER == __LITTLE_ENDIAN

        assert(u64.byte[0] == 0x88);
        assert(u64.byte[1] == 0x77);
        assert(u64.byte[2] == 0x66);
        assert(u64.byte[3] == 0x55);
        assert(u64.byte[4] == 0x44);
        assert(u64.byte[5] == 0x33);
        assert(u64.byte[6] == 0x22);
        assert(u64.byte[7] == 0x11);

        assert(u32.byte[0] == 0x44);
        assert(u32.byte[1] == 0x33);
        assert(u32.byte[2] == 0x22);
        assert(u32.byte[3] == 0x11);

        assert(u16.byte[0] == 0x22);
        assert(u16.byte[1] == 0x11);

        assert(u8.byte[0]  == 0x11);

#else

        assert(u64.byte[0] == 0x11);
        assert(u64.byte[1] == 0x22);
        assert(u64.byte[2] == 0x33);
        assert(u64.byte[3] == 0x44);
        assert(u64.byte[4] == 0x55);
        assert(u64.byte[5] == 0x66);
        assert(u64.byte[6] == 0x77);
        assert(u64.byte[7] == 0x88);

        assert(u32.byte[0] == 0x11);
        assert(u32.byte[1] == 0x22);
        assert(u32.byte[2] == 0x33);
        assert(u32.byte[3] == 0x44);

        assert(u16.byte[0] == 0x11);
        assert(u16.byte[1] == 0x22);

        assert(u8.byte[0]  == 0x11);

#endif

    }

    /**************************************************************************/

    BEGIN("\\xb5b\\x05\\x00\\x02\\0\\x06\\x8a\\x98\\xc1");
        yodel_ubx_ack_t data = YODEL_UBX_ACK_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_ACK_Class, YODEL_UBX_ACK_NAK_Id));
        assert(yodel_ubx_ack(&data, message, size) == 0);
        assert(data.state == 0);
    END;

    BEGIN("\\xb5b\\x05\\x01\\x02\\0\\x06\\x8b\\x99\\xc2");
        yodel_ubx_ack_t data = YODEL_UBX_ACK_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_ACK_Class, YODEL_UBX_ACK_ACK_Id));
        assert(yodel_ubx_ack(&data, message, size) == 0);
        assert(data.state == !0);
    END;

    BEGIN("\\xb5b\\x06\\x8b\\f\\0\\x01\\0\\0\\0\\x11\\0\\x03@\\xa0\\x86\\x01\\0\\x19'");
        yodel_ubx_cfg_valget_t data = YODEL_UBX_CFG_VALGET_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_CFG_VALGET_Class, YODEL_UBX_CFG_VALGET_Id));
        assert(yodel_ubx_cfg_valget(message, size) == 0);
    END;

    BEGIN("\\xb5b\\x06\\x8b\\t\\0\\x01\\0\\0\\0\\xbf\\x02\\x91 \\x01\\x0e\\xf5");
        yodel_ubx_cfg_valget_t data = YODEL_UBX_CFG_VALGET_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_CFG_VALGET_Class, YODEL_UBX_CFG_VALGET_Id));
        assert(yodel_ubx_cfg_valget(message, size) == 0);
    END;

    BEGIN("\\xb5b\\n\\t<\\0\\xc1\\x81\\0\\0\\0\\0\\x01\\0\\0\\x80\\0\\0\\xdfg\\0\\0L\\0\\x91\\x14\\x01\\x02\\x01\\x85\\xbe\\xff\\x01\\0\\xff\\0\\x01\\x03\\x02\\x10\\xff\\x12\\x13\\x14\\x15\\x0e\\n\\v\\x0fD\\x16\\x05\\xeeZ\\0\\0\\0\\0\\xdb{\\0\\0\\0\\0\\0\\0!M");
        yodel_ubx_mon_hw_t data = YODEL_UBX_MON_HW_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_MON_HW_Class, YODEL_UBX_MON_HW_Id));
        assert(yodel_ubx_mon_hw(&data, message, size) == 0);
    END;

    BEGIN("\\xb5b\\n\\x04\\xdc\\0EXT CORE 1.00 (94e56e)\\0\\0\\0\\0\\0\\0\\0\\000190000\\0\\0ROM BASE 0x118B2060\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0FWVER=HPG 1.11\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0PROTVER=27.10\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0MOD=ZED-F9P\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0GPS;GLO;GAL;BDS\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0QZSS\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x9au");
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_MON_VER_Class, YODEL_UBX_MON_VER_Id));
        assert(yodel_ubx_mon_ver(message, size) == 0);
    END;

    BEGIN("\\xb5b\\x01\\x03\\x10\\0h\\x15i\\x0f\\x05\\xdd\\0\\bkn\\0\\0\\xde\\x1e\\xbf\\0\\x87V");
        yodel_ubx_nav_status_t data = YODEL_UBX_NAV_STATUS_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_NAV_STATUS_Class, YODEL_UBX_NAV_STATUS_Id));
        assert(yodel_ubx_nav_status(&data, message, size) == 0);
    END;

    BEGIN("\\xb5b\\x01;(\\0\\0\\0\\0\\0\\xf8\\x83\\xac\\x0e<\\0\\0\\0\\xb7\\x14Z\\xf8hh\\xc2\\xe3\\x8ai5\\x18\\xe9\\xf1\\xf2\\0\\xe6\\x1a\\x01\\0=\\0\\0\\0\\x01\\0\\0\\0\\xb2\\x1f");
        yodel_ubx_nav_svin_t data = YODEL_UBX_NAV_SVIN_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_NAV_SVIN_Class, YODEL_UBX_NAV_SVIN_Id));
        assert(yodel_ubx_nav_svin(&data, message, size) == 0);
    END;

    BEGIN("\\xb5b\\x022\\b\\0\\x02\\0\\0\\0\\0\\0\\xce\\x04\\x10>");
        yodel_ubx_rxm_rtcm_t data = YODEL_UBX_RXM_RTCM_INITIALIZER;
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_RXM_RTCM_Class, YODEL_UBX_RXM_RTCM_Id));
        assert(yodel_ubx_rxm_rtcm(&data, message, size) == 0);
    END;

    BEGIN("\\xb5b\\n6\\xa8\\0\\0\\x04\\0\\0\\0\\x01\\x05\\xff\\0\\x01\\0\\0\\xec8\\0\\0\\0\\x0e\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x02\\0\\0H\\x8dV\\x01\\0\\t\\0\\0\\x80\\x1f\\xf2\\x03\\x05\\r\\0\\0\\xc1\\xdc\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x1b\\0\\0\\0\\0\\x03\\0\\0\\x84\\xf5p\\0\\x014\\0\\0\\xc8\\x03\\0\\0\\0\\0\\0\\0C\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x01\\x01\\0\\0>\\xff\\xf1\\x03\\0\\0\\0\\0\\x1e\\x1bP\\x01\\x06\\n\\0\\0Py\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0~(");
        yodel_ubx_mon_comms_t data = YODEL_UBX_MON_COMMS_INITIALIZER;
        memset(&data, 0xA5, sizeof(data));
        fprintf(stderr, "\"%s\"[%zu]\n", string, length);
        diminuto_dump(stderr, message, size);
        assert(yodel_is_ubx_class_id(message, size, YODEL_UBX_MON_COMMS_Class, YODEL_UBX_MON_COMMS_Id));
        assert(yodel_ubx_mon_comms(&data, message, size) == 4);
		assert(data.prefix.version == 0);
		assert(data.prefix.nPorts == 4);
		assert(data.prefix.txErrors == 0x00);
		assert(data.prefix.protIds[0] == 0);
		assert(data.prefix.protIds[1] == 1);
		assert(data.prefix.protIds[2] == 5);
		assert(data.prefix.protIds[3] == 255);
		assert(data.port[0].portId == 0x0100); /* ??? */
		assert(data.port[0].txPending == 0);
		assert(data.port[0].txBytes == 14572);
		assert(data.port[0].txUsage == 0);
		assert(data.port[0].txPeakUsage == 14);
		assert(data.port[0].rxPending == 0);
		assert(data.port[0].rxBytes == 0);
		assert(data.port[0].rxUsage == 0);
		assert(data.port[0].rxPeakUsage == 0);
		assert(data.port[0].overrunErrs == 0);
		assert(data.port[0].msgs[0] == 0);
		assert(data.port[0].msgs[1] == 0);
		assert(data.port[0].msgs[2] == 0);
		assert(data.port[0].msgs[3] == 0);
		assert(data.port[0].skipped == 0);
		assert(data.port[1].portId == 0x0200); /* ??? */
		assert(data.port[1].txPending == 0);
		assert(data.port[1].txBytes == 22449480);
		assert(data.port[1].txUsage == 0);
		assert(data.port[1].txPeakUsage == 9);
		assert(data.port[1].rxPending == 0);
		assert(data.port[1].rxBytes == 66199424);
		assert(data.port[1].rxUsage == 5);
		assert(data.port[1].rxPeakUsage == 13);
		assert(data.port[1].overrunErrs == 0);
		assert(data.port[1].msgs[0] == 56513);
		assert(data.port[1].msgs[1] == 0);
		assert(data.port[1].msgs[2] == 0);
		assert(data.port[1].msgs[3] == 0);
		assert(data.port[1].skipped == 27);
		assert(data.port[2].portId == 0x0300); /* ??? */
		assert(data.port[2].txPending == 0);
		assert(data.port[2].txBytes == 7402884);
		assert(data.port[2].txUsage == 1);
		assert(data.port[2].txPeakUsage == 52);
		assert(data.port[2].rxPending == 0);
		assert(data.port[2].rxBytes == 968);
		assert(data.port[2].rxUsage == 0);
		assert(data.port[2].rxPeakUsage == 0);
		assert(data.port[2].overrunErrs == 0);
		assert(data.port[2].msgs[0] == 67);
		assert(data.port[2].msgs[1] == 0);
		assert(data.port[2].msgs[2] == 0);
		assert(data.port[2].msgs[3] == 0);
		assert(data.port[2].skipped == 0);
		assert(data.port[3].portId == 0x0101); /* ??? */
		assert(data.port[3].txPending == 0);
		assert(data.port[3].txBytes == 66191166);
		assert(data.port[3].txUsage == 0);
		assert(data.port[3].txPeakUsage == 0);
		assert(data.port[3].rxPending == 0);
		assert(data.port[3].rxBytes == 22027038);
		assert(data.port[3].rxUsage == 6);
		assert(data.port[3].rxPeakUsage == 10);
		assert(data.port[3].overrunErrs == 0);
		assert(data.port[3].msgs[0] == 31056);
		assert(data.port[3].msgs[1] == 0);
		assert(data.port[3].msgs[2] == 0);
		assert(data.port[3].msgs[3] == 0);
		assert(data.port[3].skipped == 0);
    END;

    /**************************************************************************/

    return 0;
}
