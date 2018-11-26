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
    }

    /**************************************************************************/

    {
    	yodel_ubx_header_t header = { 0 };
    	unsigned char * buffer;

    	buffer = (unsigned char *)&header;

    	buffer[YODEL_UBX_SYNC_1] = YODEL_STIMULUS_SYNC_1;
    	buffer[YODEL_UBX_SYNC_2] = YODEL_STIMULUS_SYNC_2;
    	buffer[YODEL_UBX_CLASS] = 0xaa;
    	buffer[YODEL_UBX_ID] = 0x55;
    	buffer[YODEL_UBX_LENGTH_LSB] = 0xa5;
    	buffer[YODEL_UBX_LENGTH_MSB] = 0x5a;

    	assert(header.sync_1 == YODEL_STIMULUS_SYNC_1);
    	assert(header.sync_2 == YODEL_STIMULUS_SYNC_2);
    	assert(header.class == 0xaa);
    	assert(header.id == 0x55);
    	assert(le16toh(header.length) == 0x5aa5);
    }

    /**************************************************************************/

    {
    	union { uint64_t integer; uint8_t byte[sizeof(uint64_t)]; } u64 = { 0x1122334455667788ULL };
    	union { uint32_t integer; uint8_t byte[sizeof(uint32_t)]; } u32 = { 0x11223344UL };
    	union { uint16_t integer; uint8_t byte[sizeof(uint16_t)]; } u16 = { 0x1122U };

#if __BYTE_ORDER == __LITTLE_ENDIAN
    	assert(u64.byte[0] == 0x88);
    	assert(u32.byte[0] == 0x44);
    	assert(u16.byte[0] == 0x22);
#else
    	assert(u64.byte[0] == 0x11);
    	assert(u32.byte[0] == 0x11);
    	assert(u16.byte[0] == 0x11);
#endif

    	COM_DIAG_YODEL_LETOH(u64.integer);
    	COM_DIAG_YODEL_LETOH(u32.integer);
    	COM_DIAG_YODEL_LETOH(u16.integer);

    	assert(u64.byte[0] == 0x88);
    	assert(u32.byte[0] == 0x44);
    	assert(u16.byte[0] == 0x22);
    }

    /**************************************************************************/

    return 0;
}
