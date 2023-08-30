/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_DEFAULTS_
#define _H_COM_DIAG_HAZER_GPSTOOL_DEFAULTS_

/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This defines the gpstool Defaults.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "types.h"

/**
 * This is the default convergence threshold in units of 10^-4 meters
 * a.k.a. centicentimeters or ccm. 2.5cm or 0.025m would be 250ccm.
 */
enum DefaultThreshold {
    DEFAULT_THRESHOLD_CENTICENTIMETERS = 250,
};

#endif
