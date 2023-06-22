/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the NMEA unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_minmaxof.h"
#include <stdio.h>
#include "com/diag/hazer/common.h"
#include "./unittest.h"

int main(void)
{
    {
        assert(common_abs64((int64_t)0) == (int64_t)0);
        assert(common_abs64((int64_t)1) == (int64_t)1);
        assert(common_abs64((int64_t)-1) == (int64_t)1);
        assert(common_abs64(diminuto_maximumof(int64_t)) == diminuto_maximumof(int64_t));
        /*
         * In two's complement encoding, the dynamic range of the 
         * positive integers is one greater than that of the negative
         * integers because zero is considered positive. Consider that
         * taking the two's complement of the largest (numerically smallest)
         * negative number yields the same negative number.
         */
        assert(common_abs64(diminuto_minimumof(int64_t) + 1) == (diminuto_maximumof(int64_t)));
    }

    {
        int ch;
        int ii;

        /*
         * These functions used to be part of common.
         */

        ii = 0;
        for (ch = 0x00; ch <= 0xff; ++ch) {
            if (ch == '$') {
                assert(hazer_is_nmea(&ch, 1));
            } else {
                assert(!hazer_is_nmea(&ch, 1));
            }
            if (ch == 0xb5) {
                assert(yodel_is_ubx(&ch, 1));
            } else {
                assert(!yodel_is_ubx(&ch, 1));
            }
            if (ch == 0xd3) {
                assert(tumbleweed_is_rtcm(&ch, 1));
            } else {
                assert(!tumbleweed_is_rtcm(&ch, 1));
            }
            if (ch == 0x10) {
                assert(calico_is_cpo(&ch, 1));
            } else {
                assert(!calico_is_cpo(&ch, 1));
            }
            ++ii;
        }

        assert(ii == 256);
    }

    {
        static const hazer_state_t nmea[] = {
            HAZER_STATE_STOP,
            HAZER_STATE_START,
            HAZER_STATE_PAYLOAD,
            HAZER_STATE_MSN,
            HAZER_STATE_LSN,
            HAZER_STATE_CR,
            HAZER_STATE_LF,
            HAZER_STATE_END,
        };
        static const yodel_state_t ubx[] = {
            YODEL_STATE_STOP,
            YODEL_STATE_START,
            YODEL_STATE_SYNC_2,
            YODEL_STATE_CLASS,
            YODEL_STATE_ID,
            YODEL_STATE_LENGTH_1,
            YODEL_STATE_LENGTH_2,
            YODEL_STATE_PAYLOAD,
            YODEL_STATE_CK_A,
            YODEL_STATE_CK_B,
            YODEL_STATE_END,
        };
        static const tumbleweed_state_t rtcm[] = {
            TUMBLEWEED_STATE_STOP,
            TUMBLEWEED_STATE_START,
            TUMBLEWEED_STATE_LENGTH_1,
            TUMBLEWEED_STATE_LENGTH_2,
            TUMBLEWEED_STATE_PAYLOAD,
            TUMBLEWEED_STATE_CRC_1,
            TUMBLEWEED_STATE_CRC_2,
            TUMBLEWEED_STATE_CRC_3,
            TUMBLEWEED_STATE_END,
        };
        static const calico_state_t cpo[] = {
            CALICO_STATE_STOP,
            CALICO_STATE_START,
            CALICO_STATE_ID,
            CALICO_STATE_SIZE,
            CALICO_STATE_SIZE_DLE,
            CALICO_STATE_PAYLOAD,
            CALICO_STATE_PAYLOAD_DLE,
            CALICO_STATE_CS,
            CALICO_STATE_CS_DLE,
            CALICO_STATE_DLE,
            CALICO_STATE_ETX,
            CALICO_STATE_END,
        };
        int nn;
        int uu;
        int rr;
        int cc;
        int ii;

        ii = 0;
        for (nn = 0; nn < countof(nmea); ++nn) {
            for (uu = 0; uu < countof(ubx); ++uu) {
                for (rr = 0; rr < countof(rtcm); ++rr) {
                    for (cc = 0; cc < countof(cpo); ++cc) {
                        if ((nn == 1) && (uu == 1) && (rr == 1) && (cc == 1)) {
                            assert(!common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        } else if ((nn != 0) && (nn != 1)) {
                            assert(!common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        } else if ((uu != 0) && (uu != 1)) {
                            assert(!common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        } else if ((rr != 0) && (rr != 1)) {
                            assert(!common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        } else if ((cc != 0) && (cc != 1)) {
                            assert(!common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        } else {
                            assert(common_machine_is_stalled(nmea[nn], ubx[uu], rtcm[rr], cpo[cc]));
                        }
                        ++ii;
                    }
                }
            }
        }

        assert(ii > 0);
        assert(ii == (countof(nmea) * countof(ubx) * countof(rtcm) * countof(cpo)));
    }

    return 0;
}
