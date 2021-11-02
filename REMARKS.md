# Remarks

N.B. Most of the snapshots below were taken from earlier versions of Hazer and
its gpstool utility. The snapshots were cut and pasted from actual output and
may differ slightly (or greatly) from that of the most current version. For
some of these, a \* was used to mean the degree symbol; later versions of
Hazer display the actual degree symbol using Unicode. I've tried to update
the command line examples to reflect the current code, but I may have missed
a few here or there, or it may not have been possible due to the availability
of the hardware under test.

## Sending Commands

gpstool can send initialization commands to the GPS device. These can be either
NMEA sentences (without escape sequences) or binary UBX sentence (with
escape sequences). In either case, gpstool will automatically append the
appropriate ending sequences including a computed NMEA or UBX checksum.
Here is an example of gpstool writing proprietary NMEA and binary UBX sequences
to a NaviSys GR-701W which has a UBlox 7 chipset.

    > gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E \
          -W "\$PUBX,00" \
          -W "\$PUBX,03" \
          -W "\$PUBX,04" \
          -W "\\xb5\\x62\\x06\\x01\\x08\\x00\\x02\\x13\\x00\\x01\\x00\\x00\\x00\\x00" \
          -W "\\xb5\\x62\\x0a\\x04\\x00\\x00" \
          -W "\\xb5\\x62\\x06\\x31\\x00\\x00" \
          -W "\\xb5\\x62\\x06\\x3e\\x00\\x00" \
          -W "\\xb5\\x62\\x06\\x06\\x00\\x00"

(The code to write commands to the device depends on being driven by incoming
data from the device, so if the device is initially silent, this won't work.
All the GPS receivers I've tested are chatty by default, so this hasn't been an
issue for me.)

If a written command is of zero length (really: has a NUL or \0 character as its
first character), gpstool exits. This can be used by a script, for example, to
use gpstool to send a command to change the baud rate of the GPS device serial
port and exit, and then start a new gpstool with the new baud rate.

As a special case, you can send initialization commands to a U-blox
device and wait until it responds with an ACK or NAK before sending the next
U-blox command.

    > gpstool -D /dev/ttyACM0 -b 230400 -8 -n -1 \
        -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x01\x00\x03\x20\x00' \
        -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x05\x00\x53\x10\x00' \
        -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x77\x10\x01' \
        -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x04\x00\x78\x10\x00' \
        -U '\xb5\x62\x06\x8a\x09\x00\x00\x01\x00\x00\x6b\x02\x91\x20\x01' \
        -U '\xb5\x62\x06\x01\x03\x00\x01\x14\x01' \
        -U ''

## Forwarding Datagrams

Here is an example of using gpstool to read an NMEA sentence stream from a
serial device at 115200 8n1, display the data using ANSI escape sequences to
control the output terminal, and forwards NMEA sentences to a remote
instance of itself listing on port 5555 on host "lead".

    > gpstool -D /dev/ttyUSB0 -b 115200 -8 -n -1 -E -G lead:5555
    
    $GPRMC,164659.00,A,3947.65335,N,10509.20343,W,0.063,,310718,,,D*63\r\n
    MAP 2018-07-31T16:46:59Z 39*47'39.20"N,105*09'12.20"W  5631.82' N     0.072mph PPS 1
    RMC 39.794222,-105.153391  1716.600m   0.000*    0.063knots [10] 9 10 5 0 4
    GSA {  51   6  12  48  28  19   2  24   3  17 } [10] pdop 1.70 hdop 0.86 vdop 1.47
    GSV [01] sat   2 elv 28 azm 206 snr 40dBHz con GPS
    GSV [02] sat   3 elv 14 azm  59 snr 27dBHz con GPS
    GSV [03] sat   6 elv 67 azm 166 snr 37dBHz con GPS
    GSV [04] sat  12 elv 23 azm 308 snr 33dBHz con GPS
    GSV [05] sat  17 elv 54 azm  44 snr 17dBHz con GPS
    GSV [06] sat  19 elv 72 azm 352 snr 31dBHz con GPS
    GSV [07] sat  22 elv  5 azm  39 snr 21dBHz con GPS
    GSV [08] sat  24 elv 40 azm 287 snr 18dBHz con GPS
    GSV [09] sat  28 elv 30 azm 112 snr 32dBHz con GPS
    GSV [10] sat  46 elv 38 azm 215 snr 34dBHz con GPS
    GSV [11] sat  48 elv 36 azm 220 snr 39dBHz con GPS
    GSV [12] sat  51 elv 44 azm 183 snr 43dBHz con GPS

## Receiving Datagrams

Here is the remote instance of gpstool receiving the NMEA stream via
the UDP socket on port 5555.

    > gpstool -G :5555 -E

    $GPGLL,3947.65274,N,10509.20212,W,164744.00,A,D*7F\r\n
    MAP 2018-07-31T16:47:44Z 39*47'39.16"N,105*09'12.12"W  5612.79' N     0.048mph PPS 0
    GGA 39.794212,-105.153369  1710.800m   0.000*    0.042knots [10] 9 10 5 0 4
    GSA {  51   6  12  48  28  19   2  24   3  17 } [10] pdop 1.71 hdop 0.86 vdop 1.48
    GSV [01] sat   2 elv 28 azm 206 snr 38dBHz con GPS
    GSV [02] sat   3 elv 14 azm  58 snr 26dBHz con GPS
    GSV [03] sat   6 elv 67 azm 165 snr 38dBHz con GPS
    GSV [04] sat  12 elv 24 azm 308 snr 33dBHz con GPS
    GSV [05] sat  17 elv 54 azm  44 snr 21dBHz con GPS
    GSV [06] sat  19 elv 72 azm 353 snr 33dBHz con GPS
    GSV [07] sat  22 elv  5 azm  39 snr 18dBHz con GPS
    GSV [08] sat  24 elv 41 azm 287 snr 19dBHz con GPS
    GSV [09] sat  28 elv 29 azm 112 snr 29dBHz con GPS
    GSV [10] sat  46 elv 38 azm 215 snr 34dBHz con GPS
    GSV [11] sat  48 elv 36 azm 220 snr 39dBHz con GPS
    GSV [12] sat  51 elv 44 azm 183 snr 42dBHz con GPS

## Using screen

You can use the screen utility, available for MacOS and Linux/GNU, to capture
the NMEA stream on a serial port. (And on Windows systems, I use PuTTY.)

    > screen /dev/cu.usbserial-FT8WG16Y 9600 8n1

    $GPRMC,190019.00,A,3947.65139,N,10509.20196,W,0.053,,060818,,,D*66
    $GPVTG,,T,,M,0.053,N,0.099,K,D*20
    $GPGGA,190019.00,3947.65139,N,10509.20196,W,2,10,1.05,1707.9,M,-21.5,M,,0000*5B
    $GPGSA,A,3,06,19,24,51,02,12,48,29,25,05,,,1.75,1.05,1.40*08
    $GPGSV,4,1,14,02,77,008,30,05,42,164,48,06,32,051,20,09,03,060,*71
    $GPGSV,4,2,14,12,73,214,28,17,04,101,13,19,24,092,20,24,06,217,31*71
    $GPGSV,4,3,14,25,45,305,29,29,17,294,11,31,03,327,08,46,38,215,30*71
    $GPGSV,4,4,14,48,36,220,32,51,44,183,42*7C
    $GPGLL,3947.65139,N,10509.20196,W,190019.00,A,D*7E
    $GPRMC,190020.00,A,3947.65143,N,10509.20192,W,0.044,,060818,,,D*63
    $GPVTG,,T,,M,0.044,N,0.081,K,D*2F
    $GPGGA,190020.00,3947.65143,N,10509.20192,W,2,09,1.05,1707.9,M,-21.5,M,,0000*50
    $GPGSA,A,3,06,19,24,51,02,12,48,25,05,,,,1.75,1.05,1.40*03
    $GPGSV,4,1,14,02,77,008,31,05,42,164,48,06,32,051,21,09,03,060,23*70
    $GPGSV,4,2,14,12,73,214,29,17,04,101,12,19,24,092,19,24,06,217,31*7B
    $GPGSV,4,3,14,25,45,305,29,29,17,294,09,31,03,327,06,46,38,215,31*77
    $GPGSV,4,4,14,48,36,220,32,51,44,183,43*7D
    $GPGLL,3947.65143,N,10509.20192,W,190020.00,A,D*7D
    $GPRMC,190021.00,A,3947.65143,N,10509.20191,W,0.050,,060818,,,D*64
    $GPVTG,,T,,M,0.050,N,0.092,K,D*28
    $GPGGA,190021.00,3947.65143,N,10509.20191,W,2,10,1.05,1708.0,M,-21.5,M,,0000*5C

## Using gpsd

You can test GPS devices independently of this software using the
excellent Linux open source GPS stack. Here is just a simple example of
stopping the GPS daemon if it has already been started (make sure you
are not going to break something doing this), restarting it in non-deamon
debug mode, and running a client against it. In this example, I use the
Garmin GLO Bluetooth device I have already set up, and the X11 GPS client.
When I'm done, I restart gpsd in normal daemon mode.

    > sudo service gpsd stop
    > gpsd -N /dev/rfcomm0 &
    > xgps
    ...
    > kill %+
    > sudo service start gpsd

## Using socat

You can use the socat utility, available for Linux/GNU and MacOS flavored
systems, to capture the NMEA stream on the UDP port.

    > socat UDP6-RECVFROM:5555,reuseaddr,fork STDOUT

    $GPGSA,M,3,32,10,14,18,31,11,24,08,21,27,01,,1.3,0.8,1.1*33
    $GPGSV,3,1,12,32,79,305,39,10,66,062,41,14,58,247,35,18,40,095,34*72
    $GPGSV,3,2,12,31,21,180,45,11,20,309,27,24,19,044,30,08,17,271,31*7A
    $GPGSV,3,3,12,21,13,156,39,27,13,233,33,01,09,320,20,51,43,183,42*72

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

## Using Bluetooth

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

## Using One Pulse Per Second

Some GPS devices provide a 1Hz One Pulse Per Second (1PPS) signal that is, if
implemented correctly, closely phase locked to GPS time. Hazer and its
gpstool utility are user-space software running on a non-real-time operating
system, so any periodic action by Hazer is at best approximate in terms of
period. But handling 1PPS even with some jitter is useful for casual testing
of GPS devices.

You can test GPS devices like the NaviSys GR-701W that provide 1PPS by toggling
the Data Carrier Detect (DCD) modem control line. This includes devices that
have a USB serial interface. Note the addition of the -c flag.

    > gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -c
    
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

You can also test GPS devices like the MakerFocus USB-Port-GPS that provide
1PPS by toggling a General Purpose I/O (GPIO) pin. This example (which I've run
on a Raspberry Pi) uses pin 18. Note the addition of the -I flag. (You may have
to run gpstool as root to access the GPIO pins.)

    # gpstool -D /dev/ttyUSB1 -b 9600 -8 -n -1 -I 18 -E
    
    $GPGSV,3,1,11,23,85,357,39,16,62,069,40,09,45,311,37,51,43,183,43*75\r\n
    MAP 2018-05-08T14:50:25Z 39*47'39.17"N,105*09'12.19"W  5588.51' N     0.000mph
    GGA 39.794215,-105.153387  1703.400m   0.000*    0.000knots [09] 8 9 5 3 3
    GSA {   8   7  22  26  23   9   3  16  27 } [09] pdop 1.94 hdop 1.03 vdop 1.64
    GSV [01] sat  23 elv 85 azm 357 snr 39dBHz con GPS
    GSV [02] sat  16 elv 62 azm  69 snr 40dBHz con GPS
    GSV [03] sat   9 elv 45 azm 311 snr 37dBHz con GPS
    GSV [04] sat  51 elv 43 azm 183 snr 44dBHz con GPS
    GSV [05] sat  26 elv 34 azm  48 snr 28dBHz con GPS
    GSV [06] sat   3 elv 33 azm 200 snr 44dBHz con GPS
    GSV [07] sat   7 elv 23 azm 268 snr 28dBHz con GPS
    GSV [08] sat  27 elv 20 azm 126 snr 36dBHz con GPS
    GSV [09] sat  22 elv 16 azm 183 snr 42dBHz con GPS
    GSV [10] sat   8 elv  5 azm 160 snr 34dBHz con GPS
    GSV [11] sat  31 elv  2 azm  73 snr  0dBHz con GPS

gpstool can assert an output GPIO pin in approximate syntonization with the
1PPS signal derived from either DCD or GPIO. This example (which I've run on a
Raspberry Pi with the GR-701W) uses pin 16. Note the addition of the -p flag.
(Again, you may have to run gpstool as root to access the GPIO pins.)

    # gpstool -D /dev/ttyUSB0 -b 9600 -8 -n -1 -E -c -p 16

The GPIO functions implemented in Diminuto and used by gpstool may get confused
if gpstool exits ungracefully leaving GPIO pins configured. If necessary, you
can deconfigure GPIO pins using the Diminuto pintool utility

    # pintool -p 18 -n
    # pintool -p 16 -n

## True Versus Magnetic Bearings

GPS devices compute the true bearing by comparing successive position fixes to
determine your speed and direction. Hence, the true bearing, e.g. "135.000\*T",
is only reliable if you are moving, and at a speed fast enough to be within
the resolution of the accuracy of the position fix. The magnetic bearing is an
actual magnetic compass bearing, but is only provided by GPS devices which also
have a magnetic compass; otherwise it will be displayed as "0.000\*M". The
cardinal compass direction, e.g. "SE", is based on the true bearing.

## Google Earth Pro

In February 2017 I used Hazer with Google Earth Pro, the desktop version of the
web based application. Today, August 2018, the real-time GPS feature of Google
Earth Pro no longer seems to work with latest version, 7.3.2, for the Mac (I
haven't tried it for other operating systems). Neither does 7.3.1. But 7.1.8
works.

Google Earth Pro only accepts GPS data on a serial port, or at least something
that kinda sorta looks like a serial port. I've used a FIFO (named pipe), via
the mkfifo(1) command, for stuff like this in the past, but there doesn't seem
to be any way to get Pro to recognize the FIFO; only serial(ish) devices may
apply here.

So I processed the NMEA stream from a serial-attached GPS device using gpstool
running on a Raspberry Pi, then forwarded it via UDP datagrams to another
gpstool on the Raspberry Pi. (This is the bin/producer.sh script.)

    gpstool -D /dev/ttyUSB0 -b 4800 -8 -n -1 -6 -c -G ip6-localhost:5555 -E

Then I used the second gpstool to forward the NMEA stream out a serial port
across a two FTDI USB-to-serial adaptors hooked back-to-back with a null
modem in between, to a Mac running Google Earth Pro. (This is the
bin/provider.sh script.)

    gpstool -G :5555 -K -D /dev/ttyUSB1 -b 4800 -8 -n -1

I used the "GPS Import Realtime" drop down menu in Google Earth Pro to select
the USB serial device representing the other end of the FTDI "milking
machine". The menu liked the device "/dev/cu.usbserial-XXXXXXXX" where the Xs
are some kind of internal identifier on MacOS where Pro was running. 

> Empirically, and according to a Google search, anecdotally, but undocumentedly,
> Google Earth Pro appears to only accept serial input at 4800 baud. More recent
> and advanced GPS devices default to 9600 baud, and can overrun a 4800 baud
> serial port. So I used a USGlobalSat BU-353S4, which defaults to 4800 baud, as
> my GPS device on the Linux server.

As Rube Goldberg as this is, it seems to work.

> The last time I tried this, in the summer of 2018, the current version of
> Google Earth at that time, 7.3.2, no longer supported this feature (even
> though it was documented). Neither did the prior version of 7.3.1. But 7.1.8,
> which was still available, worked as expected.

> In the summer of 2019, Google Earth Pro 7.1.8 no longer works on the most
> recent edition of MacOS, 10.15 "Catalina". I can no longer do real-time
> moving map displays using Google Earth Pro. Instead, I use the -C
> command line option of gpstool to save the PVT fixes to a CSV file,
> use the csv2kml script in Hazer to convert the CSV file to Keyhole
> Markup Langauge (KML), then import the resulting KML file into Google Earth
> Pro for visualization.

## NMEA TXT Sentences

Some devices are chatty and emit interesting and sometimes useful information
as NMEA TXT sentences. These are recognized by Hazer and logged by gpstool.
Some of the functional tests save standard error output in log files under the
build artifact directory, or redirected to the system log using the Diminuto
log command.

    2019-06-18T19:11:44.744147Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,HW UBX-M8030 00080000*60"
    2019-06-18T19:11:44.744971Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,ROM CORE 3.01 (107888)*2B"
    2019-06-18T19:11:44.745115Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,GPS;GLO;GAL;BDS*77"
    2019-06-18T19:11:44.745269Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,SBAS;IMES;QZSS*49"
    2019-06-18T19:11:44.745419Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,GNSS OTP=GPS;GLO*37"
    2019-06-18T19:11:44.745606Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,LLC=FFFFFFFF-FFFF7CBF-FFED7FAA-FFFFFFFF-FFFFFFF9*52"
    2019-06-18T19:11:44.745764Z <INFO> [27876] {7f65cb0405c0} Parse NMEA TXT "$GNTXT,01,01,02,PF=3FF*4B"

The u-blox ZED-F9P issues unsolicited warning and error messages as NMEA TXT
messages. For the most part these are undocumented. Like all other such
messages, they are logged.

    2019-11-15T17:09:05.039345Z <INFO> [24404] {7f1c8e05f600} Parse NMEA TXT "$GNTXT,01,01,00,MISM c 2 t 82497615*6C"

## Phantom GPS Satellite PRN 4

Around 2018-11-29T12:00-07:00, I was testing some changes to Hazer with
the Ublox-8 based BU353W10 receiver by comparing its results to those
of the web site <https://in-the-sky.org/satmap_radar.php> that presents
a real-time sky map of the visible orbiting space vehicles (not just
GPS). I noticed that my BU353W10 was reporting GPS PRN 4 as "in view"
with a zero elevation and zero azimuth; that vehicle wasn't reported by
the sky map. Worse: a little web-search-fu told me that there was no PRN
4; it does not appear in the most recent GPS almanac. The GPS vehicle
using PRN 4 was decommisioned and its pseudo-random number code has
not yet been reused.

Before I could do much else, PRN 4 dropped from view.

PRN 4 reappeared the next morning around 2018-11-30T09:00-07:00. I
quickly dumped the raw NMEA and verified using NMEA 0183 Version 4.10
pp. 96-97 that I wasn't decoding the GSV sentence incorrectly.

    $GPGSV,4,1,15,04,,,36,05,04,062,22,10,27,253,32,13,32,053,38*43\r\n
    $GPGSV,4,2,15,15,60,092,39,16,15,289,26,20,53,269,,21,72,336,22*75\r\n
    $GPGSV,4,3,15,24,06,129,33,26,10,264,15,27,11,322,31,29,36,170,50*78\r\n
    $GPGSV,4,4,15,46,38,215,45,48,36,220,43,51,44,183,44*43\r\n

However, I noticed that the elevation and azimuth for PRN 4 weren't actually
zero: they were empty strings, although the SNR was a reasonable (and changing
over time) value. I coded up a change to Hazer to detect this and mark it, and
to gpstool to display a '?' next to that SAT entry. I was able to test this
before PRN 4 again dropped from view.

PRN 4 reappeared about twenty minutes later.

    SAT [  1]     4:   0*elv    0*azm   33dBHz   ?                         GPS
    SAT [  2]     8:   3*elv  328*azm    0dBHz                             GPS
    SAT [  3]    10:  43*elv  273*azm   35dBHz <                           GPS
    SAT [  4]    13:  16*elv   41*azm    0dBHz                             GPS
    SAT [  5]    15:  48*elv   58*azm   37dBHz <                           GPS
    SAT [  6]    16:   6*elv  270*azm   26dBHz <                           GPS
    SAT [  7]    20:  67*elv  305*azm   35dBHz <                           GPS
    SAT [  8]    21:  82*elv   82*azm   27dBHz <                           GPS
    SAT [  9]    24:  20*elv  112*azm   30dBHz <                           GPS
    SAT [ 10]    27:  28*elv  311*azm   35dBHz <                           GPS
    SAT [ 11]    29:  14*elv  171*azm   46dBHz <                           GPS
    SAT [ 12]    32:  12*elv  207*azm   41dBHz <                           GPS
    SAT [ 13]    46:  38*elv  215*azm   45dBHz                             GPS
    SAT [ 14]    48:  36*elv  220*azm   44dBHz <                           GPS
    SAT [ 15]    51:  44*elv  183*azm   45dBHz <                           GPS
    SAT [ 16]    68:  19*elv   41*azm   27dBHz <                           GLONASS
    SAT [ 17]    69:  66*elv    6*azm   33dBHz <                           GLONASS
    SAT [ 18]    70:  42*elv  246*azm   18dBHz <                           GLONASS
    SAT [ 19]    77:   2*elv   15*azm   22dBHz                             GLONASS
    SAT [ 20]    78:   3*elv   60*azm   18dBHz                             GLONASS
    SAT [ 21]    83:  20*elv  152*azm   35dBHz <                           GLONASS
    SAT [ 22]    84:  78*elv  169*azm   30dBHz <                           GLONASS
    SAT [ 23]    85:  40*elv  326*azm   28dBHz <                           GLONASS
    SAT [ 24]    89:  67*elv    6*azm    0dBHz                             GLONASS

It continued to drop from view and reappear. Its period of appearance does
not coincide with the GPS orbital period.

Neither NMEA 0183 4.10 nor Ublox 8 R15 suggests any interpretation of the
empty elevation and azimuth fields. As always, I'm assuming this somehow is
a bug in my code. But it does occur to me that PRN 4 would be useful for
testing a ground-based GPS transmitter; the period of its appearance would
make sense for a transmitter in the continental America time zones.

Checking the FAA Notice To Airmen (NOTAM) notifications on the web, there are
GPS disruptions scheduled for the late November/early December time frame,
centered on the White Sands Missle Range (WSMR) in New Mexico, and the Yuma
Proving Grounds (YPG) in Arizona, either of which is potentially within
range of my location in Denver Colorado. So this could be the U.S. military
doing testing.

I temporarily added code to gpstool to monitor the comings and goings
of GPS PRN 4 and remark upon them in the system log. Here's an example
of what those log messages look like during an actual run of just a
few hours. '#' is the initial state value when gpstool starts running,
'?' means GPS PRN 4 came into view, and ' ' means it exited from view. The
log also includes the initial and maximum signal strength, and the
transmission duration in milliseconds.  All clock times are in MST.

Here is the log for a twenty-four hour period.

    Dec  3 09:32:06 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '#' now '?' at 25dBHz
    Dec  3 09:42:03 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 38dBHz for 597010ms
    Dec  3 09:48:59 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 34dBHz
    Dec  3 09:58:51 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 592009ms
    Dec  3 10:01:11 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 35dBHz
    Dec  3 10:29:57 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 40dBHz for 1726038ms
    Dec  3 10:30:11 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 38dBHz
    Dec  3 10:32:34 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 39dBHz for 143006ms
    Dec  3 10:33:14 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 35dBHz
    Dec  3 10:34:27 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 40dBHz for 72998ms
    Dec  3 10:34:50 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 33dBHz
    Dec  3 10:35:38 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 37dBHz for 48001ms
    Dec  3 10:35:50 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 37dBHz
    Dec  3 11:37:43 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 43dBHz for 3713086ms
    Dec  3 11:42:22 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 29dBHz
    Dec  3 11:49:29 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 427014ms
    Dec  3 11:59:15 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 25dBHz
    Dec  3 11:59:16 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 25dBHz for 999ms
    Dec  3 13:35:06 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 23dBHz
    Dec  3 13:35:07 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 23dBHz for 998ms
    Dec  3 16:37:48 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 31dBHz
    Dec  3 16:45:52 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 484015ms
    Dec  3 17:08:03 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 29dBHz
    Dec  3 17:11:16 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 193003ms
    Dec  3 17:14:37 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 30dBHz
    Dec  3 17:19:16 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 34dBHz for 279011ms
    Dec  3 17:26:53 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 31dBHz
    Dec  3 17:41:51 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 898023ms
    Dec  3 18:00:33 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 30dBHz
    Dec  3 18:06:14 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 341011ms
    Dec  3 18:09:17 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 33dBHz
    Dec  3 18:22:07 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 37dBHz for 770011ms
    Dec  3 18:26:10 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 34dBHz
    Dec  3 18:59:11 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 39dBHz for 1981048ms
    Dec  3 19:00:54 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 33dBHz
    Dec  3 19:30:34 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 38dBHz for 1780046ms
    Dec  3 19:33:50 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 31dBHz
    Dec  3 19:44:47 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 36dBHz for 657014ms
    Dec  3 19:55:24 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 27dBHz
    Dec  3 20:02:55 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 33dBHz for 451009ms
    Dec  4 05:52:37 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 26dBHz
    Dec  4 05:52:39 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 26dBHz for 2001ms
    Dec  4 05:52:40 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 26dBHz
    Dec  4 05:52:41 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 26dBHz for 1007ms
    Dec  4 08:10:39 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 31dBHz
    Dec  4 08:16:56 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 33dBHz for 377014ms
    Dec  4 08:19:21 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 29dBHz
    Dec  4 08:22:17 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 31dBHz for 176011ms
    Dec  4 08:28:52 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 32dBHz
    Dec  4 08:40:15 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 34dBHz for 683019ms
    Dec  4 09:03:34 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 33dBHz
    Dec  4 09:17:36 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 38dBHz for 842031ms
    Dec  4 09:22:47 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was ' ' now '?' at 35dBHz
    Dec  4 09:39:55 nickel gpstool[20052]: gpstool: phantom GPS PRN 4 was '?' now ' ' at 35dBHz for 1027038ms

The durations in this sample last for anywhere from a second to half an hour.
During this period the transmissions ceased at 20:00MST and resumed at 06:00MST.
This seems correlated with typical "working hours" in the U.S. continental
time zones.

On 2018-12-04, the U. S. Coast Guard straightened me out.

    *** GENERAL MESSAGE TO ALL GPS USERS ***
    ON APPROXIMATELY 10 OCT 2018 SVN36 WILL RESUME TRANSMITTING L-BAND
    UTILIZING PRN04. AT L-BAND ACTIVATION, SVN36/PRN04 WILL BE UNUSABLE
    UNTIL FURTHER NOTICE.  ADDITIONALLY, NO BROADCAST ALMANACS WILL
    INCLUDE SVN36/PRN04 UNTIL FURTHER NOTICE.
    *** GENERAL MESSAGE TO ALL GPS USERS ***

On 2018-12-23, the first of the next generation Block IIIA GPS satellites
was launched.

    *** GENERAL MESSAGE TO ALL GPS USERS ***
    GPS III SVN74 (PRN04) WAS LAUNCHED ON 23 DEC 2018 (2018 JDAY 357)
    AT 1351 ZULU. THIS SATELLITE WILL UNDERGO EXTENSIVE ON-ORBIT CHECK
    OUT AND TESTING PRIOR TO BEING SET HEALTHY.  A USABINIT NANU WILL
    BE SENT WHEN THE SATELLITE IS SET ACTIVE TO SERVICE.
    *** GENERAL MESSAGE TO ALL GPS USERS ***

Note that the new satellite uses PRN 4. I suspect now that the earlier
use of PRN 4 by SVN36 was some kind of control segment testing in advance
of the launch of SVN74.

## GPS Constellation Status 2019-02-20

The Global Positioning Systems Directoriate proposed a change to the
"Navstar GPS Control Segment to User Support Community Interfaces"
(ICD-GPS-240 and ICD-GPS-87) to modify the GPS Operational Advisory
Message to eliminate information about the orbital plane/slot and clock.
This change was approved by the Interface Control Working Group on
2018-12. I've captured a portion of a recent Advisory Message here to
snapshot this information before it is removed.

    GPS Constellation Active Nanu Status 2/20/19
    Plane       Slot    SVN     PRN     Block   Clock
    A           1       65      24      IIF     CS      
    A           2       52      31      IIR-M   RB      
    A           3       64      30      IIF     RB      
    A           4       48      7       IIR-M   RB      
    B           1       56      16      IIR     RB      
    B           2       62      25      IIF     RB      
    B           3       44      28      IIR     RB      
    B           4       58      12      IIR-M   RB      
    B           5       71      26      IIF     RB      
    C           1       57      29      IIR-M   RB      
    C           2       66      27      IIF     RB      
    C           3       72      8       IIF     CS      
    C           4       53      17      IIR-M   RB      
    C           5       59      19      IIR     RB      
    D           1       61      2       IIR     RB      
    D           2       63      1       IIF     RB      
    D           3       45      21      IIR     RB      
    D           4       67      6       IIF     RB      
    D           5       46      11      IIR     RB      
    D           6       34      18      IIA     RB
    E           1       69      3       IIF     RB      
    E           2       73      10      IIF     RB      
    E           3       50      5       IIR-M   RB      
    E           4       51      20      IIR     RB      
    E           6       47      22      IIR     RB      
    F           1       70      32      IIF     RB      
    F           2       55      15      IIR-M   RB      
    F           3       68      9       IIF     RB      
    F           4       60      23      IIR     RB      
    F           5       41      14      IIR     RB      
    F           6       43      13      IIR     RB      

    GPS Expandable Slot Designations
    B5=B1F
    D5=D2F
    F5=F2F
    B1=B1A
    D2=D2A
    F2=F2A

## Ardusimple SimpleRTK2B

The Ardusimple SimpleRTK2B board features a ZED-F9P "9th generation" U-Blox
chip. It can be equipped with radios like the ZigBee-based XBee. This allows
one SimpleRTK2B to be used as a stationary "base" in high-precision "survey-in"
mode, and a second SimpleRTK2B in mobile "rover" mode; the base transmits
position corrections to the rover via the radio using the RTCM protocol once
the survey has been completed to the configured level of accuracy. This
is a form of Differential GNSS and can, over time, achieve very high position
(and time) accuracy and precision.

                                Radio Antenna
                                      :
                                [ XBee3 SX ]
                                      ^ 
                                      | 
                                      v 
                     Header <--> XBee UART <--> FTDI <--> USB <--> { XCTU }
                                      ^
                                      |
                                      v
                                  UBX UART2
                                      |
                                      v
    Header <--> UBX UART1 <--> [ UBX-ZED-F9P ] <--> USB <--> { u-center or gpstool }
                                      :
                                  GPS Antenna

In the diagram above I show where you would connect the XBee configuration tool
XCTU, or the u-blox configuration tool u-center, but in normal use I only have
a host running gpstool connected.

I used XCTU to configure the XBee radio (setting it to Transparent mode, as a
Zigbee Router, and a serial port baud rate of 38400 8N1) and persist that
configuration in the XBee's non-volatile memory.

The ZED-F9P is configured at run-time using gpstool to send it commands. This
configuration is in volatile memory so that the GPS receiver reverts back to its
factory defaults when it is power cycled. Among other things, this allows me to
use SimpleRTK2B boards interchangeably in the field.

> Ultimately, neither the the Digi Xbee3 SX 900MHz radios, nor the Digi Xbee3
> LTE-M cellular radios which have the same form factor and are pin compatible,
> met my needs, although I tried both, the latter using AT&T LTE-M SIMs. As
> far as I can tell, however, both worked as advertised.

## Google Maps

The mapstool  utility converts strings containing latitude and longitude
coordinates of various formats (several of which are output by the gpstool
utility) into a decimal degrees format that is understood by Google Maps.
You can cut and paste the output of mapstool directly into the search bar
of the Google Maps web page.

    $ mapstool "39.794212196, -105.153349930"# HPP format
    39.794212196, -105.153349930
    
    $ mapstool "39 47 39.16390(N) 105 09 12.05974(W)"# NGS format
    39.794212194, -105.153349928
    
    $ mapstool "39°47'39.163\"N, 105°09'12.060\"W"# POS format
    39.794211944, -105.153350000

HPP is the U-blox UBX High Precision Position format. It is also a format
directly supported by Google Maps.

NGS is the format used in the National Geodetic Survey datasheets. It is
mostly for this format that I wrote this utility.

POS is the standard gpstool format. It is also a format (minus the BASH
backslash escapes) directly supported by Google Maps.

Nine significant fractional digits are used in the decimal degrees output
because that allows for about three significant fractional seconds digits for
the input. Also, billionths of a degree is the precision provided by the UBX
HPP format.

## Haversine and Geodesic Distances

The utility haversine computes the distance between two coordinates expressed
in decimal degrees using the Haversine algorithm based on great circles. This
is a simple trigonometric approach that (incorrectly) assumes the Earth is a
perfect sphere. The output is expressed in meters. The input can be cut and
pasted directly from the mapstool utility (see above).

    $ haversine 39.794212194, -105.153349928 39.794211944, -105.153350000
    0.0285029342

The utility geodesic computes the distance between two coordinates expressed
in decimal degress using the Geodesic algorithm based on an elliptical model
of the Earth from the WGS84 datum (this is a model used by most GPS receivers).
The output is expressed in meters. The input can be cut and pasted directly
from the mapstool utility (see above). This is a more accurate, but much
more computationally complex, approach than using great circles. This utility
makes use of the geodesic.h and geodesic.c files from Charles F. F. Karney's
geographiclib, which is licensed under the MIT license.

    $ geodesic 39.794212194, -105.153349928 39.794211944, -105.153350000
    0.0284344407

## Latitude/Longitude Precision

This is a useful way to interpret the precision implied by the number
of decimal places presented in degrees of latitude and longitude. A big
Thank You to Amy Fox of Blis, a company that provides location-powered
advertising abd analytics, who wrote about this in the Blis blog. Ms. Fox
also referenced a similar table in Wikipedia.

* 0 - 1.0 - 111 km - country or large region
* 1 - 0.1 - 11.1 km - large city or district
* 2 - 0.01 - 1.11 km - town or village
* 3 - 0.001 - 111 m - neighborhood, street
* 4 - 0.0001 - 11.1 m - individual street, large buildings
* 5 - 0.00001 - 1.11 m - individual tree, houses
* 6 - 0.000001 - 11.1 cm - individual humans
* 7 - 0.0000001 - 1.11 cm - practical limit of commercial surveying
* 8 - 0.00000001 - 1.11 mm - specialized surveying, tectonic plate movement
* 9 - 0.000000001 - 111 um - width of a strand of thread, microscopy

A. Fox, "Precision Matters: The Critical
Importance of Decimal Places", blis.com, 2017-07-09,
<https://blis.com/precision-matters-critical-importance-decimal-places-five-lowest-go/>

Wikipedia, "Wikipedia, "Decimal Degrees", 2020-11-14, <https://en.wikipedia.org/wiki/Decimal_degrees>

## Geocode Examples

Latitude/Longitude: 39°47'39.392"N, 105°09'12.293"W    

Decimal Degrees: 39.7942756, -105.1534148    

What3Words.com:	///beard.hobby.funds    

maps.google.com plus.codes: QRVW+PJ    

## Stupid Cat Tricks

gpstool has a couple of deliberate design features that might seem to be
problems, but they are there for a reason.

gpstool can be configured to receive NMEA sentences, UBX packets, and RTCM
messages via datagrams using User Datagram Protocol (UDP). This approach has
a big security hole: this technique has no authentication mechanism, nor is
the data stream encrypted. This allows gpstool to be spoofed by another
computer sending it bogus datagrams, or for a Man In The Middle attack to
spy on the data being forwarded.

I used UDP specifically because of its low overhead and the fact that
datagrams are tossed away rather than being sent late if the sending
queue somehow becomes clogged. I wrote about this at length in my blog.

C. Overclock, "Better Never Than Late", 2017-02-16,
<https://coverclock.blogspot.com/2017/02/better-never-than-late.html>

I do think I should fix this - probably using something like the Secure
Real-Time Protocol (SRTP) - but that hasn't happened yet.

You will also notice that there is no buffering between the source
of input data - be it standard input, a UDP socket, a serial port,
or a named pipe - and the NMEA, UBX, and RTCM state machines that feed
validated data into the processing stage of gpstool. This was done for
much the same reason: to keep gpstool as real-time as possible. It is
better to lose some data - after all, a solution update will typically
arrive in the next second - and have to resynchronize to the input stream
because gpstool could not keep up with the input source, than to add
latency to the display of the positioning, navigation, and timing
information.

However, it is really easy to add buffering - potentially a lot of it - just
by rethinking the command line. When you connect two applications via a pipe

    ls -l | more

you are creating an asynchronous ring buffer between the data source
(ls -l) and the data sink (more). According to pipe(7), the buffer is
sixteen virtual pages in size. With a typical virtual page size of four
kilobytes (which is true of all the systems I regularly use, including
the Raspberry Pi 4), that makes the buffer sixty-four kilobytes in size.

So you can add a bunch of buffering just by using a utility like socat
(socket catenate) to read data from the input source (like a serial port
in this example) and pipe it to gpstool.

    socat -u OPEN:/dev/ttyACM0,b9600 - | gpstool -S - -E

You can add even more buffering, sixty-four kilobytes at a time, by
adding more cat commands.

    socat -u OPEN:/dev/ttyACM0,b9600 - | cat | cat | gpstool -S - -E

Do this enough (like I have) and the latency will become obvious as
you notice a discrepancy between the actual system time reported in
the gpstool "LOC" (local) output line and the GNSS time reported in
the gpstool "TIM" (time) output line.
