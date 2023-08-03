/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_FIX_
#define _H_COM_DIAG_HAZER_GPSTOOL_FIX_

/**
 * @file
 * @copyright Copyright 2017-2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This declares and defines the gpstool fix functions.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_log.h"
#include "types.h"
#include "globals.h"

/**
 * Do the busywork necessary to mark the acquisition of a Fix.
 * @param string is the string to log at level NOTICE.
 */
static inline void fix_acquired(const char * string)
{
    if (Fix < 0) {
        Fix = Now;
        DIMINUTO_LOG_NOTICE("Fix Acquired %llds %s\n", (diminuto_lld_t)((Fix - Event) / Frequency), string);
        Event = Fix;
    }
}

/**
 * Do the busywork necessary to mark the relinquishment of a Fix.
 * @param string is the string to log at level NOTICE.
 */
static inline void fix_relinquished(const char * string)
{
    if (Fix >= 0) {
        Event = Now;
        DIMINUTO_LOG_NOTICE("Fix Lost %llds %s\n", (diminuto_lld_t)((Event - Fix) / Frequency), string);
        Fix = -1;
    }
}

#endif
