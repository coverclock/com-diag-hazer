
#!/bin/bash
# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
# Create KML of points where position fix changes.
# usage: csv2kmlchanges < CSVFILE > KMLFILECHANGES

INIT=""
FIX0=""
SAT0=""

while read NAM NUM FIX SYS SAT CLK TIM LAT LON HAC MSL GEO VAC SOG COG ROL PIT YAW RAC PAC YAC OBS MAC; do

	if [[ "${NUM}" == "NUM," ]]; then
		continue
	fi

	if [[ -z "${INIT}" ]]; then
		NAME=${NAM##\"}
		NAME=${NAME%%\",}

		echo '<?xml version="1.0" encoding="utf-8" standalone="yes"?>'
		echo '<kml xmlns="http://www.opengis.net/kml/2.2">'
		echo '  <Document>'
		echo '    <name><![CDATA['${NAME}']]></name>'
		echo '    <visibility>1</visibility>'
		echo '    <open>1</open>'
		echo '    <Snippet><![CDATA[See <a href="https://github.com/coverclock/com-diag-hazer/Hazer/bin/csv2kml.sh">csv2kml</a>]]></Snippet>'
		echo '    <Folder id="Tracks">'
		echo '      <name>Tracks</name>'
		echo '      <visibility>1</visibility>'
		echo '      <open>0</open>'

		INIT=Y
	fi

	if [[ "${FIX0}" != "${FIX}" || "${SAT0}" != "${SAT}" ]]; then

		LABEL=${NUM%,}
		TYPE=$(csvfix2str ${FIX})
		LATITUDE=${LAT%,}
		LONGITUDE=${LON%,}
		ALTITUDE=${MSL%,}

		echo '      <Placemark>'
		echo '        <name><![CDATA['${LABEL}']]></name>'
		echo '        <Snippet></Snippet>'
		echo '        <description><![CDATA['${TYPE},${SAT}${SOG}${ROL}${PIT}${YAW}']]></description>'
		echo '        <Point>'
		echo '          <coordinates>'
		echo "            ${LONGITUDE},${LATITUDE},${ALTITUDE} "
		echo '          </coordinates>'
		echo '        </Point>'
		echo '      </Placemark>'

		FIX0=${FIX}
		SAT0=${SAT}

	fi

done

echo '    </Folder>'
echo '  </Document>'
echo '</kml>'

exit 0
