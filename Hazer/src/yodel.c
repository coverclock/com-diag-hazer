/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "com/diag/hazer/yodel.h"
#include "../src/yodel.h"

/******************************************************************************
 *
 ******************************************************************************/

static FILE * debug  = (FILE *)0;

FILE * yodel_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

/******************************************************************************
 *
 ******************************************************************************/

int yodel_initialize(void)
{
    return 0;
}

int yodel_finalize(void)
{
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/

/*
 * Ublox 8, p. 134
 */
yodel_state_t yodel_machine(yodel_state_t state, int ch, void * buffer, size_t size, char ** bp, size_t * sp, size_t * lp)
{
    int done = !0;
    yodel_action_t action = YODEL_ACTION_SKIP;

    /*
     * Short circuit state machine for some characters.
     */

    switch (ch) {

    case EOF:
        DEBUG("EOF %d!\n", ch);
        state = YODEL_STATE_EOF;
        break;

    default:
        /* Do nothing. */
        break;

    }

    /*
     * Advance state machine based on stimulus.
     */

    switch (state) {

    case YODEL_STATE_EOF:
        *bp = (char *)buffer;
        *sp = 0;
        break;

    case YODEL_STATE_START:
        if (ch == YODEL_STIMULUS_SYNC_1) {
            DEBUG("UBX 0x%02x.\n", ch);
            state = YODEL_STATE_SYNC_2;
            action = YODEL_ACTION_SAVE;
            *bp = (char *)buffer;
            *sp = size;
            *lp = 0;
        }
        break;

    case YODEL_STATE_SYNC_2:
        if (ch == YODEL_STIMULUS_SYNC_2) {
            state = YODEL_STATE_CLASS;
            action = YODEL_ACTION_SAVE;
        } else {
            state = YODEL_STATE_START;
        }
        break;

    case YODEL_STATE_CLASS:
        state = YODEL_STATE_ID;
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_ID:
        state = YODEL_STATE_LENGTH_1;
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_LENGTH_1:
        /*
         * Ublox8, p. 134: "little endian"
         */
        *lp = ((unsigned)ch); /* LSB */
        DEBUG("LENGTH1 0x%02x %zu.\n", ch, *lp);
        state = YODEL_STATE_LENGTH_2;
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_LENGTH_2:
        /*
         * Ublox8, p. 134: "little endian"
         */
        *lp |= ((unsigned)ch) << 8; /* MSB */
        DEBUG("LENGTH2 0x%02x %zu.\n", ch, *lp);
        state = YODEL_STATE_PAYLOAD;
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_PAYLOAD:
        if (((*lp)--) > 1) {
            state = YODEL_STATE_PAYLOAD;
        } else {
            state = YODEL_STATE_CK_A;
        }
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_CK_A:
        state = YODEL_STATE_CK_B;
        action = YODEL_ACTION_SAVE;
        break;

    case YODEL_STATE_CK_B:
        state = YODEL_STATE_END;
        action = YODEL_ACTION_TERMINATE;
        break;

    case YODEL_STATE_END:
        DEBUG("END 0x%02x!\n", ch);
        break;

    /*
     * No default: must handle all cases.
     */

    }

    /*
     * Perform associated action.
     */

    switch (action) {

    case YODEL_ACTION_SKIP:
        DEBUG("SKIP 0x%02x?\n", ch);
        break;

    case YODEL_ACTION_SAVE:
        if ((*sp) > 0) {
            *((*bp)++) = ch;
            (*sp) -= 1;
            DEBUG("SAVE 0x%02x.\n", ch);
        } else {
            state = YODEL_STATE_START;
            DEBUG("LONG!\n");
        }
        break;

    case YODEL_ACTION_TERMINATE:
        /*
         * It seems like it's not really meaningful to NUL-terminate a binary
         * UBX packet, but it is. Doing so simplifies user code that doesn't
         * know yet the format of the data in the buffer, e.g. in the case of
         * IP datagrams. And it guarantees that we don't run off the end of
         * some UBX messages (like UBX-MON-VER) that contain null terminated
         * strings in their payloads.
         */
        if ((*sp) > 1) {
            *((*bp)++) = ch;
            (*sp) -= 1;
            DEBUG("SAVE 0x%02x.\n", ch);
            *((*bp)++) = '\0';
            (*sp) -= 1;
            DEBUG("SAVE 0x%02x.\n", '\0');
            (*sp) = size - (*sp);
        } else {
            state = YODEL_STATE_START;
            DEBUG("LONG!\n");
        }
        break;

    /*
     * No default: must handle all cases.
     */

    }

    /*
     * Done.
     */

    return state;
}

/******************************************************************************
 *
 ******************************************************************************/

/*
 * The portion of the buffer being summed includes the length, but we have
 * to compute the length first to do the checksum. Seems chicken-and-egg,
 * but I've seen the same thing in TCP headers; in fact [Ublox p. 74] says
 * this is the Fletcher checksum from RFC 1145. It refers to this as an
 * eight-bit checksum, but the result is really sixteen bits (CK_A and
 * CK_B), although it is performed eight-bits at a time on the input data.
 */
const void * yodel_checksum_buffer(const void * buffer, size_t size, uint8_t * ck_ap, uint8_t * ck_bp)
{
    const void * result = (void *)0;
    const uint8_t * bp = (const uint8_t *)buffer;
    uint8_t ck_a = 0;
    uint8_t ck_b = 0;
    uint16_t length = 0;

    /*
     * Ublox8, p. 134: "little endian"
     */
    length = ((uint8_t)(bp[YODEL_UBX_LENGTH_MSB])) << 8;
    length |= ((uint8_t)(bp[YODEL_UBX_LENGTH_LSB]));
    length += YODEL_UBX_SUMMED;

    if ((length + YODEL_UBX_UNSUMMED) <= size) {

        for (bp += YODEL_UBX_CLASS; length > 0; --length) {
            ck_a += *(bp++);
            ck_b += ck_a;
        }

        *ck_ap = ck_a;
        *ck_bp = ck_b;

        result = bp;

    }

    return result;
}

ssize_t yodel_length(const void * buffer, size_t size)
{
       ssize_t result = -1;
       uint16_t length = 0;
       const unsigned char * packet = (const char *)0;

       packet = (const char *)buffer;

       if (size < YODEL_UBX_SHORTEST) {
           /* Do nothing. */
       } else if (packet[YODEL_UBX_SYNC_1] != YODEL_STIMULUS_SYNC_1) {
           /* Do nothing. */
       } else if (packet[YODEL_UBX_SYNC_2] != YODEL_STIMULUS_SYNC_2) {
           /* Do nothing. */
       } else {
           /*
            * Ublox8, p. 134: "little endian"
            */
           length = packet[YODEL_UBX_LENGTH_MSB] << 8;
           length |= packet[YODEL_UBX_LENGTH_LSB];
           if (length > (size - YODEL_UBX_SHORTEST)) {
               /* Do nothing. */
           } else {
               result = length;
               result += YODEL_UBX_SHORTEST;
           }
       }

       return result;
}

/******************************************************************************
 *
 ******************************************************************************/

int yodel_ubx_nav_hpposllh(yodel_ubx_nav_hpposllh_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_NAV_HPPOSLLH_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_NAV_HPPOSLLH_Id) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_NAV_HPPOSLLH_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), sizeof(*mp));
        COM_DIAG_YODEL_LETOH(mp->iTOW);
        COM_DIAG_YODEL_LETOH(mp->lon);
        COM_DIAG_YODEL_LETOH(mp->lat);
        COM_DIAG_YODEL_LETOH(mp->height);
        COM_DIAG_YODEL_LETOH(mp->hMSL);
        COM_DIAG_YODEL_LETOH(mp->hAcc);
        COM_DIAG_YODEL_LETOH(mp->vAcc);
        rc = 0;
    }

    return rc;
}

int yodel_ubx_mon_hw(yodel_ubx_mon_hw_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_MON_HW_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_MON_HW_Id) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_MON_HW_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), sizeof(*mp));
        COM_DIAG_YODEL_LETOH(mp->pinSel);
        COM_DIAG_YODEL_LETOH(mp->pinBank);
        COM_DIAG_YODEL_LETOH(mp->pinDir);
        COM_DIAG_YODEL_LETOH(mp->pinVal);
        COM_DIAG_YODEL_LETOH(mp->noisePerMS);
        COM_DIAG_YODEL_LETOH(mp->agcCnt);
        COM_DIAG_YODEL_LETOH(mp->usedMask);
        COM_DIAG_YODEL_LETOH(mp->pinIrq);
        COM_DIAG_YODEL_LETOH(mp->pullH);
        COM_DIAG_YODEL_LETOH(mp->pullL);
        rc = 0;
    }

    return rc;
}

int yodel_ubx_nav_status(yodel_ubx_nav_status_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_NAV_STATUS_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_NAV_STATUS_Id) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_NAV_STATUS_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), sizeof(*mp));
        COM_DIAG_YODEL_LETOH(mp->iTOW);
        COM_DIAG_YODEL_LETOH(mp->ttff);
        COM_DIAG_YODEL_LETOH(mp->msss);
        rc = 0;
    }

    return rc;
}

int yodel_ubx_ack(yodel_ubx_ack_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_ACK_Class) {
        /* Do nothing. */
    } else if ((hp[YODEL_UBX_ID] != YODEL_UBX_ACK_ACK_Id) && (hp[YODEL_UBX_ID] != YODEL_UBX_ACK_NAK_Id)) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_ACK_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), YODEL_UBX_ACK_Length);
        mp->state = (hp[YODEL_UBX_ID] == YODEL_UBX_ACK_ACK_Id);
        rc = 0;
    }

    return rc;
}

int yodel_ubx_cfg_valget(void * bp, ssize_t length)
{
    int rc = -1;
    unsigned char * hp = (unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_CFG_VALGET_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_CFG_VALGET_Id) {
        /* Do nothing. */
    } else if (length < (YODEL_UBX_SHORTEST + YODEL_UBX_CFG_VALGET_Length)) {
        /* Do nothing. */
    } else {
    	yodel_ubx_cfg_valget_t * pp = (yodel_ubx_cfg_valget_t *)0;
    	char * bb = (char *)0;
    	const char * ee = (const char *)0;
    	yodel_ubx_cfg_valget_key_t kk = 0;
    	size_t ss = 0;
    	size_t ll = 0;
    	uint8_t vv1 = 0;
    	uint16_t vv16 = 0;
    	uint32_t vv32 = 0;
    	uint64_t vv64 = 0;

    	rc = 0;

    	pp = (yodel_ubx_cfg_valget_t *)&(hp[YODEL_UBX_PAYLOAD]);
    	ee = &(hp[length - YODEL_UBX_CHECKSUM]);

    	for (bb = &(pp->cfgData[0]); bb < ee; bb += ll) {

			if ((bb + sizeof(kk)) > ee) {
				rc = -1;
				break;
			}

			memcpy(&kk, bb, sizeof(kk));
			kk = le32toh(kk);
			memcpy(bb, &kk, sizeof(kk));

			bb += sizeof(kk);

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
			default:
				ll = 0;
				break;
			}

			if (ll == 0) {
				rc = -1;
				break;
			}

			if ((bb + ll) > ee) {
				rc = -1;
				break;
			}

			switch (ss) {
			case YODEL_UBX_CFG_VALGET_Size_TWO:
				memcpy(&vv16, bb, sizeof(vv16));
				vv16 = le16toh(vv16);
				memcpy(bb, &vv16, sizeof(vv16));
				break;
			case YODEL_UBX_CFG_VALGET_Size_FOUR:
				memcpy(&vv32, bb, sizeof(vv32));
				vv32 = le32toh(vv32);
				memcpy(bb, &vv32, sizeof(vv32));
				break;
			case YODEL_UBX_CFG_VALGET_Size_EIGHT:
				memcpy(&vv64, bb, sizeof(vv64));
				vv64 = le64toh(vv64);
				memcpy(bb, &vv64, sizeof(vv64));
				break;
			default:
				break;
			}

		}
    }

    return rc;
}

int yodel_ubx_mon_ver(const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_MON_VER_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_MON_VER_Id) {
        /* Do nothing. */
    } else {
        rc = 0;
    }

    return rc;
}

int yodel_ubx_rxm_rtcm(yodel_ubx_rxm_rtcm_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_RXM_RTCM_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_RXM_RTCM_Id) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_RXM_RTCM_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), sizeof(*mp));
        COM_DIAG_YODEL_LETOH(mp->subType);
        COM_DIAG_YODEL_LETOH(mp->refStation);
        COM_DIAG_YODEL_LETOH(mp->msgType);
        rc = 0;
    }

    return rc;
}

int yodel_ubx_nav_svin(yodel_ubx_nav_svin_t * mp, const void * bp, ssize_t length)
{
    int rc = -1;
    const unsigned char * hp = (const unsigned char *)bp;

    if (hp[YODEL_UBX_CLASS] != YODEL_UBX_NAV_SVIN_Class) {
        /* Do nothing. */
    } else if (hp[YODEL_UBX_ID] != YODEL_UBX_NAV_SVIN_Id) {
        /* Do nothing. */
    } else if (length != (YODEL_UBX_SHORTEST + YODEL_UBX_NAV_SVIN_Length)) {
        /* Do nothing. */
    } else {
        memcpy(mp, &(hp[YODEL_UBX_PAYLOAD]), sizeof(*mp));
        COM_DIAG_YODEL_LETOH(mp->iTOW);
        COM_DIAG_YODEL_LETOH(mp->dur);
        COM_DIAG_YODEL_LETOH(mp->meanX);
        COM_DIAG_YODEL_LETOH(mp->meanY);
        COM_DIAG_YODEL_LETOH(mp->meanZ);
        COM_DIAG_YODEL_LETOH(mp->meanAcc);
        COM_DIAG_YODEL_LETOH(mp->obs);
        rc = 0;
    }

    return rc;
}
