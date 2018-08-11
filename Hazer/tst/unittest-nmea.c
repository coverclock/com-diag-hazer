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
#include <assert.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"

int main(void)
{
	{
		static const char * DATA = "$GNGGA,135627.00,3947.65338,N,10509.20216,W,2,12,0.67,1708.6,M,-21.5,M,,0000*4E\r\n";
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_position_t position = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		char msn = 0;
		char lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };

		strncpy(buffer, DATA, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		assert(strcmp(DATA, buffer) == 0);

		length = hazer_length(buffer, sizeof(buffer));
		assert(length == strlen(buffer));

		pointer = (char *)hazer_checksum(buffer, length, &cs);
		assert(pointer != (char *)0);
		assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

		rc = hazer_checksum2characters(cs, &msn, &lsn);
		assert(rc == 0);
		assert(pointer[1] == msn);
		assert(pointer[2] == lsn);
		assert(pointer[3] == '\r');
		assert(pointer[4] == '\n');

		rc = hazer_characters2checksum(msn, lsn, &ck);
		assert(rc == 0);
		assert(ck == cs);

		count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
		assert(count == 16);

		length = hazer_serialize(temporary, sizeof(temporary), vector, count);
		assert(length == (strlen(temporary) + 1));
		temporary[length - 1] = msn;
		temporary[length] = lsn;
		temporary[length + 1] = '\r';
		temporary[length + 2] = '\n';
		temporary[length + 3] = '\0';
		assert(strcmp(DATA, temporary) == 0);

		rc = hazer_parse_gga(&position, vector, count);
		assert(rc == 0);
		assert(strcmp(position.label, "GGA") == 0);
		assert(position.sat_used == 12);
	}

	{
		static const char * DATA = "$GNRMC,135628.00,A,3947.65337,N,10509.20223,W,0.010,,070818,,,D*74\r\n";
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_position_t position = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		char msn = 0;
		char lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };

		strncpy(buffer, DATA, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		assert(strcmp(DATA, buffer) == 0);

		length = hazer_length(buffer, sizeof(buffer));
		assert(length == strlen(buffer));

		pointer = (char *)hazer_checksum(buffer, length, &cs);
		assert(pointer != (char *)0);
		assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

		rc = hazer_checksum2characters(cs, &msn, &lsn);
		assert(rc == 0);
		assert(pointer[1] == msn);
		assert(pointer[2] == lsn);
		assert(pointer[3] == '\r');
		assert(pointer[4] == '\n');

		rc = hazer_characters2checksum(msn, lsn, &ck);
		assert(rc == 0);
		assert(ck == cs);

		count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
		assert(count == 14);

		length = hazer_serialize(temporary, sizeof(temporary), vector, count);
		assert(length == (strlen(temporary) + 1));
		temporary[length - 1] = msn;
		temporary[length] = lsn;
		temporary[length + 1] = '\r';
		temporary[length + 2] = '\n';
		temporary[length + 3] = '\0';
		assert(strcmp(DATA, temporary) == 0);

		rc = hazer_parse_rmc(&position, vector, count);
		assert(rc == 0);
		assert(strcmp(position.label, "RMC") == 0);
	}

	{
		static const char * DATA = "$GNGLL,3947.65337,N,10509.20223,W,135628.00,A,D*6A\r\n";
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_position_t position = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		char msn = 0;
		char lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };

		strncpy(buffer, DATA, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		assert(strcmp(DATA, buffer) == 0);

		length = hazer_length(buffer, sizeof(buffer));
		assert(length == strlen(buffer));

		pointer = (char *)hazer_checksum(buffer, length, &cs);
		assert(pointer != (char *)0);
		assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

		rc = hazer_checksum2characters(cs, &msn, &lsn);
		assert(rc == 0);
		assert(pointer[1] == msn);
		assert(pointer[2] == lsn);
		assert(pointer[3] == '\r');
		assert(pointer[4] == '\n');

		rc = hazer_characters2checksum(msn, lsn, &ck);
		assert(rc == 0);
		assert(ck == cs);

		count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
		assert(count == 9);

		length = hazer_serialize(temporary, sizeof(temporary), vector, count);
		assert(length == (strlen(temporary) + 1));
		temporary[length - 1] = msn;
		temporary[length] = lsn;
		temporary[length + 1] = '\r';
		temporary[length + 2] = '\n';
		temporary[length + 3] = '\0';
		assert(strcmp(DATA, temporary) == 0);

		rc = hazer_parse_gll(&position, vector, count);
		assert(rc == 0);
		assert(strcmp(position.label, "GLL") == 0);
	}

	{
		static const char * DATA = "$GNVTG,,T,,M,0.021,N,0.040,K,D*3F\r\n";
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_position_t position = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		char msn = 0;
		char lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };

		strncpy(buffer, DATA, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		assert(strcmp(DATA, buffer) == 0);

		length = hazer_length(buffer, sizeof(buffer));
		assert(length == strlen(buffer));

		pointer = (char *)hazer_checksum(buffer, length, &cs);
		assert(pointer != (char *)0);
		assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

		rc = hazer_checksum2characters(cs, &msn, &lsn);
		assert(rc == 0);
		assert(pointer[1] == msn);
		assert(pointer[2] == lsn);
		assert(pointer[3] == '\r');
		assert(pointer[4] == '\n');

		rc = hazer_characters2checksum(msn, lsn, &ck);
		assert(rc == 0);
		assert(ck == cs);

		count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
		assert(count == 11);

		length = hazer_serialize(temporary, sizeof(temporary), vector, count);
		assert(length == (strlen(temporary) + 1));
		temporary[length - 1] = msn;
		temporary[length] = lsn;
		temporary[length + 1] = '\r';
		temporary[length + 2] = '\n';
		temporary[length + 3] = '\0';
		assert(strcmp(DATA, temporary) == 0);

		rc = hazer_parse_vtg(&position, vector, count);
		assert(rc == 0);
		assert(strcmp(position.label, "VTG") == 0);
	}

	{
		static const char * DATA = "$GNGSA,A,3,07,11,15,18,19,13,30,28,51,01,48,17,1.27,0.64,1.10*1C\r\n";
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_active_t active = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		uint8_t msn = 0;
		uint8_t lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };

		strncpy(buffer, DATA, sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';
		assert(strcmp(DATA, buffer) == 0);

		length = hazer_length(buffer, sizeof(buffer));
		assert(length == strlen(buffer));

		pointer = (char *)hazer_checksum(buffer, length, &cs);
		assert(pointer != (char *)0);
		assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

		rc = hazer_checksum2characters(cs, &msn, &lsn);
		assert(rc == 0);
		assert(pointer[1] == msn);
		assert(pointer[2] == lsn);
		assert(pointer[3] == '\r');
		assert(pointer[4] == '\n');

		rc = hazer_characters2checksum(msn, lsn, &ck);
		assert(rc == 0);
		assert(ck == cs);

		count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
		assert(count == 19);

		length = hazer_serialize(temporary, sizeof(temporary), vector, count);
		assert(length == (strlen(temporary) + 1));
		temporary[length - 1] = msn;
		temporary[length] = lsn;
		temporary[length + 1] = '\r';
		temporary[length + 2] = '\n';
		temporary[length + 3] = '\0';
		assert(strcmp(DATA, temporary) == 0);

		rc = hazer_parse_gsa(&active, vector, count);
		assert(rc == 0);
		assert(strcmp(active.label, "GSA") == 0);
		assert(active.active == 12);
		assert(active.pdop == 1.27);
		assert(active.hdop == 0.64);
		assert(active.vdop == 1.10);
	}

	{
		static const char * DATA[] = {
			"$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22*7A\r\n",
			"$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40*7B\r\n",
			"$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38*77\r\n",
			"$GPGSV,4,4,15,46,38,215,40,48,36,220,34,51,44,183,45*47\r\n",
		};
		hazer_buffer_t buffer = { 0 };
		hazer_vector_t vector = { 0 };
		hazer_view_t view = { 0 };
		ssize_t length = -1;
		size_t count = 0;
		int rc = -1;
		char * pointer = (char *)0;
		uint8_t cs = 0;
		char msn = 0;
		char lsn = 0;
		uint8_t ck = 0;
		hazer_buffer_t temporary = { 0 };
		int ii = 0;

		for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

			strncpy(buffer, DATA[ii], sizeof(buffer));
			buffer[sizeof(buffer) - 1] = '\0';
			assert(strcmp(DATA[ii], buffer) == 0);

			length = hazer_length(buffer, sizeof(buffer));
			assert(length == strlen(buffer));

			pointer = (char *)hazer_checksum(buffer, length, &cs);
			assert(pointer != (char *)0);
			assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);

			rc = hazer_checksum2characters(cs, &msn, &lsn);
			assert(rc == 0);
			assert(pointer[1] == msn);
			assert(pointer[2] == lsn);
			assert(pointer[3] == '\r');
			assert(pointer[4] == '\n');

			rc = hazer_characters2checksum(msn, lsn, &ck);
			assert(rc == 0);
			assert(ck == cs);

			count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
			assert(((ii == 3) && (count == 17)) || (count == 21));

			length = hazer_serialize(temporary, sizeof(temporary), vector, count);
			assert(length == (strlen(temporary) + 1));
			temporary[length - 1] = msn;
			temporary[length] = lsn;
			temporary[length + 1] = '\r';
			temporary[length + 2] = '\n';
			temporary[length + 3] = '\0';
			assert(strcmp(DATA[ii], temporary) == 0);

			rc = hazer_parse_gsv(&view, vector, count);
			assert(((ii == 3) && (rc == 0)) || (rc > 0));
			assert(strcmp(view.label, "GSV") == 0);
			assert(view.view == 15);

		}
	}
}
