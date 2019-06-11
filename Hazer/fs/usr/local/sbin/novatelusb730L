#!/bin/bash
# Copyright 2019 Digital Aggregates Corporation, Colorado USA
# Licensed unter the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Novatel Generic MiFi USB730L v1410p9030
rmmod rndis_host || true
/usr/sbin/usb_modeswitch -v 0x1410 -p 0x9030 -u 2 && dhclient eth1
