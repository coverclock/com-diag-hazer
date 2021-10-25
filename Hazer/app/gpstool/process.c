/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This implements the gpstool Process API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_countof.h"
#include "com/diag/diminuto/diminuto_log.h"
#include <string.h>
#include "types.h"
#include "constants.h"
#include "globals.h"
#include "process.h"

void process_ubx_cfg_valget(const void * buffer, ssize_t length)
{
    const yodel_ubx_cfg_valget_t * pp = (const yodel_ubx_cfg_valget_t *)&(((const char *)buffer)[YODEL_UBX_PAYLOAD]);
    const char * bb = (const char *)0;
    const char * ee = &((const char *)buffer)[length - YODEL_UBX_CHECKSUM];
    const char * layer = (const char *)0;
    yodel_ubx_cfg_valget_key_t kk = 0;
    size_t ss = 0;
    size_t ll = 0;
    uint8_t vv1 = 0;
    uint16_t vv16 = 0;
    uint32_t vv32 = 0;
    uint64_t vv64 = 0;
    int ii = 0;

    switch (pp->layer) {
    case YODEL_UBX_CFG_VALGET_Layer_RAM:
        layer = "RAM";
        break;
    case YODEL_UBX_CFG_VALGET_Layer_BBR:
        layer = "BBR";
        break;
    case YODEL_UBX_CFG_VALGET_Layer_NVM:
        layer = "NVM";
        break;
    case YODEL_UBX_CFG_VALGET_Layer_ROM:
        layer = "ROM";
        break;
    default:
        layer = "INV";
        break;
    }

    for (bb = &(pp->cfgData[0]); bb < ee; bb += ll) {

        memcpy(&kk, bb, sizeof(kk));

        ss = (kk >> YODEL_UBX_CFG_VALGET_Key_Size_SHIFT) & YODEL_UBX_CFG_VALGET_Key_Size_MASK;

        switch (ss) {
        case YODEL_UBX_CFG_VALGET_Size_BIT:
        case YODEL_UBX_CFG_VALGET_Size_ONE:
            ll = 1;
            break;
        case YODEL_UBX_CFG_VALGET_Size_TWO:
            ll = 2;
            break;
        case YODEL_UBX_CFG_VALGET_Size_FOUR:
            ll = 4;
            break;
        case YODEL_UBX_CFG_VALGET_Size_EIGHT:
            ll = 8;
            break;
        }

        if (ll == 0) {
            break;
        }

        bb += sizeof(kk);

        switch (ss) {
        case YODEL_UBX_CFG_VALGET_Size_BIT:
            memcpy(&vv1, bb, sizeof(vv1));
            DIMINUTO_LOG_INFORMATION("Process UBX-CFG-VALGET v%d %s [%d] 0x%08x 0x%01x\n", pp->version, layer, ii, kk, vv1);
            break;
        case YODEL_UBX_CFG_VALGET_Size_ONE:
            memcpy(&vv1, bb, sizeof(vv1));
            DIMINUTO_LOG_INFORMATION("Process UBX-CFG-VALGET v%d %s [%d] 0x%08x 0x%02x\n", pp->version, layer, ii, kk, vv1);
            break;
        case YODEL_UBX_CFG_VALGET_Size_TWO:
            memcpy(&vv16, bb, sizeof(vv16));
            DIMINUTO_LOG_INFORMATION("Process UBX-CFG-VALGET v%d %s [%d] 0x%08x 0x%04x\n", pp->version, layer, ii, kk, vv16);
            break;
        case YODEL_UBX_CFG_VALGET_Size_FOUR:
            memcpy(&vv32, bb, sizeof(vv32));
            DIMINUTO_LOG_INFORMATION("Process UBX-CFG-VALGET v%d %s [%d] 0x%08x 0x%08x\n", pp->version,layer, ii, kk, vv32);
            break;
        case YODEL_UBX_CFG_VALGET_Size_EIGHT:
            memcpy(&vv64, bb, sizeof(vv64));
            DIMINUTO_LOG_INFORMATION("Process UBX-CFG-VALGET v%d %s [%d] 0x%08x 0x%016llx\n", pp->version, layer, ii, kk, (unsigned long long)vv64);
            break;
        }

        ++ii;

    }

}

void process_ubx_mon_comms(const yodel_ubx_mon_comms_t * pp, int count)
{
    int ii = 0;
    int jj = 0;

    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS version = %u\n", pp->prefix.version);
    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS nPorts = %u\n", pp->prefix.nPorts);
    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS txErrors = 0x%02x\n", pp->prefix.txErrors);

    for (ii = 0; ii < diminuto_countof(pp->prefix.protIds); ++ii) {

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS protIds[%d] = %u\n", ii, pp->prefix.protIds[ii]);

    }

    for (ii = 0; ii < count; ++ii) {

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] portId = 0x%04x\n", ii, pp->port[ii].portId);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] txPending = %u\n", ii, pp->port[ii].txPending);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] txBytes = %u\n", ii, pp->port[ii].txBytes);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] txUsage = %u\n", ii, pp->port[ii].txUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] txPeakUsage = %u\n", ii, pp->port[ii].txPeakUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] rxPending = %u\n", ii, pp->port[ii].rxPending);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] rxBytes = %u\n", ii, pp->port[ii].rxBytes);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] rxUsage = %u\n", ii, pp->port[ii].rxUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] rxPeakUsage = %u\n", ii, pp->port[ii].rxPeakUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] overrunErrs = %u\n", ii, pp->port[ii].overrunErrs);

        for (jj = 0; jj < countof(pp->port[ii].msgs); ++jj) {
            DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] msgs[%d] = %u\n", ii, jj, pp->port[ii].msgs[jj]);
        }

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] skipped = %u\n", ii, pp->port[ii].skipped);
    }

}

void process_ubx_mon_ver(const void * buffer, ssize_t length)
{
    const char * bb = &((const char *)buffer)[YODEL_UBX_PAYLOAD];
    const char * ee = &((const char *)buffer)[length - YODEL_UBX_CHECKSUM];

    do {

        if (bb >= ee) {
            break;
        }

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-VER SW \"%s\"\n", bb);

        bb += YODEL_UBX_MON_VER_swVersion_LENGTH;

        if (bb >= ee) {
            break;
        }

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-VER HW \"%s\"\n", bb);

        bb += YODEL_UBX_MON_VER_hwVersion_LENGTH;

        while (bb < ee) {
            DIMINUTO_LOG_INFORMATION("Process UBX-MON-VER EX \"%s\"\n", bb);
            bb += YODEL_UBX_MON_VER_extension_LENGTH;
        }

    } while (0);

}
