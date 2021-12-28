# U-Blox SARA-R5 Configuration for Wheatstone

## References

<https://learn.sparkfun.com/tutorials/lte-gnss-breakout---sara-r5-hookup-guide/hardware-overview>

## Hardware Configuration

The Sparkfun SARA-R5 board requires opening (cutting) and
closing (soldering) traces to enable dual UART operation.
Refer to the SparkFun Hookup Guide (URL above), but here
is a summary of what I did. I used a small ceramic blade
tool to do the cutting, and a Weller soldering station to
do the soldering.

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

/UART/0 a.k.a. "UART" a.k.a. "UART 1" will then accept AT
commands for SW configuration.

/UART/1 a.k.a. "AUX UART" a.k.a. "UART 2" will emit GNSS
NMEA output once the SW configuration is completed.

What the /dev devices are will depend on the order that the
USB-to-serial devices enumerate; I plug in "UART 1" first
so it will be /dev/ttyUSB0, and then "UART 2" so it will
be /dev/ttyUSB1. Both devices enumerate as v=1a86, p=7523.

## Software Configuration

Some of these commands only need to be done once; the
module will save the settings in NVRAM.

Some of these commands need to be done everytime the
module is rebooted.

### General Configuration

#### Reset Module

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

    AT+UGPS=1,0,71

#### Enable Desired NMEA Sentences

    AT+UGGGA=1
    AT+UGGLL=1
    AT+UGGSA=1
    AT+UGGSV=1
    AT+UGRMC=1
    AT+UGVTG=1
    AT+UGZDA=1

### LTE-M Configuration
