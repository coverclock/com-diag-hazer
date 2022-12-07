/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021-2022 Digital Aggregates Corporation, Colorado, USA.
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
    const unsigned char * bb = (const unsigned char *)0;
    const unsigned char * ee = &((const unsigned char *)buffer)[length - YODEL_UBX_CHECKSUM];
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
        layer = "UNK";
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

void process_ubx_mon_comms(const void * buffer, ssize_t length)
{
    int ii = 0;
    int jj = 0;
    yodel_ubx_mon_comms_t comms = YODEL_UBX_MON_COMMS_INITIALIZER;
    const uint8_t * bp = &(((const uint8_t *)buffer)[YODEL_UBX_PAYLOAD]);

    memcpy(&comms.prefix, bp, sizeof(comms.prefix));

    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS version               %u\n", comms.prefix.version);
    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS nPorts                %u\n", comms.prefix.nPorts);
    DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS txErrors              0x%02x\n", comms.prefix.txErrors);

    for (ii = 0; ii < diminuto_countof(comms.prefix.protIds); ++ii) {

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS protIds[%d]            %u\n", ii, comms.prefix.protIds[ii]);

    }

    bp += sizeof(comms.prefix);

    for (ii = 0; ii < comms.prefix.nPorts; ++ii) {

        memcpy(&comms.port[ii], bp, sizeof(comms.port[ii]));

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d] portId        0x%04x\n", ii, comms.port[ii].portId);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   txPending   %u\n", ii, comms.port[ii].txPending);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   txBytes     %u\n", ii, comms.port[ii].txBytes);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   txUsage     %u\n", ii, comms.port[ii].txUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   txPeakUsage %u\n", ii, comms.port[ii].txPeakUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   rxPending   %u\n", ii, comms.port[ii].rxPending);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   rxBytes     %u\n", ii, comms.port[ii].rxBytes);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   rxUsage     %u\n", ii, comms.port[ii].rxUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   rxPeakUsage %u\n", ii, comms.port[ii].rxPeakUsage);
        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   overrunErrs %u\n", ii, comms.port[ii].overrunErrs);

        for (jj = 0; jj < countof(comms.port[ii].msgs); ++jj) {
            DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   msgs[%d]     %u\n", ii, jj, comms.port[ii].msgs[jj]);
        }

        DIMINUTO_LOG_INFORMATION("Process UBX-MON-COMMS port[%d]   skipped     %u\n", ii,  comms.port[ii].skipped);

        bp += sizeof(comms.port[ii]);
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
