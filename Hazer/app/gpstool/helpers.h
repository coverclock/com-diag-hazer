/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_
#define _H_COM_DIAG_HAZER_GPSTOOL_HELPERS_

/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares and defines the gpstool Helpers.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_frequency.h"
#include "com/diag/diminuto/diminuto_log.h"
#include "com/diag/diminuto/diminuto_time.h"
#include "types.h"
#include "globals.h"

/**
 * Return true if specified number of seconds has elapsed, and if so
 * update the previous elapsed seconds value-result variable.
 * @param wasp points to the variable containing previous elapsed seconds.
 * @param seconds is the number of seconds desired to elapse, <0 for never, 0 for always.
 * @return true if the specified number of seconds has elapsed.
 */
extern int expired(seconds_t * wasp, seconds_t seconds);

/**
 * Common function to count down the expiration fields in the database.
 * @param ep points to the expiration field to count down.
 * @param elapsed is the number of ticks to count down.
 */
extern void countdown(hazer_expiry_t * ep, diminuto_sticks_t elapsed);

/**
 * Track RTK updates by encoding each received RTCM message as a single
 * character in a shifting string.
 * @param number is the RTCM message number.
 * @param up points to the updates union.
 */
extern void collect(int number, tumbleweed_updates_t * up);

/**
 * Return true if the second field in the vector matches the PUBX message
 * id. No length checks are done here, so the safeties are off.
 * @param vector is the Hazer parsed vector (a 2D array).
 * @param id is the two letter message identifier.
 * @return true if the id matches the field.
 */
static inline int is_pubx(const hazer_vector_t vector, const char * id)
{
    return ((vector[1][0] == id[0]) && (vector[1][1] == id[1]) && (vector[1][2] == '\0'));
}

/**
 * Return true if the NMEA sentence name following the talker matches
 * three letter name. No length checks are done here, so the safeties are
 * off.
 * @param vector is the Hazer parsed vector (a 2D array).
 * @param name is the three letter name.
 * @return true if the name matches the field.
 */
static inline int is_nmea(const hazer_vector_t vector, const char * name)
{
    return ((vector[0][3] == name[0]) && (vector[0][4] == name[1]) && (vector[0][5] == name[2]) && (vector[0][6] == '\0'));
}

/**
 * Do the busywork necessary to mark the acquisition of a Fix.
 * @param string is the string to log at level NOTICE.
 */
static inline void acquire_fix(const char * string)
{
    if (Fix < 0) {
        Fix = Now;
        DIMINUTO_LOG_NOTICE("Fix Acquired %llds %s\n", (long long signed int)((Fix - Event) / Frequency), string);
        Event = Fix;
    }
}

/**
 * Do the busywork necessary to mark the relinquishment of a Fix.
 * @param string is the string to log at level NOTICE.
 */
static inline void relinquish_fix(const char * string)
{
    if (Fix >= 0) {
        Event = Now;
        DIMINUTO_LOG_NOTICE("Fix Lost %llds %s\n", (long long signed int)((Event - Fix) / Frequency), string);
        Fix = -1;
    }
}

/**
 * Returns true if there are GSV views pending for any constellation.
 * @param va points to the array of all satellites being viewed.
 * @return true if there are GSV views pending for any constellation.
 */
extern int has_pending(const hazer_view_t va[]);

#endif
