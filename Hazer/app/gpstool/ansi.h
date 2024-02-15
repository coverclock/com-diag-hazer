/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_HAZER_GPSTOOL_ANSI_
#define _H_COM_DIAG_HAZER_GPSTOOL_ANSI_

/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This defines the gpstool ANSI escape sequences.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include "com/diag/diminuto/diminuto_ansi.h"

/**
 * ANSI [1,1] erase to end of screen.
 */
static const char ANSI_INI[] = DIMINUTO_ANSI_POSITION_CURSOR(1,1) DIMINUTO_ANSI_ERASE_SCREEN;

/**
 * ANSI [1,1] erase to end of line.
 */
static const char ANSI_INP[] = DIMINUTO_ANSI_POSITION_CURSOR(1,1) DIMINUTO_ANSI_ERASE_LINE;

/**
 * ANSI [2,1] erase to end of line.
 */
static const char ANSI_OUT[] = DIMINUTO_ANSI_POSITION_CURSOR(2,1) DIMINUTO_ANSI_ERASE_LINE;

/**
 * ANSI [3,1] locate cursor.
 */
static const char ANSI_LOC[] = DIMINUTO_ANSI_POSITION_CURSOR(3,1);

/**
 * ANSI [x,y] erase to end of screen.
 */
static const char ANSI_END[] = DIMINUTO_ANSI_ERASE_SCREEN;

#endif
