#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Converts the number of seconds since the UNIX epoch to an ISO8601 timestamp.

date -d "@${1:-0}" -u '+%Y-%m-%dT%H:%M:%S+00:00'
