#!/bin/bash
# Copyright 2018 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

COMMANDS='
    "\$PUBX,00"
    "\$PUBX,03"
    "\$PUBX,04"
    "\\xb5\\x62\\x06\\x00\\x00\\x00"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\0x01\0x20\0x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\0x01\0x21\0x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\0x01\0x22\0x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\0x0d\0x01\0x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\0x0d\0x03\0x01"
    "\\xb5\\x62\\x0a\\x04\\x00\\x00"
    "\\xb5\\x62\\x06\\x06\\x00\\x00"
    "\\xb5\\x62\\x06\\x31\\x00\\x00"
    "\\xb5\\x62\\x06\\x3e\\x00\\x00"
'
