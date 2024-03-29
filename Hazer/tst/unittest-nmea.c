/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2018-2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the NMEA unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include <errno.h>
#include "com/diag/hazer/hazer.h"
#include "com/diag/hazer/yodel.h"
#include "./unittest.h"

#define position positions[HAZER_SYSTEM_GNSS]
#define view views[HAZER_SYSTEM_GPS]

/*
 * Swiped from gpstool.
 */
void gbs(const hazer_fault_t * fp)
{
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    uint64_t nanoseconds = 0;
    hazer_talker_t talker = HAZER_TALKER_TOTAL;
    hazer_system_t system = HAZER_SYSTEM_TOTAL;
    int signal = 0;

    hazer_format_nanoseconds2timestamp(fp->utc_nanoseconds, &year, &month, &day, &hour, &minute, &second, &nanoseconds);

    if (fp->talker >= HAZER_TALKER_TOTAL) {
        talker = HAZER_TALKER_GNSS;
    } else {
        talker = fp->talker;
    }

    system = hazer_map_nmea_to_system(fp->system);
    if (system >= HAZER_SYSTEM_TOTAL) {
        system = HAZER_SYSTEM_GNSS;
    }

    if (fp->signal >= HAZER_GNSS_SIGNALS) {
        signal = 0;
    } else {
        signal = fp->signal;
    }

    fprintf(stderr, "Fault %02d:%02d:%02dZ %s %s %s %d %.3lfm %.3lfm %.3lfm %.3lf%% %.3lfm %.3lf\n",
        hour, minute, second,
        HAZER_TALKER_NAME[talker],
        HAZER_SYSTEM_NAME[system],
        HAZER_SIGNAL_NAME[system][signal],
        fp->id,
        (double)(fp->lat_millimeters) / 1000.0,
        (double)(fp->lon_millimeters) / 1000.0,
        (double)(fp->alt_millimeters) / 1000.0,
        (double)(fp->probability) / 1000.0,
        (double)(fp->est_millimeters) / 1000.0,
        (double)(fp->std_deviation) / 1000.0);
}

int main(void)
{
    hazer_debug(stderr);

    {
        assert(HAZER_NANOSECONDS_INITIALIZER == 0xffffffffffffffffULL);
        assert(HAZER_NANOSECONDS_UNSET == 0xffffffffffffffffULL);
    }

    {
        static const char DATA[] = "$GNRMC,135628.00,A,3947.65337,N,10509.20223,W,0.010,,070818,,,M*7D\r\n";

        assert(DATA[HAZER_NMEA_SYNC     + 0] == '$');

        assert(DATA[HAZER_NMEA_TALKER   + 0] == 'G');
        assert(DATA[HAZER_NMEA_TALKER   + 1] == 'N');

        assert(DATA[HAZER_NMEA_NAME     + 0] == 'R');
        assert(DATA[HAZER_NMEA_NAME     + 1] == 'M');
        assert(DATA[HAZER_NMEA_NAME     + 2] == 'C');

        assert(DATA[HAZER_NMEA_NAMEEND  + 0] == ',');

        assert(hazer_is_nmea(DATA[0]));

        assert(hazer_is_nmea_name(DATA, sizeof(DATA), "RMC"));
    }

    {
        static const char * DATA = "$GNGGA,135627.00,3947.65338,N,10509.20216,W,2,12,0.67,1708.6,M,-21.5,M,,0000*4E\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GGA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 16);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gga(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "GGA") == 0);
        assert(position.sat_used == 12);
        assert(position.utc_nanoseconds == 50187000000000ULL);
        assert(position.dmy_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.tot_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.old_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.lat_nanominutes == 2387653380000LL);
        assert(position.lon_nanominutes == -6309202160000LL);
        assert(position.alt_millimeters == 1708600LL);
        assert(position.sep_millimeters == -21500LL);
        assert(position.quality == HAZER_QUALITY_DIFFERENTIAL);
        assert(position.safety == HAZER_SAFETY_UNKNOWN);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNGGA,135627.00,3947.65338,N,10509.20216,W,2,0,0.67,1708.6,M,-21.5,M,,0000*7D\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        static const hazer_position_t POSITION = HAZER_POSITION_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GGA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 16);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        errno = ~0;
        rc = hazer_parse_gga(&position, vector, count);
        assert(rc < 0);
        assert(errno == 0);
        assert(memcmp(&position, &POSITION, sizeof(position)) == 0);
    }

    {
        static const char * DATA = "$GNRMC,135628.00,A,3947.65337,N,10509.20223,W,0.010,,070818,,,M*7D\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "RMC");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 14);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_rmc(&position, vector, count);
        /* RMC A mode with M status is okay. */
        assert(rc == 0);
        assert(strcmp(position.label, "RMC") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.dmy_nanoseconds == 1533600000000000000ULL); /* date -u -d "August 7 2018" +"%s.%N" */
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
        assert(position.sog_microknots == 10000ULL);
        assert(position.cog_nanodegrees == 0LL);
        assert(position.quality == HAZER_QUALITY_MANUAL);
        assert(position.safety == HAZER_SAFETY_UNKNOWN);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNRMC,135628.00,V,3947.65337,N,10509.20223,W,0.010,,070818,,,D*63\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "RMC");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 14);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_rmc(&position, vector, count);
        /* RMC V indicator with A or D mode is now okay. */
        assert(rc == 0);
        assert(strcmp(position.label, "RMC") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.dmy_nanoseconds == 1533600000000000000ULL); /* date -u -d "August 7 2018" +"%s.%N" */
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
        assert(position.sog_microknots == 10000ULL);
        assert(position.cog_nanodegrees == 0LL);
        assert(position.quality == HAZER_QUALITY_DIFFERENTIAL);
        assert(position.safety == HAZER_SAFETY_UNKNOWN);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNRMC,135628.00,A,3947.65337,N,10509.20223,W,0.010,,070818,,,D,S*0B\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "RMC");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 15); /* Because of the extra safety field. */

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_rmc(&position, vector, count);
        /* RMC V indicator with A or D mode is now okay; also SAFE. */
        assert(rc == 0);
        assert(strcmp(position.label, "RMC") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.dmy_nanoseconds == 1533600000000000000ULL); /* date -u -d "August 7 2018" +"%s.%N" */
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
        assert(position.sog_microknots == 10000ULL);
        assert(position.cog_nanodegrees == 0LL);
        assert(position.quality == HAZER_QUALITY_DIFFERENTIAL);
        assert(position.safety == HAZER_SAFETY_SAFE);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNRMC,135628.00,V,3947.65337,N,10509.20223,W,0.010,,070818,,,M*6A\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        static const hazer_position_t POSITION = HAZER_POSITION_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "RMC");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 14);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        errno = ~0;
        rc = hazer_parse_rmc(&position, vector, count);
        /* RMC V indicator without A or D mode is not okay. */
        assert(rc < 0);
        assert(errno == 0);
        assert(memcmp(&position, &POSITION, sizeof(position)) == 0);
    }

    {
        static const char * DATA = "$GNGLL,3947.65337,N,10509.20223,W,135628.00,A,D*6A\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GLL");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 9);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gll(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "GLL") == 0);
        assert(position.utc_nanoseconds == 50188000000000ULL);
        assert(position.dmy_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.tot_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.old_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.lat_nanominutes == 2387653370000LL);
        assert(position.lon_nanominutes == -6309202230000LL);
        assert(position.quality == HAZER_QUALITY_DIFFERENTIAL);
        assert(position.safety == HAZER_SAFETY_UNKNOWN);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNVTG,,T,,M,0.021,N,0.040,K,D*3F\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "VTG");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 11);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_vtg(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "VTG") == 0);
        assert(position.utc_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.dmy_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.tot_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.old_nanoseconds == HAZER_NANOSECONDS_UNSET);
        assert(position.cog_nanodegrees == 0LL);
        assert(position.mag_nanodegrees == 0LL);
        assert(position.sog_microknots == 21000LL);
        assert(position.sog_millimetersperhour == 40000LL);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        static const char * DATA = "$GNVTG,,T,,M,0.021,N,0.040,K,N*35\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        static const hazer_position_t POSITION = HAZER_POSITION_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "VTG");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 11);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        errno = ~0;
        rc = hazer_parse_vtg(&position, vector, count);
        assert(rc < 0);
        assert(errno == 0);
        assert(memcmp(&position, &POSITION, sizeof(position)) == 0);
    }

    {
        static const char * DATA = "$GNGSA,A,3,07,11,15,18,19,13,30,28,51,01,48,17,1.27,0.64,1.10*1C\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GSA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 19);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gsa(&active, vector, count);
        assert(rc == 0);
        assert(strcmp(active.label, "GSA") == 0);
        assert(active.active == 12);
        assert(active.pdop == 127);
        assert(active.hdop == 64);
        assert(active.vdop == 110);
        assert(active.tdop == 9999);
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
        static const char * DATA = "$GNGSA,A,3,07,11,15,18,19,13,30,28,51,01,48,17,1.27,0.64,1.10,F*76\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_active_t active = HAZER_ACTIVE_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GSA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 20);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gsa(&active, vector, count);
        assert(rc == 0);
        assert(strcmp(active.label, "GSA") == 0);
        assert(active.active == 12);
        assert(active.pdop == 127);
        assert(active.hdop == 64);
        assert(active.vdop == 110);
        assert(active.tdop == 9999);
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
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_views_t views = HAZER_VIEWS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy((char *)buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], (const char *)buffer) == 0);

            length = hazer_length(buffer, sizeof(buffer));
            assert(length == strlen((const char *)buffer));

            pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
            assert(pointer != (char *)0);
            assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
            assert(pointer[1] == msn);
            assert(pointer[2] == lsn);
            assert(pointer[3] == '\r');
            assert(pointer[4] == '\n');

            rc = hazer_is_nmea(buffer[0]);
            assert(rc == !0);

            rc = hazer_is_nmea_name(buffer, length, "GSV");
            assert(rc == !0);

            count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
            assert(((ii == 3) && (count == 17)) || (count == 21));

            size = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(size == (strlen((const char *)temporary) + 1));
            temporary[size - 1] = msn;
            temporary[size] = lsn;
            temporary[size + 1] = '\r';
            temporary[size + 2] = '\n';
            temporary[size + 3] = '\0';
            assert(strcmp(DATA[ii], (const char *)temporary) == 0);

            rc = hazer_parse_gsv(&view, vector, count);
            assert(rc == HAZER_SYSTEM_GNSS);
            assert(strcmp(view.label, "GSV") == 0);

            view.sig[0].timeout = 0;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == 0);
            view.sig[0].timeout = 1;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == ((ii < 3) ? !0 : 0));
            view.sig[0].timeout = 1;

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.signals == 1);
        assert(view.signal == 0);
        assert(view.pending == 0);

        assert(view.sig[0].channels == 15);
        assert(view.sig[0].visible == 15);

        assert(view.sig[0].sat[0].id == 1);
        assert(view.sig[0].sat[0].elv_degrees == 37);
        assert(view.sig[0].sat[0].azm_degrees == 78);
        assert(view.sig[0].sat[0].snr_dbhz == 36);

        assert(view.sig[0].sat[1].id == 6);
        assert(view.sig[0].sat[1].elv_degrees == 2);
        assert(view.sig[0].sat[1].azm_degrees == 184);
        assert(view.sig[0].sat[1].snr_dbhz == 29);

        assert(view.sig[0].sat[2].id == 7);
        assert(view.sig[0].sat[2].elv_degrees == 28);
        assert(view.sig[0].sat[2].azm_degrees == 143);
        assert(view.sig[0].sat[2].snr_dbhz == 44);

        assert(view.sig[0].sat[3].id == 8);
        assert(view.sig[0].sat[3].elv_degrees == 0);
        assert(view.sig[0].sat[3].azm_degrees == 48);
        assert(view.sig[0].sat[3].snr_dbhz == 22);

        assert(view.sig[0].sat[4].id == 11);
        assert(view.sig[0].sat[4].elv_degrees == 36);
        assert(view.sig[0].sat[4].azm_degrees == 59);
        assert(view.sig[0].sat[4].snr_dbhz == 30);

        assert(view.sig[0].sat[5].id == 13);
        assert(view.sig[0].sat[5].elv_degrees == 36);
        assert(view.sig[0].sat[5].azm_degrees == 270);
        assert(view.sig[0].sat[5].snr_dbhz == 37);

        assert(view.sig[0].sat[6].id == 15);
        assert(view.sig[0].sat[6].elv_degrees == 15);
        assert(view.sig[0].sat[6].azm_degrees == 304);
        assert(view.sig[0].sat[6].snr_dbhz == 28);

        assert(view.sig[0].sat[7].id == 17);
        assert(view.sig[0].sat[7].elv_degrees == 63);
        assert(view.sig[0].sat[7].azm_degrees == 226);
        assert(view.sig[0].sat[7].snr_dbhz == 40);

        assert(view.sig[0].sat[8].id == 18);
        assert(view.sig[0].sat[8].elv_degrees == 24);
        assert(view.sig[0].sat[8].azm_degrees == 52);
        assert(view.sig[0].sat[8].snr_dbhz == 32);

        assert(view.sig[0].sat[9].id == 19);
        assert(view.sig[0].sat[9].elv_degrees == 32);
        assert(view.sig[0].sat[9].azm_degrees == 223);
        assert(view.sig[0].sat[9].snr_dbhz == 36);

        assert(view.sig[0].sat[10].id == 28);
        assert(view.sig[0].sat[10].elv_degrees == 67);
        assert(view.sig[0].sat[10].azm_degrees == 20);
        assert(view.sig[0].sat[10].snr_dbhz == 28);

        assert(view.sig[0].sat[11].id == 30);
        assert(view.sig[0].sat[11].elv_degrees == 59);
        assert(view.sig[0].sat[11].azm_degrees == 149);
        assert(view.sig[0].sat[11].snr_dbhz == 38);

        assert(view.sig[0].sat[12].id == 46);
        assert(view.sig[0].sat[12].elv_degrees == 38);
        assert(view.sig[0].sat[12].azm_degrees == 215);
        assert(view.sig[0].sat[12].snr_dbhz == 40);

        assert(view.sig[0].sat[13].id == 48);
        assert(view.sig[0].sat[13].elv_degrees == 36);
        assert(view.sig[0].sat[13].azm_degrees == 220);
        assert(view.sig[0].sat[13].snr_dbhz == 34);

        assert(view.sig[0].sat[14].id == 51);
        assert(view.sig[0].sat[14].elv_degrees == 44);
        assert(view.sig[0].sat[14].azm_degrees == 183);
        assert(view.sig[0].sat[14].snr_dbhz == 45);
    }

    {
        static const char * DATA[] = {
            "$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22,1*67\r\n",
            "$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40,2*65\r\n",
            "$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38,*5B\r\n",
            "$GPGSV,4,4,15,46,38,215,40,48,36,220,34,51,44,183,45,3*58\r\n",
        };
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_views_t views = HAZER_VIEWS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;
        int jj = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy((char *)buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], (const char *)buffer) == 0);

            length = hazer_length(buffer, sizeof(buffer));
            assert(length == strlen((const char *)buffer));

            pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
            assert(pointer != (char *)0);
            assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
            assert(pointer[1] == msn);
            assert(pointer[2] == lsn);
            assert(pointer[3] == '\r');
            assert(pointer[4] == '\n');

            rc = hazer_is_nmea(buffer[0]);
            assert(rc == !0);

            rc = hazer_is_nmea_name(buffer, length, "GSV");
            assert(rc == !0);

            count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
            assert(((ii == 3) && (count == 18)) || (count == 22));

            size = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(size == (strlen((const char *)temporary) + 1));
            temporary[size - 1] = msn;
            temporary[size] = lsn;
            temporary[size + 1] = '\r';
            temporary[size + 2] = '\n';
            temporary[size + 3] = '\0';
            assert(strcmp(DATA[ii], (const char *)temporary) == 0);

            jj = hazer_parse_gsv(&view, vector, count);
            assert(jj == (ii == 0) ? HAZER_SYSTEM_GPS : (ii == 1) ? HAZER_SYSTEM_GLONASS : (ii == 2) ? HAZER_SYSTEM_GNSS : HAZER_SYSTEM_GALILEO);

            view.sig[jj].timeout = 0;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == 0);
            view.sig[jj].timeout = 1;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == ((ii < 3) ? !0 : 0));
            view.sig[jj].timeout = 0;

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.signals == 4);
        assert(view.signal == 3);
        assert(view.pending == 0);

        assert(view.sig[1].channels == 4);
        assert(view.sig[1].visible == 15);

        assert(view.sig[1].sat[0].id == 1);
        assert(view.sig[1].sat[0].elv_degrees == 37);
        assert(view.sig[1].sat[0].azm_degrees == 78);
        assert(view.sig[1].sat[0].snr_dbhz == 36);

        assert(view.sig[1].sat[1].id == 6);
        assert(view.sig[1].sat[1].elv_degrees == 2);
        assert(view.sig[1].sat[1].azm_degrees == 184);
        assert(view.sig[1].sat[1].snr_dbhz == 29);

        assert(view.sig[1].sat[2].id == 7);
        assert(view.sig[1].sat[2].elv_degrees == 28);
        assert(view.sig[1].sat[2].azm_degrees == 143);
        assert(view.sig[1].sat[2].snr_dbhz == 44);

        assert(view.sig[1].sat[3].id == 8);
        assert(view.sig[1].sat[3].elv_degrees == 0);
        assert(view.sig[1].sat[3].azm_degrees == 48);
        assert(view.sig[1].sat[3].snr_dbhz == 22);

        assert(view.sig[2].channels == 4);
        assert(view.sig[2].visible == 15);

        assert(view.sig[2].sat[0].id == 11);
        assert(view.sig[2].sat[0].elv_degrees == 36);
        assert(view.sig[2].sat[0].azm_degrees == 59);
        assert(view.sig[2].sat[0].snr_dbhz == 30);

        assert(view.sig[2].sat[1].id == 13);
        assert(view.sig[2].sat[1].elv_degrees == 36);
        assert(view.sig[2].sat[1].azm_degrees == 270);
        assert(view.sig[2].sat[1].snr_dbhz == 37);

        assert(view.sig[2].sat[2].id == 15);
        assert(view.sig[2].sat[2].elv_degrees == 15);
        assert(view.sig[2].sat[2].azm_degrees == 304);
        assert(view.sig[2].sat[2].snr_dbhz == 28);

        assert(view.sig[2].sat[3].id == 17);
        assert(view.sig[2].sat[3].elv_degrees == 63);
        assert(view.sig[2].sat[3].azm_degrees == 226);
        assert(view.sig[2].sat[3].snr_dbhz == 40);

        assert(view.sig[0].channels == 4);
        assert(view.sig[0].visible == 15);

        assert(view.sig[0].sat[0].id == 18);
        assert(view.sig[0].sat[0].elv_degrees == 24);
        assert(view.sig[0].sat[0].azm_degrees == 52);
        assert(view.sig[0].sat[0].snr_dbhz == 32);

        assert(view.sig[0].sat[1].id == 19);
        assert(view.sig[0].sat[1].elv_degrees == 32);
        assert(view.sig[0].sat[1].azm_degrees == 223);
        assert(view.sig[0].sat[1].snr_dbhz == 36);

        assert(view.sig[0].sat[2].id == 28);
        assert(view.sig[0].sat[2].elv_degrees == 67);
        assert(view.sig[0].sat[2].azm_degrees == 20);
        assert(view.sig[0].sat[2].snr_dbhz == 28);

        assert(view.sig[0].sat[3].id == 30);
        assert(view.sig[0].sat[3].elv_degrees == 59);
        assert(view.sig[0].sat[3].azm_degrees == 149);
        assert(view.sig[0].sat[3].snr_dbhz == 38);

        assert(view.sig[3].channels == 3);
        assert(view.sig[3].visible == 15);

        assert(view.sig[3].sat[0].id == 46);
        assert(view.sig[3].sat[0].elv_degrees == 38);
        assert(view.sig[3].sat[0].azm_degrees == 215);
        assert(view.sig[3].sat[0].snr_dbhz == 40);

        assert(view.sig[3].sat[1].id == 48);
        assert(view.sig[3].sat[1].elv_degrees == 36);
        assert(view.sig[3].sat[1].azm_degrees == 220);
        assert(view.sig[3].sat[1].snr_dbhz == 34);

        assert(view.sig[3].sat[2].id == 51);
        assert(view.sig[3].sat[2].elv_degrees == 44);
        assert(view.sig[3].sat[2].azm_degrees == 183);
        assert(view.sig[3].sat[2].snr_dbhz == 45);
    }

    {
        /* As seen on the GlobalSat BU-353W10. */
        static const char * DATA[] = {
            "$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22*7A\r\n",
            "$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40*7B\r\n",
            "$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38*77\r\n",
            "$GPGSV,4,4,15,46,38,215,40,,,,,,,,45*47\r\n",
        };
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_views_t views = HAZER_VIEWS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy((char *)buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], (const char *)buffer) == 0);

            length = hazer_length(buffer, sizeof(buffer));
            assert(length == strlen((const char *)buffer));

            pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
            assert(pointer != (char *)0);
            assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
            assert(pointer[1] == msn);
            assert(pointer[2] == lsn);
            assert(pointer[3] == '\r');
            assert(pointer[4] == '\n');

            rc = hazer_is_nmea(buffer[0]);
            assert(rc == !0);

            rc = hazer_is_nmea_name(buffer, length, "GSV");
            assert(rc == !0);

            count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
            assert(((ii == 3) && (count == 17)) || (count == 21));

            size = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(size == (strlen((const char *)temporary) + 1));
            temporary[size - 1] = msn;
            temporary[size] = lsn;
            temporary[size + 1] = '\r';
            temporary[size + 2] = '\n';
            temporary[size + 3] = '\0';
            assert(strcmp(DATA[ii], (const char *)temporary) == 0);

            rc = hazer_parse_gsv(&view, vector, count);
            assert(rc == HAZER_SYSTEM_GNSS);
            assert(strcmp(view.label, "GSV") == 0);

            view.sig[0].timeout = 0;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == 0);
            view.sig[0].timeout = 1;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == ((ii < 3) ? !0 : 0));
            view.sig[0].timeout = 1;

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.signals == 1);
        assert(view.signal == 0);
        assert(view.pending == 0);

        assert(view.sig[0].channels == 13);
        assert(view.sig[0].visible == 15);

        assert(view.sig[0].sat[0].id == 1);
        assert(view.sig[0].sat[0].elv_degrees == 37);
        assert(view.sig[0].sat[0].azm_degrees == 78);
        assert(view.sig[0].sat[0].snr_dbhz == 36);

        assert(view.sig[0].sat[1].id == 6);
        assert(view.sig[0].sat[1].elv_degrees == 2);
        assert(view.sig[0].sat[1].azm_degrees == 184);
        assert(view.sig[0].sat[1].snr_dbhz == 29);

        assert(view.sig[0].sat[2].id == 7);
        assert(view.sig[0].sat[2].elv_degrees == 28);
        assert(view.sig[0].sat[2].azm_degrees == 143);
        assert(view.sig[0].sat[2].snr_dbhz == 44);

        assert(view.sig[0].sat[3].id == 8);
        assert(view.sig[0].sat[3].elv_degrees == 0);
        assert(view.sig[0].sat[3].azm_degrees == 48);
        assert(view.sig[0].sat[3].snr_dbhz == 22);

        assert(view.sig[0].sat[4].id == 11);
        assert(view.sig[0].sat[4].elv_degrees == 36);
        assert(view.sig[0].sat[4].azm_degrees == 59);
        assert(view.sig[0].sat[4].snr_dbhz == 30);

        assert(view.sig[0].sat[5].id == 13);
        assert(view.sig[0].sat[5].elv_degrees == 36);
        assert(view.sig[0].sat[5].azm_degrees == 270);
        assert(view.sig[0].sat[5].snr_dbhz == 37);

        assert(view.sig[0].sat[6].id == 15);
        assert(view.sig[0].sat[6].elv_degrees == 15);
        assert(view.sig[0].sat[6].azm_degrees == 304);
        assert(view.sig[0].sat[6].snr_dbhz == 28);

        assert(view.sig[0].sat[7].id == 17);
        assert(view.sig[0].sat[7].elv_degrees == 63);
        assert(view.sig[0].sat[7].azm_degrees == 226);
        assert(view.sig[0].sat[7].snr_dbhz == 40);

        assert(view.sig[0].sat[8].id == 18);
        assert(view.sig[0].sat[8].elv_degrees == 24);
        assert(view.sig[0].sat[8].azm_degrees == 52);
        assert(view.sig[0].sat[8].snr_dbhz == 32);

        assert(view.sig[0].sat[9].id == 19);
        assert(view.sig[0].sat[9].elv_degrees == 32);
        assert(view.sig[0].sat[9].azm_degrees == 223);
        assert(view.sig[0].sat[9].snr_dbhz == 36);

        assert(view.sig[0].sat[10].id == 28);
        assert(view.sig[0].sat[10].elv_degrees == 67);
        assert(view.sig[0].sat[10].azm_degrees == 20);
        assert(view.sig[0].sat[10].snr_dbhz == 28);

        assert(view.sig[0].sat[11].id == 30);
        assert(view.sig[0].sat[11].elv_degrees == 59);
        assert(view.sig[0].sat[11].azm_degrees == 149);
        assert(view.sig[0].sat[11].snr_dbhz == 38);

        assert(view.sig[0].sat[12].id == 46);
        assert(view.sig[0].sat[12].elv_degrees == 38);
        assert(view.sig[0].sat[12].azm_degrees == 215);
        assert(view.sig[0].sat[12].snr_dbhz == 40);
    }

    {
        /* Haven't seen this but it's a logical extrapolation from the prior test. */
        static const char * DATA[] = {
            "$GPGSV,4,1,15,01,37,078,36,06,02,184,29,07,28,143,44,08,00,048,22,1*67\r\n",
            "$GPGSV,4,2,15,11,36,059,30,13,36,270,37,15,15,304,28,17,63,226,40,1*66\r\n",
            "$GPGSV,4,3,15,18,24,052,32,19,32,223,36,28,67,020,28,30,59,149,38,1*6A\r\n",
            "$GPGSV,4,4,15,46,38,215,40,,,,,,,,45,1*5A\r\n",
        };
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_views_t views = HAZER_VIEWS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };
        int ii = 0;

        for (ii = 0; ii < (sizeof(DATA) / sizeof(DATA[0])); ++ii) {

            strncpy((char *)buffer, DATA[ii], sizeof(buffer));
            buffer[sizeof(buffer) - 1] = '\0';
            assert(strcmp(DATA[ii], (const char *)buffer) == 0);

            length = hazer_length(buffer, sizeof(buffer));
            assert(length == strlen((const char *)buffer));

            pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
            assert(pointer != (char *)0);
            assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
            assert(pointer[1] == msn);
            assert(pointer[2] == lsn);
            assert(pointer[3] == '\r');
            assert(pointer[4] == '\n');

            rc = hazer_is_nmea(buffer[0]);
            assert(rc == !0);

            rc = hazer_is_nmea_name(buffer, length, "GSV");
            assert(rc == !0);

            count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
            assert(((ii == 3) && (count == 18)) || (count == 22));

            size = hazer_serialize(temporary, sizeof(temporary), vector, count);
            assert(size == (strlen((const char *)temporary) + 1));
            temporary[size - 1] = msn;
            temporary[size] = lsn;
            temporary[size + 1] = '\r';
            temporary[size + 2] = '\n';
            temporary[size + 3] = '\0';
            assert(strcmp(DATA[ii], (const char *)temporary) == 0);

            rc = hazer_parse_gsv(&view, vector, count);
            assert(rc == HAZER_SYSTEM_GPS);
            assert(strcmp(view.label, "GSV") == 0);

            view.sig[0].timeout = 0;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == 0);
            view.sig[0].timeout = 1;
            rc = hazer_has_pending_gsv(views, HAZER_SYSTEM_GPS);
            assert(rc == ((ii < 3) ? !0 : 0));
            view.sig[0].timeout = 1;

        }

        assert(strcmp(view.label, "GSV") == 0);
        assert(view.signals == 2);
        assert(view.signal == 1);
        assert(view.pending == 0);

        assert(view.sig[1].channels == 13);
        assert(view.sig[1].visible == 15);

        assert(view.sig[1].sat[0].id == 1);
        assert(view.sig[1].sat[0].elv_degrees == 37);
        assert(view.sig[1].sat[0].azm_degrees == 78);
        assert(view.sig[1].sat[0].snr_dbhz == 36);

        assert(view.sig[1].sat[1].id == 6);
        assert(view.sig[1].sat[1].elv_degrees == 2);
        assert(view.sig[1].sat[1].azm_degrees == 184);
        assert(view.sig[1].sat[1].snr_dbhz == 29);

        assert(view.sig[1].sat[2].id == 7);
        assert(view.sig[1].sat[2].elv_degrees == 28);
        assert(view.sig[1].sat[2].azm_degrees == 143);
        assert(view.sig[1].sat[2].snr_dbhz == 44);

        assert(view.sig[1].sat[3].id == 8);
        assert(view.sig[1].sat[3].elv_degrees == 0);
        assert(view.sig[1].sat[3].azm_degrees == 48);
        assert(view.sig[1].sat[3].snr_dbhz == 22);

        assert(view.sig[1].sat[4].id == 11);
        assert(view.sig[1].sat[4].elv_degrees == 36);
        assert(view.sig[1].sat[4].azm_degrees == 59);
        assert(view.sig[1].sat[4].snr_dbhz == 30);

        assert(view.sig[1].sat[5].id == 13);
        assert(view.sig[1].sat[5].elv_degrees == 36);
        assert(view.sig[1].sat[5].azm_degrees == 270);
        assert(view.sig[1].sat[5].snr_dbhz == 37);

        assert(view.sig[1].sat[6].id == 15);
        assert(view.sig[1].sat[6].elv_degrees == 15);
        assert(view.sig[1].sat[6].azm_degrees == 304);
        assert(view.sig[1].sat[6].snr_dbhz == 28);

        assert(view.sig[1].sat[7].id == 17);
        assert(view.sig[1].sat[7].elv_degrees == 63);
        assert(view.sig[1].sat[7].azm_degrees == 226);
        assert(view.sig[1].sat[7].snr_dbhz == 40);

        assert(view.sig[1].sat[8].id == 18);
        assert(view.sig[1].sat[8].elv_degrees == 24);
        assert(view.sig[1].sat[8].azm_degrees == 52);
        assert(view.sig[1].sat[8].snr_dbhz == 32);

        assert(view.sig[1].sat[9].id == 19);
        assert(view.sig[1].sat[9].elv_degrees == 32);
        assert(view.sig[1].sat[9].azm_degrees == 223);
        assert(view.sig[1].sat[9].snr_dbhz == 36);

        assert(view.sig[1].sat[10].id == 28);
        assert(view.sig[1].sat[10].elv_degrees == 67);
        assert(view.sig[1].sat[10].azm_degrees == 20);
        assert(view.sig[1].sat[10].snr_dbhz == 28);

        assert(view.sig[1].sat[11].id == 30);
        assert(view.sig[1].sat[11].elv_degrees == 59);
        assert(view.sig[1].sat[11].azm_degrees == 149);
        assert(view.sig[1].sat[11].snr_dbhz == 38);

        assert(view.sig[1].sat[12].id == 46);
        assert(view.sig[1].sat[12].elv_degrees == 38);
        assert(view.sig[1].sat[12].azm_degrees == 215);
        assert(view.sig[1].sat[12].snr_dbhz == 40);
    }

    {
        static const char * DATA = "$GNZDA,171305.00,12,05,2023,00,00*7C\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "ZDA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 8);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_zda(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "ZDA") == 0);
        assert(position.utc_nanoseconds == 61985000000000ULL);
        assert(position.dmy_nanoseconds == 1683849600000000000ULL);
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.tz_nanoseconds == 0LL);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        /* NMEA 0183 4.10 p. 132 Note 1: Chatham Island, NZ */
        static const char * DATA = "$GNZDA,171305.00,12,05,2023,-12,45*53\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "ZDA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 8);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_zda(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "ZDA") == 0);
        assert(position.utc_nanoseconds == 61985000000000ULL);
        assert(position.dmy_nanoseconds == 1683849600000000000ULL);
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.tz_nanoseconds == -45900000000000LL);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        /* NMEA 0183 4.10 p. 132 Note 1: The Cook Islands (I've been there!) */
        static const char * DATA = "$GNZDA,171305.00,12,05,2023,10,30*7E\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_positions_t positions = HAZER_POSITIONS_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        assert(!hazer_is_valid_time(&position));

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "ZDA");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 8);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_zda(&position, vector, count);
        assert(rc == 0);
        assert(strcmp(position.label, "ZDA") == 0);
        assert(position.utc_nanoseconds == 61985000000000ULL);
        assert(position.dmy_nanoseconds == 1683849600000000000ULL);
        assert(position.tot_nanoseconds == (position.utc_nanoseconds + position.dmy_nanoseconds));
        assert(position.old_nanoseconds == position.tot_nanoseconds);
        assert(position.tz_nanoseconds == 37800000000000LL);

        position.timeout = 0;
        assert(!hazer_is_valid_time(&position));
        assert(!hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
        position.timeout = 1;
        assert(hazer_is_valid_time(&position));
        assert(hazer_has_valid_time(positions, HAZER_SYSTEM_GNSS));
    }

    {
        /* Trimble GBS example, which lacks GNSS System ID and Signal ID. */
        static const char * DATA = "$GPGBS,015509.00,-0.031,-0.186,0.219,19,0.000,-0.354,6.972*4D\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_fault_t fault = HAZER_FAULT_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GBS");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 10);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gbs(&fault, vector, count);
        assert(rc == 0);

        assert(fault.utc_nanoseconds == 6909000000000ULL);
        assert(fault.lat_millimeters == -31LL);
        assert(fault.lon_millimeters == -186LL);
        assert(fault.alt_millimeters == 219LL);
        assert(fault.probability == 0LL);
        assert(fault.est_millimeters == -354LL);
        assert(fault.std_deviation == 6972LL);
        assert(fault.id == 19);
        assert(fault.talker == HAZER_TALKER_GPS);
        assert(fault.system == HAZER_SYSTEM_TOTAL);
        assert(fault.signal == HAZER_GNSS_SIGNALS);

        gbs(&fault);
    }

    {
        /* Trimble GBS example, but with NMEA 0183 4.10 fields. */
        static const char * DATA = "$GPGBS,015509.00,-0.031,-0.186,0.219,19,0.000,-0.354,6.972,1,2*4E\r\n";
        hazer_buffer_t buffer = HAZER_BUFFER_INITIALIZER;
        hazer_vector_t vector = HAZER_VECTOR_INITIALIZER;
        hazer_fault_t fault = HAZER_FAULT_INITIALIZER;
        ssize_t length = -1;
        ssize_t size = -1;
        size_t count = 0;
        int rc = -1;
        char * pointer = (char *)0;
        uint8_t msn = 0;
        uint8_t lsn = 0;
        hazer_buffer_t temporary = { 0 };

        strncpy((char *)buffer, DATA, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = '\0';
        assert(strcmp(DATA, (const char *)buffer) == 0);

        length = hazer_length(buffer, sizeof(buffer));
        assert(length == strlen((const char *)buffer));

        pointer = (char *)hazer_checksum_buffer(buffer, length, &msn, &lsn);
        assert(pointer != (char *)0);
        assert(pointer[0] == HAZER_STIMULUS_CHECKSUM);
        assert(pointer[1] == msn);
        assert(pointer[2] == lsn);
        assert(pointer[3] == '\r');
        assert(pointer[4] == '\n');

        rc = hazer_is_nmea(buffer[0]);
        assert(rc == !0);

        rc = hazer_is_nmea_name(buffer, length, "GBS");
        assert(rc == !0);

        count = hazer_tokenize(vector, sizeof(vector) / sizeof(vector[0]), buffer, length);
        assert(count == 12);

        size = hazer_serialize(temporary, sizeof(temporary), vector, count);
        assert(size == (strlen((const char *)temporary) + 1));
        temporary[size - 1] = msn;
        temporary[size] = lsn;
        temporary[size + 1] = '\r';
        temporary[size + 2] = '\n';
        temporary[size + 3] = '\0';
        assert(strcmp(DATA, (const char *)temporary) == 0);

        rc = hazer_parse_gbs(&fault, vector, count);
        assert(rc == 0);

        assert(fault.utc_nanoseconds == 6909000000000ULL);
        assert(fault.lat_millimeters == -31LL);
        assert(fault.lon_millimeters == -186LL);
        assert(fault.alt_millimeters == 219LL);
        assert(fault.probability == 0LL);
        assert(fault.est_millimeters == -354LL);
        assert(fault.std_deviation == 6972LL);
        assert(fault.id == 19);
        assert(fault.talker == HAZER_TALKER_GPS);
        assert(fault.system == HAZER_SYSTEM_GPS);
        assert(fault.signal == 2);

        gbs(&fault);
    }

    return 0;
}
