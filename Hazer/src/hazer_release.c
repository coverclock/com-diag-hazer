/* vi: set ts=4 expandtab shiftwidth=4: */
/**
 * @file
 *
 * Copyright 2017-2020 Digital Aggregates Corporation, Colorado, USA<BR>
 * Licensed under the terms in LICENSE.txt<BR>
 * Chip Overclock (coverclock@diag.com)<BR>
 * http://www.diag.com/navigation/downloads/Hazer.html<BR>
 * The purpose of this translation unit is to embed the release string
 * inside the library or shared object. The object module will be statically
 * linked into an application only if the translation unit makes explicit
 * references to the storage here as external references.
 */

#include "com/diag/hazer/hazer_release.h"

const char COM_DIAG_HAZER_RELEASE_KEYWORD[] = "COM_DIAG_HAZER_RELEASE=" COM_DIAG_HAZER_RELEASE;
const char * COM_DIAG_HAZER_RELEASE_VALUE = &COM_DIAG_HAZER_RELEASE_KEYWORD[sizeof("COM_DIAG_HAZER_RELEASE=") - 1];
