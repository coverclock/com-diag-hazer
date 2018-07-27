/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "com/diag/hazer/hazer.h"

int main(void)
{

    /**************************************************************************/

	{
		int rc = 0;
		uint8_t msn = 0;
		uint8_t lsn = 0;
		uint8_t cs = 0;
		uint8_t ck = 0;
		char lsc = '\0';
		char msc = '\0';
		static const char NIB[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

		rc = hazer_characters2checksum('0' - 1, '0', &cs);
		assert(rc < 0);

		rc = hazer_characters2checksum('F' + 1, 'F', &cs);
		assert(rc < 0);

		rc = hazer_characters2checksum('0', '0' - 1, &cs);
		assert(rc < 0);

		rc = hazer_characters2checksum('F', 'F' + 1, &cs);
		assert(rc < 0);

		for (lsn = 0; lsn < 16; ++lsn) {
			for (msn = 0; msn < 16; ++msn) {
				ck = (msn << 4) | lsn;
				rc = hazer_characters2checksum(NIB[msn], NIB[lsn], &cs);
				assert(rc == 0);
				assert(ck == cs);
				rc = hazer_checksum2characters(ck, &msc, &lsc);
				assert(rc == 0);
				assert(msc == NIB[msn]);
				assert(lsc == NIB[lsn]);
			}
		}

		cs = hazer_checksum("", 0);
		/* There is no wrong answer here, we just want to make sure it doesn't core dump. */

		cs = hazer_checksum("$V*TU\r\n", 8);
		assert(cs == 0x56);

		cs = hazer_checksum("$VW*TU\r\n", 9);
		assert(cs == 0x01);

		cs = hazer_checksum("$VWX*TU\r\n", 10);
		assert(cs == 0x59);

		cs = hazer_checksum("$VWXY*TU\r\n", 11);
		assert(cs == 0x00);

		cs = hazer_checksum("$VWXYZ*TU\r\n", 12);
		assert(cs == 0x5A);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		uint8_t cs = 0;
		uint8_t ck = 0;
		unsigned char msn = '\0';
		unsigned char lsn = '\0';
		int rc = 0;
		static const unsigned char NMEA[] = "$GPGSV,4,3,13,24,39,292,21,28,32,109,36,46,38,215,35,48,36,220,37*78\r\n";

		size = hazer_parse_length(NMEA, sizeof(NMEA));
		assert(size == (sizeof(NMEA) - 1));

		cs = hazer_checksum(NMEA, sizeof(NMEA));

		assert(NMEA[sizeof(NMEA) - 5] == '7');
		assert(NMEA[sizeof(NMEA) - 4] == '8');

		rc = hazer_characters2checksum(NMEA[sizeof(NMEA) - 5], NMEA[sizeof(NMEA) - 4], &ck);
		assert(rc == 0);
		assert(cs == ck);

		rc = hazer_checksum2characters(ck, &msn, &lsn);
		assert(rc == 0);
		assert(msn == NMEA[sizeof(NMEA) - 5]);
		assert(lsn == NMEA[sizeof(NMEA) - 4]);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		uint8_t cs = 0;
		uint8_t ck = 0;
		unsigned char msn = '\0';
		unsigned char lsn = '\0';
		int rc = 0;
		static const unsigned char NMEA[] = "$GPGSV,4,3,13,24,39,292,21,28,32,109,36,46,38,215,35,48,36,220,37*89\r\n"; /* Bad checksum. */

		size = hazer_parse_length(NMEA, sizeof(NMEA));
		assert(size == (sizeof(NMEA) - 1));

		cs = hazer_checksum(NMEA, sizeof(NMEA));

		assert(NMEA[sizeof(NMEA) - 5] == '8');
		assert(NMEA[sizeof(NMEA) - 4] == '9');

		rc = hazer_characters2checksum(NMEA[sizeof(NMEA) - 5], NMEA[sizeof(NMEA) - 4], &ck);
		assert(rc == 0);
		assert(cs != ck);

		rc = hazer_checksum2characters(ck, &msn, &lsn);
		assert(rc == 0);
		assert(msn == NMEA[sizeof(NMEA) - 5]);
		assert(lsn == NMEA[sizeof(NMEA) - 4]);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0d, 0xca };

		assert(UBLOX[HAZER_CONSTANT_UBLOX_LENGTH_LSB] == 0x04);
		assert(UBLOX[HAZER_CONSTANT_UBLOX_LENGTH_MSB] == 0x00);

		size = UBLOX[HAZER_CONSTANT_UBLOX_LENGTH_MSB] << 8;
		size |= UBLOX[HAZER_CONSTANT_UBLOX_LENGTH_LSB];
		assert(size == 4);

		size = hazer_parse_length(UBLOX, sizeof(UBLOX));
		assert(size == -(ssize_t)sizeof(UBLOX));

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc == 0);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0c, 0xca }; /* Bad checksum. */

		size = hazer_parse_length(UBLOX, sizeof(UBLOX));
		assert(size == -(ssize_t)sizeof(UBLOX));

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc < 0);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0d, 0xc8 }; /* Bad checksum. */

		size = hazer_parse_length(UBLOX, sizeof(UBLOX));
		assert(size == -(ssize_t)sizeof(UBLOX));

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc < 0);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 5, 4, 0x0d, 0xca }; /* Payload corrupted. */

		size = hazer_parse_length(UBLOX, sizeof(UBLOX));
		assert(size == -(ssize_t)sizeof(UBLOX));

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc < 0);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x01, 1, 2, 3, 4, 0x0d, 0xca }; /* Length too long. */

		size = hazer_parse_length(UBLOX, sizeof(UBLOX)); /* Overflows buffer. */
		assert(size == 0);

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc < 0);
	}

    /**************************************************************************/

	{
		ssize_t size = 0;
		int rc = 0;
		static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x02, 0x00, 1, 2, 3, 4, 0x0d, 0xca }; /* Length too short. */

		size = hazer_parse_length(UBLOX, sizeof(UBLOX)); /* But not correct. */
		assert(size < 0);

		rc = hazer_validate(UBLOX, sizeof(UBLOX));
		assert(rc < 0);
	}

    /**************************************************************************/


}
