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
    	yodel_ubx_header_t headers[2];

    	assert(sizeof(yodel_ubx_header_t) == (YODEL_UBX_UNSUMMED + YODEL_UBX_SUMMED));

    	assert(sizeof(headers) == (2 * sizeof(yodel_ubx_header_t)));
    }

    /**************************************************************************/

    {
    	yodel_record_t record;

    	assert(sizeof(record) >= sizeof(yodel_buffer_t));

    	record.message.buffer[YODEL_UBX_SYNC_1] = YODEL_STIMULUS_SYNC_1;
    	record.message.buffer[YODEL_UBX_SYNC_2] = YODEL_STIMULUS_SYNC_2;
    	record.message.buffer[YODEL_UBX_CLASS] = 0xaa;
    	record.message.buffer[YODEL_UBX_ID] = 0x55;
    	record.message.buffer[YODEL_UBX_LENGTH_LSB] = 0xa5;
    	record.message.buffer[YODEL_UBX_LENGTH_MSB] = 0x5a;

    	assert(record.message.header.sync_1 == record.message.buffer[YODEL_UBX_SYNC_1]);
    	assert(record.message.header.sync_2 == record.message.buffer[YODEL_UBX_SYNC_2]);
    	assert(record.message.header.class == record.message.buffer[YODEL_UBX_CLASS]);
    	assert(record.message.header.id == record.message.buffer[YODEL_UBX_ID]);
    	assert(le16toh(record.message.header.length) == 0x5aa5);
    }

    /**************************************************************************/

    {
    	struct Structure { uint8_t offset; yodel_record_t record; } structure __attribute__ ((aligned (8)));
    	void * pointer;
    	uintptr_t integer1;
    	uintptr_t integer2;

    	assert(sizeof(struct Structure) == sizeof(structure));

    	assert(offsetof(struct Structure, offset) == 0);
    	assert(offsetof(struct Structure, record) == 8);

    	assert((offsetof(struct Structure, record.message.header.length) % 2) == 0);
    	assert((offsetof(struct Structure, record.message.header.payload) % 8) == 0);

    	assert(sizeof(structure) > (sizeof(structure.offset) + sizeof(structure.record)));

    	pointer = &(structure);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= 8 - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= sizeof(uint64_t) - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.message.header.length);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= sizeof(uint16_t) - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.message.header.payload[0]);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= sizeof(uint64_t) - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.message.header);
    	integer1 = (uintptr_t)pointer;
    	pointer = &(structure.record.message.buffer);
    	integer2 = (uintptr_t)pointer;
    	assert(integer1 == integer2);
    }

    /**************************************************************************/

    {
    	struct Structure { uint8_t offset; yodel_record_t record; } structure __attribute__ ((aligned (8)));
    	yodel_ubx_mon_hw_t * payload;
    	void * pointer;
    	uintptr_t integer;

    	assert(sizeof(yodel_ubx_mon_hw_t) == YODEL_UBX_MON_HW_Length);

    	payload = (yodel_ubx_mon_hw_t *)&(structure.record.message.header.payload);
    	pointer = &(payload->pinSel);
    	integer = (uintptr_t)pointer;
    	integer &= sizeof(payload->pinSel) - 1;
    	assert(integer == 0);
    }

    /**************************************************************************/

    {
    	struct Structure { uint8_t offset; yodel_record_t record; } structure __attribute__ ((aligned (8)));
    	yodel_ubx_nav_status_t * payload;
    	void * pointer;
    	uintptr_t integer;

    	assert(sizeof(yodel_ubx_nav_status_t) == YODEL_UBX_NAV_STATUS_Length);

    	payload = (yodel_ubx_nav_status_t *)&(structure.record.message.header.payload);
    	pointer = &(payload->iTOW);
    	integer = (uintptr_t)pointer;
    	integer &= sizeof(payload->iTOW) - 1;
    	assert(integer == 0);
    }

    /**************************************************************************/

    return 0;
}
