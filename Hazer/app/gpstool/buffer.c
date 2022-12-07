/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implementation of the gpstool Buffer API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <errno.h>
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_phex.h"
#include "constants.h"
#include "buffer.h"

void write_buffer(FILE * fp, const void * buffer, size_t size)
{
    if (fwrite(buffer, size, 1, fp) < 1) {
        errno = EIO;
        diminuto_perror("write_buffer: fwrite");
    } else if (fflush(fp) == EOF) {
        errno = EIO;
        diminuto_perror("write_buffer: fflush");
    } else {
        /* Do nothing. */
    }
}

void print_buffer(FILE * fp, const void * buffer, size_t size, size_t limit)
{
    const char * bb = (const char *)0;
    size_t current = 0;
    int end = 0;

    for (bb = (const char *)buffer; size > 0; --size) {
        diminuto_phex_emit(fp, *(bb++), UNLIMITED, 0, !0, 0, &current, &end, 0);
        if (current >= limit) { break; }
    }
    fputc('\n', fp);
}

void dump_buffer(FILE * fp, const void * buffer, size_t size)
{
    const unsigned char * bb = (const unsigned char *)0;

    for (bb = (const unsigned char *)buffer; size > 0; --size) {
        fprintf(fp, "\\x%2.2x", *(bb++));
    }
    fputc('\n', fp);
}
