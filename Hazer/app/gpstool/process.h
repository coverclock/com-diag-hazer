/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_PROCESS_
#define _H_COM_DIAG_HAZER_GPSTOOL_PROCESS_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Process API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/hazer/yodel.h"

extern void process_ubx_cfg_valget(const uint8_t * buffer, ssize_t length);

extern void process_ubx_mon_comms(const yodel_ubx_mon_comms_t * pp, int count);

extern void process_ubx_mon_ver(const uint8_t * buffer, ssize_t length);

#endif
