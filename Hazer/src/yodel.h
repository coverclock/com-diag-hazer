/* vi: set ts=4 expandtab shiftwidth=4: */
#ifndef _H_COM_DIAG_YODEL_PRIVATE_
#define _H_COM_DIAG_YODEL_PRIVATE_

/**
 * @file
 *
 * Copyright 2018 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock <coverclock@diag.com><BR>
 * http://github.com/coverclock/com-diag-hazer<BR>
 * These are the private definitions for the yodel API.
 */

#if 1
#   define DEBUG(...)   ((debug != (FILE *)0) ? fprintf(debug, "UBX " __VA_ARGS__) : 0)
#else
#   define DEBUG(...)   ((void)0)
#endif

#endif
