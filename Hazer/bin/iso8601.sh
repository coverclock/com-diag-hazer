#!/bin/bash
# Copyright 2020-2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# ABSTRACT
#
# Converts the number of seconds since the UNIX epoch to an ISO8601 timestamp
# in UTC.
#
# USAGE
#
# iso8601 [ EPOCHSECONDS ]
#
# EXAMPLES
#
# $ iso8601
# 2021-03-30T16:02:16.248194197+00:00
#
# $ iso8601 0
# 1970-01-01T00:00:00.000000000+00:00
#
# $ iso8601 1587742431.047610056
# 2020-04-24T15:33:51.047610056+00:00
#
# $ iso8601 1587742431.000000000
# 2020-04-24T15:33:51.000000000+00:00
#
# $ iso8601 1587742431
# 2020-04-24T15:33:51.000000000+00:00

exec date -d "@${1:-$(date '+%s.%N')}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00'
