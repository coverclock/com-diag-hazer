/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 * @copyright Copyright 2023 Digital Aggregates Corporation, Colorado, USA.
 * @note Licensed under the terms in LICENSE.txt.
 * @brief This is a unit test of the version info.
 * @author Chip Overclock <mailto:coverclock@diag.com>
 * @see Hazer <https://github.com/coverclock/com-diag-hazer>
 * @details
 * This is a unit test of the version info.
 */

#include "com/diag/diminuto/diminuto_unittest.h"
#include "com/diag/hazer/hazer_version.h"

int main(void)
{
    SETLOGMASK();

    {
        TEST();

        ASSERT(COM_DIAG_HAZER_RELEASE_VALUE != (const char *)0);
        ASSERT(COM_DIAG_HAZER_REVISION_VALUE != (const char *)0);
        ASSERT(COM_DIAG_HAZER_VINTAGE_VALUE != (const char *)0);

        CHECKPOINT("RELEASE=\"%s\"\n", COM_DIAG_HAZER_RELEASE_VALUE);
        CHECKPOINT("REVISION=\"%s\"\n", COM_DIAG_HAZER_REVISION_VALUE);
        CHECKPOINT("VINTAGE=\"%s\"\n", COM_DIAG_HAZER_VINTAGE_VALUE);

        STATUS();
	}

    EXIT();
}

