/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2021 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the PUBX unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

int main(void)
{
    hazer_debug(stderr);

    {
        static const char * DATA = "$PUBX,00,180730.00,3948.04788,N,10510.62820,W,1703.346,G3,6528077,4616048,1.234,290.12,2.345,,1.23,4.56,7.89,4,0,0*4C\r\n";
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_position_t position = { 0 };
        hazer_active_t active = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy(buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen(buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 22);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_pubx_position(&position, &active, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "PUBX") == 0);
        assert(position.sat_used == 4);

        assert(position.utc_nanoseconds == 65250000000000ULL);
        assert(position.tot_nanoseconds == 65250000000000ULL);
        assert(position.lat_nanominutes == 2388047880000LL);
        assert(position.lon_nanominutes == -6310628200000LL);
        assert(position.sep_millimeters == 1703346LL);
        assert(position.sog_millimetersperhour == 1234000LL);
        assert(position.cog_nanodegrees == 290120000000LL);
        assert(strcmp(active.label, "PUBX") == 0);
        assert(active.hdop == 123);
        assert(active.vdop == 456);
        assert(active.tdop == 789);
    }

    {
        static const char * DATA = "$PUBX,03,19,5,-,051,34,,000,10,U,240,16,,000,13,-,072,38,,000,15,e,113,56,,000,16,-,309,15,,000,18,U,321,61,,000,20,e,061,04,,000,23,-,248,48,,000,25,e,193,-2,,000,26,-,276,17,,000,29,-,167,58,,000,65,-,047,68,,000,66,e,201,43,,000,72,-,033,21,,000,79,-,284,02,,000,80,-,335,03,,000,81,-,326,35,,000,87,e,115,37,,000,88,e,056,72,,000*16\r\n";
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_view_t views[HAZER_SYSTEM_TOTAL] = { { 0, } };
        hazer_active_t actives[HAZER_SYSTEM_TOTAL] = { { 0, } };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy(buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen(buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 118);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_pubx_svstatus(views, actives, vector, count);
        assert(rc != 0);
        
        //assert(strcmp(view.label, "PUBX") == 0);
        //assert(view.view == 19);
        //assert(view.channels == 19);
        //assert(view.pending == 0);

#if 0
5,-,051,34,,000,
10,U,240,16,,000,
13,-,072,38,,000,
15,e,113,56,,000,
16,-,309,15,,000,
18,U,321,61,,000,
20,e,061,04,,000,
23,-,248,48,,000,
25,e,193,-2,,000,
26,-,276,17,,000,
29,-,167,58,,000,
65,-,047,68,,000,
66,e,201,43,,000,
72,-,033,21,,000,
79,-,284,02,,000,
80,-,335,03,,000,
81,-,326,35,,000,
87,e,115,37,,000,
88,e,056,72,,000
#endif

#if 0
        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);
        assert(view.sat[0].signal == 1);
        assert(view.sat[0].phantom == 1);
        assert(view.sat[0].untracked == 1);
#endif
    }

    {
        static const char * DATA = "$PUBX,04,180729.00,200821,497248.99,2171,18,-21669119,376.950,21*3E\r\n";
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_position_t position = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy(buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen(buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 11);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_pubx_time(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "PUBX") == 0);
        assert(position.utc_nanoseconds == 65249000000000ULL);
        assert(position.dmy_nanoseconds == 1629417600000000000ULL); /* date -u -d "August 20 2021" +"%s.%N" */
        assert(position.tot_nanoseconds == (65249000000000ULL + 1629417600000000000ULL));
    }

    return 0;
}
