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
#include <string.h>
#include <assert.h>
#include "com/diag/diminuto/diminuto_dump.h"
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
        dally_context_t context = { 0, };
        dally_packet_t packet;
        assert(context.state == DALLY_STATE_START);
        assert(dally_init(&context, &packet) == &context);
        assert(context.packetp == &packet);
        assert(context.wordp == (dally_word_t *)0);
        assert(context.count == 0);
        assert(context.word == 0);
        assert(context.state == DALLY_STATE_HEADING);
        assert(dally_fini(&context) == (dally_context_t *)0);
        assert(context.state == DALLY_STATE_START);
    }

    {
        dally_context_t context;
        dally_packet_t packet;
        (void)memset(&context, 0xff, sizeof(context));
        (void)memset(&packet, 0xff, sizeof(packet));
        assert(dally_debug(stderr) == (FILE *)0);
        assert(dally_init(&context, &packet) == &context);
        assert(context.state == DALLY_STATE_HEADING);
        assert(dally_machine(&context, DALLY_HEADING));
        assert(context.state == DALLY_STATE_FLAG);
        assert(dally_machine(&context, DALLY_FLAG_DATA));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 9);
        assert(context.word == (dally_word_t)0x0000);
        assert(dally_machine(&context, 0x22U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x11U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 8);
        assert(context.word == (dally_word_t)0x1122);
        assert(dally_machine(&context, 0x44U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x33U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 7);
        assert(context.word == (dally_word_t)0x3344);
        assert(dally_machine(&context, 0x66U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x55U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 6);
        assert(context.word == (dally_word_t)0x5566);
        assert(dally_machine(&context, 0x88U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x77U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 5);
        assert(context.word == (dally_word_t)0x7788);
        assert(dally_machine(&context, 0xaaU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x99U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 4);
        assert(context.word == (dally_word_t)0x99aa);
        assert(dally_machine(&context, 0xccU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xbbU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 3);
        assert(context.word == (dally_word_t)0xbbcc);
        assert(dally_machine(&context, 0xeeU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xddU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 2);
        assert(context.word == (dally_word_t)0xddee);
        assert(dally_machine(&context, 0x00U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xffU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 1);
        assert(context.word == (dally_word_t)0xff00);
        assert(dally_machine(&context, 0xadU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xdeU));
        assert(context.state == DALLY_STATE_FINAL);
        assert(context.count == 0);
        assert(context.word == (dally_word_t)0xdead);
        assert(packet.d.prefix.header == (dally_word_t)DALLY_HEADING);
        assert(packet.d.prefix.flag == (dally_word_t)DALLY_FLAG_DATA);
        assert(packet.d.ax == (dally_word_t)0x1122);
        assert(packet.d.ay == (dally_word_t)0x3344);
        assert(packet.d.az == (dally_word_t)0x5566);
        assert(packet.d.wx == (dally_word_t)0x7788);
        assert(packet.d.wy == (dally_word_t)0x99aa);
        assert(packet.d.wz == (dally_word_t)0xbbcc);
        assert(packet.d.roll == (dally_word_t)0xddee);
        assert(packet.d.pitch == (dally_word_t)0xff00);
        assert(packet.d.yaw == (dally_word_t)0xdead);
        assert(dally_debug((FILE *)0) == stderr);
    }

    {
        dally_context_t context;
        dally_packet_t packet;
        (void)memset(&context, 0xff, sizeof(context));
        (void)memset(&packet, 0xff, sizeof(packet));
        assert(dally_debug(stderr) == (FILE *)0);
        assert(dally_init(&context, &packet) == &context);
        assert(context.state == DALLY_STATE_HEADING);
        assert(dally_machine(&context, DALLY_HEADING));
        assert(context.state == DALLY_STATE_FLAG);
        assert(dally_machine(&context, DALLY_FLAG_REGISTER));
        assert(context.state == DALLY_STATE_REGISTER_LOW);
        assert(context.word == (dally_word_t)0x0000);
        assert(dally_machine(&context, DALLY_REGISTER_MAGNETICFIELD));
        assert(context.state == DALLY_STATE_REGISTER_HIGH);
        assert(dally_machine(&context, 0x00U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 8);
        assert(context.word == (dally_word_t)DALLY_REGISTER_MAGNETICFIELD);
        assert(dally_machine(&context, 0x44U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x33U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 7);
        assert(context.word == (dally_word_t)0x3344);
        assert(dally_machine(&context, 0x66U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x55U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 6);
        assert(context.word == (dally_word_t)0x5566);
        assert(dally_machine(&context, 0x88U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x77U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 5);
        assert(context.word == (dally_word_t)0x7788);
        assert(dally_machine(&context, 0xaaU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0x99U));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 4);
        assert(context.word == (dally_word_t)0x99aa);
        assert(dally_machine(&context, 0xccU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xbbU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 3);
        assert(context.word == (dally_word_t)0xbbcc);
        assert(dally_machine(&context, 0xeeU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xddU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 2);
        assert(context.word == (dally_word_t)0xddee);
        assert(dally_machine(&context, 0x00U));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xffU));
        assert(context.state == DALLY_STATE_DATA_LOW);
        assert(context.count == 1);
        assert(context.word == (dally_word_t)0xff00);
        assert(dally_machine(&context, 0xadU));
        assert(context.state == DALLY_STATE_DATA_HIGH);
        assert(dally_machine(&context, 0xdeU));
        assert(context.state == DALLY_STATE_FINAL);
        assert(context.count == 0);
        assert(context.word == (dally_word_t)0xdead);
        assert(packet.r.prefix.header == (dally_word_t)DALLY_HEADING);
        assert(packet.r.prefix.flag == (dally_word_t)DALLY_FLAG_REGISTER);
        assert(packet.r.reg == (dally_word_t)DALLY_REGISTER_MAGNETICFIELD);
        assert(packet.r.data[0] == (dally_word_t)0x3344);
        assert(packet.r.data[1] == (dally_word_t)0x5566);
        assert(packet.r.data[2] == (dally_word_t)0x7788);
        assert(packet.r.data[3] == (dally_word_t)0x99aa);
        assert(packet.r.data[4] == (dally_word_t)0xbbcc);
        assert(packet.r.data[5] == (dally_word_t)0xddee);
        assert(packet.r.data[6] == (dally_word_t)0xff00);
        assert(packet.r.data[7] == (dally_word_t)0xdead);
        assert(dally_debug((FILE *)0) == stderr);
    }

    {
        /*
         * Captured using the following command using a WT901BLECL50.
         *
         * wt901setup | serialtool -D /dev/ttyUSB0 -T -b 115200 -8 -1 -n -P | dump
         *
         * This is declared int because that what functions like fgetc(3)
         * return.
         */
        static const int DATA[8][20] = {
            { 0x55, 0x61, 0xd7, 0xff, 0xd7, 0xff, 0x27, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xff, 0xc8, 0x00, 0x6d, 0x5c },
            { 0x55, 0x71, 0x30, 0x00, 0x0f, 0x01, 0x04, 0x15, 0x1f, 0x18, 0x85, 0x02, 0xd7, 0xff, 0xd7, 0xff, 0x26, 0x08, 0x00, 0x00 },
            { 0x55, 0x71, 0x31, 0x00, 0x04, 0x15, 0x1f, 0x19, 0x8a, 0x02, 0xd7, 0xff, 0xd7, 0xff, 0x26, 0x08, 0x00, 0x00, 0x00, 0x00 },
            { 0x55, 0x71, 0x32, 0x00, 0x1f, 0x1a, 0x8f, 0x02, 0xd8, 0xff, 0xd7, 0xff, 0x27, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
            { 0x55, 0x71, 0x33, 0x00, 0x94, 0x02, 0xd9, 0xff, 0xd8, 0xff, 0x28, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x01 },
            { 0x55, 0x71, 0x3a, 0x00, 0x2c, 0x01, 0x16, 0xff, 0x90, 0xfe, 0x32, 0xff, 0xc8, 0x00, 0x6d, 0x5c, 0xc2, 0x08, 0x00, 0x00 },
            { 0x55, 0x71, 0x40, 0x00, 0xc1, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
            { 0x55, 0x71, 0x51, 0x00, 0x1a, 0x36, 0x5a, 0xfe, 0x60, 0xff, 0xfc, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
        };
        dally_context_t context;
        dally_packet_t packet[8];
        dally_state_t state;
        int ii;
        int jj;

        for (ii = 0; ii < 8; ++ii) {
            dally_init(&context, &(packet[ii]));
            for (jj = 0; jj < 20; ++jj) {
                state = dally_machine(&context, DATA[ii][jj]);
                assert(state != DALLY_STATE_START);
                assert(state != DALLY_STATE_ERROR);
                assert(((jj < 19) && (state != DALLY_STATE_FINAL)) || ((jj == 19) && (state == DALLY_STATE_FINAL))
                );
            }
            fprintf(stderr, "DATA[%d]:\n", ii);
            diminuto_dump(stderr, &(DATA[ii]), sizeof(DATA[ii]));
            fprintf(stderr, "packet[%d]:\n", ii);
            diminuto_dump(stderr, &(packet[ii]), sizeof(packet[ii]));
            dally_fini(&context);
        }
    }

    return 0;
}
