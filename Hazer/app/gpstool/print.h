/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_PRINT_
#define _H_COM_DIAG_HAZER_GPSTOOL_PRINT_

/**
 * @file
 * @copyright Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares the gpstool Print API.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "com/diag/diminuto/diminuto_types.h"
#include "types.h"

/**
 * Print all of the active satellites used for the most recent fix.
 * @param fp points to the FILE stream.
 * @param aa points to the array of active satellites.
 */
extern void print_actives(FILE * fp, const hazer_active_t aa[]);

/**
 * Print all of the satellites currently being viewed by the receiver.
 * @param fp points to the FILE stream.
 * @param va points to the array of all satellite being viewed.
 * @param aa points to the array of active satellites.
 */
extern void print_views(FILE *fp, const hazer_view_t va[], const hazer_active_t aa[]);

/**
 * Print the local (Juliet) time (and the release string).
 * @param fp points to the FILE stream.
 */
extern void print_local(FILE * fp);

/**
 * Print the hardware monitor details.
 * @param fp points to the FILE stream.
 * @param hp points to the hardware monitor details.
 */
extern void print_hardware(FILE * fp, const yodel_hardware_t * hp);

/**
 * Print the navigation status details.
 * @param fp points to the FILE stream.
 * @param sp points to the navigation status details.
 */
extern void print_status(FILE * fp, const yodel_status_t * sp);

/**
 * Print all of the navigation position fixes.
 * @param fp points to the FILE stream.
 * @param pa points to an array of positions.
 * @param pps is the current value of the 1PPS strobe.
 * @param bytes is the total number of bytes sent and received over the network.
 */
extern void print_positions(FILE * fp, const hazer_position_t pa[], int pps, uint64_t bytes);

/**
 * Print information about the base and the rover that communicate via RTCM.
 * @param fp points to the FILE stream.
 * @param bp points to the base structure.
 * @param rp points to the rover structure.
 * @param kp points to the message structure.
 * @param up points to the updates structure.
 */
extern void print_corrections(FILE * fp, const yodel_base_t * bp, const yodel_rover_t * rp, const tumbleweed_message_t * kp, const tumbleweed_updates_t * up);

/**
 * Print information about the high-precision positioning solution that UBX
 * provides. I think this is the same result as NMEA but is expressed with
 * the maximum precision available in the underlying device and beyond which
 * NMEA can express.
 * @param fp points to the FILE stream.
 * @param sp points to the solutions structure.
 */
extern void print_solution(FILE * fp, const yodel_solution_t * sp);

/**
 * @param fp points to the FILE stream.
 * @param sp points to the attitude structure.
 */
extern void print_attitude(FILE * fp, const yodel_attitude_t * sp);

/**
 * @param fp points to the FILE stream.
 * @param sp points to the odometer structure.
 */
extern void print_odometer(FILE * fp, const yodel_odometer_t * sp);

/**
 * @param fp points to the FILE stream.
 * @param sp points to the position/velocity/time structure.
 */
extern void print_posveltim(FILE * fp, const yodel_posveltim_t * sp);

#endif
