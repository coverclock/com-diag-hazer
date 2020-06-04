#!/bin/bash
# Copyright 2018-2019 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

# NMEA-PUBX-POSITION
# NMEA-PUBX-SVSTATUS
# NMEA-PUBX-TIME
# NMEA-PUBX-RATE GSV @1Hz
# NMEA-PUBX-RATE VTG @1Hz
# UBX-CFG-PRT [0] (all)
# UBX-CFG-MSG [3] UBX-NAV-TIMEGPS @1Hz
# UBX-CFG-MSG [3] UBX-NAV-TIMEUTC @1Hz
# UBX-CFG-MSG [3] UBX-NAV-CLOCK @1Hz
# UBX-CFG-MSG [3] UBX-TIM-TP @1Hz
# UBX-CFG-MSG [3] UBX-TIM-TM2 @1Hz
# UBX-CFG-MSG [3] UBX-MON-HW @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-STATUS @1Hz (ublox8 > fw18)
# UBX-CFG-MSG [3] UBX-NAV-ATT @1Hz (ublox8 > fw19)
# UBX-CFG-MSG [3] UBX-NAV-ODO @1Hz (ublox8 > fw15)
# UBX-CFG-MSG [3] UBX-NAV-PVT @1Hz (ublox8 > fw15)
# UBX-CFG-ITFM [8] LE(0x96b156YX) LE(0x0000631e) (X[4]=broadband threshold signed dB, Y[5]=continuous wave threshold signed dB)
# UBX-MON-VER [0]
# UBX-CFG-DAT [0]
# UBX-CFG-TPS [0]
# UBX-CFG-GNSS [0]

COMMANDS='
    "\$PUBX,00"
    "\$PUBX,03"
    "\$PUBX,04"
    "\$PUBX,40,GSV,0,0,0,1,0,0"
    "\$PUBX,40,VTG,0,0,0,1,0,0"
    "\\xb5\\x62\\x06\\x00\\x00\\x00"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x20\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x21\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x22\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x0d\\x01\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x0d\\x03\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x0a\\x09\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x03\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x05\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x09\\x01"
    "\\xb5\\x62\\x06\\x01\\x03\\x00\\x01\\x07\\x01"
    "\\xb5\\x62\\x06\\x39\\x08\\x00\\xf7\\x56\\xb1\\x96\\x1e\\x63\\x00\\x00"
    "\\xb5\\x62\\x0a\\x04\\x00\\x00"
    "\\xb5\\x62\\x06\\x06\\x00\\x00"
    "\\xb5\\x62\\x06\\x31\\x00\\x00"
    "\\xb5\\x62\\x06\\x3e\\x00\\x00"
'
