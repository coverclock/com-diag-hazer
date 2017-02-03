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
standard input. The serialtool in the Diminuto project is a useful way to
read data from serial-attached GPS devices like the BU-353S4.

    https://github.com/coverclock/com-diag-diminuto

Example:

    serialtool -D /dev/ttyUSB0 -b 4800 -8 -1 -n -l | gpstool -e

    $GPRMC,162135.000,A,3947.6521,N,10509.2024,W,0.00,109.12,030217,,,D
    RMC 2017-02-03T16:21:35Z { 39 47' 39.12"N 105 09' 12.14"W } 5623.29' 109.12true 0.00mph
    GSA { 32 10 14 18 24  8 11 21 31 27  1 } [11/12] pdop 1.30 hdop 0.80 vdop 1.10
    GSV [01/12/48] sat 32 elv 78 azm 281 snr 38dBHz
    GSV [02/12/48] sat 10 elv 68 azm  51 snr 38dBHz
    GSV [03/12/48] sat 14 elv 54 azm 242 snr 40dBHz
    GSV [04/12/48] sat 18 elv 43 azm  91 snr 39dBHz
    GSV [05/12/48] sat 24 elv 22 azm  46 snr 33dBHz
    GSV [06/12/48] sat  8 elv 19 azm 275 snr 36dBHz
    GSV [07/12/48] sat 11 elv 18 azm 312 snr 31dBHz
    GSV [08/12/48] sat 21 elv 17 azm 156 snr 41dBHz
    GSV [09/12/48] sat 31 elv 16 azm 181 snr 43dBHz
    GSV [10/12/48] sat 27 elv 16 azm 236 snr 31dBHz
    GSV [11/12/48] sat  1 elv  6 azm 320 snr 24dBHz
    GSV [12/12/48] sat 51 elv 43 azm 183 snr 45dBHz

This software is an original work of its author(s).

Contact:

    Chip Overclock
    Digital Aggregates Corporation
    3440 Youngfield Street, Suite 209
    Wheat Ridge CO 80033 USA
    http://www.diag.com
    mailto:coverclock@diag.com
