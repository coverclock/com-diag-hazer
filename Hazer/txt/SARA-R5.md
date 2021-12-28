# SparkFun SARA-R5 Configuration for Wheatstone

These are my notes for using the SparkFun SARA-R5 board, which features
a U-Blox SARA-R5 module. The U-Blox module incorporates both a GNSS
receiver, an LTE-M radio, and a fairly complex interface accessed via
AT commands. The SparkFun board comes initially set up for one-UART
operation that connects the GNSS module to the LTE-M radio. For my
purposes, I had to modify the board (as directed by the SparkFun
instructions) for dual-UART operation.

## References

<https://learn.sparkfun.com/tutorials/lte-gnss-breakout---sara-r5-hookup-guide/hardware-overview>

U-Blox, "SARA-R5 series - LTE-M/NB-IoT modules with secure cloud - AT
commands manual", UBX-19047455-R06, 2020-09-28

U-Blox, "SARA-R5 series - Internet applications development guide -
Application note", UBX-20032566-R01, 2020-10-26

## Hardware Configuration

The Sparkfun SARA-R5 board requires opening (cutting) and closing
(soldering) traces to enable dual UART operation.  Refer to the SparkFun
Hookup Guide (URL above), but here is a summary of what I did. I used
a small ceramic blade tool to do the cutting, and a Weller soldering
station to do the soldering.

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
v=1a86, p=7523.

## Software Configuration

Some of these commands only need to be done once; the module will save
the settings in NVRAM.

Some of these commands need to be done everytime the module is rebooted.

### General Configuration

#### Reboot Module

    AT+CFUN=16

#### Set UART to 115200 baud

    AT+IPR=115200

#### Enable Dual UART Operation

    AT+USIO=4

#### Enable GPIO 1 (a.k.a. 16) for Network Status Indication

    AT+UGPIOC=16,2

#### Enable GPIO 6 (a.k.a. 19) for Time Pulse Output

    AT+UGPIOC=19,22

### GNSS Configuration

#### Set GNSS to Profile 1 (GNSS data flow to and from AUX UART)

    AT+UGPRF=1

#### Set Time Pulse to 1PPS using GNSS/LTE (best effort)

    AT+UTIME=1,1

#### Power Down GNSS

    AT+UGPS=0

#### Power Up GNSS with GPS, GLONASS, GALILEO, SBAS

This needs to be done everytime the module is rebooted.

    AT+UGPS=1,0,71

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

#### Turn Radio Off (GNSS continues to run)

    AT+CFUN=0

#### Set Mobile Network Operator Profile

Mine is AT&T. Your mileage may vary.

    AT+UMNOPROF=2

#### Reboot Module

    AT+CFUN=16

#### Turn Radio Off (again)

    AT+CFUN=0

#### Set Packet Data Protocol Context and Access Point Name

    AT+CGDCONT=0,"IP","<MNO APN name>"

#### Turn Radio Back On

    AT+CFUN=1

#### Set Packet Switched Data Configuration: Protocol, APN, DNS1, DNS2, CID

    AT+UPSD=0,0,0
    AT+UPSD=0,1,"m2m.com.attz"
    AT+UPSD=0,4,"8.8.8.8"
    AT+UPSD=0,5,"8.8.4.4"
    AT+UPSD=0,100,0

#### Activate Packet Switched Data Configuration

    AT+UPSDA=0,3

#### Get Connection Status

    AT+COPS?

