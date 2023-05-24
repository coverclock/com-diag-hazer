# Display

The display examples below were cut and pasted from actual running
instances of gpstool, but not necessarily the same instance, running
on the same target with the same GNSS hardware, with the same command
line options, using the same version of gpstool. The gpstool display
is event driven: each line is displayed if and only if the appropriate
NMEA, PUBX, UBX, or RTCM input was received from the device under test,
within the timeout window, regardless of command line options or how
the GNSS device was initialized. This means for those lines containing
data that the device is slow to update (the update interval is greater
than the timeout interval), the lines may come and go.

While NMEA (and UBX amd RTCM too for that matter) is good about updating
the application with new information, it is not so good about letting the
application know when that data is no longer relevant. For that reason,
all of the data read from the device has associated with it an expiration
time in seconds. This can be set from the command line, in the range 0
to the default of 255. If the data is not updated within that duration by
new sentences or messages from the GPS device, it is no longer displayed.

    INP [ 14] \xd3\0\bL\xe0\0\x8a\0\0\0\0\xa8\xf7*

INPut is the most recent data read from the device, either NMEA
sentences or UBX packets, with binary data converted into standard C
escape sequences.  The number in the square brackets is the length of
the input data.

    OUT [  7] \xb5b\x06>\0\0\0

OUTput is the most recent data written to the device, as specified on the
command line using the -W (NMEA sentence) or -U (UBX message)  options,
minus the end matter like a checksum etc., which is added independently
of when this line was emitted. The number in the square brackets is the
length of the output data minus the end matter.

    LOC 2021-08-28T15:13:50.053-07:00+01 00:00:08.299 44.0.0      3002494  cadmium

LOCal is the current local time provided by the host system, the elapsed
time to first fix, the software release number, the process id, and the
local host name.  The local time, with a fractional part in milliseconds,
includes the time zone offset from UTC in hours and minutes, and the
current daylight saving time (DST) offset in hours. The next field is the
elapsed time since the application began running. The following fields
are the Hazer release string, the Process IDentifier of the application,
and the first eight characters of the name of the host system.

    TIM 2021-08-28T21:13:50.000-00:00+00 00:00:00.297 pps                  GNSS

TIMe is the most recent time solution in UTC, the elapsed time of the
initial position fix (or dashes if no position fix has occurred yet, or
asterisks if it is more than a day), and the current value of the One
Pulse Per Second (1PPS) signal ("PPS" if true, "pps" if false) if the
device provides it and it was enabled on the command line using -c (using
data carrier detect or DCD) or -I (using general purpose input/output
or GPIO).

    POS 39°47'39.258"N, 105°09'12.172"W    39.7942383, -105.1533813 Aq Ns  GNSS

POSition is the most recent position solution, latitude and longitude,
in degrees, hours, minutes, and decimal seconds, and in decimal
degrees. Either format can be cut and pasted directly into Google Maps,
and the latter into Google Earth.  The underlying position data is stored
as binary integers in billionths of a minute (nanominutes). But the
device under test may not provide that much accuracy; the actual number of
significant digits for various data is reported by the INT line (below),
but even this may be optimistic - or, for that matter, pessimistic - when
compared to what the device is capable of. In particular, technologies
like Wide Area Augmentation System (WAAS), multi-band GNSS, differential
GPS, Real-Time Kinematics (RTK), and long-term surveying, can potentially
achieve remarkable accuracy.

The next field indicates the quality of the fix, and may be set by
the NMEA GGA, GLL, or RMC sentences, or the U-blox proprietary PUBX,00
sentence:

* N - No fix;
* A - Autonomous e.g. Standard Positioning Service (SPS);
* D - Differential including SBAS and WAAS as well as DGNSS;
* X - Precise Positioning Service (PPS) - obsolete (selective availability);
* R - Real Time Kinematics (RTK) using integers;
* F - Real Time Kinematics (RTK) using floats;
* E - Estimated e.g. Dead Reckoning (Inertial Measurement Unit);
* M - Manual;
* S - Simulator;
* P - Precise Point Positioning (PPP);
* \- - Unset (no sentence with a quality metric received or field not present);
* ? - Invalid.

The last field is a navigation safety indicator, provided by the NMEA RMC
sentence, and gives a clue as to the confidence in and timeliness of the fix:

* S - Safe;
* C - Caution;
* U - Unsafe;
* V - Invalid or no safety indication reported.
* \- - Unset (no sentence with a safety metric received or field not present);
* ? - Invalid.

(This line intentionally not left blank.)

    ALT    5608.53'   1709.500m MSL    5537.99'   1688.000m GEO            GNSS

ALTitude is the most recent altitude solution, in feet and meters, both above
Mean Sea Level (MSL), and above the selected GEOdetic datum (typically
WGS84). (Similar comments here regarding precision as those for POS.)

    COG N     0.000000000°T    0.000000000°M                               GNSS

Course Over Ground is the most recent course solution, in cardinal compass
direction, and the bearing in degrees true, and degrees magnetic (if
available).  (Similar comments here regarding precision as those for POS.)

    SOG       0.012mph       0.010knots       0.018kph       0.005m/s      GNSS

Speed Over Ground is the most recent speed solution, in miles per hour,
knots (nautical miles per hour), kilometers per hour, and maters per
second. (Similar comments here regarding precision as those for POS.)

    INT ZDA [12] DMY TOT (  9 10  5  3  0  0  4  4 )             39526129B GNSS

INTernal is some clues about the Hazer state, including the name of
the sentence (GLL in the example above) that most recently updated
the solution, the total number of satellites that contributed to that
solution, an indication as to whether the day-month-year value has been
set (only occurs once the RMC sentence has been received), an indication
as to whether time is incrementing monotonically (it can appear to run
backwards when receiving UDP packets because UDP may reorder them),
and some metrics as to the number of significant digits provided for
various values provided by the device.  INT also includes the total
number of bytes sent or received - 395,261,29B in this example - over
the network. This allows you to keep track of your network utilization,
especially important when paying for data on your LTE mobile provider. The
right-most field is the name of the device from which gpstool is reading.

    MON -jamming  +history  50indicator  63maximum                         ttyACM1

MONitor displays some of the results received in the UBX-MON-HW message if
enabled.  u-blox 8 chips with firmware revision 18 and above can provide
clues to jamming based on the received signal strength. (N.B. I don't
have a way to test this.)  This requires that the jamming/interference
monitor (ITFM) be calibrated using the UBX-CFG-ITFM message.

    STA -spoofing -history       1985ms     303985ms     0epoch            ttyACM1

STAtus displays some of the results received in the UBX-NAV-STATUS message
if enabled. u-blox 8 chips with firmware revision 18 and above can provide
clues to spoofing based on comparing navigation solutions from multiple
GNSSes if available. (N.B. I don't have a way to test this.) Also shown
are the milliseconds since first fix and milliseconds uptime provided
by the message.

    ATT    0.0° roll ±  20.0°  -73.6° pitch ±  78.8°   57.7° yaw ±  85.0°  IMU

ATTitude indicates the roll, pitch, and yaw orientation of the u-blox
module in those units which are equipped with an intertial measurement
unit with a gyroscope and accellerometers.

    ODO      7.671mi     12.345km (     42.185mi     67.890km ) ±       0m IMU

ODOmeter indicates the resettable and semi-persistent odometer reading
available from some u-blox modules, in both miles and kilometers (the
native units are meters), along with the error estimate in meters.

    NED         -5mm/s north         -2mm/s east        -41mm/s down (3)   IMU

North-East-Down indicates the orientation of the vehicle frame reading
available from the Intertial Measurement Unit (IMU) in some u-blox
modules. The character in the parenthesis indicates the nature of the
ensemble GNSS and IMU fix: '-' for no fix; '!' for a dead reckoning fix
only; '2' for a 2D GNSS fix; '3' for a 3D GNSS fix; '+' for a combined
GNSS + dead reckoning fix; '\*' for a time only fix; and '?' for an error.

    HPP   39.794267897, -105.153420946 ±     0.5237m                       GNSS

High Precision Position shows the position available from some u-blox
devices, along with its estimated accuracy. This may differ (slightly)
from the position reported via POS and ALT due to the conversion of
units done to conform to the NMEA format. When available, the HPP data
is expected to be more precise.

    HPA   1709.4855m MSL   1687.9856m WGS84 ±     0.8001m                  GNSS

High Precision Altitude shows the altitude available from some u-blox
devices, along with its estimated accuracy. This may differ (slightly)
from the altitude reported via ALT due to the conversion of units done
to conform to the NMEA format. When available, the HPA data is expected
to be more precise.

    NGS  39 47 39.36442(N) 105 09 12.31540(W)                              GNSS

National Geodetic Survey shows the same high precision position as HPP
but in the format used in the NGS data sheets for coordinates of artifacts
such as NGS and municipal survey markers. This makes it easier to compare
the Hazer position against examples from the NGS database.

    BAS 1active 0valid      18019sec      18020obs       0.1675m           DGNSS

BASe shows information about the u-blox device operating in base
station mode. In base (stationary) mode, it shows if the device is
actively surveying or if the survey has resolved to a valid location,
how many seconds and observations have been consumed during the survey,
and what the mean error is.

    ROV     0:  1094 (    0)                                               DGNSS

ROVer shows information about the u-blox device operating in rover modes.
In rover (mobile) mode, it shows what RTCM message was last received
and from whom.

    RTK 1094 [ 129] rover    <ERNrCERN>                                    DGNSS

Real-Time Kinematics show the latest RTCM message received, when operating
in base mode (in which case the message was read from the device), or
in rover mode (in which case the message was received from the base in
a datagram via UDP and written to the device).  The lengths of the most
recent message is shown, as is the mode of the system, base or rover. The
character sequence between the angle brackets records the last eight RTCM
messages that were received, the newest one indicated by the rightmost
character in the sequence, as the sequence is progressively shifted left
as new messages are received.

    ACT [1] {    25    20    23    15     5    29 } [ 6] [ 7] [19] [21] 3D NAVSTAR

ACTive is the list of active satellites, typically provided seperately
for each system or constellation by the device, showing each satellites
identifying number (for GPS, this is its pseudo-random noise or PRN code
number, but other systems using other conventions), and the number of
active satellites in this list, the number of active satellites for this
constellation, and the number of active satellites total.  Unlike the
other report lines, the system or constellation to which the data applies
is derived from (in order, depending on availability) the system id in the
GSA sentence (only available on devices that support later NMEA versions),
or an analysis of the Space Vehicle Identifier based on NMEA conventions,
or the talker specified at the beginning of the sentence. The reason
for this is that some devices (I'm looking at you, GN803G), specify GNSS
as the talker for all GSA sentences when they are computing an ensemble
solution (one based on multiple constellations); this causes ambiguity
between this case and the case of successive GSA sentences in which the
active satellite list has changed. Hazer independently tries to determine
the constellation to which the GSA sentence refers when the talker is
GNSS.  The fourth metric is the maximum number of space vehicles (SVs)
or satellites used in the solution since the application began. This
is useful when testing different antenna locations, particularly when
using the device in a fixed base survey mode. In the example above,
the first ACT line indicates that 6 SVs in the NAVSTAR constellation
are represented in this particular line, the solution includes 10 total
NAVSTAR SVs (so 4 more appear in a subsequent ACT line for NAVSTAR),
the solution is using 29 SVs total among all constellations, and at one
time as many as 31 SVs were used in the solution. (The U-blox 9 receiver
used for this example has a maximum of 32 RF channels, so receiving 31
SVs indicates that antenna placement is good; in this particular case
I have the antenna installed in a skylight in my kitchen that is near
the peak of the roof of my home.)

The next field indicates the type of fix, as provided by the NMEA GSA
or the U-blox proprietary PUBX,00 sentences:

* NF - No fix;
* 2D - 2-dimensional autonomous fix (no altitude);
* 3D - 3-dimensional autonomous fix;
* RK - Combined satellite and dead reckoning fix;
* D2 - 2-dimensional differential fix (no altitude):
* D3 - 3-dimensional differential fix;
* TT - time solution only;
* DR - Dead Reckoning a.k.a. Estimate;
* NS - No Satellites;
* -- - Unset (no sentence with a fix type received or field not present);
* ?? - Invalid.

(This line intentionally not left blank.)

    DOP   1.09pdop   0.61hdop   0.90vdop                                   NAVSTAR

Dilution Of Precision is the position, horizontal, and vertical DOP -
measures of the quality of the position fix (smaller is better) - based on
the real-time geometry of the satellites upon which the current solution
is based. If multiple constellations are reported, but the DOPs are all
the same, the device is typically computing an ensemble solution using
multiple constellations.

    SAT [  6]     1id  24°elv   44°azm   19dBHz  L1_C/A   G <              NAVSTAR

SATellite, generated from the NMEA GSV sentence or the U-blox PUBX SVSTATUS
sentence, is the list of satellites, or Space Vehicles (SV), in view. It
includes an index that is purely an artifact of Hazer, the Space Vehicle
IDentifier (specific to the constellation); its elevation and azimuth
in degrees; its the signal strength (really, a carrier to noise density
ratio) in deciBels Hertz; the name of the signal band (according to
NMEA 0183) if specified ("ANY" if it is not, and spaces in the name
are replaced with an underscore); and zero or more flags. A flag of
'G' indicates that this SV was reported by the GSV sentence, while a 'P'
indicates the PUBX sentence; a flag of '<' indicates that the satellite is
on the active list (see ACT above, provided by the NMEA GSA sentence or
by the U-blox PUBX SVSTATUS sentence); a '?' indicates that the azimuth
and/or the elevation were empty (but displays as zero), and typically
means that the almanac has not yet been received (a cold start), but
occasionally indicates that the satellite is not in the received almanac,
which I've seen when GPS ground control is testing a new satellite;
a '!' indicates that the signal strength was empty (but displays as
zero), where some receivers use this to indicate the satellite is in the
almanac but is not being received over an available RF channel); a '-'
indicates that a PUBX SVSTATUS message has indicated that the satellite
was excluded in the solution (this is apparently different from having
the satellite's ephemeris but not using it in the solution).
