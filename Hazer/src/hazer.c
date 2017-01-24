/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in README.h<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Assay.html<BR>
 */

#include <stdio.h>
#include <errno.h>
#include "hazer.h"
#include "com/diag/hazer/hazer.h"

static FILE * debug  = (FILE *)0;

#if 1
#   define DEBUG(...)   ((debug != (FILE *)0) ? fprintf(debug, __VA_ARGS__) : 0)
#else
#   define DEBUG(...)   ((void)0)
#endif

FILE * hazer_debug(FILE * now)
{
    FILE * was;

    was = debug;
    debug = now;

    return was;
}

ssize_t hazer_sentence_read(FILE *fp, void * buffer, size_t size)
{
    char * bb;
    size_t ss;
    int ch;

    while (!0) {

        bb = (char *)buffer;
        ss = size;

        DEBUG("BEGIN\n");

        while (!0) {
            if ((ch = fgetc(fp)) == EOF) {
                DEBUG("EOF 0x%x\n", ch);
                return 0;
            }
            if (ch == HAZER_NMEA_SENTENCE_START) {
                DEBUG("START '%c'\n", ch);
                break;
            }
            if (ch == HAZER_NMEA_SENTENCE_ENCAPSULATION) {
                DEBUG("ENCAPSULATION '%c'\n", ch);
                break;
            }
            DEBUG("SKIP 0x%x\n", ch);
        }

        if (ss == 0) {
            continue;
        }
        *(bb++) = ch;
        --ss;

        while (!0) {
            if ((ch = fgetc(fp)) == EOF) {
                DEBUG("EOF 0x%x\n", ch);
                return 0;
            }
            if (ss == 0) {
                break;
            }
            *(bb++) = ch;
            --ss;
            if (ch == HAZER_NMEA_SENTENCE_CR) {
                DEBUG("CR 0x%x\n", ch);
                break;
            }
            DEBUG("SAVE '%c'\n", ch);
        }

        if (ss == 0) {
            continue;
        }

        if ((ch = fgetc(fp)) == EOF) {
            DEBUG("EOF 0x%x\n", ch);
            return 0;
        }
        if (ch != HAZER_NMEA_SENTENCE_LF) {
            continue;
        }
        DEBUG("LF 0x%x\n", ch);
        if (ss == 0) {
            continue;
        }
        *(bb++) = ch;
        --ss;

        DEBUG("NUL\n");
        if (ss == 0) {
            continue;
        }
        *(bb++) = '\0';
        --ss;

        DEBUG("END\n");
        break;

    }

    return size - ss;
}
