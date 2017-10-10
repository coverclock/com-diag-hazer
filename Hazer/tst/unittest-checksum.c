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
