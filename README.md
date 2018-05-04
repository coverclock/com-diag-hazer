com-diag-hazer
==============

# Copyright

Copyright 2017 by the Digital Aggregates Corporation, Colorado, USA.

# License

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

# Abstract

This file is part of the Digital Aggregates Corporation Hazer
package. Hazer is a simple C-based parser of the National Marine
Electronics Association (NMEA 0183 4.10) strings produced most Global
Positioning System (GPS) devices.  Unlike the Drover project, Hazer does
its own NMEA parsing.  Hazer includes a gpstool utility to display the
interpreted GPS data. gpstool accepts NMEA sentences from standard input,
from a serial(ish) device, or from a UDP socket.

This software is an original work of its author(s).

Hazer has been successfully tested with the following GPS chipsets.

Quectel L80-R    
SiRF Star II    
SiRF Star III    
SiRF Star IV    
U-Blox 7    
U-Blox LEA-6T    
U-Blox M8    

Hazer has been successfully tested with the following serial-to-USB chipsets.

Cygnal Integrated Products    
FTDI    
Prolific    
U-Blox (probably integrated into the GPS chip itself)    

Hazer has been successfully tested with the following GPS devices.

USGlobalSat BU-353S4 (SiRF Star IV/Prolific, 4800 8N1, v067Bp2303, ttyUSB, 1Hz) [1]    
USGlobalSat ND-105C (SiRF Star III/Prolific, 4800 8N1, v067Bp2303, ttyUSB, 1Hz)    
USGlobalSat BU-353S4-5Hz (SiRF Star IV/Prolific, 115200 8N1, v067Bp2303, ttyUSB, 5Hz)    
Stratux Vk-162 Gmouse (U-Blox 7, 9600 8N1, v1546p01A7, ttyACM, 1Hz) [2]    
Eleduino Gmouse (U-Blox 7, 9600 8N1, v1546p01A7, ttyACM, 1Hz) [2]    
Generic Gmouse (U-Blox 7, 9600 8N1, v1546p01A7, ttyACM, 1Hz) [2]    
Pharos GPS-360 (SiRF Star II/Prolific, 4800 8N1, v067BpAAA0, ttyUSB, 1Hz) [3]    
Pharos GPS-500 (SiRF Star III/Prolific, 4800 8N1, v067BpAAA0, ttyUSB, 1Hz) [3]    
MakerFocus USB-Port-GPS (Quectel L80-R/Cygnal, 9600 8N1, v10C4pEA60, ttyUSB, 1Hz) [2]    
Sourcingbay GM1-86 (U-Blox 7, 9600 8n1, p1546v01A7, ttyACM, 1Hz) [2]    
Uputronics Raspberry Pi GPS Expansion Board v4.1 (U-Blox M8, 9600 8n1, N/A, ttyAMA, 1Hz) [4]    
Jackson Labs Technologies CSAC GPSDO (U-Blox LEA-6T, 115200 8n1, N/A, ttyACM, 1Hz)    
Garmin GLO (unknown, Bluetooth, N/A, rfcomm, 10Hz) [4]    
NaviSys GR-701W (U-Blox 7/Prolific, 9600 8N1, v067Bp2303, ttyUSB, 1Hz) [5] [6]    

[1] My favorite unit so far, all things considered; also my first.    
[2] Emits all sorts of interesting stuff in $GPTXT sentences.    
[3] Install udev rules in overlay to prevent ModemManager from toying with device.    
[4] Receives GPS (U.S., formerly "Navstar")  and GLONASS (Russian) constellations.    
[5] Receives GPS (U.S.), GLONASS (Russian), and QZSS (Japanese) constellations.    
[6] Supports One Pulse Per Second (1PPS) by toggling Data Carrier Detect (DCD).    

Hazer has been tested on the following targets and platforms.

"Mercury"    
Dell OptiPlex 7040    
Intel Core i7-6700T @ 2.8GHz x 8    
Ubuntu 14.04.4    
Linux 4.2.0    
gcc 4.8.4    

"Nickel"    
Intel NUC5i7RYH    
Intel Core i7-5557U @ 3.10GHz x 8    
Ubuntu 16.04.2    
Linux 4.10.0    
gcc 5.4.0    

"Zinc"    
Raspberry Pi 3 Model B    
Broadcom BCM2837 Cortex-A53 ARMv7 @ 1.2GHz x 4    
Raspbian GNU/Linux 8.0    
Linux 4.4.34    
gcc 4.9.2    

# Contact

Chip Overclock    
Digital Aggregates Corporation    
3440 Youngfield Street, Suite 209    
Wheat Ridge CO 80033 USA    
http://www.diag.com    
mailto:coverclock@diag.com    

# Links

<https://github.com/coverclock/com-diag-hazer>

<http://coverclock.blogspot.com/2017/02/better-never-than-late.html>

<https://www.flickr.com/photos/johnlsloan/albums/72157678580152480>

# Build

Clone, build, and install Hazer in /usr/local.

    cd ~
    mkdir -p src
    cd src
    git clone https://github.com/coverclock/com-diag-hazer
    cd com-diag-hazer/Hazer
    make pristine all
    sudo make install


# Notes

    > gpstool -?

    usage: gpstool [ -d ] [ -v ] [ -D DEVICE ] [ -b BPS ] [ -7 | -8 ]  [ -e | -o | -n ] [ -1 | -2 ] [ -l | -m ] [ -h ] [ -s ] [ -c [ -p PIN ] ] [ -W NMEA ] [ -R | -E ] [ -A ADDRESS ] [ -P PORT ] [ -O ]
           -1          Use one stop bit for DEVICE.
           -2          Use two stop bits for DEVICE.
           -4          Use IPv4 for ADDRESS, PORT.
           -6          Use IPv6 for ADDRESS, PORT.
           -7          Use seven data bits for DEVICE.
           -8          Use eight data bits for DEVICE.
           -A ADDRESS  Send sentences to ADDRESS.
           -D DEVICE   Use DEVICE.
           -E          Like -R but use ANSI escape sequences.
           -O          Write sentences to DEVICE.
           -P PORT     Send to or receive from PORT.
           -R          Print a report on standard output.
           -W NMEA     Append * and checksum to NMEA and emit to DEVICE.
           -b BPS      Use BPS bits per second for DEVICE.
           -c          Wait for DCD to be asserted (implies -m).
           -d          Display debug output on standard error.
           -e          Use even parity for DEVICE.
           -l          Use local control for DEVICE.
           -m          Use modem control for DEVICE.
           -o          Use odd parity for DEVICE.
           -p PIN      Assert GPIO PIN with 1PPS (requires -c).
           -n          Use no parity for DEVICE.
           -h          Use RTS/CTS for DEVICE.
           -r          Reverse use of standard output and error.
           -s          Use XON/XOFF for DEVICE.
           -v          Display verbose output on standard error.

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

    > gpstool -D /dev/ttyUSB0 -b 115200 -8 -n -1 -E -6 -A lead -P 5555

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

You can use the socat utility, available for Linux/GNU and MacOS flavored
systems, to capture the NMEA stream on the UDP port.

    > socat UDP6-RECVFROM:5555,reuseaddr,fork STDOUT

    $GPGSA,M,3,32,10,14,18,31,11,24,08,21,27,01,,1.3,0.8,1.1*33
    $GPGSV,3,1,12,32,79,305,39,10,66,062,41,14,58,247,35,18,40,095,34*72
    $GPGSV,3,2,12,31,21,180,45,11,20,309,27,24,19,044,30,08,17,271,31*7A
    $GPGSV,3,3,12,21,13,156,39,27,13,233,33,01,09,320,20,51,43,183,42*72

You can use the screen utility, also available for MacOS and Linux/GNU,
to capture the NMEA stream on a serial port.

    > screen /dev/tty.usbserial-FT98KOIH 4800 8n1

    $GPGSA,M,3,32,10,14,18,24,08,21,11,27,31,01,51,1.3,0.8,1.1*37
    $GPGSA,M,3,32,10,14,18,24,08,21,27,31,,,,1.7,1.0,1.3*3D
    $GPGSV,3,1,12,32,77,276,37,10,69,048,31,14,53,241,37,18,44,090,23*7F
    $GPGSV,3,2,12,24,23,047,28,08,19,276,33,21,19,156,32,11,17,312,23*79
    $GPGSV,3,3,12,27,16,237,32,31,15,181,39,01,05,321,23,51,43,183,42*71

You may be tempted (I was) to dispense with gpstool entirely and use
socat to forward NMEA strings to a remote site.  Be aware that when
used in UDP consumer mode, gpstool expects every UDP datagram to be
a fully formed NMEA sentence, because that's how it sends them in UDP
producer mode. socat isn't so polite. Since the occasional UDP packet
will inevitably be lost, if the output of socat is piped into gpstool,
it will see a lot of corruption in the input stream. Using gpstool on
both ends means only entire NMEA sentences are lost, and gpstool can
recover from this.

    > socat OPEN:/dev/ttyUSB0,b115200 UDP6-SENDTO:[::1]:5555

The RMC and GGA sentences contain UTC timestamps (and the RMC contains
a DMY datestamp). Hazer rejects sentences for which time runs backwards.
Although this should be impossible for the sentences in the stream from
a GPS device, it is entirely possible for the UDP stream from a Hazer
producer, since UDP packet ordering is not guaranteed.

You might be tempted to use TCP instead of UDP. That sounds like a good
idea: guaranteed delivery, packets always in order. However, this can
introduce a lot of lantency in the NMEA stream. The result is the NMEA
stream received by the consumer may lag signficantly behind real-time,
and that lag increases the longer the system runs. So over time it
diverges more and more with reality.  It is better to lose an NMEA
sentence than have it delayed. After all, another more up-to-date sentence
is on the way right behind it.

You can use gpstool with Bluetooth GPS units like the Garmin GLO.

    > sudo bluetoothctl
    power on
    agent on
    scan on
    ...
    scan off
    pair 01:23:45:67:89:AB
    quit
    > sudo rfcomm bind 0 01:23:45:67:89:AB 1
    > sudo chmod 666 /dev/rfcomm0
    > gpstool -D /dev/rfcomm0 -E

    $GPVTG,350.4,T,341.6,M,000.08,N,0000.15,K,D*18\r\n
    $GPVTG,350.4,T,341.6,M,000.08,N,0000.15,K,D*18\r\n
    MAP 2017-09-14T14:22:05Z 39*47'39.20"N,105*09'12.13"W  5613.45' N     0.092mph
    GGA 39.794223,-105.153371  1711.000m 350.400*    0.080knots [12] 10 11 5 4 5
    GSA {  30  28  84   2  19   6  91  24  12  22  72   3 } [12] pdop 1.20 hdop 0.70 vdop 1.00
    GSV [01] sat  51 elv 43 azm 182 snr 45dBHz con GPS
    GSV [02] sat  30 elv  4 azm 161 snr 31dBHz con GPS
    GSV [03] sat  28 elv 35 azm 105 snr 22dBHz con GPS
    GSV [04] sat  84 elv 45 azm 245 snr 37dBHz con GPS
    GSV [05] sat   2 elv 21 azm 204 snr 36dBHz con GPS
    GSV [06] sat  19 elv 74 azm 346 snr 42dBHz con GPS
    GSV [07] sat   6 elv 56 azm 175 snr 45dBHz con GPS
    GSV [08] sat  91 elv 66 azm   5 snr 25dBHz con GPS
    GSV [09] sat  24 elv 36 azm 301 snr 26dBHz con GPS
    GSV [10] sat  12 elv 13 azm 304 snr 32dBHz con GPS
    GSV [11] sat  22 elv  7 azm  46 snr 24dBHz con GPS
    GSV [12] sat  72 elv 39 azm 326 snr 30dBHz con GPS
    GSV [13] sat   3 elv 14 azm  67 snr 27dBHz con GPS

    > sudo rfcomm release 0
    > sudo bluetoothctl
    power off
    quit

You can test GPS devices independently of this software using the
excellent Linux open source GPS stack. Here is just a simple example of
stopping the GPS daemon if it has already been started (make sure you
are not going to break something doing this), restarting it in non-deamon
debug mode, and running a client against it. In this example, I use the
Garmin GLO Bluetooth device I have already set up, and the X11 GPS client.

    > sudo service gpsd stop
    > gpsd -N /dev/rfcomm0 &
    > xgps
    ...
    > kill %+
    > sudo service start gpsd

You can test GPS devices like the NaviSys GR-701W that support One
Pulse Per Second (1PPS) by toggling the Data Carrier Detect (DCD) modem
control line. This includes devices that have a USB serial interface. Such
devices can be used for precision timing applications. (Note the addition
of the -c flag.)

    > gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -c
    
    $GPRMC,174227.00,A,3947.65321,N,10509.20367,W,0.027,,040518,,,D*68\r\n
    $GPRMC,174227.00,A,3947.65321,N,10509.20367,W,0.027,,040518,,,D*68\r\n
    MAP 2018-05-04T17:42:27Z 39*47'39.19"N,105*09'12.22"W  5600.00' N     0.031mph 1PPS
    RMC 39.794220,-105.153395  1706.900m   0.000*    0.027knots [10] 9 10 5 0 4
    GSA {   9   7   8  30  51  27  23  48  28  11 } [10] pdop 2.13 hdop 1.01 vdop 1.88
    GSV [01] sat   5 elv 10 azm 297 snr 22dBHz con GPS
    GSV [02] sat   7 elv 72 azm 348 snr 31dBHz con GPS
    GSV [03] sat   8 elv 56 azm  92 snr 23dBHz con GPS
    GSV [04] sat   9 elv 52 azm 187 snr 36dBHz con GPS
    GSV [05] sat  11 elv 18 azm 143 snr 34dBHz con GPS
    GSV [06] sat  16 elv  1 azm  55 snr  0dBHz con GPS
    GSV [07] sat  18 elv  4 azm 125 snr 27dBHz con GPS
    GSV [08] sat  23 elv 18 azm 161 snr 43dBHz con GPS
    GSV [09] sat  27 elv 29 azm  48 snr 31dBHz con GPS
    GSV [10] sat  28 elv 35 azm 246 snr 32dBHz con GPS
    GSV [11] sat  30 elv 48 azm 307 snr 35dBHz con GPS
    GSV [12] sat  46 elv 38 azm 215 snr 43dBHz con GPS
    GSV [13] sat  48 elv 36 azm 220 snr 42dBHz con GPS
    GSV [14] sat  51 elv 44 azm 183 snr 37dBHz con GPS

If the platform on which you are running Hazer supports GPIO, you can
assert an output pin in time with the DCD signal to provide a 1PPS strobe.
This example (which I've run on a Raspberry Pi) uses pin 16. Note that
you may have to run gpstool as root to access the GPIO pins. Also, if
gpstool exits ungracefully (which will typically be the case), you may
have to use Diminuto's pintool to deallocate the GPIO pin, as shown below.

    # pintool -p 16 -n
    # gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -c -p 16

