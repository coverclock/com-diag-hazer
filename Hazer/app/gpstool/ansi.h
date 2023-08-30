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

/**
 * ANSI [1,1] erase to end of screen.
 */
static const char ANSI_INI[] = "\033[1;1H\033[0J";

/**
 * ANSI [1,1] erase to end of line.
 */
static const char ANSI_INP[] = "\033[1;1H\033[0K";

/**
 * ANSI [2,1] erase to end of line.
 */
static const char ANSI_OUT[] = "\033[2;1H\033[0K";

/**
 * ANSI [3,1] locate cursor.
 */
static const char ANSI_LOC[] = "\033[3;1H";

/**
 * ANSI [x,y] erase to end of screen.
 */
static const char ANSI_END[] = "\033[0J";

#endif
