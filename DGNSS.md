# Differential GNSS Using Tumbleweed

Tumbleweed uses three Raspberry Pi 3B+ SBCs and two Ardusimple SimpleRTK2B
boards, each equipped with a U-blox UBX-ZED-F9P receiver chip and a multi-band
GPS active antenna, to implement a Differential GNSS system which is (at least
theoretically) capable of centimeter-level accuracy. One of the Pis is a
stationary base station running its ZED-F9P in "survey-in" mode, one of the Pis
is a mobile rover whose ZED-F9P receives RTK updates from the base station, and
one of the Pis is a router that receives RTK updates from one base station
and forwards them to one or more rovers. (You can combine the base and the
router on to one Pi, but I chose not to configure my set up that way.)

N.B. The Diminuto logging system used by Hazer writes log messages
to standard error if the caller is an interactive process, and to the
system log if the caller is a daemon. Running some of these scripts in
the background and then logging off to let them continue to run causes
the logging system to perceive that they have transitioned from being
interactive to being a daemon (and this isn't a mistake). Consequently,
subsequent log messages will go to the system log and not the error
log file. This can seem a little mysterious.

(In the following examples, I use a script called "peruse", which in turn
uses a Diminuto script called "observe". I also make use of the SIGHUP
capability of gpstool using a script called "hups" which sends a HUP
(hangup) signal to any running instance of gpstool everytime you hit
the RETURN (ENTER) key. You can read more about these scripts in the
section on headless operation below.)

## Router

The Tumbleweed router, which is on my LAN, must have a static IP address
or a usable Dynamic DNS (DDNS) address (which is what I do) that can be
reached through the firewall. ":tumbleweed" identifies the service on the
localhost defined in /etc/services from which to receive and send RTK update
datagrams. (My Tumbleweed router is code-named "eljefe". You may come across
this and related names in logs, screen shots, etc.)

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    router :tumbleweed &
    peruse router err

## Base

The Tumbleweed base is typically on my LAN, but can be on the WAN by
changing the hostname through which the router is addressed. "tumbleweed:"
idenfities the hostname of the router on the LAN as defined in /etc/hosts,
and ":tumbleweed" the service on the router on the LAN defined as in
/etc/services (typically I defined this to be port 21010) to which to
send RTK update datagrams. (I have used two different Tumbleweed bases;
the portable version I use with a tripod-mounted anntenna is code-named
"bodega" and the one whose antenna is permanently fixed is "hacienda".)

In one window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    survey tumbleweed:tumbleweed /dev/tumbleweed & peruse survey err

In another window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    peruse survey out

Depending on the specified accuracy - encoded in a message sent to the
chip by the script - it can take days for the receiver to arrive at a
solution that has the required radius of error. This depends greatly on
antenna placement as well as other factors that may be less under your
control. Putting the antenna in my front yard, which has the usual ground
clutter of trees and adjacent houses, resulted in taking about two days
to get to a resolution of ten centimeters, about four inches. Using a
permanently attached antenna near the peak of the roof of my house took
a little over twelve hours to get the same resolution.

Because of this potentially lengthy duration of the survey, you don't want
to do it more than once. First rule is: don't move the antenna. (If you
do, you'll have to do another survey, no matter what.) Second rule is:
don't restart the receiver.

N.B.: Experience in testing the F9P receiver suggests that survey mode has
a maximum duration of thirty (30) days. At thirty days plus one second
(2592001 seconds total), the error distance calculated by the receiver
ceases to become any smaller and the elapsed time of the survey ceases to
become any larger. The gpstool utility reports both of these values in
the BAS output line. The thirty day limit is probably documented somewhere,
and may even be configurable, but if so I haven't found it. See [u-blox 9
Integration, 3.1.5.5.1, "Survey-in"] for details about survey mode.

Fortunately, once a survey is successfully completed, the base station
script saves the pertinent results from a successful survey in a file
whose names ends in ".fix".  This is a human-readable ASCII file that
stores the values in a form that can later be imported into another
script that runs the receiver in fixed mode, in which the receiver
is told what its location is, and so immediately begins transmitting
corrections based on this information.

In one window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    fixed tumbleweed:tumbleweed /dev/tumbleweed & peruse fixed err

In another window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    peruse fixed out

The choice between running the base in survey mode or in fixed mode
is automated in a script that does latter if the base.fix file is
present and seems sane, and the former if it is not. This allows you to
restart the base station and have it do the right thing depending on
whether or not the survey had been previously completed.

In one window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    base tumbleweed:tumbleweed /dev/tumbleweed & peruse base err

In another window:

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    peruse base out

The .fix file contains the following UBX variables in character hex format
in this order.

    FIXED_POS_ACC
    LAT
    LATHP
    LON
    LONHP
    HEIGHT
    HEIGHTHP

Here is an example of an actual base.fix file that resulted from a
survey done with 10 cm (1000 x 0.1mm) accuracy; note that UBX stores
its variables in little-endian byte order.

    \xe8\x03\x00\x00
    \xb1\x1f\xb8\x17
    \x20
    \x91\xdc\x52\xc1
    \xe5
    \x01\x94\x02\x00
    \x1d

### Geocodes

Here are some geocodes - different ways of encoding and representing
the location of the base I am using.

* Decimal degrees: 39.794275645, -105.153414843
* Plus Codes: 85FPQRVW+PJ
* What3Words: ///beard.hobby.funds
* Degrees Lat Long:  39.7942756°, -105.1534148°
* Degrees Minutes: 39°47.65654', -105°09.20489'
* Degrees Minutes Seconds:  39°47'39.3923", -105°09'12.2934"
* UTM: 13S 486865mE 4404935mN
* UTM centimeter: 13S 486865.33mE 4404935.48mN
* MGRS: 13SDE8686504935
* Grid North: -0.1°
* GARS: 150LV28
* Maidenhead: DM79KT10OP10
* GEOREF: EJQK50794765

## Rover

A Tumbleweed rover (there can be more than one) is typically on the WAN,
and is agnostic as to the Internet connection (I use a USB LTE modem).
"tumbleweed.test:" stands in for the fully qualified domain name
(FQDN) on the WAN as (for me) defined by DDNS and resolved via DNS,
and ":tumbleweed" the service on the router to which send keep alive
datagrams and receive RTK update datagrams as defined in /etc/services.
(My rover is code-named "mochila".)

I use just one window in these examples, but you can use two like
I did for survey etc. above to monitor the error and output streams
in parallel.

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    rover tumbleweed.test:tumbleweed /dev/tumblweed &
    peruse rover err# Control-C to exit upon seeing "Ready".
    peruse rover out

The rover can also generate a CSV file as it runs that will contain the
results of the PVT solution every time is is generated and reported. This
CSV file can be imported into a spreadsheet like Excel, converted into a
stream of NMEA RMC sentences using the csv2rmc script for use with other
GPS tools (including gpstool itself), or converted into a KML (Keyhole
Markup Language) XML file using the csv2kml script for use with geodesy
tools like Google Earth.

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    benchmark tumbleweed.test:tumbleweed /dev/tumbleweed &
    peruse benchmark err# Control-C to exit upon seeing "Ready".
    tail -f out/host/tmp/benchmark.csv | csv2rmc | gpstool -R

The rover can also be run as an uncorrected mobile unit, basically a
receiver that doesn't support Differential GNSS. (It might be a good
idea to power cycle the F9P before changing from rover to mobile; some
of the rover configuration seems to be sticky.)

    cd ~/src/com-diag-hazer/Hazer
    . out/host/bin/setup
    # Power cycle the F9P if previously configured for corrections.
    mobile /dev/tumbleweed &
    peruse mobile err# Control-C to exit upon seeing "Ready".
    peruse mobile out

Once I'm confident that everything works and I don't need to check the
error log (you can always look at it later), I use this shorthand to
fire up the rover. (Something similar works for the base, router, and
mobile scripts too.)

    cd ~/src/*hazer/Hazer
    . out/host/bin/setup
    rover tumbleweed.test:tumbleweed & sleep 5 ; peruse rover out

In the field, I commonly run the rover using the benchmark script,
which is a rover that saves each UBX precision fix by appending the
values of the solution to a Comma Separated Value (CSV) file which can
be post-processed in any number of ways for analysis. I combined the use
of the benchmark script with the peruse script (to full-screen display
the results in real-time) with the hups script (to save the most recent
full-screen display to a time-stamped file) in a script named "field".
This vastly simplifies field testing (which as I like to point out, is
often literally in a field somewhere) and reduces the number of hands
and fingers you need to do this kind of testing.

    cd ~/src/*hazer/Hazer
    . out/host/bin/setup
    field tumbleweed.test
    # Hit the return key to save the most recent display.

## Headless

I'm running all three, router, base, and rover, as simple background
processes.  But it is also possible to run them in daemon mode, in
which case messages normally written to standard error are logged to
the system log, which on the Pi is typically in /var/log/syslog. (This
also happens when you run the scripts in the background and then later
logout. This can be a little confusing since the log will transition
from going into the error log file to going into the system log.)

Also, I run all three in "headless" mode, where the screens normally
written to standard output are instead written to a file, and a script
is used to display the file as it changes; this decouples the router,
rover, and base software from the display terminal while still allowing
an on-demand real-time display. (This requires that the inotify-tools
package be installed on the Pi.)

Both gpstool (which implements the base and the rover) and rtktool (which
implements the router) can be run as daemons via a command line switch
(although I have not done so in these examples); the headless mode can
still be used.

When running in headless mode and receving a hangup signal (SIGHUP),
gpstool will checkpoint the headless file: it will create a hardlink to
the current headless output file with a timestamp prefix with a resolution
in microseconds. This is particularly useful in field testing to capture
the relevant data at a specific point in time and space. The hup and hups
scripts use the pkill command to send a SIGHUP to all gpstool instances.

## Networking

Note that the actual IP address of neither the base nor the rover need
be known. This is important because the rover (and sometimes the base)
is on the a WAN, in my set up via an LTE modem to a mobile service
provider. (Despite my best udev fu, these modems, working in end-user
mode, worked like crap until I upgraded to Raspbian 10, at which point
they worked flawlessly.) In this case, not only can the IP address on the
WAN not be known ahead of time until the modem connects to the carrier
network, over the span of a long base survey which can take many many
hours depending on the desired level of precision (configured via a UBX
message defined in the base script) and the quality of the view of the
sky afforded to the base, the IP address provided by the WAN via DHCP, as
well as the port number, may change. In my tests, with the base antenna
set up in my front yard, an accuracy of ten centimeters (about four
inches) requires about sixteen hours of "survey-in" time (your mileage
may vary). During this time the mobile network can disconnect from the
modem and reconnect. The base and Tumbleweed router routinely deal with
this; but it means the IP address you begin with in your survey will
not be the one you end up with when the survey is complete or as the
base sends subsequent updates to the rover.

The Tumbleweed router does require a fixed IP address for this to
work. In my setup, the Tumbleweed router connects to my home IP router
(in my case, directly via wired Ethernet). But my IP router gets its
globally routable WAN IP address from my internet provider via DHCP. My
IP router supports Dynamic DNS (DDNS), in which it automatically sends
an notification to my DDNS provider regarding the assigned IP address,
who then changes the DNS database to map a fixed FQDN to the provided
address. This FQDN is represented by "tumbleweed.test" above. I have
configured the firewall in my IP router to forward the port named
"tumbleweed" above to and from the same port on the static IP address
on my LAN that I assigned to the Tumbleweed router.

As mentioned above, it is not unusual to see a WAN connected rover or base
drop off and reappear on the WAN, often with a different port number and
IP address. The router makes a note of this; here are the results of an
overnight test. The IPv4 addresses are expressed in IPv6 notation, which is
what the router uses natively. Also, I've obfuscated portions of the WAN
addresses. The base and rover can use either IPv4 or IPv6. You can see the
base and rover changing ports and IP addresses, and even sometimes switching
between the WAN and the LAN when they have access to both (the latter via my
WiFI WLAN).

    2019-06-25T23:29:47.791661Z <INFO> [744] {76f6b530} Begin
    2019-06-25T23:29:47.792252Z <INFO> [744] {76f6b530} Router (3) ":tumbleweed" [::]:21010
    2019-06-25T23:29:47.792322Z <INFO> [744] {76f6b530} Start
    2019-06-25T23:30:52.219618Z <NOTE> [744] {76f6b530} Client New base [::ffff:XXX.YYY.0.61]:6571
    2019-06-25T23:30:52.219789Z <NOTE> [744] {76f6b530} Client Set base [::ffff:XXX.YYY.0.61]:6571
    2019-06-25T23:36:28.213760Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T00:33:19.063809Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T00:33:28.416375Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T00:58:59.064859Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T00:59:08.301363Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T02:38:48.012487Z <NOTE> [744] {76f6b530} Client Old base [::ffff:XXX.YYY.0.61]:6571
    2019-06-26T02:38:48.013164Z <NOTE> [744] {76f6b530} Client New base [::ffff:192.168.1.188]:43289
    2019-06-26T02:38:48.013336Z <NOTE> [744] {76f6b530} Client Set base [::ffff:192.168.1.188]:43289
    2019-06-26T02:39:28.253289Z <NOTE> [744] {76f6b530} Client Old base [::ffff:192.168.1.188]:43289
    2019-06-26T02:39:29.070321Z <NOTE> [744] {76f6b530} Client New base [::ffff:XXX.YYY.1.209]:8231
    2019-06-26T02:39:29.070540Z <NOTE> [744] {76f6b530} Client Set base [::ffff:XXX.YYY.1.209]:8231
    2019-06-26T03:05:28.058945Z <NOTE> [744] {76f6b530} Client New rover [::ffff:192.168.1.1]:42804
    2019-06-26T03:05:39.069692Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.33.234]:2645
    2019-06-26T03:05:48.256039Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.9.215]:4224
    2019-06-26T03:05:59.069366Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:192.168.1.1]:42804
    2019-06-26T06:09:28.058577Z <NOTE> [744] {76f6b530} Client New rover [::ffff:192.168.1.1]:42804
    2019-06-26T06:09:39.065573Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.9.215]:4224
    2019-06-26T06:09:48.108028Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.16.196]:7052
    2019-06-26T06:09:59.065301Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:192.168.1.1]:42804
    2019-06-26T06:10:08.058974Z <NOTE> [744] {76f6b530} Client New rover [::ffff:192.168.1.182]:42804
    2019-06-26T06:10:19.066025Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.16.196]:7052
    2019-06-26T06:10:28.266752Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.16.196]:7040
    2019-06-26T06:10:39.065709Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:192.168.1.182]:42804
    2019-06-26T06:43:20.066215Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.16.196]:7040
    2019-06-26T06:43:48.268595Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.16.196]:7040
    2019-06-26T09:13:20.020202Z <NOTE> [744] {76f6b530} Client Old base [::ffff:XXX.YYY.1.209]:8231
    2019-06-26T09:13:20.220657Z <NOTE> [744] {76f6b530} Client New base [::ffff:XXX.YYY.13.157]:5803
    2019-06-26T09:13:20.220928Z <NOTE> [744] {76f6b530} Client Set base [::ffff:XXX.YYY.13.157]:5803
    2019-06-26T10:58:19.069629Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:XXX.YYY.16.196]:7040
    2019-06-26T10:58:28.052347Z <NOTE> [744] {76f6b530} Client New rover [::ffff:192.168.1.1]:42804
    2019-06-26T10:59:08.257400Z <NOTE> [744] {76f6b530} Client New rover [::ffff:XXX.YYY.17.45]:4970
    2019-06-26T10:59:18.069004Z <NOTE> [744] {76f6b530} Client Old rover [::ffff:192.168.1.1]:42804
    2019-06-26T14:01:28.248198Z <NOTE> [744] {76f6b530} Client Old base [::ffff:XXX.YYY.13.157]:5803
    2019-06-26T14:01:29.059184Z <NOTE> [744] {76f6b530} Client New base [::ffff:XXX.YYY.27.107]:4679
    2019-06-26T14:01:29.059431Z <NOTE> [744] {76f6b530} Client Set base [::ffff:XXX.YYY.27.107]:4679
    2019-06-26T14:02:08.263694Z <NOTE> [744] {76f6b530} Client Old base [::ffff:XXX.YYY.27.107]:4679
    2019-06-26T14:02:09.063249Z <NOTE> [744] {76f6b530} Client New base [::ffff:XXX.YYY.27.107]:4686
    2019-06-26T14:02:09.063542Z <NOTE> [744] {76f6b530} Client Set base [::ffff:XXX.YYY.27.107]:4686

When a client of the router changes its port or IP address, it appears to be
a new rover or base. In the case of the rover, it is immediately registered
with the router, which begins feeding it RTK updates from the current base.
The entry for the old rover times out when no more keep alives are received,
and it is removed from the list of active rovers on the router. In the case of
the base, the new base is rejected until similarly the old base times out and
is removed when no more updates are received, at which point the new base is
registered on the router and its updates are used to feed the rovers.

All internet communication is via UDP datagram. Each datagram contains one
and only one fully validated RTCM message complete with checksum and preceeded
by a four-byte sequence number. UDP datagrams received out of sequence are
dropped by the router or the rover.

The internet route to the rover for RTK updates is determined by the address
and port from which a keep alive is received by the router from the rover.
The keep alive message is an RTCM message with a zero-length payload. A keep
alive message is sent by each rover to the router once every twenty-five
seconds. Thirty seconds is a typical timeout after which a firewall or router
will remove the UDP return route. (This mechanism was inspired by a similar one
used by SIP to route RTP packets via UDP to VoIP phones.)

## Hardware

Although Tumbleweed has been implemented using the Ardusimple SimpleRTK2B
board, the same software runs on the SparkFun GPS-RTK2 board which uses the
same U-blox UBX-ZED-F9P receiver chip. Ublox has since introduced their own
F9P application board, the C099-F9P, but I haven't tried it. Since I ended
up implementing the inter-board communication channel on the Raspberry Pi,
I don't need the support for various radio technologies that both the
Ardusimple and the Ublox boards provide.

## Issues

Note that the UDP data stream is not encrypted, nor is the source
of the datagrams authenticated, so this mechanism is not secure.
It should be. I'm pondering how best to accomplish that.

Datagram Transport Layer Security (DTLS) provides encryption and
authentication for UDP protocols by adapting TLS (SSL) to datagram
semantics, specifically dealing with lost or reordered packets.
Unfortunately in doing so, DTLS eliminates the very advantages that
caused me to choose UDP over TCP.

Most Linux distros will require that you either be in the "dialout"
group (preferred), or run as root, in order to access serial devices
like GPS receivers.

Also read the Issues section in the Diminuto README.
