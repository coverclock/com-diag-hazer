/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2024 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the Dally unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include <assert.h>
#include "com/diag/hazer/dally.h"

int main(void)
{
    {
        assert(DALLY_PAYLOAD_DATA_WORDS == 9);
        assert(DALLY_PAYLOAD_REGISTER_WORDS == 8);
    }

    {
        assert(sizeof(dally_byte_t) == 1);
        assert(sizeof(dally_word_t) == 2);
        assert(sizeof(dally_datum_t) == 8);
        assert(sizeof(dally_words_t) == 20);
        assert(sizeof(dally_bytes_t) == 20);
        assert(sizeof(dally_prefix_t) == 2);
        assert(sizeof(dally_data_t) == 20);
        assert(sizeof(dally_register_t) == 20);
        assert(sizeof(dally_packet_t) == 20);
    }

    {
        dally_machine_t machine = { 0, };
        dally_packet_t packet;
        assert(machine.state == DALLY_STATE_START);
        assert(dally_init(&machine, &packet) == &machine);
        assert(machine.packetp == &packet);
        assert(machine.wordp == (dally_word_t *)0);
        assert(machine.count == 0);
        assert(machine.word == 0);
        assert(machine.state == DALLY_STATE_HEADING);
        assert(dally_fini(&machine) == (dally_machine_t *)0);
        assert(machine.state == DALLY_STATE_START);
    }

    return 0;
}
