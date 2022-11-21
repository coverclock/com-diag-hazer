# SparkFun SARA-R5 Configuration for Wheatstone

> THIS IS A WORK IN PROGRESS (WIP). So far I have managed to ping
> external hosts on the internet, including my own web server.

These are my notes for using the SparkFun SARA-R5 board, which features
a U-blox SARA-R510M8S-00B module. The U-blox module incorporates both
a M8030 GNSS receiver and an LTE-M radio, and provides a fairly complex
interface accessed via AT commands. The SparkFun board comes initially
set up for one-UART operation that connects the GNSS module to the
LTE-M radio. For my purposes, I modified the board as described in
the SparkFun instructions for dual-UART operation. This required both
hardware changes (opening and closing traces) and software configuration
changes (both persistent and non-persistent).

There is some evidence that this device exhibits an issue that I have
encountered in other cellular devices: it draws more power than the
Raspberry Pi can provide over its USB A ports. This manifests in two
ways: the device cannot use enough transmit power to connect to the
cellular network; and the Raspberry Pi resets. It would probably be
wise to power the board from external power using pins it has for just
this purpose. Currently I'm using an powered USB hub between the board
and the Pi.

Note that the "Wheatstone" project designation includes work I've done
with other LTE-M radio modules, including ones not made by U-blox, and
boards not made by SparkFun.

## Links

<https://learn.sparkfun.com/tutorials/lte-gnss-breakout---sara-r5-hookup-guide/hardware-overview>

<https://www.sparkfun.com/products/18031>

<https://www.u-blox.com/en/product/sara-r5-series>

## References

SparkFun, "SparkFun LTE GNSS Breakout - SARA-R510M8S",
<https://github.com/sparkfun/SparkFun_LTE_GNSS_Breakout_SARA-R510M8S>

U-blox, "SARA-R4/SARA-R5 series - LwM2M objects and commands - Application
note", UBX-18068860-R04, 2020-02-26

U-blox, "SARA-R5 series - Application development guide - Application
note", UBX-20009652-R01, 2020-10-28

U-blox, "SARA-R5 series - Internet applications development guide -
Application note", UBX-20032566-R01, 2020-10-26

U-blox, "SARA-R5 series - LTE-M/NB-IoT modules with secure cloud -
AT commands manual", UBX-19047455-R06, 2020-09-28

U-blox, "SARA-R5 series - LTE-M/NB-IoT modules with secure cloud -
Data sheet", UBX-19016638-R07, 2020-10-02

U-blox, "SARA-R5 series - LTE-M/NB-IoT modules with secure cloud -
Product summary", UBX-18051286-R13, 2021

U-blox, "SARA-R5 series - LTE-M/NB-IoT modules with secure cloud -
System integration manual", UBX-19041356-R04, 2020-10-12

U-blox, "SARA-R5 series - Positioning implementation - Application
note", UBX-20012413-R01, 2020-10-02

U-blox, "SARA-R5 series - Positioning and timing implementation - Application
note", UBX-20012413-R03, 2021-06-28

## Hardware Configuration

The SparkFun SARA-R5 board requires opening (cutting) and closing
(soldering) traces to enable dual UART operation.  Refer to the SparkFun
Hookup Guide (link above), but here is a summary of what I did. I used
a small ceramic blade tool to do the cutting, and a Weller soldering
station to do the soldering. (Note to SparkFun: it would be great for
hobbiests if these were jumpers; users concerned about vibration could
still put a solder bridge between the pins or even wirewrap them
together.)

* Open (cut) "DSR O RTS2 O".
* Open (cut) "RI CTS2 O".
* Open (cut) "DCD RXD2 O".
* Open (cut) "DTR TXD2 I".
* Open (cut) "OUT".
* Close (solder) "CTS2 O".
* Close (solder) "RTS2 I".
* Close (solder) "RXD2 O".
* Close (solder) "TXD2 I".
* Close (solder) "IN".

/UART/0 a.k.a. UART a.k.a. UART 1 will then accept AT commands for
SW configuration.

/UART/1 a.k.a. AUX UART a.k.a. UART 2 will emit GNSS NMEA output
once the SW configuration is completed.

What the /dev devices are will depend on the order that the USB-to-serial
devices enumerate; I plug in "UART 1" first so it will be /dev/ttyUSB0,
and then "UART 2" so it will be /dev/ttyUSB1. Both devices enumerate as
v=1a86, p=7523; haven't figured out yet how to discriminate between the
two for udev hot-plug magic. Both devices appear to support autobaud;
I'm using both at 115200 BPS.

## Software Configuration

Some of these commands only need to be done once; the module will save
the settings persistently in NVRAM.

Some of these commands need to be done every time the module is rebooted.

### Scripted Configuration

This is the order that works.

    sarar5autobaud.exp
    sarar5factoryreset.exp
    # Reset device (e.g. using SparkFun's reset button)
    sarar5autobaud.exp
    sarar5setupcommon.exp
    sarar5setupgnss.exp
    sarar5setupltem.exp

### Manual Configuration

#### Enable Verbose Messages

    AT+CMEE=2

#### Set UART to 115200 baud (PERSISTENT)

    AT+IPR=115200

#### Enable Dual UART Operation (PERSISTENT)

    AT+USIO=4

### GNSS Configuration

#### Set GNSS to Profile 1: GNSS data flow to and from AUX UART (PERSISTENT)

    AT+UGPRF=1

#### Power Down GNSS

    AT+UGPS=0

#### Power Up GNSS with GPS, GLONASS, GALILEO, BEIDOU, SBAS

This needs to be done every time the module is rebooted.

    AT+UGPS=1,0,79

#### Enable Desired NMEA Sentences

    AT+UGGGA=1
    AT+UGGLL=1
    AT+UGGSA=1
    AT+UGGSV=1
    AT+UGRMC=1
    AT+UGVTG=1

It was at this point I started seeing the enabled NMEA sentences
being emitted from the AUX UART a.k.a. UART 2.

### LTE-M Configuration

I use AT&T as my LTE-M Mobile Network Operator (MNO). The Access Point
Name (APN) and other parameters are specific to the service plan I have
with AT&T. Your mileage may vary.

#### Turn Radio Off; GNSS continues to run

    AT+CFUN=0

#### Set Mobile Network Operator Profile to AT&T (PERSISTENT)

    AT+UMNOPROF=2

#### Reboot Module

    AT+CFUN=16t status

#### Set Packet Data Protocol Context, Protocol, and Access Point Name

    AT+CGDCONT=1,"IPV4V6","m2m64.com.attz"

#### Set Packet Switched Data Profile 1: Protocol, APN, DNS1, DNS2, CID

    AT+UPSD=1,0,2
    AT+UPSD=1,1,"m2m64.com.attz"
    AT+UPSD=1,4,"8.8.8.8"
    AT+UPSD=1,5,"8.8.4.4"
    AT+UPSD=1,100,1

##### Turn Radio On

    AT+CFUN=1

#### Activate Packet Switched Data Profile 1

    AT+UPSDA=1,3

#### Enable MNO Registration

    AT+CREG=1

### Other Useful Stuff

#### Factory Reset

    AT+UFACTORY=0.0

#### Extract IMEI

    AT+CGSN=255

#### Get MNO Registration Status

    AT+CREG?

#### Get Connection Status

    AT+COPS?

#### Ping

    AT+UPING="www.google.com"

### Issues

I have gotten the host computer into a state in which the ttyUSB serial
devices no longer enumerate until I reboot the host. This is one of the
advantages of using an SBC like a Raspberry Pi instead of your desktop
or laptop.

The enumeration of the ttyUSB serial devices depends on the order in
whcih you plug them in. They appear to enumeration with the same
vendor and device identifiers, so I have not found an easy with which
to descriminate between them.
