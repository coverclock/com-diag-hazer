/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the implemention of the gpstool Sync API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_assert.h"
#include "com/diag/diminuto/diminuto_dump.h"
#include <stdint.h>
#include <stdio.h>
#include "sync.h"

static uint8_t sync_buffer[sizeof(datagram_payload_t)] = { 0, };

static uint8_t * sync_here = sync_buffer;

void sync_out(int ch)
{
    ssize_t sync_length = 0;

    sync_length = sync_here - sync_buffer;
    diminuto_contract(sync_length >= 0);
    diminuto_contract(sync_length < SYNC_SIZE);

    *(sync_here++) = ch;
    sync_length += 1;

    if (sync_length >= SYNC_SIZE) {
        fputs("Unknown:\n", stderr);
        diminuto_dump(stderr, sync_buffer, sync_length);
        sync_here = sync_buffer;
    }
}

void sync_in(size_t length)
{
    static int synced = 0;
    ssize_t sync_length = 0;

    sync_length = sync_here - sync_buffer;
    diminuto_contract(sync_length >= 0);
    diminuto_contract(sync_length < SYNC_SIZE);

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

void sync_end(void)
{
    ssize_t sync_length = 0;

    sync_length = sync_here - sync_buffer;
    diminuto_contract(sync_length >= 0);
    diminuto_contract(sync_length < SYNC_SIZE);

    if (sync_length > 0) {
        fputs("Unknown:\n", stderr);
        diminuto_dump(stderr, sync_buffer, sync_length);
        sync_here = sync_buffer;
    }
}
