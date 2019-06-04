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
#include "./unittest.h"

int main(void)
{

    /**************************************************************************/

    {
        int rc = 0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        uint8_t cs = 0;
        uint8_t ck = 0;
        uint8_t lsc = '\0';
        uint8_t msc = '\0';
        const void * bp = (const void *)0;
        static const uint8_t NIB[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', };

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
                hazer_checksum2characters(ck, &msc, &lsc);
                assert(msc == NIB[msn]);
                assert(lsc == NIB[lsn]);
            }
        }

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer((const void *)0, 0, &msc, &lsc);
        assert(bp == (const void *)0);
        assert(msc == '?');
        assert(lsc == '!');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("", 0, &msc, &lsc);
        assert(bp == (const void *)0);
        assert(msc == '?');
        assert(lsc == '!');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$", 1, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '\0');
        assert(msc == '0');
        assert(lsc == '0');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$", 2, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '\0');
        assert(msc == '0');
        assert(lsc == '0');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$*", 2, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '0');
        assert(lsc == '0');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$*", 3, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '0');
        assert(lsc == '0');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$V*TU\r\n", 8, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '5');
        assert(lsc == '6');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$VW*TU\r\n", 9, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '0');
        assert(lsc == '1');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$VWX*TU\r\n", 10, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '5');
        assert(lsc == '9');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$VWXY*TU\r\n", 11, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '0');
        assert(lsc == '0');

        msc = '?';
        lsc = '!';
        bp = hazer_checksum_buffer("$VWXYZ*TU\r\n", 12, &msc, &lsc);
        assert(bp != (const void *)0);
        assert(*(unsigned char *)bp == '*');
        assert(msc == '5');
        assert(lsc == 'A');
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        uint8_t ck = 0;
        unsigned char msn = '\0';
        unsigned char lsn = '\0';
        int rc = 0;
        const unsigned char * bp = (const void *)0;
        static const unsigned char NMEA[] = "$GPGSV,4,3,13,24,39,292,21,28,32,109,36,46,38,215,35,48,36,220,37*78\r\n";

        size = hazer_length(NMEA, sizeof(NMEA));
        assert(size == (sizeof(NMEA) - 1));

        bp = hazer_checksum_buffer(NMEA, sizeof(NMEA), &msn, &lsn);
        assert(bp == &NMEA[sizeof(NMEA) - 6]);
        assert(msn == bp[1]);
        assert(lsn == bp[2]);
        assert(bp[0] == '*');
        assert(bp[1] == '7');
        assert(bp[2] == '8');

        rc = hazer_characters2checksum(bp[1], bp[2], &ck);
        assert(rc == 0);
        assert(ck == 0x78);

        hazer_checksum2characters(ck, &msn, &lsn);
        assert(msn == bp[1]);
        assert(lsn == bp[2]);
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        uint8_t ck = 0;
        unsigned char msn = '\0';
        unsigned char lsn = '\0';
        int rc = 0;
        const unsigned char * bp = (const void *)0;
        static const unsigned char NMEA[] = "$GPGSV,4,3,13,24,39,292,21,28,32,109,36,46,38,215,35,48,36,220,37*89\r\n"; /* Bad checksum. */

        size = hazer_length(NMEA, sizeof(NMEA));
        assert(size == (sizeof(NMEA) - 1));

        bp = hazer_checksum_buffer(NMEA, sizeof(NMEA), &msn, &lsn);
        assert(bp == &NMEA[sizeof(NMEA) - 6]);
        assert(bp[0] == '*');
        assert(bp[1] == '8');
        assert(bp[2] == '9');

        rc = hazer_characters2checksum(bp[1], bp[2], &ck);
        assert(rc == 0);
        assert(ck == 0x89);

        hazer_checksum2characters(ck, &msn, &lsn);
        assert(msn == bp[1]);
        assert(lsn == bp[2]);
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0d, 0xca, '\0' };
        const unsigned char * here = (const char *)0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;

        assert(UBX[YODEL_UBX_LENGTH_LSB] == 0x04);
        assert(UBX[YODEL_UBX_LENGTH_MSB] == 0x00);

        size = UBX[YODEL_UBX_LENGTH_MSB] << 8;
        size |= UBX[YODEL_UBX_LENGTH_LSB];
        assert(size == 4);

        size = yodel_length(UBX, sizeof(UBX));
        assert(size == (sizeof(UBX) - 1));

        here = (const char *)yodel_checksum_buffer(UBX, size, &ck_a, &ck_b);
        assert(here != (const unsigned char *)0);
        assert((ck_a == here[0]) && (ck_b == here[1]));
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0c, 0xca, '\0' }; /* Bad checksum. */
        const unsigned char * here = (const char *)0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;

        size = yodel_length(UBX, sizeof(UBX));
        assert(size == (sizeof(UBX) - 1));

        here = (const char *)yodel_checksum_buffer(UBX, size, &ck_a, &ck_b);
        assert(here != (const unsigned char *)0);
        assert((ck_a != here[0]) || (ck_b != here[1]));
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 3, 4, 0x0d, 0xc8, '\0' }; /* Bad checksum. */
        const unsigned char * here = (const char *)0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;

        size = yodel_length(UBX, sizeof(UBX));
        assert(size == (sizeof(UBX) - 1));

        here = (const char *)yodel_checksum_buffer(UBX, size, &ck_a, &ck_b);
        assert(here != (const unsigned char *)0);
        assert((ck_a != here[0]) || (ck_b != here[1]));
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x00, 1, 2, 5, 4, 0x0d, 0xca, '\0' }; /* Payload corrupted. */
        const unsigned char * here = (const char *)0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;

        size = yodel_length(UBX, sizeof(UBX));
        assert(size == (sizeof(UBX) - 1));

        here = (const char *)yodel_checksum_buffer(UBX, size, &ck_a, &ck_b);
        assert(here != (const unsigned char *)0);
        assert((ck_a != here[0]) || (ck_b != here[1]));
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBLOX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x04, 0x01, 1, 2, 3, 4, 0x0d, 0xca, '\0' }; /* Length too long for buffer. */

        size = yodel_length(UBLOX, sizeof(UBLOX)); /* Length overflows buffer. */
        assert(size < 0);
    }

    /**************************************************************************/

    {
        ssize_t size = 0;
        int rc = 0;
        static const uint8_t UBX[] = { 0xb5, 0x62, 0xa5, 0x5a, 0x02, 0x00, 1, 2, 3, 4, 0x0d, 0xca, '\0' }; /* Length too short for payload. */
        const unsigned char * here = (const char *)0;
        uint8_t ck_a = 0;
        uint8_t ck_b = 0;

        size = yodel_length(UBX, sizeof(UBX)); /* Length not correct. */
        assert(size >= 0);

        here = (const char *)yodel_checksum_buffer(UBX, size, &ck_a, &ck_b);
        assert(here != (const unsigned char *)0);
        assert((ck_a != here[0]) || (ck_b != here[1]));
    }

    /**************************************************************************/


}
