/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2017-2018 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is the Tokenize unit test.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 */

#include <stdio.h>
#include "com/diag/hazer/hazer.h"
#include "./unittest.h"

int main(void)
{
    ssize_t size = 0;
    ssize_t count = 0;
    hazer_buffer_t buffer = { 0 };
    hazer_vector_t vector = { 0 };
    hazer_buffer_t datagram = { 0 };
    static const size_t SIZE = sizeof(buffer);
    static const size_t COUNT = sizeof(vector) / sizeof(vector[0]);
    static const char BUFFER[] = "$ABCDE,1,22,333*44\r\n";
    static const char DATAGRAM[] = "$ABCDE,1,22,333*";

    /**************************************************************************/

    count = hazer_tokenize((char **)0, 0, (void *)0, 0);
    assert(count == 0);

    count = hazer_tokenize((char **)0, 0, buffer, SIZE);
    assert(count == 0);

    memset(vector, ~0, sizeof(vector));
    count = hazer_tokenize(vector, 0, (void *)0, 0);
    assert(count == 0);
    assert(vector[0] == (char *)~0);

    memset(vector, ~0, sizeof(vector));
    count = hazer_tokenize(vector, 0, buffer, 0);
    assert(count == 0);
    assert(vector[0] == (char *)~0);

    memset(vector, ~0, sizeof(vector));
    count = hazer_tokenize(vector, 0, buffer, SIZE);
    assert(count == 0);
    assert(vector[0] == (char *)~0);

    /**************************************************************************/

    memset(vector, ~0, sizeof(vector));
    count = hazer_tokenize(vector, 1, (void *)0, 0);
    assert(count == 1);
    assert(vector[0] == (char *)0);

    memset(vector, ~0, sizeof(vector));
    count = hazer_tokenize(vector, 1, buffer, 0);
    assert(count == 1);
    assert(vector[0] == (char *)0);

    /**************************************************************************/

    memset(vector, ~0, sizeof(vector));
    strcpy(buffer, BUFFER);
    count = hazer_tokenize(vector, COUNT, buffer, sizeof(buffer));
    assert(count == 5);
    assert(vector[4] == (char *)0);
    assert(strcmp(vector[0], "$ABCDE") == 0);
    assert(strcmp(vector[1], "1") == 0);
    assert(strcmp(vector[2], "22") == 0);
    assert(strcmp(vector[3], "333") == 0);

    /**************************************************************************/

    size = hazer_serialize((void *)0, 0, (char **)0, 0);
    assert(size == 0);

    size = hazer_serialize((void *)0, 0, vector, COUNT);
    assert(size == 0);

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, 0, (char **)0, 0);
    assert(size == 0);
    assert(datagram[0] == (unsigned char)~0);

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, 0, vector, 0);
    assert(size == 0);
    assert(datagram[0] == (unsigned char)~0);

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, 0, vector, COUNT);
    assert(size == 0);
    assert(datagram[0] == (unsigned char)~0);

    /**************************************************************************/

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, 1, (char **)0, 0);
    assert(size == 1);
    assert(datagram[0] == '\0');

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, 1, vector, 0);
    assert(size == 1);
    assert(datagram[0] == '\0');

    /**************************************************************************/

    memset(datagram, ~0, sizeof(datagram));
    size = hazer_serialize(datagram, sizeof(datagram), vector, count);
    assert(size == sizeof(DATAGRAM));
    assert(strcmp(datagram, DATAGRAM) == 0);

    /**************************************************************************/

    return 0;
}
