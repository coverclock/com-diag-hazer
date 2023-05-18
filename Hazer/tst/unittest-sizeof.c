/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the NMEA unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include <errno.h>
#include "com/diag/hazer/common.h"
#include "com/diag/hazer/coordinates.h"
#include "com/diag/hazer/datagram.h"
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/tumbleweed.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

#define PRINTSIZEOF(_TYPE_) \
    fprintf(stderr, "sizeof(%s)=%zu\n", #_TYPE_, sizeof(_TYPE_))

int main(void)
{
    hazer_position_t position[HAZER_SYSTEM_TOTAL];
    hazer_active_t active[HAZER_SYSTEM_TOTAL];
    hazer_view_t view[HAZER_SYSTEM_TOTAL];

    PRINTSIZEOF(coordinates_format_t);
    PRINTSIZEOF(datagram_buffer_t);
    PRINTSIZEOF(datagram_header_t);
    PRINTSIZEOF(datagram_sequence_t);
    PRINTSIZEOF(hazer_action_t);
    PRINTSIZEOF(hazer_active_t);
    PRINTSIZEOF(hazer_band_t);
    PRINTSIZEOF(hazer_context_t);
    PRINTSIZEOF(hazer_expiry_t);
    PRINTSIZEOF(hazer_mode_t);
    PRINTSIZEOF(hazer_nmeaid_t);
    PRINTSIZEOF(hazer_nmea_t);
    PRINTSIZEOF(hazer_position_t);
    PRINTSIZEOF(hazer_pubxid_t);
    PRINTSIZEOF(hazer_satellite_t);
    PRINTSIZEOF(hazer_state_t);
    PRINTSIZEOF(hazer_system_t);
    PRINTSIZEOF(hazer_talker_t);
    PRINTSIZEOF(hazer_view_t);
    PRINTSIZEOF(tumbleweed_action_t);
    PRINTSIZEOF(tumbleweed_context_t);
    PRINTSIZEOF(tumbleweed_state_t);
    PRINTSIZEOF(yodel_action_t);
    PRINTSIZEOF(yodel_context_t);
    PRINTSIZEOF(yodel_id_t);
    PRINTSIZEOF(yodel_state_t);
    PRINTSIZEOF(yodel_system_t);
    PRINTSIZEOF(yodel_ubx_ack_t);
    PRINTSIZEOF(yodel_ubx_cfg_valget_key_t);
    PRINTSIZEOF(yodel_ubx_cfg_valget_t);
    PRINTSIZEOF(yodel_ubx_header_t);
    PRINTSIZEOF(yodel_ubx_mon_comms_t);
    PRINTSIZEOF(yodel_ubx_mon_hw_t);
    PRINTSIZEOF(yodel_ubx_nav_att_t);
    PRINTSIZEOF(yodel_ubx_nav_clock_t);
    PRINTSIZEOF(yodel_ubx_nav_hpposllh_t);
    PRINTSIZEOF(yodel_ubx_nav_odo_t);
    PRINTSIZEOF(yodel_ubx_nav_pvt_t);
    PRINTSIZEOF(yodel_ubx_nav_status_t);
    PRINTSIZEOF(yodel_ubx_nav_svin_t);
    PRINTSIZEOF(yodel_ubx_nav_timegps_t);
    PRINTSIZEOF(yodel_ubx_nav_timeutc_t);
    PRINTSIZEOF(yodel_ubx_rxm_rtcm_t);
    PRINTSIZEOF(yodel_ubx_tim_tp_t);

    PRINTSIZEOF(active);
    PRINTSIZEOF(position);
    PRINTSIZEOF(view);
}
