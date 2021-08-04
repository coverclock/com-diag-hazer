/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implemention of the gpstool Sync API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include "constants.h"

static uint8_t * sync_buffer = (uint8_t *)0;
static uint8_t * sync_here = (uint8_t *)0;

void sync_out(int ch)
{
    size_t sync_length = 0;

    if (sync_buffer == (uint8_t *)0) {
        diminuto_assert(SYNCBUFFER > 0);
        sync_buffer = (uint8_t *)malloc(SYNCBUFFER);
        diminuto_assert(sync_buffer != (uint8_t *)0);
        sync_here = sync_buffer;
    }

    sync_length = sync_here - sync_buffer;

    if (sync_length >= SYNCBUFFER) {
        fputs("UNK:\n", stderr);
        diminuto_dump(stderr, sync_buffer, sync_length);
        sync_here = sync_buffer;
    }

    *(sync_here++) = ch;
}

void sync_in(void)
{
    size_t sync_length = 0;

    if (sync_buffer != (uint8_t *)0) {

        sync_length = sync_here - sync_buffer;

        if (sync_length > 0) {
            fputs("UNK:\n", stderr);
            diminuto_dump(stderr, sync_buffer, sync_length);
            sync_here = sync_buffer;
        }

    }
}

void sync_end(void)
{
    size_t sync_length = 0;

    if (sync_buffer != (uint8_t *)0) {

        sync_length = sync_here - sync_buffer;

        if (sync_length > 0) {
            fputs("UNK:\n", stderr);
            diminuto_dump(stderr, sync_buffer, sync_length);
            sync_here = sync_buffer;
        }

        free(sync_buffer);
        sync_buffer = (uint8_t *)0;
        sync_here = sync_buffer;
    }
}
