#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# UBX-MON-VER [0]

COMMANDS='
    "\$PUBX,00"
    "\$PUBX,03"
    "\$PUBX,04"
    "\\xb5\\x62\\x0a\\x04\\x00\\x00"
'
