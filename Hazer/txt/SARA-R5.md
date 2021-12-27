# U-Blox SARA-R5 Configuration for Wheatstone

## References

<https://learn.sparkfun.com/tutorials/lte-gnss-breakout---sara-r5-hookup-guide/hardware-overview>

## AT Commands

The Sparkfun SARA-R5 board requires opening (cutting) and
closing (soldering) traces to enable dual UART operation.

/UART/0 a.k.a. "UART" a.k.a. "UART 1" will then accept AT
commands for SW configuration.

/UART/1 a.k.a. "AUX UART" a.k.a. "UART 2" will emit GNSS
NMEA output.

What the /dev devices are will depend on the order that the
USB-to-serial devices enumerate; I plug in "UART 1" first
so it will be /dev/ttyUSB0, and then "USART 2" so it will
be /dev/ttyUSB1.

### Reset Module

    AT+CFUN=16

### Set UART to 115200 baud

    AT+IPR=115200

### Enable Dual UART Operation

    AT+USIO=4

### Enable GPIO 1 (a.k.a. 16) for Network Status Indication

    AT+UGPIOC=16,2

### Enable GPIO 6 (a.k.a. 19) for Time Pulse Output

    AT+UGPIOC=19,22

### Set GNSS to Profile 1 (GNSS data flow to and from AUX UART)

    AT+UGPRF=1

### Set Time Pulse to 1PPS using GNSS/LTE (best effort)

    AT+UTIME=1,1

### Power Down GNSS

    AT+UGPS=0

### Power Up GNSS with GPS, GLONASS, SBAS

    AT+UGPS=1,0,71

### Enable Desired NMEA Sentences

    AT+UGGGA=1
    AT+UGGLL=1
    AT+UGGSA=1
    AT+UGGSV=1
    AT+UGRMC=1
    AT+UGVTG=1
    AT+UGZDA=1
