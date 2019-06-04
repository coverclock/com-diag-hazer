/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "./unittest.h"

int main(void)
{
	tumbleweed_debug(stderr);

    /**************************************************************************/

	/*
	 * This implementation of CRC-24Q (for "Qualcomm") was based on, but not
	 * copied from, the program crc24q.c written by by Eric S. Raymond in the
	 * gpsd (GPS Daemon) repository at https://github.com/ukyg9e5r6k7gubiekd6/gpsd.
	 * Since I completely rewrote it for Tumbleweed, any bugs are strictly mine.
	 */
    {
    	static const uint32_t POLYNOMIAL = 0x1864cfb;
    	uint32_t table[256];
    	unsigned int ii;
    	unsigned int jj;
    	uint32_t hh;

    	table[0] = 0;
    	table[1] = POLYNOMIAL;

    	hh = table[1];

    	for (ii = 2; ii < countof(table); ii *= 2) {
    		if (((hh <<= 1) & (1 << 24)) != 0) {
    			hh ^= POLYNOMIAL;
    		}
    		for (jj = 0; jj < ii; ++jj) {
    			table[ii + jj] = table[jj] ^ hh;
    		}
    	}

    	printf("const uint32_t TUMBLEWEED_CRC24Q[%zu] = {\n", countof(table));
    	for (ii = 0; ii < countof(table); ++ii) {
    		printf("%s0x%08x,%s", ((ii % 4) == 0) ? "    ": " ", table[ii], ((ii % 4) == 3) ? "\n" : "");
    	}
    	printf("};\n");

    	for (ii = 0; ii < countof(table); ++ii) {
    		assert(TUMBLEWEED_CRC24Q[ii] == table[ii]);
    	}
    }

    /**************************************************************************/

    {
    	uint8_t crc1 = 0x11;
    	uint8_t crc2 = 0x22;
    	uint8_t crc3 = 0x44;
    	uint32_t crc = 0x776655;

    	tumbleweed_checksum2characters(0xa55a55, &crc1, &crc2, &crc3);
    	assert(crc1 == 0xa5);
    	assert(crc2 == 0x5a);
    	assert(crc3 == 0x55);

    	tumbleweed_characters2checksum(crc1, crc2, crc3, &crc);
    	assert(crc == 0xa55a55);
    }

    /**************************************************************************/

    {
    	/* RTCM 10403.3, p. 265 */
		static const uint8_t EXAMPLE[] = "\\xD3\\x00\\x13\\x3E\\xD7\\xD3\\x02\\x02\\x98\\x0E\\xDE\\xEF\\x34\\xB4\\xBD\\x62\\xAC\\x09\\x41\\x98\\x6F\\x33\\x36\\x0B\\x98";
		static const int NUMBER = 1005;

		BEGIN(EXAMPLE);
			ssize_t ss;
			ss = tumbleweed_length(message, size);
			fprintf(stderr, "\"%s\"[%zu] %zu %zu\n", string, length, size, ss);
	    	diminuto_dump(stderr, message, size);
			assert(ss == (TUMBLEWEED_RTCM_SHORTEST + (message[TUMBLEWEED_RTCM_LENGTH_MSB] << 8) + message[TUMBLEWEED_RTCM_LENGTH_LSB]));

		END;

		BEGIN(EXAMPLE);
			const void * bb;
			uint8_t crc_1 = 0xaa;
			uint8_t crc_2 = 0x55;
			uint8_t crc_3 = 0xa5;
			bb = tumbleweed_checksum_buffer(message, size, &crc_1, &crc_2, &crc_3);
			fprintf(stderr, "\"%s\"[%zu] 0x%02x 0x%02x 0x%02x\n", string, length, crc_1, crc_2, crc_3);
	    	diminuto_dump(stderr, message, size);
			assert(crc_1 == message[size - 3]);
			assert(crc_2 == message[size - 2]);
			assert(crc_3 == message[size - 1]);
			assert((unsigned char *)bb == &(message[size - 3]));
		END;

		BEGIN(EXAMPLE);
			int mm;
			mm = tumbleweed_message(message, size);
			fprintf(stderr, "\"%s\"[%zu] %d %d\n", string, length, NUMBER, mm);
	    	diminuto_dump(stderr, message, size);
			assert(mm == NUMBER);
		END;

		BEGIN(EXAMPLE);
			tumbleweed_state_t state = TUMBLEWEED_STATE_START;
			uint8_t buffer[TUMBLEWEED_RTCM_LONGEST + 1];
			tumbleweed_context_t context;
			int ii;
			tumbleweed_initialize();
			for (ii = 0; ii < size; ++ii) {
				state = tumbleweed_machine(state, message[ii], buffer, countof(buffer), &context);
				if (state == TUMBLEWEED_STATE_END) { break; }
			}
			fprintf(stderr, "\"%s\"[%zu] %zu %d %d %zu\n", string, length, size, state, ii, tumbleweed_size(&context));
	    	diminuto_dump(stderr, message, size);
	    	diminuto_dump(stderr, buffer, tumbleweed_size(&context));
			assert(state == TUMBLEWEED_STATE_END);
			assert(tumbleweed_size(&context) == (size + 1));
			assert(buffer[tumbleweed_size(&context) - 1] == '\0');
			assert(memcmp(message, buffer, size) == 0);
			tumbleweed_finalize();
		END;

		BEGIN(EXAMPLE);
			tumbleweed_state_t state = TUMBLEWEED_STATE_START;
			uint8_t buffer[0];
			tumbleweed_context_t context;
			state = tumbleweed_machine(state, message[0], buffer, 0, &context);
			assert(state == TUMBLEWEED_STATE_STOP);
			assert(tumbleweed_size(&context) == 0);
		END;
    }

    /**************************************************************************/

    {
		static const uint8_t KEEPALIVE[] = "\\xd3\\x00\\x00\\x47\\xea\\x4b";

		BEGIN(KEEPALIVE);
			assert(size == countof(TUMBLEWEED_KEEPALIVE));
			assert(memcmp(message, TUMBLEWEED_KEEPALIVE, countof(TUMBLEWEED_KEEPALIVE)) == 0);
		END;

		BEGIN(KEEPALIVE);
			ssize_t ss;
			ss = tumbleweed_length(message, size);
			fprintf(stderr, "\"%s\"[%zu] %zu %zu\n", string, length, size, ss);
	    	diminuto_dump(stderr, message, size);
			assert(ss == TUMBLEWEED_RTCM_SHORTEST);
		END;

		BEGIN(KEEPALIVE);
			const void * bb;
			uint8_t crc_1 = 0xaa;
			uint8_t crc_2 = 0x55;
			uint8_t crc_3 = 0xa5;
			bb = tumbleweed_checksum_buffer(message, size, &crc_1, &crc_2, &crc_3);
			fprintf(stderr, "\"%s\"[%zu] 0x%02x 0x%02x 0x%02x\n", string, length, crc_1, crc_2, crc_3);
	    	diminuto_dump(stderr, message, size);
			assert(crc_1 == message[size - 3]);
			assert(crc_2 == message[size - 2]);
			assert(crc_3 == message[size - 1]);
			assert((unsigned char *)bb == &(message[size - 3]));
		END;

		BEGIN(KEEPALIVE);
			int mm;
			mm = tumbleweed_message(message, size);
			fprintf(stderr, "\"%s\"[%zu] %d %d\n", string, length, -1, mm);
	    	diminuto_dump(stderr, message, size);
			assert(mm < 0);
		END;

		BEGIN(KEEPALIVE);
			tumbleweed_state_t state = TUMBLEWEED_STATE_START;
			uint8_t buffer[TUMBLEWEED_RTCM_LONGEST + 1];
			tumbleweed_context_t context;
			int ii;
			tumbleweed_initialize();
			for (ii = 0; ii < size; ++ii) {
				state = tumbleweed_machine(state, message[ii], buffer, countof(buffer), &context);
				if (state == TUMBLEWEED_STATE_END) { break; }
			}
			fprintf(stderr, "\"%s\"[%zu] %zu %d %d %zu\n", string, length, size, state, ii, tumbleweed_size(&context));
	    	diminuto_dump(stderr, message, size);
	    	diminuto_dump(stderr, buffer, tumbleweed_size(&context));
			assert(state == TUMBLEWEED_STATE_END);
			assert(tumbleweed_size(&context) == (size + 1));
			assert(buffer[tumbleweed_size(&context) - 1] == '\0');
			assert(memcmp(message, buffer, size) == 0);
			tumbleweed_finalize();
		END;
    }

    /**************************************************************************/

    return 0;
}
