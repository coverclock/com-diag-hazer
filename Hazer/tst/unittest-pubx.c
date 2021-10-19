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
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_position_t position = HAZER_POSITION_INITIALIZER;
        hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

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

        rc = hazer_is_pubx_id(vector, "00");
        assert(rc == !0);

        rc = hazer_parse_pubx_position(&position, &active, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "PUBX") == 0);
        assert(position.sat_used == 4);

        assert(position.utc_nanoseconds == 65250000000000ULL);
        assert(position.tot_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.lat_nanominutes == 2388047880000LL);
        assert(position.lon_nanominutes == -6310628200000LL);
        assert(position.sep_millimeters == 1703346LL);
        assert(position.sog_millimetersperhour == 1234000LL);
        assert(position.cog_nanodegrees == 290120000000LL);
        assert(strcmp(active.label, "PUBX") == 0);
        assert(active.hdop == 123);
        assert(active.vdop == 456);
        assert(active.tdop == 789);

        assert(!hazer_is_valid_time(&position));
    }

    {
        static const char * DATA = "$PUBX,03,19,5,-,051,34,,000,10,U,240,16,10,000,13,-,072,38,,000,15,e,113,56,20,000,16,-,309,15,,000,18,U,321,61,30,000,20,e,061,04,40,000,23,-,248,48,,000,25,e,193,-2,50,000,26,-,276,17,,000,29,-,167,58,,000,65,-,047,68,,000,66,U,201,43,60,000,72,-,033,21,,000,79,-,284,02,,000,80,-,335,03,,000,81,-,326,35,,000,87,e,115,37,70,000,88,U,056,72,80,000*1E\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_view_t view[HAZER_SYSTEM_TOTAL] = HAZER_VIEWS_INITIALIZER;
        hazer_active_t active[HAZER_SYSTEM_TOTAL] = HAZER_ACTIVES_INITIALIZER;
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

        rc = hazer_is_pubx_id(vector, "03");
        assert(rc == !0);

        rc = hazer_parse_pubx_svstatus(view, active, vector, count);
        assert(rc != 0);

/*
$PUBX,03,19,
5,-,051,34,,000,    GPS
10,U,240,16,10,000,   GPS
13,-,072,38,,000,   GPS
15,e,113,56,20,000,   GPS
16,-,309,15,,000,   GPS
18,U,321,61,30,000,   GPS
20,e,061,04,40,000,   GPS
23,-,248,48,,000,   GPS
25,e,193,-2,50,000,   GPS
26,-,276,17,,000,   GPS
29,-,167,58,,000,   GPS
*/
        
        assert(strcmp(view[HAZER_SYSTEM_GPS].label, "PUBX") == 0);
        assert(view[HAZER_SYSTEM_GPS].view == 19);
        assert(view[HAZER_SYSTEM_GPS].channels == 11);
        assert(view[HAZER_SYSTEM_GPS].pending == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[0].id == 5);
        assert(view[HAZER_SYSTEM_GPS].sat[0].azm_degrees == 51);
        assert(view[HAZER_SYSTEM_GPS].sat[0].elv_degrees == 34);
        assert(view[HAZER_SYSTEM_GPS].sat[0].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[0].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[0].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[0].untracked == !0);

        assert(view[HAZER_SYSTEM_GPS].sat[1].id == 10);
        assert(view[HAZER_SYSTEM_GPS].sat[1].azm_degrees == 240);
        assert(view[HAZER_SYSTEM_GPS].sat[1].elv_degrees == 16);
        assert(view[HAZER_SYSTEM_GPS].sat[1].snr_dbhz == 10);
        assert(view[HAZER_SYSTEM_GPS].sat[1].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[1].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[1].untracked == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[2].id == 13);
        assert(view[HAZER_SYSTEM_GPS].sat[2].azm_degrees == 72);
        assert(view[HAZER_SYSTEM_GPS].sat[2].elv_degrees == 38);
        assert(view[HAZER_SYSTEM_GPS].sat[2].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[2].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[2].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[2].untracked == !0);

        assert(view[HAZER_SYSTEM_GPS].sat[3].id == 15);
        assert(view[HAZER_SYSTEM_GPS].sat[3].azm_degrees == 113);
        assert(view[HAZER_SYSTEM_GPS].sat[3].elv_degrees == 56);
        assert(view[HAZER_SYSTEM_GPS].sat[3].snr_dbhz == 20);
        assert(view[HAZER_SYSTEM_GPS].sat[3].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[3].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[3].untracked == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[4].id == 16);
        assert(view[HAZER_SYSTEM_GPS].sat[4].azm_degrees == 309);
        assert(view[HAZER_SYSTEM_GPS].sat[4].elv_degrees == 15);
        assert(view[HAZER_SYSTEM_GPS].sat[4].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[4].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[4].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[4].untracked == !0);

        assert(view[HAZER_SYSTEM_GPS].sat[5].id == 18);
        assert(view[HAZER_SYSTEM_GPS].sat[5].azm_degrees == 321);
        assert(view[HAZER_SYSTEM_GPS].sat[5].elv_degrees == 61);
        assert(view[HAZER_SYSTEM_GPS].sat[5].snr_dbhz == 30);
        assert(view[HAZER_SYSTEM_GPS].sat[5].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[5].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[5].untracked == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[6].id == 20);
        assert(view[HAZER_SYSTEM_GPS].sat[6].azm_degrees == 61);
        assert(view[HAZER_SYSTEM_GPS].sat[6].elv_degrees == 4);
        assert(view[HAZER_SYSTEM_GPS].sat[6].snr_dbhz == 40);
        assert(view[HAZER_SYSTEM_GPS].sat[6].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[6].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[6].untracked == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[7].id == 23);
        assert(view[HAZER_SYSTEM_GPS].sat[7].azm_degrees == 248);
        assert(view[HAZER_SYSTEM_GPS].sat[7].elv_degrees == 48);
        assert(view[HAZER_SYSTEM_GPS].sat[7].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[7].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[7].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[7].untracked == !0);

        assert(view[HAZER_SYSTEM_GPS].sat[8].id == 25);
        assert(view[HAZER_SYSTEM_GPS].sat[8].azm_degrees == 193);
        assert(view[HAZER_SYSTEM_GPS].sat[8].elv_degrees == -2);
        assert(view[HAZER_SYSTEM_GPS].sat[8].snr_dbhz == 50);
        assert(view[HAZER_SYSTEM_GPS].sat[8].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[8].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[8].untracked == 0);

        assert(view[HAZER_SYSTEM_GPS].sat[9].id == 26);
        assert(view[HAZER_SYSTEM_GPS].sat[9].azm_degrees == 276);
        assert(view[HAZER_SYSTEM_GPS].sat[9].elv_degrees == 17);
        assert(view[HAZER_SYSTEM_GPS].sat[9].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[9].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[9].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[9].untracked == !0);

        assert(view[HAZER_SYSTEM_GPS].sat[10].id == 29);
        assert(view[HAZER_SYSTEM_GPS].sat[10].azm_degrees == 167);
        assert(view[HAZER_SYSTEM_GPS].sat[10].elv_degrees == 58);
        assert(view[HAZER_SYSTEM_GPS].sat[10].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[10].signal == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[10].phantom == 0);
        assert(view[HAZER_SYSTEM_GPS].sat[10].untracked == !0);

/*
$PUBX,03,19,
65,-,047,68,,000,   GLONASS
66,U,201,43,60,000,   GLONASS
72,-,033,21,,000,   GLONASS
79,-,284,02,,000,   GLONASS
80,-,335,03,,000,   GLONASS
81,-,326,35,,000,   GLONASS
87,e,115,37,70,000,   GLONASS
88,U,056,72,80,000    GLONASS
*/
        
        assert(strcmp(view[HAZER_SYSTEM_GPS].label, "PUBX") == 0);
        assert(view[HAZER_SYSTEM_GLONASS].view == 19);
        assert(view[HAZER_SYSTEM_GLONASS].channels == 8);
        assert(view[HAZER_SYSTEM_GLONASS].pending == 0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[0].id == 65);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].azm_degrees == 47);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].elv_degrees == 68);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[0].untracked == !0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[1].id == 66);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].azm_degrees == 201);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].elv_degrees == 43);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].snr_dbhz == 60);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[1].untracked == 0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[2].id == 72);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].azm_degrees == 33);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].elv_degrees == 21);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[2].untracked == !0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[3].id == 79);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].azm_degrees == 284);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].elv_degrees == 2);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[3].untracked == !0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[4].id == 80);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].azm_degrees == 335);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].elv_degrees == 3);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[4].untracked == !0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[5].id == 81);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].azm_degrees == 326);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].elv_degrees == 35);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].snr_dbhz == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[5].untracked == !0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[6].id == 87);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].azm_degrees == 115);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].elv_degrees == 37);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].snr_dbhz == 70);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[6].untracked == 0);

        assert(view[HAZER_SYSTEM_GLONASS].sat[7].id == 88);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].azm_degrees == 56);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].elv_degrees == 72);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].snr_dbhz == 80);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].signal == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].phantom == 0);
        assert(view[HAZER_SYSTEM_GLONASS].sat[7].untracked == 0);
    }

    {
        static const char * DATA = "$PUBX,04,180729.00,200821,497248.99,2171,18,-21669119,376.950,21*3E\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_position_t position = HAZER_POSITION_INITIALIZER;
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

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

        rc = hazer_is_pubx_id(vector, "04");
        assert(rc == !0);

        rc = hazer_parse_pubx_time(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "PUBX") == 0);
        assert(position.utc_nanoseconds == 65249000000000ULL);
        assert(position.dmy_nanoseconds == 1629417600000000000ULL); /* date -u -d "August 20 2021" +"%s.%N" */
        assert(position.tot_nanoseconds == (65249000000000ULL + 1629417600000000000ULL));

        position.ticks = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(&position, 1));
        position.ticks = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(&position, 1));
    }

    return 0;
}
