com-diag-hazer
=================

Copyright 2017 by the Digital Aggregates Corporation, Colorado, USA.

LICENSE

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

Alternative commercial licensing terms are available from the copyright
holder. Contact Digital Aggregates Corporation for more information.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, contact

    Free Software Foundation, Inc.
    59 Temple Place, Suite 330
    Boston MA 02111-1307 USA
    http://www.gnu.org/copyleft/lesser.txt

ABSTRACT

This file is part of the Digital Aggregates Corporation Hazer package. Hazer is
a simple C-based parser of the National Marine Electronics Association (NMEA)
strings produced by the USGlobalSat BU-353S4 Global Positioning System (GPS)
device, a tiny little external GPS receiver that emits NMEA strings over its
built-in serial-to-USB adaptor. The BU-353S4 is based on the SiRF Star IV
chipset. If you want to futz around with satellite geolocation, the BU-353S4
is a inexpensive  and easy way to do it. Hazer parses GGA, GSA, GSV, and RMC
sentences produced by a GPS "talker". While I used the BU-353S4 to test the
software, it is likely usable for any GPS receiver that conforms to NMEA 0183
4.10. Unlike the Drover project, Hazer does its own NMEA parsing. It includes
a gpstool to display processed GPS data. gpstool accepts NMEA sentences from
standard input or from a serial(ish) device.

This library is built on top of the Digital Aggretates Corporation Diminuto
package. Diminuto is a general purpose C-based systems programming library that
supports serial port configuration, socket-based communication, and a passle
of other stuff. If you don't build Diminuto where the Makefile expects it,
some minor Makefile hacking might be required.

    https://github.com/coverclock/com-diag-diminuto

Here is an example of using gpstool to read an NMEA sentence stream from a
serial device at 4800 8n1, display the data using ANSI escape sequences to
control the output terminal, and send the raw parsed time and position data
in a printable form to a remote server via UDP port 5555.

    > gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E -P 5555

    $GPRMC,205040.400,A,3947.6529,N,10509.2027,W,0.17,252.01,080217,,,D
    RMC 2017-02-08T20:50:40Z { 39 47' 39.17"N 105 09' 12.16"W } 5636.41' W 0.20mph
    MAP { 39.794215,-105.153378 } 1718.000 252.010 0.170 [10] 8 9 5 5 3
    GSA {  3 23 16 26 22 31  9 14  7  6 } [10/12/12] pdop 1.50 hdop 0.90 vdop 1.10
    GSV [01/12/48] sat  3 elv 59 azm 220 snr 40dBHz
    GSV [02/12/48] sat 23 elv 58 azm 315 snr 41dBHz
    GSV [03/12/48] sat 16 elv 58 azm 135 snr 46dBHz
    GSV [04/12/48] sat 26 elv 54 azm  76 snr 34dBHz
    GSV [05/12/48] sat 22 elv 46 azm 194 snr 48dBHz
    GSV [06/12/48] sat 31 elv 22 azm  56 snr 31dBHz
    GSV [07/12/48] sat  9 elv 20 azm 301 snr 24dBHz
    GSV [08/12/48] sat 14 elv 14 azm 105 snr 29dBHz
    GSV [09/12/48] sat  7 elv  8 azm 249 snr 34dBHz
    GSV [10/12/48] sat  6 elv  5 azm 310 snr 28dBHz
    GSV [11/12/48] sat 29 elv  2 azm  39 snr 22dBHz
    GSV [12/12/48] sat 51 elv 43 azm 183 snr 43dBHz

The following command is useful for testing the reception of the UDP datagrams.

    > socat UDP-RECVFROM:5555,reuseaddr,fork STDOUT

    1486579223000000000 39794206666 -105153371666 1713600 225890000000 0
    1486579223000000000 39794206666 -105153371666 1713600 225890000000 0
    1486579224000000000 39794206666 -105153371666 1713600 225890000000 0
    1486579224000000000 39794206666 -105153371666 1713600 225890000000 0
    1486579225000000000 39794206666 -105153371666 1713600 225890000000 0

The datagram fields are POSIX time in nanoseconds, latitude in nanodegrees,
longitude in nanodegrees, altitude in millimeters, course in nanodegrees, and
speed in microknots. The weird units are to preserve as many significant digits
as possible, support a wide dynamic range, eliminate any losses due to unit
conversions, and defer any use of floating point to the application.

Hazer has been successfully tested with the following devices.

    USGlobalSat BU-535S4 (SiRF Star IV chipset, 4800 bps)
    USGlobalSat ND-105C (SiRF Star III chipset, 4800 bps)
    USGlobalSat BU-353S4-5hz (SiRF Star IV chipset, 115200 bps)

This software is an original work of its author(s).

Contact:

    Chip Overclock
    Digital Aggregates Corporation
    3440 Youngfield Street, Suite 209
    Wheat Ridge CO 80033 USA
    http://www.diag.com
    mailto:coverclock@diag.com
