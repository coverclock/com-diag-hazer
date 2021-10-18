/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
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
    ssize_t sync_length = 0;

    if (sync_buffer == (uint8_t *)0) {
        diminuto_assert(SYNCBUFFER > 0);
        sync_buffer = (uint8_t *)malloc(SYNCBUFFER);
        diminuto_assert(sync_buffer != (uint8_t *)0);
        sync_here = sync_buffer;
    }

    sync_length = sync_here - sync_buffer;
    diminuto_assert(sync_length >= 0);
    diminuto_assert(sync_length < SYNCBUFFER);

    *(sync_here++) = ch;
    sync_length += 1;

    if (sync_length >= SYNCBUFFER) {
        fputs("Unknown:\n", stderr);
        diminuto_dump(stderr, sync_buffer, sync_length);
        sync_here = sync_buffer;
    }
}

void sync_in(size_t length)
{
    static int synced = 0;
    ssize_t sync_length = 0;

    if (sync_buffer != (uint8_t *)0) {

        sync_length = sync_here - sync_buffer;
        diminuto_assert(sync_length >= 0);
        diminuto_assert(sync_length < SYNCBUFFER);

        if (length < sync_length) {
            sync_length -= length;
        }

        if (sync_length > 0) {
            if (synced) {
                fputs("Unknown:\n", stderr);
            } else {
                fputs("Initial:\n", stderr);
                synced = !0;
            }
            diminuto_dump(stderr, sync_buffer, sync_length);
            sync_here = sync_buffer;
        }

    }
}

void sync_end(void)
{
    ssize_t sync_length = 0;

    if (sync_buffer != (uint8_t *)0) {

        sync_length = sync_here - sync_buffer;
        diminuto_assert(sync_length >= 0);
        diminuto_assert(sync_length < SYNCBUFFER);

        if (sync_length > 0) {
            fputs("Unknown:\n", stderr);
            diminuto_dump(stderr, sync_buffer, sync_length);
            sync_here = sync_buffer;
        }

        free(sync_buffer);
        sync_buffer = (uint8_t *)0;
        sync_here = sync_buffer;
    }
}
