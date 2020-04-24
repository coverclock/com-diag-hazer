#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Converts the number of seconds since the UNIX epoch to an ISO8601 timestamp.
# Examples
# $ iso8601 1587742431.047610056
# 2020-04-24T15:33:51.047610056+00:00
# $ iso8601 1587742431.000000000
# 2020-04-24T15:33:51.000000000+00:00
# $ iso8601 1587742431
# 2020-04-24T15:33:51.000000000+00:00

date -d "@${1:-0}" -u '+%Y-%m-%dT%H:%M:%S.%N+00:00'
