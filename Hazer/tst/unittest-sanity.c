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
#include <string.h>
#include <stdint.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "./unittest.h"

int main(void)
{

	{
		/*
		 * UBX-ZED-F9P, 2019-06-04
		 */
		static const uint8_t * DATA[] = {
	        "$GNRMC,171629.00,A,3947.65423,N,10509.20101,W,0.023,,040619,,,A,V*07\r\n",
	        "$GNVTG,,T,,M,0.023,N,0.043,K,A*3B\r\n",
	        "$GNGGA,171629.00,3947.65423,N,10509.20101,W,1,12,0.66,1711.8,M,-21.5,M,,*4C\r\n",
	        "$GNGSA,A,3,07,11,30,19,28,01,17,13,15,18,,,1.26,0.66,1.08,1*03\r\n",
	        "$GNGSA,A,3,79,82,80,83,81,,,,,,,,1.26,0.66,1.08,2*00\r\n",
	        "$GNGSA,A,3,02,11,25,08,36,,,,,,,,1.26,0.66,1.08,3*07\r\n",
	        "$GNGSA,A,3,,,,,,,,,,,,,1.26,0.66,1.08,4*08\r\n",
	        "$GPGSV,3,1,11,01,37,079,34,07,24,142,38,08,03,048,24,11,38,060,27,1*6E\r\n",
	        "$GPGSV,3,2,11,13,37,271,32,15,17,302,23,17,59,222,40,18,24,053,29,1*6A\r\n",
	        "$GPGSV,3,3,11,19,34,221,37,28,69,002,38,30,56,146,47,1*52\r\n",
	        "$GPGSV,3,1,11,01,37,079,25,07,24,142,32,08,03,048,,11,38,060,,6*60\r\n",
	        "$GPGSV,3,2,11,13,37,271,,15,17,302,19,17,59,222,36,18,24,053,,6*6F\r\n",
	        "$GPGSV,3,3,11,19,34,221,,28,69,002,,30,56,146,38,6*52\r\n",
	        "$GLGSV,3,1,09,70,02,254,,71,10,300,27,72,03,345,,73,14,313,31,1*72\r\n",
	        "$GLGSV,3,2,09,79,40,103,36,80,66,003,27,81,34,035,34,82,76,114,33,1*73\r\n",
	        "$GLGSV,3,3,09,83,34,190,21,1*46\r\n",
	        "$GLGSV,3,1,09,70,02,254,,71,10,300,,72,03,345,,73,14,313,20,3*75\r\n",
	        "$GLGSV,3,2,09,79,40,103,30,80,66,003,34,81,34,035,34,82,76,114,28,3*7F\r\n",
	        "$GLGSV,3,3,09,83,34,190,38,3*4C\r\n",
	        "$GAGSV,2,1,07,02,69,345,38,07,12,302,,08,15,252,23,11,37,053,28,7*76\r\n",
	        "$GAGSV,2,2,07,25,43,093,34,30,23,300,,36,58,134,42,7*40\r\n",
	        "$GAGSV,2,1,07,02,69,345,35,07,12,302,,08,15,252,29,11,37,053,30,2*7D\r\n",
	        "$GAGSV,2,2,07,25,43,093,,30,23,300,,36,58,134,40,2*40\r\n",
	        "$GBGSV,1,1,00,*47\r\n",
	        "$GBGSV,1,1,00,*47\r\n",
	        "$GNGLL,3947.65423,N,10509.20101,W,171629.00,A,A*6F\r\n",
		};
		ssize_t size;
		ssize_t length;
        const uint8_t * pointer;
        char msn;
        char lsn;
        int ii;
        int rc;
        uint8_t cs;
        hazer_buffer_t buffer;
        hazer_context_t context;
        hazer_state_t state;

        hazer_debug(stderr);
        hazer_initialize();

        for (ii = 0; ii < countof(DATA); ++ii) {

        	fprintf(stderr, "Sentence %d\n", ii);

        	size = strlen(DATA[ii]);
        	diminuto_dump(stderr, DATA[ii], size);

        	length = hazer_length(DATA[ii], size);
        	assert(length == size);

        	msn = '\0';
        	lsn = '\0';
        	pointer = (char *)hazer_checksum_buffer(DATA[ii], size, &msn, &lsn);
        	assert(pointer != (uint8_t *)0);
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

        	length = hazer_size(&context);
        	length -= 1; /* Included trailing NUL. */
			assert(size == length);
        }

        hazer_finalize();
        hazer_debug((FILE *)0);
	}

	{
		/*
		 * UBX-ZED-F9P, 2019-06-04
		 */
		static const uint8_t * DATA[] = {
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\x18\\xec\\x01\\x0eB\\xdfR\\xc1c\\x1e\\xb8\\x17\\xbf\\xca\\x19\\0\\xbb\\x1e\\x1a\\0\\x1f\\x1c\\xff\\xffw4\\0\\0\\xfbV\\0\\0\\x9a\\x9c",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\x18\\xec\\x01\\x0e\\x95\\0\\0\\0\\x10\\x15Z\\xf8\\xffh\\xc2\\xe3\\xdaj5\\x18\\x1e\\xe4\\xfa\\0^\\xe8\\0\\0\\x96\\0\\0\\0\\0\\x01\\0\\0\\xf9\\xea",
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\0\\xf0\\x01\\x0eD\\xdfR\\xc1b\\x1e\\xb8\\x17\\xd3\\xca\\x19\\0\\xcf\\x1e\\x1a\\0\\x16.\\xfc\\xfc\\x894\\0\\0@W\\0\\0\\n\\xd0",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\0\\xf0\\x01\\x0e\\x96\\0\\0\\0\\x11\\x15Z\\xf8\\xfeh\\xc2\\xe3\\xdaj5\\x18\\xe4(\\x0e\\0\\xe5\\xe8\\0\\0\\x97\\0\\0\\0\\0\\x01\\0\\0\\x8c\\n",
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\xe8\\xf3\\x01\\x0eG\\xdfR\\xc1a\\x1e\\xb8\\x17\\xfd\\xca\\x19\\0\\xf9\\x1e\\x1a\\0\\xd0*\\xff\\xff\\x984\\0\\0\\x91W\\0\\0g\\xd2",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\xe8\\xf3\\x01\\x0e\\x97\\0\\0\\0\\x11\\x15Z\\xf8\\xfeh\\xc2\\xe3\\xdaj5\\x18\\x0e\\x06$\\0l\\xe9\\0\\0\\x98\\0\\0\\0\\0\\x01\\0\\0\\x1fp",
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\xd0\\xf7\\x01\\x0eK\\xdfR\\xc1`\\x1e\\xb8\\x171\\xcb\\x19\\0-\\x1f\\x1a\\0\\xef\\x01\\xfd\\xfc\\xad4\\0\\0\\xdeW\\0\\0\\x13v",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\xd0\\xf7\\x01\\x0e\\x98\\0\\0\\0\\x12\\x15Z\\xf8\\xfeh\\xc2\\xe3\\xdbj5\\x18\\xd5\\xe1\\xd6\\0\\xf0\\xe9\\0\\0\\x99\\0\\0\\0\\0\\x01\\0\\0\\xe7%",
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\xb8\\xfb\\x01\\x0eM\\xdfR\\xc1_\\x1e\\xb8\\x17N\\xcb\\x19\\0J\\x1f\\x1a\\0\\x1a)\\x01\\x01\\xc24\\0\\0LX\\0\\0\\x1a\\x9a",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\xb8\\xfb\\x01\\x0e\\x99\\0\\0\\0\\x12\\x15Z\\xf8\\xfdh\\xc2\\xe3\\xdbj5\\x18\\xff\\x1e\\xed\\0t\\xea\\0\\0\\x9a\\0\\0\\0\\0\\x01\\0\\0\\xd7\\x11",
            "\\xb5b\\x01\\x14$\\0\\0\\0\\0\\0\\xa0\\xff\\x01\\x0eN\\xdfR\\xc1_\\x1e\\xb8\\x17n\\xcb\\x19\\0j\\x1f\\x1a\\0*\\xf3\\x01\\x01\\xd64\\0\\0\\xb6X\\0\\0\\x9fh",
            "\\xb5b\\x01;(\\0\\0\\0\\0\\0\\xa0\\xff\\x01\\x0e\\x9a\\0\\0\\0\\x12\\x15Z\\xf8\\xfdh\\xc2\\xe3\\xdbj5\\x18)\\xf6\\x05\\0\\xf6\\xea\\0\\0\\x9b\\0\\0\\0\\0\\x01\\0\\0a\\x15",
		};
        const uint8_t * pointer;
        uint8_t csa;
        uint8_t csb;
        int ii;
        int rc;
        yodel_buffer_t buffer;
        yodel_context_t context;
        yodel_state_t state;

		yodel_debug(stderr);
		yodel_initialize();

		for (ii = 0; ii < countof(DATA); ++ii) {
			BEGIN(DATA[ii]);

        	    fprintf(stderr, "Packet %d\n", ii);

			    diminuto_dump(stderr, message, size);

				length = yodel_length(message, size);
				assert(length == size);

				csa = 0;
				csb = 0;
				pointer = (char *)yodel_checksum_buffer(message, size, &csa, &csb);
				assert(pointer != (uint8_t *)0);
				assert(pointer[0] == csa);
				assert(pointer[1] == csb);

				state = YODEL_STATE_START;
				pointer = message;
				while ((length--) > 0) {
					state = yodel_machine(state, *(pointer++), buffer, sizeof(buffer), &context);
					if (state == YODEL_STATE_END) { break; }
					assert(state != YODEL_STATE_STOP);
				}
				assert(state == YODEL_STATE_END);
				assert(memcmp(message, buffer, size) == 0);

				length = yodel_size(&context);
	        	length -= 1; /* Included trailing NUL. */
	        	assert(size == length);

			END;
		}

		yodel_finalize();
		yodel_debug((FILE *)0);

	}

	{
		/*
		 * UBX-ZED-F9P, 2019-06-04
		 */
		static const uint8_t * DATA[] = {
            "\\xd3\\0\\x98C \\08\\a\\xb0b\\0\\0A\\x14p\\n\\0\\0\\0\\0 \\0\\x80\\0}ui)\\x89)H\\xc9\\x89H\\xa8\\xb0\\x85\\xfc\\xfa\\x1a\\x85\\x93w\\xbf\\xb5\\x1e/\\xcd\\xaf\\xd1C\\x0e\\xc6p\\xf5y\\x13\\xd6q \\xe2y\\x98\\x1d\\xe7\\x1a[\\xc3\\x87\\b\\x01\\xfd\\x8f\\xc4\\0\\xd3\\xf0]\\x05\\x81\\xc2\\xe5\\xfa`\\xc0\\x15K \\xdf\\xe1\\x03y\\xcc\\x0ea\\x13\\xddv/C\\xfd@\\xe8z\\x04\\x1bZ\\xb7v7w7w@\\x01\\x133@\\xdc\\x14H\\xec\\xb3_0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\xa4\\xad\\x1a",
	        "\\xd3\\0\\bL\\xe0\\0\\x8a\\0\\0\\0\\0\\xa8\\xf7*",
	        "\\xd3\\0\\x98C \\08\\a\\xc0\\x02\\0\\0A\\x14p\\n\\0\\0\\0\\0 \\0\\x80\\0}ui)\\x89)H\\xc9\\x89H\\xa8\\xb0\\x86\\0\\xfa\\x9a\\xa5\\x8by\\xbe\\xb5\\x1e7\\x91\\xefW\\xc5\\x8f\\vw}\\xac\\xd4\\x82k\\xfc\\xd81\\xb3\\x1dB[>\\x8eR\\x1d\\x9b\\xfb\\xaa\\v\\xf9=\\xb0\\xad\\x05\\x03\\x02\\xe7\\xfex\\xfb\\xd5\\xee \\xd5\\x92\\x03P\\x96\\x0f5\\xf3\\xd3)\\x9f`e\\x03\\x9bX\\x0e\\xe6\\xe6\\xb7v7w7w@\\x01\\x13S@\\xe4\\x14J\\xec\\xb3_0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\xbc\\0\\xa7",
	        "\\xd3\\0^D`\\08\\a\\xc0\\0\\0\\0 \\x90\\0@\\b\\0\\0\\0 \\x01\\0\\0oi\\xca\\x89\\xca)\\xf0\\xb5\\xcb\\xad$\\a\\xf7\">\\x90!\\x83\\x80v\\x01:\\x12\\xb5\\xb63\\x81\\x7f\\xc9\\x13?K\\xcc\\x01~3\\xfd\\xde(\\x06\\x1f\\xc0\\x94V{!\\xa1\\xef\\xdcv\\xec\\xe6^`\\x04\\xd1\\xba\\xdb\\xd1U@\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\x8c\\xa8T",
	        "\\xd3\\0\\bL\\xe0\\0\\x8a\\0\\0\\0\\0\\xa8\\xf7*",
	        "\\xd3\\0\\x98C \\08\\a\\xcf\\xa2\\0\\0A\\x14p\\n\\0\\0\\0\\0 \\0\\x80\\0}ui)\\x89)H\\xc9\\x89H\\xa8\\xb0\\x86\\x06\\xfb\\x1a\\xa5\\x83{\\xbd\\xb5\\x1eGV\\xbe\\xdf\\x18\\x0e\\xb0~\\x85\\x97\\x15,f\\xe4\\xce\\x19\\xce\\xbc\\x9e\\\\!\\xb9 s3\\xf9\\xc8\\xc7\\xf1\\xb7\\xfe\\xfd$|Cc\\x02\\x95D\\x16\\x9f@\\xcbs\\xc3(\\x1d\\x10\\x0e\\xcb\\xc8\\xe5\\xcf|\\xe0>N\\xfb\\xf9\\xb5n\\xb7v7w7w@\\x01\\x13S@\\xe44J\\xec\\xb3_8\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\xcc\\xf21",
	        "\\xd3\\0{C\\xc0\\0Qg\\x04b\\0\\0\\x01A\\xf0\\0\\0\\0\\0\\0 \\x80\\0\\0W\\xfa\\x9c\\x9a\\x8a\\x82\\x8e\\x80\\x8f\\xe1*:\\x12\\x04@\\xa0\\x8ba;S?\\xe6\\xceN\\x85\\x15u,!\\xfc\\xd4\\0W\\xd9O\\xc2$\\xf9\\x02tZ\\x0e\\x11\\xe48\\xf4 \\xedd\\x02\\xad\\xed\\v*\\xe7\\xfc\\xe6\\x9f\\xfa\\xb2?w\\xae\\xff\\xd7('gI\\xad\\xd7\\xdd\\xdc\\xdd\\0\\x0e?#\\xcd\\xc5\\x14P\\xbb0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0C\\xa0\\xf9",
	        "\\xd3\\0^D`\\08\\a\\xcf\\xa0\\0\\0 \\x90\\0@\\b\\0\\0\\0 \\x01\\0\\0oi\\xca\\x89\\xca)\\xf0\\xad\\xcf\\xad\\xa4'\\xe9d\\xb3\\x16\\xf8\\xe5?P~\\xef\\n\\xd5\\xd7{\\xc3\\xc0Y\\xcd\\x01\\x8e\\xb6x\\xde\\xf5\\xfb\\x8c?\\xfc\\xd8@T\\xb3}4\\xd7\\xf8)f\\xec\\xe6^`\\x04\\xd1\\xba\\xe3\\xd1U@\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0p\\x9a\\x8a",
	        "\\xd3\\0\\bL\\xe0\\0\\x8a\\0\\0\\0\\0\\xa8\\xf7*",
			"\\xd3\\0mD`\\08\\vy\"\\0\\0!\\x90\\0@\\b\\0\\0\\0 \\x01\\0\\0j\\xdar\\xe2\\xa2r\\x8a{\\xf6\\xe2g\\xdc\\xaa\\xb4d\\x9d\\xae@hHl\\xd8\\xa1,\\\\]\"_\\xc69\\x87w\\xfe X(H+\\xe1\\xb0\\xc3\\x84\\xbc\\xb1\\xe4\\x19\\xe8\"\\xcc\\x80\\xc0\\x8d\\xf6\\xbb\\xe6n\\x80M\\x1bN\\xbc\\xf5\\x14\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0\\0ze\\xf0",
			"\\xd3\\0\\x1eF@\\08\\n\\x9e`\\0\\0\\0\\0\\0 \\0\\0\\0\\0 \\0\\0\\0S\\xac\\xfc\\xcc~e\\xd8Z`X=\\xae",
			"\\xd3\\0\\0\\x47\\xea\\x4b",
		};
        const uint8_t * pointer;
        uint8_t crc1;
        uint8_t crc2;
        uint8_t crc3;
        int ii;
        int rc;
        tumbleweed_buffer_t buffer;
        tumbleweed_context_t context;
        tumbleweed_state_t state;

		tumbleweed_debug(stderr);
		tumbleweed_initialize();

		for (ii = 0; ii < countof(DATA); ++ii) {
			BEGIN(DATA[ii]);

        	    fprintf(stderr, "Message %d\n", ii);

		        diminuto_dump(stderr, message, size);

				length = tumbleweed_length(message, size);
				assert(length == size);

				crc1 = 0;
				crc2 = 0;
				crc3 = 0;
				pointer = (char *)tumbleweed_checksum_buffer(message, size, &crc1, &crc2, &crc3);
				assert(pointer != (uint8_t *)0);
				assert(pointer[0] == crc1);
				assert(pointer[1] == crc2);
				assert(pointer[2] == crc3);

				state = TUMBLEWEED_STATE_START;
				pointer = message;
				while ((length--) > 0) {
					state = tumbleweed_machine(state, *(pointer++), buffer, sizeof(buffer), &context);
					if (state == TUMBLEWEED_STATE_END) { break; }
					assert(state != TUMBLEWEED_STATE_STOP);
				}
				assert(state ==TUMBLEWEED_STATE_END);
				assert(memcmp(message, buffer, size) == 0);

				length = tumbleweed_size(&context);
	        	length -= 1; /* Included trailing NUL. */
	        	assert(size == length);

			END;
		}

		tumbleweed_finalize();
		tumbleweed_debug((FILE *)0);

	}

}
