# Issues

Here is a list of weird things I've observed in my travels using
various GNSS modules. U-blox may appear to be over represented
here, but that's only because they're my favorite GNSS device
manufacturer.

## Wrong number of satellites reported for GLONASS on U-Blox UBX-ZED-F9P

The Ardusimple SimpleRTK2B board uses the U-Blox ZED-F9P GPS receiver.
I'm pretty sure the firmware in the ZED-F9P-00B-01 chip on my SimpleRTK2B
board has a bug. I believe this GSV sentence that it emitted is incorrect.

    $GLGSV,3,3,11,85,26,103,25,86,02,152,29,1*75\r\n

This GSV sentence says it is the third of three GSV sentences for the
GLONASS constellation, and there are eleven total satellites cited in
the three GSV sentences.

GSV is unusual amongst the typical NMEA sentences in that it is variable
length. Each GSV sentence can contain at most four satellites, each
represented by four numbers: the space vehicle ID (which is specific to
the constellation), the SV's elevation in degrees, the SV's azimuth in
degrees, and the signal strength in dB Hz.

The two prior GSV sentences for this report of GLONASS satellites in
view would have had at most four satellites, for a total of eight,
leaving three satellites for this final sentence in the sequence. This
sentence only has the metrics for two satellites. Note also that this
messages has the optional signal identifier as its last field before
the checksum.

I think either there should be a third set of four fields for the eleventh
satellite, or the total count should be ten instead of eleven. My software
has been modified to account for this malformed message; it originally
core dumped with a segmentation violation.

2019-06-06: U-Blox says this FW bug will be fixed in a subsequent release.

<https://portal.u-blox.com/s/question/0D52p00008WRsgMCAT/ubxzedf9p-incorrect-number-of-satellites-in-view-for-nmea-gsv-for-glonass>

## Lost Characters on Gen 8 and Gen 9 U-blox Modules using USB ACM Port

I've been troubleshooting a weird issue with sequences of characters being
lost on the modem-ish (ttyACM) USB connection on a U-blox UBX-ZED-F9P
(generation 9) chip. This occurs when using the Ardusimple SimpleRTK2B and
Sparkfun GPS-RTK2 boards. I also see it a U-Blox UBX-M8030 (generation 8)
chip in a GlobalSat BU353W10 dongle. I've seen in on Intel (Dell) and
ARM (Raspberry Pi 3B+ and 4B) systems.

I've seen it using my software,

    gpstool -D /dev/ttyACM0 -b 115200 -8 -n -1 -m -v -R

using screen

    screen /dev/ttyACM0 115200 8n1

using socat,

    socat OPEN:/dev/ttyACM0,b115200 -

and even just using cat (after configuring the baud rate of the port)

    cat /dev/ttyACM0

to collect data off the USB port.

I've also seen it in the binary packet view (which is like using the -v
verbose option with gpstool) when running the u-blox u-center tool on
a Windows 10 laptop.

I've used the Linux usbmon USB debugging tool to establish that the
characters are missing at a very low level in the USB driver. And finally
I used a Total Phase USB Protocol Analyzer hardware tool to verify that
the characters are already missing as the data comes out of the GPS
module, before it ever reaches my Linux system.

Although this insures that the data isn't being dropped by my software or
my hardware, my hardware and the underlying USB hardware and OS drivers
likely have something to do with it: I see it occur much more often on
the Intel servers, and on those, more often on the faster processors.

I've described this at length in the article

<https://coverclock.blogspot.com/2019/06/this-is-what-you-have-to-deal-with.html>

The u-blox support forum has another person reporting something similar.

<https://portal.u-blox.com/s/question/0D52p00008vA2ElCAK/seeing-partial-nmea-sentences-on-usb-interface>

## End Of File (EOF) on U-blox UBX-ZED-F9P when using Ubuntu VM

Several times, while running this software under Ubunto 19.10 in a
virtual machine on a Lenovo ThinkPad T430s running Windows 10 using a
U-blox ZED-F9P receiver on a SparkFun GPS-RTK2 board - and only under
those circumstances - I've seen gpstool receive an EOF from the input
stream. The standard I/O function ferror() returned false and feof()
returned true (so the underlying standard I/O library thinks it was a
real EOF - which in this context I think means a disconnect - and not
an error). The tool fired right back up with no problem. This happens
very infrequently, and my suspicion is that VMware Workstation 15 Pro is
disconnecting the USB interface from the VM for some reason, maybe as a
result of Windows 10 power management on the laptop. This is something
to especially worry about if you are running a long term survey which
would be interrupted by this event. (I was doing this mostly to test
VMware and my Ubuntu installation on my field laptop.)

## Corruption of Serial Data Stream by LTE-M RF on U-blox UBX-CAM-M8Q

For my Wheatstone project, I used a Digi XBee LTE-M radio in "transparent
mode" to send JSON datagrams via UDP based on output from gpstool.
My initial test setup had me reading the serial port of a U-Blox CAM-M8Q
GNSS receiver using an FTDI-to-serial convertor. It ran flawlessly for
days.

In my second test setup, I was running gpstool on a Raspberry Pi and
reading the output stream of the CAM-M8Q via the RPi's serial port. I
used typical jumper wires to go from the CAM-M8Q (really, from the pins
on the Digi XBIB-CU-TH development board that were connected to the
CAM-M8Q on the Digi XBIB-C-GPS daughter board) to the RPi.

This worked initially. But a couple of days later I started getting
garbage characters - typically with a lot of bits set, like 0xf4 - in the
middle of NMEA sentences, causing gpstool to reject the sentence (checksum
failure) and resync with the stream. Eventually I could only run a test
for a few minutes; the NMEA stream would desync and then fail to resync.
If I restarted gpstool, it would run okay for a few minutes, then fail in
the same way.

It took me most of a morning to debug this. It turned out the jumper wires
for the serial connection, which were unshielded and not twisted pair, had
gotten right up against the cellular antenna for the LTE-M radio as I had
moved stuff around on my lab bench. The transmit RF power was apparently
enough to induce a signal on the serial connection, which could only
occasionally be decoded by the UART in the RPi as a valid (but incorrrect)
data frame.

I switched to a different cellular antenna, rearranged my lab bench,
and my test has run flawlessly for many hours since then.

## U-Blox CAM-M8Q PUBX,04 Time Message

GNSS receivers manufactured by U-blox support two different kinds of
proprietary messaging for both output and input: the UBX binary
protocol, and the PUBX ("P UBX") proprietary NMEA extensions. The
different PUBX messages are distinguished by the second field in the
proprietary NMEA-format message which contains a two digit number.

The PUBX,04 message provides the current time and date in UTC. This
timestamp can be emitted even when the U-blox device does not have a
current fix. It is apparently based on its internal battery-backed clock,
and, I assume, depends on the reliability of the device's own clock
oscillator, and how recently a prior fix was achieved before a warm or
hot start.

My reading of the UBLOX M8 R24 documentation doesn't suggest that there
is anything in the message to indicate that this time is, when the device
clock is in holdover, synthesized and not disciplined by a current 3D
position fix.

I have seen the NMEA sentences GGA, GLL, and RMC also contain a
synthesized time, but they contain indicators that there is no active fix,
so the application can know to discount these times.

I find it misleading.

## U-blox CAM-M8Q RMC failing to report time after 3D lock

While testing the PUBX,04 feature mentioned above, I encountered a
case where the U-blox CAM-M8Q reported a 3D lock with seven satellites
active, was reporting the correct UTC time in the PUBX,04 TIME message,
but the NMEA RMC message did not report the time (indicated via a 'V'
that it had no lock). I power cycled the device, and it worked correctly
on the subsequent identical test sequence.

## Bad Elf GPS Pro+ Invalid Message

The Bad Elf GPS Pro+ device is documented to use the MediaTek (MTK)
chipset.  Hazser 46.0.4 received the following RMC sentence from the
device (BE-GPS-2300, SN 020661, FW 2.1.60, HW 8.0.0), as documented in a
"contenate" (-C) file.

    $GPRMC,203410.000,A3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*68

This is a malformed sentence that should be as follows (note the addition
of a comma after the "A").

    $GPRMC,203410.000,A,3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*44

I initially assumed that this was an error in my software, perhaps in its
ability to keep up with the serial data stream. However, the checksum "68"
in the received packet is correct given the missing comma. Which means the
sentence was checksummed and transmitted by the MTK as it was received.

    checksum '$GPRMC,203410.000,A3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*'
    $GPRMC,203410.000,A3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*68\r\n

Checksumming the correct sentence yields a checksum of "44".

    checksum '$GPRMC,203410.000,A,3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*'
    $GPRMC,203410.000,A,3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*44\r\n

This can be checked by running just the correct sentence with the new checksum
through gpstool.

    echo -n -e '$GPRMC,203410.000,A,3947.6492,N,10509.1907,W,0.05,68.88,131021,,,D*44\r\n' | gpstool -S - -R

I have tested other GNSS devices with the MTK chipset and haven't run into
this issue. I have to run the GPS Pro+ for several hours to recreate it. The
fact that the checksum is correct for the invalid sentence as transmitted
suggests this is not an issue in the serial-to-USB conversion.

Bad Elf support ticket 43008 submitted 2021-10-14 08:14MDT.

Update 2021-10-18: an engineer at Bad Elf has explained that the GPS
Pro Plus+ must alter and regenerate the NMEA sentences produced by the
MTK GPS device to make them palatable to the native parser in the Apple
iPhone and iPad products. This explains the correct checksums on the
incorrect sentences.

### Addendum 1

Here's another example: there is a missing comma before the "N" direction
indication, yet the checksum is correct. It took almost five hours for
this to show up (although a similar error might have occurred earlier
and not been in a field checked by an assert).

    checksum '$GPRMC,002525.000,A,3947.6529N,10509.2015,W,0.01,68.88,161021,,,D*65'
    $GPRMC,002525.000,A,3947.6529N,10509.2015,W,0.01,68.88,161021,,,D*65\r\n

Compare with the correct sentence:

    checksum '$GPRMC,002525.000,A,3947.6529,N,10509.2015,W,0.01,68.88,161021,,,D*65'
    $GPRMC,002525.000,A,3947.6529,N,10509.2015,W,0.01,68.88,161021,,,D*49\r\n

### Addendum 2

Yet another: note the missing comma after the "W".

    checksum '$GPRMC,012859.000,A,3947.6540,N,10509.2019,W0.01,125.17,191021,,,D*50'
    $GPRMC,012859.000,A,3947.6540,N,10509.2019,W0.01,125.17,191021,,,D*50\r\n

### Addendum 3

A more insidius example in which the decimal point in the course on
ground was lost, "12517" instead of "125.17".

    checksum '$GPRMC,000411.000,A,3947.6531,N,10509.2017,W,0.04,12517,201021,,,D*56'
    $GPRMC,000411.000,A,3947.6531,N,10509.2017,W,0.04,12517,201021,,,D*56\r\n

### Addendum 4

I'm not sure what to make of this: this is a malformed sentence from a
BU353W10 GPS USB dongle that has a u-blox M8 chip. It reflects a dropped
decimal point in the latitude field, "39476540" instead of "3947.6540",
yet the checksum emitted by the device is correct for the malformed
sentence.

    checksum '$GPRMC,140422.000,A,39476540,N,10509.2006,W,0.00,125.17,211021,,,D*50'
    $GPRMC,140422.000,A,39476540,N,10509.2006,W,0.00,125.17,211021,,,D*50\r\n

My latest version of Hazer, which includes a lot more code to validate the
NMEA sentences (thanks to the GPS Pro+), detected this.

    2021-10-21T14:04:22.573436Z "wheatstone" <EROR> [27067] {b6f80530} app/gpstool/main.c@2418: $GPRMC,140422.000,A,39476540,N,10509.2006,W,0.00,125.17,211021,,,D*50: "Numerical result out of range" (34)

I wish I could blame my own code for this (because then I could fix it),
but I don't see how. gpstool does *not* regenerate the checksum in any way
before processing the sentence (but it does compute the checksum to verify
it).

    checksum '$GPRMC,140422.000,A,39476540,N,10509.2006,W,0.00,125.17,211021,,,D*50'
    $GPRMC,140422.000,A,39476540,N,10509.2006,W,0.00,125.17,211021,,,D*50\r\n

Compare this against the checksum of the corrected sentence.

    checksum '$GPRMC,140422.000,A,3947.6540,N,10509.2006,W,0.00,125.17,211021,,,D*50'
    $GPRMC,140422.000,A,3947.6540,N,10509.2006,W,0.00,125.17,211021,,,D*7E\r\n

