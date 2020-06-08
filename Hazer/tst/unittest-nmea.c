/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2018-2019 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * https://github.com/coverclock/com-diag-hazer<BR>
 */

#include <stdio.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

int main(void)
{
    hazer_debug(stderr);

    {
        static const char * DATA = "$GNGGA,135627.00,3947.65338,N,10509.20216,W,2,12,0.67,1708.6,M,-21.5,M,,0000*4E\r\n";
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
        assert(count == 16);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_gga(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "GGA") == 0);
        assert(position.sat_used == 12);
        assert(position.utc_nanoseconds == 50187000000000ULL);
        assert(position.tot_nanoseconds == 50187000000000ULL);
        assert(position.lat_nanominutes == 2387653380000LL);
        assert(position.lon_nanominutes == -6309202160000LL);
        assert(position.alt_millimeters == 1708600LL);
        assert(position.sep_millimeters == -21500LL);
    }

    {
        static const char * DATA = "$GNRMC,135628.00,A,3947.65337,N,10509.20223,W,0.010,,070818,,,D*74\r\n";
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
        assert(count == 14);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_rmc(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "RMC") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.dmy_nanoseconds == 1533600000000000000ULL); /* date -u -d "August 7 2018" +"%s.%N" */
        assert(position.tot_nanoseconds == (1533600000000000000ULL + 50188000000000ULL));
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
        assert(position.sog_microknots == 10000ULL);
        assert(position.cog_nanodegrees == 0LL);

    }

    {
        static const char * DATA = "$GNGLL,3947.65337,N,10509.20223,W,135628.00,A,D*6A\r\n";
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
        assert(count == 9);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_gll(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "GLL") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.tot_nanoseconds == 50188000000000ULL);
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
    }

    {
        static const char * DATA = "$GNVTG,,T,,M,0.021,N,0.040,K,D*3F\r\n";
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

        rc = hazer_parse_vtg(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "VTG") == 0);
        assert(position.cog_nanodegrees == 0LL);
        assert(position.mag_nanodegrees == 0LL);
        assert(position.sog_microknots == 21000LL);
        assert(position.sog_millimeters == 40000LL);
    }

    {
        static const char * DATA = "$GNGSA,A,3,07,11,15,18,19,13,30,28,51,01,48,17,1.27,0.64,1.10*1C\r\n";
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_active_t active = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
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
        assert(count == 19);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_gsa(&active, vector, count);
        assert(rc == 0);
        assert(strcmp(active.label, "GSA") == 0);
        assert(active.active == 12);
        assert(active.pdop == 127);
        assert(active.hdop == 64);
        assert(active.vdop == 110);
        assert(active.id[0] == 7);
        assert(active.id[1] == 11);
        assert(active.id[2] == 15);
        assert(active.id[3] == 18);
        assert(active.id[4] == 19);
        assert(active.id[5] == 13);
        assert(active.id[6] == 30);
        assert(active.id[7] == 28);
        assert(active.id[8] == 51);
        assert(active.id[9] == 1);
        assert(active.id[10] == 48);
        assert(active.id[11] == 17);

        assert(active.system == HAZER_SYSTEM_TOTAL);
    }

    {
        static const char * DATA = "$GNGSA,A,3,07,11,15,18,19,13,30,28,51,01,48,17,1.27,0.64,1.10,15*34\r\n";
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_active_t active = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
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
        assert(count == 20);

        length = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(length == (strlen(temporary) + 1));
        temporary[length - 1] = msn;
        temporary[length] = lsn;
        temporary[length + 1] = '\r';
        temporary[length + 2] = '\n';
        temporary[length + 3] = '\0';
        assert(strcmp(DATA, temporary) == 0);

        rc = hazer_parse_gsa(&active, vector, count);
        assert(rc == 0);
        assert(strcmp(active.label, "GSA") == 0);
        assert(active.active == 12);
        assert(active.pdop == 127);
        assert(active.hdop == 64);
        assert(active.vdop == 110);
        assert(active.id[0] == 7);
        assert(active.id[1] == 11);
        assert(active.id[2] == 15);
        assert(active.id[3] == 18);
        assert(active.id[4] == 19);
        assert(active.id[5] == 13);
        assert(active.id[6] == 30);
        assert(active.id[7] == 28);
        assert(active.id[8] == 51);
        assert(active.id[9] == 1);
        assert(active.id[10] == 48);
        assert(active.id[11] == 17);

        assert(active.system == HAZER_SYSTEM_QZSS);
    }

    {
        static const char * DATA[] = {
            "$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22*7A\r\n",
            "$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40*7B\r\n",
            "$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38*77\r\n",
            "$GPGSV,4,4,15,46,38,215,40,48,36,220,34,51,44,183,45*47\r\n",
        };
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_view_t view = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy(buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], buffer) == 0);

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
            assert(((ii == 3) && (count == 17)) || (count == 21));

            length = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(length == (strlen(temporary) + 1));
            temporary[length - 1] = msn;
            temporary[length] = lsn;
            temporary[length + 1] = '\r';
            temporary[length + 2] = '\n';
            temporary[length + 3] = '\0';
            assert(strcmp(DATA[ii], temporary) == 0);

            rc = hazer_parse_gsv(&view, vector, count);
            assert(((ii == 3) && (rc == 0)) || (rc > 0));
            assert(strcmp(view.label, "GSV") == 0);
            assert(view.view == 15);

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.channels == 15);
        assert(view.pending == 0);
        assert(view.view == 15);

        assert(view.signal[0] == 0);
        assert(view.signal[1] == 0);
        assert(view.signal[2] == 0);
        assert(view.signal[3] == 0);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);

        assert(view.sat[1].id == 6);
        assert(view.sat[1].elv_degrees == 2);
        assert(view.sat[1].azm_degrees == 184);
        assert(view.sat[1].snr_dbhz == 29);

        assert(view.sat[2].id == 7);
        assert(view.sat[2].elv_degrees == 28);
        assert(view.sat[2].azm_degrees == 143);
        assert(view.sat[2].snr_dbhz == 44);

        assert(view.sat[3].id == 8);
        assert(view.sat[3].elv_degrees == 0);
        assert(view.sat[3].azm_degrees == 48);
        assert(view.sat[3].snr_dbhz == 22);

        assert(view.sat[4].id == 11);
        assert(view.sat[4].elv_degrees == 36);
        assert(view.sat[4].azm_degrees == 59);
        assert(view.sat[4].snr_dbhz == 30);

        assert(view.sat[5].id == 13);
        assert(view.sat[5].elv_degrees == 36);
        assert(view.sat[5].azm_degrees == 270);
        assert(view.sat[5].snr_dbhz == 37);

        assert(view.sat[6].id == 15);
        assert(view.sat[6].elv_degrees == 15);
        assert(view.sat[6].azm_degrees == 304);
        assert(view.sat[6].snr_dbhz == 28);

        assert(view.sat[7].id == 17);
        assert(view.sat[7].elv_degrees == 63);
        assert(view.sat[7].azm_degrees == 226);
        assert(view.sat[7].snr_dbhz == 40);

        assert(view.sat[8].id == 18);
        assert(view.sat[8].elv_degrees == 24);
        assert(view.sat[8].azm_degrees == 52);
        assert(view.sat[8].snr_dbhz == 32);

        assert(view.sat[9].id == 19);
        assert(view.sat[9].elv_degrees == 32);
        assert(view.sat[9].azm_degrees == 223);
        assert(view.sat[9].snr_dbhz == 36);

        assert(view.sat[10].id == 28);
        assert(view.sat[10].elv_degrees == 67);
        assert(view.sat[10].azm_degrees == 20);
        assert(view.sat[10].snr_dbhz == 28);

        assert(view.sat[11].id == 30);
        assert(view.sat[11].elv_degrees == 59);
        assert(view.sat[11].azm_degrees == 149);
        assert(view.sat[11].snr_dbhz == 38);

        assert(view.sat[12].id == 46);
        assert(view.sat[12].elv_degrees == 38);
        assert(view.sat[12].azm_degrees == 215);
        assert(view.sat[12].snr_dbhz == 40);

        assert(view.sat[13].id == 48);
        assert(view.sat[13].elv_degrees == 36);
        assert(view.sat[13].azm_degrees == 220);
        assert(view.sat[13].snr_dbhz == 34);

        assert(view.sat[14].id == 51);
        assert(view.sat[14].elv_degrees == 44);
        assert(view.sat[14].azm_degrees == 183);
        assert(view.sat[14].snr_dbhz == 45);

    }

    {
        static const char * DATA[] = {
            "$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22,1*67\r\n",
            "$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40,2*65\r\n",
            "$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38,*5B\r\n",
            "$GPGSV,4,4,15,46,38,215,40,48,36,220,34,51,44,183,45,3*58\r\n",
        };
        hazer_buffer_t buffer = { 0 };
        hazer_vector_t vector = { 0 };
        hazer_view_t view = { 0 };
        ssize_t length = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        char msn = 0;
        char lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy(buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], buffer) == 0);

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
            assert(((ii == 3) && (count == 18)) || (count == 22));

            length = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(length == (strlen(temporary) + 1));
            temporary[length - 1] = msn;
            temporary[length] = lsn;
            temporary[length + 1] = '\r';
            temporary[length + 2] = '\n';
            temporary[length + 3] = '\0';
            assert(strcmp(DATA[ii], temporary) == 0);

            rc = hazer_parse_gsv(&view, vector, count);
            assert(((ii == 3) && (rc == 0)) || (rc > 0));
            assert(strcmp(view.label, "GSV") == 0);
            assert(view.view == 15);

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.channels == 15);
        assert(view.pending == 0);
        assert(view.view == 15);

        assert(view.signal[0] == 1);
        assert(view.signal[1] == 2);
        assert(view.signal[2] == 2);
        assert(view.signal[3] == 3);

        assert(view.sat[0].id == 1);
        assert(view.sat[0].elv_degrees == 37);
        assert(view.sat[0].azm_degrees == 78);
        assert(view.sat[0].snr_dbhz == 36);

        assert(view.sat[1].id == 6);
        assert(view.sat[1].elv_degrees == 2);
        assert(view.sat[1].azm_degrees == 184);
        assert(view.sat[1].snr_dbhz == 29);

        assert(view.sat[2].id == 7);
        assert(view.sat[2].elv_degrees == 28);
        assert(view.sat[2].azm_degrees == 143);
        assert(view.sat[2].snr_dbhz == 44);

        assert(view.sat[3].id == 8);
        assert(view.sat[3].elv_degrees == 0);
        assert(view.sat[3].azm_degrees == 48);
        assert(view.sat[3].snr_dbhz == 22);

        assert(view.sat[4].id == 11);
        assert(view.sat[4].elv_degrees == 36);
        assert(view.sat[4].azm_degrees == 59);
        assert(view.sat[4].snr_dbhz == 30);

        assert(view.sat[5].id == 13);
        assert(view.sat[5].elv_degrees == 36);
        assert(view.sat[5].azm_degrees == 270);
        assert(view.sat[5].snr_dbhz == 37);

        assert(view.sat[6].id == 15);
        assert(view.sat[6].elv_degrees == 15);
        assert(view.sat[6].azm_degrees == 304);
        assert(view.sat[6].snr_dbhz == 28);

        assert(view.sat[7].id == 17);
        assert(view.sat[7].elv_degrees == 63);
        assert(view.sat[7].azm_degrees == 226);
        assert(view.sat[7].snr_dbhz == 40);

        assert(view.sat[8].id == 18);
        assert(view.sat[8].elv_degrees == 24);
        assert(view.sat[8].azm_degrees == 52);
        assert(view.sat[8].snr_dbhz == 32);

        assert(view.sat[9].id == 19);
        assert(view.sat[9].elv_degrees == 32);
        assert(view.sat[9].azm_degrees == 223);
        assert(view.sat[9].snr_dbhz == 36);

        assert(view.sat[10].id == 28);
        assert(view.sat[10].elv_degrees == 67);
        assert(view.sat[10].azm_degrees == 20);
        assert(view.sat[10].snr_dbhz == 28);

        assert(view.sat[11].id == 30);
        assert(view.sat[11].elv_degrees == 59);
        assert(view.sat[11].azm_degrees == 149);
        assert(view.sat[11].snr_dbhz == 38);

        assert(view.sat[12].id == 46);
        assert(view.sat[12].elv_degrees == 38);
        assert(view.sat[12].azm_degrees == 215);
        assert(view.sat[12].snr_dbhz == 40);

        assert(view.sat[13].id == 48);
        assert(view.sat[13].elv_degrees == 36);
        assert(view.sat[13].azm_degrees == 220);
        assert(view.sat[13].snr_dbhz == 34);

        assert(view.sat[14].id == 51);
        assert(view.sat[14].elv_degrees == 44);
        assert(view.sat[14].azm_degrees == 183);
        assert(view.sat[14].snr_dbhz == 45);

    }

}
