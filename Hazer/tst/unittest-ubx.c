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
    	yodel_header_t headers[2];

    	assert(sizeof(yodel_header_t) == (YODEL_UBX_UNSUMMED + YODEL_UBX_SUMMED));

    	assert(sizeof(headers) == (2 * sizeof(yodel_header_t)));
    }

    /**************************************************************************/

    {
    	yodel_record_t record;

    	assert(sizeof(record) >= sizeof(yodel_buffer_t));

    	record.record.buffer[YODEL_UBX_SYNC_1] = YODEL_STIMULUS_SYNC_1;
    	record.record.buffer[YODEL_UBX_SYNC_2] = YODEL_STIMULUS_SYNC_2;
    	record.record.buffer[YODEL_UBX_CLASS] = 0xaa;
    	record.record.buffer[YODEL_UBX_ID] = 0x55;
    	record.record.buffer[YODEL_UBX_LENGTH_LSB] = 0xa5;
    	record.record.buffer[YODEL_UBX_LENGTH_MSB] = 0x5a;

    	assert(record.record.header.sync_1 == record.record.buffer[YODEL_UBX_SYNC_1]);
    	assert(record.record.header.sync_2 == record.record.buffer[YODEL_UBX_SYNC_2]);
    	assert(record.record.header.class == record.record.buffer[YODEL_UBX_CLASS]);
    	assert(record.record.header.id == record.record.buffer[YODEL_UBX_ID]);
    	assert(le16toh(record.record.header.length) == 0x5aa5);
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

    	assert((offsetof(struct Structure, record.record.header.length) % 2) == 0);
    	assert((offsetof(struct Structure, record.record.header.payload) % 8) == 0);

    	assert(sizeof(structure) > (sizeof(structure.offset) + sizeof(structure.record)));

    	pointer = &(structure);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= 8 - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= 8 - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.record.header.length);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= 2 - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.record.header.payload[0]);
    	integer1 = (uintptr_t)pointer;
    	integer1 &= 8 - 1;
    	assert(integer1 == 0);

    	pointer = &(structure.record.record.header);
    	integer1 = (uintptr_t)pointer;
    	pointer = &(structure.record.record.buffer);
    	integer2 = (uintptr_t)pointer;
    	assert(integer1 == integer2);

    }

    /**************************************************************************/

    return 0;
}
