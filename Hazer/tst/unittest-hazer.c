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
#include <assert.h>
#include "com/diag/diminuto/diminuto_phex.h"
#include "com/diag/hazer/hazer.h"

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

    program = ((program = strrchr(argv[0], '/')) == (char *)0) ? argv[0] : program + 1;

    if (argc > 1) {
        hazer_debug(stderr);
    }

    while (!0) {

        size = hazer_sentence_read(stdin, buffer, sizeof(buffer));

        if (size < 0) {
            fprintf(stderr, "%s: ERR\n", program);
            return 1;
        } else if (size == 0) {
            fprintf(stderr, "%s: EOF\n", program);
            return 0;
        } else {
            /* Do nothing. */
        }

        check = hazer_sentence_check(buffer, size);

        for (bb = buffer, ss = size; ss > 0; --ss) {
            diminuto_phex_emit(stderr, *(bb++), ~0, 0, 0, 0, &current, &end, 0);
        }

        fprintf(stderr, "[%lu](%lu) %s\n", size, check, (check == size) ? "OK" : "BAD");
    }

    return 0;
}
