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

This file is part of the Digital Aggregates Corporation Hazer
package. Hazer is a simple C-based parser of the National Marine
Electronics Association (NMEA) strings produced by the USGlobalSat
BU-353S4 Global Positioning System (GPS) device, a tiny little external
GPS receiver that emits NMEA strings over its built-in serial-to-USB
adaptor. The BU-353S4 is based on the SiRF Star IV chipset. If you want
to futz around with satellite geolocation, the BU-353S4 is a inexpensive
and easy way to do it. Hazer parses GGA, GSA, GSV, and RMC sentences
produced by a GPS "talker". While I used the BU-353S4 to test the
software, it is likely usable for any GPS receiver that conforms to NMEA
0183 4.10. Unlike the Drover project, Hazer does its own NMEA parsing.

Hazer includes a gpstool utility to display the interpreted GPS
data. gpstool accepts NMEA sentences from standard input, from a
serial(ish) device, or from a UDP socket.

    > gpstool -?

    usage: gpstool [ -1 | -2 ] [ -4 | -6 ] [ -7 | -8 ] [ -D DEVICE ] [ -b BPS ] [ -d ] [ -e | -o | -n ] [ -h ] [ -s ] [ -v ] [ -E ] [ -w NMEA ]
           -1          One stop bit.
           -2          Two stop bits.
           -4          IPv4.
           -6          IPv6.
           -7          Seven data bits.
           -8          Eight data bits.
           -A ADDRESS  Send to ADDRESS.
           -D DEVICE   Use DEVICE.
           -P PORT     Send to or receive from PORT.
           -E          Use ANSI escape sequences to control display.
           -b BPS      Bits per second.
           -d          Display debug output on standard error.
           -e          Even parity.
           -o          Odd parity.
           -n          No parity.
           -h          Hardware flow control (RTS/CTS).
           -r          Reverse use of standard output and error.
           -s          Software flow control (XON/XOFF).
           -v          Display verbose output on standard error.
           -w NMEA     Append * and checksum and write to standard output.

The Hazer library itself is standlone other than the usual standard
C and POSIX libraries. But the gpstool is also built on top of the
Digital Aggretates Corporation Diminuto package. Diminuto is a general
purpose C-based systems programming library that supports serial
port configuration, socket-based communication, and a passle of other
stuff. If you don't build Diminuto where the Makefile expects it, some
minor Makefile hacking might be required.

    https://github.com/coverclock/com-diag-diminuto

Here is an example of using gpstool to read an NMEA sentence stream from a
serial device at 4800 8n1, display the data using ANSI escape sequences to
control the output terminal, and forwards NMEA sentences to a remote
instance of itself listing on port 5555.

    > gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -E -6 -A ::1 -P 5555

    $GPRMC,144706.200,A,3947.6533,N,10509.2013,W,0.25,305.51,090217,,,D
    RMC 2017-02-09T14:47:06Z { 39 47' 39.19"N 105 09' 12.07"W } 5641.99' NW 0.29mph
    MAP { 39.794222,-105.153355 } 1719.700 305.510 0.250 [9] 8 9 5 5 33
    GSA { 18 10 21 32 27 24 14 15 20 } [09/11/12] pdop 1.80 hdop 1.10 vdop 1.40
    GSV [01/11/48] sat 18 elv 65 azm  50 snr 37dBHz
    GSV [02/11/48] sat 10 elv 64 azm 322 snr 40dBHz
    GSV [03/11/48] sat 21 elv 50 azm 145 snr 47dBHz
    GSV [04/11/48] sat 32 elv 50 azm 222 snr 44dBHz
    GSV [05/11/48] sat 27 elv 34 azm 266 snr 33dBHz
    GSV [06/11/48] sat 24 elv 33 azm  78 snr 37dBHz
    GSV [07/11/48] sat 14 elv 25 azm 219 snr 44dBHz
    GSV [08/11/48] sat 15 elv 14 azm  49 snr 38dBHz
    GSV [09/11/48] sat 20 elv  5 azm  90 snr 18dBHz
    GSV [10/11/48] sat 51 elv 43 azm 183 snr 42dBHz
    GSV [11/11/48] sat  8 elv 20 azm 307 snr  0dBHz

Here is the remote instance of gpstool receiving the NMEA stream via
the UDP socket on port 5555.

    > gpstool -6 -P 5555 -E

    $GPRMC,144745.200,A,3947.6542,N,10509.2020,W,0.21,305.51,090217,,,D
    RMC 2017-02-09T14:47:45Z { 39 47' 39.25"N 105 09' 12.12"W } 5633.46' NW 0.24mph
    MAP { 39.794237,-105.153367 } 1717.100 305.510 0.210 [10] 8 9 5 5 3
    GSA { 18 10 21 32 27 24 14  8 15 20 } [10/11/12] pdop 1.50 hdop 0.90 vdop 1.30
    GSV [01/11/48] sat 18 elv 65 azm  50 snr 39dBHz
    GSV [02/11/48] sat 10 elv 64 azm 322 snr 40dBHz
    GSV [03/11/48] sat 21 elv 50 azm 145 snr 46dBHz
    GSV [04/11/48] sat 32 elv 50 azm 222 snr 45dBHz
    GSV [05/11/48] sat 27 elv 34 azm 266 snr 30dBHz
    GSV [06/11/48] sat 24 elv 33 azm  78 snr 35dBHz
    GSV [07/11/48] sat 14 elv 25 azm 219 snr 44dBHz
    GSV [08/11/48] sat  8 elv 20 azm 307 snr 25dBHz
    GSV [09/11/48] sat 15 elv 14 azm  49 snr 37dBHz
    GSV [10/11/48] sat 20 elv  5 azm  90 snr 22dBHz
    GSV [11/11/48] sat 51 elv 43 azm 183 snr 42dBHz

You can also use the socat utility, available for most Linux/GNU/POSIX
flavored systems, to capture the NMEA stream on the UDP port.

    > socat UDP6-RECVFROM:5555,reuseaddr,fork STDOUT

    $GPGGA,160753.800,3947.6463,N,10509.2027,W,2,11,0.8,1727.3,M,-20.8,M,2.8,0000*7C
    $GPGSA,M,3,32,10,14,18,31,11,24,08,21,27,01,,1.3,0.8,1.1*33
    $GPGSV,3,1,12,32,79,305,39,10,66,062,41,14,58,247,35,18,40,095,34*72
    $GPGSV,3,2,12,31,21,180,45,11,20,309,27,24,19,044,30,08,17,271,31*7A
    $GPGSV,3,3,12,21,13,156,39,27,13,233,33,01,09,320,20,51,43,183,42*72
    $GPRMC,160753.800,A,3947.6463,N,10509.2027,W,0.30,206.04,090217,,,D*7D

You can dispense with gpstool entirely (which really only exists to test
the Hazer library) and use socat to forward NMEA strings to a remote site.
Be aware that when used in UDP consumer mode, gpstool expects every UDP
datagramto be a fully formed NMEA sentence, because that's how it sends
them in UDP producer mode. socat isn't so polite. Since the occasional
UDP packet will inevitably be lost, if the output of socat is piped into
gpstool, it will see a lot of corruption in the input stream. Using
gpstool on both ends means only entire NMEA sentences are lost, and
gpstool can recover from this.

    > socat OPEN:/dev/ttyUSB0,b115200 UDP6-SENDTO:[::1]:5555

Hazer has been successfully tested with the following devices.

    USGlobalSat BU-535S4 (SiRF Star IV, 4800 8N1, ttyUSB)
    USGlobalSat ND-105C (SiRF Star III, 4800 8N1, ttyUSB)
    USGlobalSat BU-353S4-5Hz (SiRF Star IV, 115200 8N1, ttyUSB)
    Stratux Vk-162 (u-blox 7, 9600 8N1, ttyACM)
    Eleduino Gmouse (u-blox 7, 9600 8N1, ttyACM)

This software is an original work of its author(s).

Contact:

    Chip Overclock
    Digital Aggregates Corporation
    3440 Youngfield Street, Suite 209
    Wheat Ridge CO 80033 USA
    http://www.diag.com
    mailto:coverclock@diag.com
