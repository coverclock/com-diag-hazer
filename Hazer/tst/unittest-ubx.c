/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2018 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif
#include <endian.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"

int main(void)
{

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
        assert(header.class == 0x11);
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
        assert(u32.byte[0] == 0x44);
        assert(u16.byte[0] == 0x22);
        assert(u8.byte[0]  == 0x11);
#else
        assert(u64.byte[0] == 0x11);
        assert(u32.byte[0] == 0x11);
        assert(u16.byte[0] == 0x11);
        assert(u8.byte[0]  == 0x11);
#endif

        COM_DIAG_YODEL_LETOH(u64.integer);
        COM_DIAG_YODEL_LETOH(u32.integer);
        COM_DIAG_YODEL_LETOH(u16.integer);
        COM_DIAG_YODEL_LETOH(u8.integer );

        assert(u64.byte[0] == 0x88);
        assert(u32.byte[0] == 0x44);
        assert(u16.byte[0] == 0x22);
        assert(u8.byte[0]  == 0x11);
    }

    /**************************************************************************/



    /**************************************************************************/

    return 0;
}
