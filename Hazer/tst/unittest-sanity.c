/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "./unittest.h"
#include "com/diag/diminuto/diminuto_countof.h"

int main(void)
{
	hazer_debug(stderr);

	{
        /*
         * Wikipedia, "NMEA 0183", <https://en.wikipedia.org/wiki/NMEA_0183>,
         * "Sample File", * 2019-06-04
         */
		static const char * DATA[] = {
				"$GPGGA,092750.000,5321.6802,N,00630.3372,W,1,8,1.03,61.7,M,55.2,M,,*76\r\n",
				"$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A\r\n",
				"$GPGSV,3,1,11,10,63,137,17,07,61,098,15,05,59,290,20,08,54,157,30*70\r\n",
				"$GPGSV,3,2,11,02,39,223,19,13,28,070,17,26,23,252,,04,14,186,14*79\r\n",
				"$GPGSV,3,3,11,29,09,301,24,16,09,020,,36,,,*76\r\n",
				"$GPRMC,092750.000,A,5321.6802,N,00630.3372,W,0.02,31.66,280511,,,A*43\r\n",
				"$GPGGA,092751.000,5321.6802,N,00630.3371,W,1,8,1.03,61.7,M,55.3,M,,*75\r\n",
				"$GPGSA,A,3,10,07,05,02,29,04,08,13,,,,,1.72,1.03,1.38*0A\r\n",
				"$GPGSV,3,1,11,10,63,137,17,07,61,098,15,05,59,290,20,08,54,157,30*70\r\n",
				"$GPGSV,3,2,11,02,39,223,16,13,28,070,17,26,23,252,,04,14,186,15*77\r\n",
				"$GPGSV,3,3,11,29,09,301,24,16,09,020,,36,,,*76\r\n",
				"$GPRMC,092751.000,A,5321.6802,N,00630.3371,W,0.06,31.66,280511,,,A*45\r\n",
		};
		ssize_t size;
		ssize_t length;
        const char * pointer;
        char msn;
        char lsn;
        int ii;
        int rc;
        uint8_t cs;
        hazer_buffer_t buffer;
        hazer_context_t context;
        hazer_state_t state;

        for (ii = 0; ii < countof(DATA); ++ii) {

        	size = strlen(DATA[ii]);
        	length = hazer_length(DATA[ii], size);
        	assert(length == size);

        	msn = '\0';
        	lsn = '\0';
        	pointer = (char *)hazer_checksum_buffer(DATA[ii], size, &msn, &lsn);
        	assert(pointer != (char *)0);
        	assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        	assert(pointer[1] == msn);
        	assert(pointer[2] == lsn);
        	assert(pointer[3] == '\r');
        	assert(pointer[4] == '\n');

        	cs = 0;
            rc = hazer_characters2checksum(msn, lsn, &cs);
            assert(rc == 0);

            hazer_checksum2characters(cs, &msn, &lsn);
        	assert(pointer[1] == msn);
        	assert(pointer[2] == lsn);

        	state = HAZER_STATE_START;
        	pointer = DATA[ii];
        	while ((length--) > 0) {
        		state = hazer_machine(state, *(pointer++), buffer, sizeof(buffer), &context);
        		if (state == HAZER_STATE_END) { break; }
        		assert(state != HAZER_STATE_STOP);
        	}
        	assert(state == HAZER_STATE_END);
        	assert(strncmp(DATA[ii], buffer, size) == 0);
        }
	}

}
