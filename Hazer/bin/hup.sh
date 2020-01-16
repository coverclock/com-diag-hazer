#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# USAGE hup [ PROGRAM ]
# EXAMPLE hup
# EXAMPLE hup gpstool
# EXAMPLE hup footool
# Send a SIGHUP to all gpstool instances if any.

exec pkill -HUP ${1:-"gpstool"}
