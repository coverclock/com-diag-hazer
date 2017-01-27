/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 *
 * EXAMPLE
 *
 * serialtool -D /dev/ttyUSB0 -b 4800 -8 -1 -n -l | unittest-hazer
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/hazer_nmea_gps.h"

int main(int argc, char * argv[])
{
    const char * program = (const char *)0;
	hazer_buffer_t buffer = { 0 };
    char * bb = (char *)0;
    ssize_t size = 0;
    ssize_t ss = 0;
    size_t current = 0;
    int end = 0;
    ssize_t check = 0;
    hazer_vector_t vector = { 0 };
    ssize_t tokens = 0;
    char ** vv = (char **)0;
    int tt = 0;
    int rc = 0;
    uint8_t cs = 0;
    uint8_t ck = 0;
    char msn = '\0';
    char lsn = '\0';
    const char * sp = (const char *)0;

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    if (argc > 1) {
        hazer_debug(stderr);
    }

    while (!0) {

        size = hazer_nmea_read(stdin, buffer, sizeof(buffer));
        assert(size >= 0);

        if (size == 0) {
            fprintf(stderr, "%s: EOF\n", program);
            return 0;
        }

        sp = index(buffer, '*');
        assert(sp != (const char *)0);
        rc = hazer_nmea_characters2checksum(sp[1], sp[2], &ck);
        assert(rc >= 0);
        cs = hazer_nmea_checksum(buffer, size);
        assert(cs == ck);
        rc = hazer_nmea_checksum2characters(cs, &msn, &lsn);
        assert(rc >= 0);
        assert(msn == sp[1]);
        assert(lsn == sp[2]);
        rc = hazer_nmea_characters2checksum(msn, lsn, &ck);
        assert(rc >= 0);
        assert(ck == cs);

        for (bb = buffer, ss = size; ss > 0; --ss) {
            diminuto_phex_emit(stderr, *(bb++), ~0, 0, 0, 0, &current, &end, 0);
        }
        fprintf(stderr, "[%ld] 0x%x %c%c 0x%x\n", size, cs, msn, lsn, ck);

        check = hazer_nmea_check(buffer, size);
        assert(check == size);

        tokens = hazer_nmea_tokenize(vector, sizeof(vector) / sizeof(vector[0]),  buffer, size);
        assert(tokens >= 0);

        for (vv = vector, tt = 1; *vv != (char *)0; ++vv, ++tt) {
            fputs(*vv, stdout);
            fputc((tt == tokens) ? '\n' : ',', stdout);
        }
        fflush(stdout);

    }

    return 0;
}
