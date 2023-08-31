#!/bin/bash
# Copyright 2020-2023 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Send a SIGHUP to all gpstool instances if any.
# USAGE hup [ PROGRAM [ SIGNAL ] ]
# EXAMPLES
#    hup
#    hup gpstool
#    hup gpstool HUP

exec pkill -${2:-"HUP"} ${1:-"gpstool"}
