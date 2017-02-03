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

This software is an original work of its author(s).

Contact:

    Chip Overclock
    Digital Aggregates Corporation
    3440 Youngfield Street, Suite 209
    Wheat Ridge CO 80033 USA
    http://www.diag.com
    mailto:coverclock@diag.com
