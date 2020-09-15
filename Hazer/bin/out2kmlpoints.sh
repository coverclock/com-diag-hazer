#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Filter that reads OUT and outputs KML XML to visualize discrete points.
# References:
#     <https://www.gpsvisualizer.com>.
#     "OGC KML 2.3", 2015 <http://docs.opengeospatial.org/is/12-007r2/12-007r2.html>
#     J. Wernecke, "The KML Handbook", Addison-Wesley, 2009

# LOC 2020-09-15T10:57:42.114-07:00+01T 00:00:04.046 37.5.1         1954 neon    
# TIM 2020-09-15T16:57:39.000-00:00+00Z 0pps                             GNSS    
# POS 39°44'33.081"N, 105°24'28.732"W    39.7425226, -105.4079813        GNSS    
# ALT    7031.74'   2143.300m MSL    6961.53'   2121.900m GEO            GNSS    
# COG NE    55.260000000°T    0.000000000°M                              GNSS    
# SOG      48.024mph      41.732knots      77.287kph      21.469m/s      GNSS    

# 2020-09-15T16:57:39.000-00:00+00Z 39.7425226, -105.4079813 2143.300m 55.260000000°T 41.732knots

COUNT=""

awk '
/^LOC / { TIM=""; LAT=""; LON=""; MSL=""; COG=""; SOG=""; }
/^TIM / { TIM=$2; }
/^POS / { LAT=$4; LON=$5; }
/^ALT / { MSL=$3; }
/^COG / { COG=$3; }
/^SOG / { SOG=$3; }
/^INT / { print TIM, LAT, LON, MSL, COG, SOG; }
' | while read TIM LAT LON MSL COG SOG; do

	if [[ -z "${COUNT}" ]]; then

		echo '<?xml version="1.0" encoding="utf-8" standalone="yes"?>'
		echo '<kml xmlns="http://www.opengis.net/kml/2.2">'
		echo '  <Document>'
		echo '    <name><![CDATA[hups]]></name>'
		echo '    <visibility>1</visibility>'
		echo '    <open>1</open>'
		echo '    <Snippet><![CDATA[See <a href="https://github.com/coverclock/com-diag-hazer/blob/master/Hazer/bin/csv2kmlpoints.sh">csv2kmlpoints</a>]]></Snippet>'
		echo '    <Folder id="Points">'
		echo '      <name>Points</name>'
		echo '      <visibility>1</visibility>'
		echo '      <open>0</open>'

		COUNT=1
	fi

	LATITUDE=${LAT%,}
	LONGITUDE=${LON%,}
	ALTITUDE=${MSL%m}
	COURSE=${COG%°T}
	SPEED=${SOG%knots}

	echo '      <Placemark>'
	echo '        <name><![CDATA['${COUNT}']]></name>'
	echo '        <Snippet></Snippet>'
	echo '        <description><![CDATA['${COURSE},${SPEED}']]></description>'
	echo '        <Point>'
	echo '          <coordinates>'
	echo "            ${LONGITUDE},${LATITUDE},${ALTITUDE} "
	echo '          </coordinates>'
	echo '        </Point>'
	echo '      </Placemark>'

	COUNT=$((${COUNT} + 1))

done

echo '    </Folder>'
echo '  </Document>'
echo '</kml>'

exit 0
