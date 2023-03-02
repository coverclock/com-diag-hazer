#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

XC=0

INPUT="\"neon\", 2, 3, 0, 11, 1599145249.632000060, 1599145249.000000000, 39.7943071, -105.1533805, 0., 0., 1688.800, 0., 0.005000, 0., 0., 0., 0., 0., 0., 0., 0, 0.\n"

OUTPUT_DEFAULT="neon 2 1599145249 39.7943071 -105.1533805 0. 2020-09-03T15:00:49Z\n"

OUTPUT_CSV="\"neon\", 2, 1599145249, 39.7943071, -105.1533805, 0., \"2020-09-03T15:00:49Z\"\n"

OUTPUT_HTML="<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"><meta http-equiv=\"Content-Style-Type\" content=\"text/css\"></head><body><h1>NAM</h1><p>neon</p><h1>NUM</h1><p>2</p><h1>TIM</h1><p>1599145249</p><h1>LAT</h1><p>39.7943071</p><h1>LON</h1><p>-105.1533805</p><h1>MSL</h1><p>0.</p><h1>LBL</h1><p>2020-09-03T15:00:49Z</p></body></html>\n"

OUTPUT_JSON="{ \"NAM\": \"neon\", \"NUM\": 2, \"TIM\": 1599145249, \"LAT\": 39.7943071, \"LON\": -105.1533805, \"MSL\": 0, \"LBL\": \"2020-09-03T15:00:49Z\" }\n"

OUTPUT_QUERY="?NAM=neon&NUM=2&TIM=1599145249&LAT=39.7943071&LON=-105.1533805&MSL=0.&LBL=2020-09-03T15:00:49Z\n"

OUTPUT_SHELL="NAM=\"neon\"; NUM=2; TIM=1599145249; LAT=39.7943071; LON=-105.1533805; MSL=0.; LBL=\"2020-09-03T15:00:49Z\"\n"

OUTPUT_YAML="NAM: neon\nNUM: 2\nTIM: 1599145249\nLAT: 39.7943071\nLON: -105.1533805\nMSL: 0.\nLBL: 2020-09-03T15:00:49Z\n"

OUTPUT_XML="<?xml version=\"1.0\" encoding=\"UTF-8\" ?><NAM>neon</NAM><NUM>2</NUM><TIM>1599145249</TIM><LAT>39.7943071</LAT><LON>-105.1533805</LON><MSL>0.</MSL><LBL>2020-09-03T15:00:49Z</LBL>\n"

echo "**********"
echo "DEFAULT"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_DEFAULT}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F -)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "CSV"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_CSV}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -c)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "HTML"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_HTML}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -h)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "JSON"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_JSON}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -j)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "QUERY"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_QUERY}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -q)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "SHELL"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_SHELL}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -s)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "YAML"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_YAML}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -y)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

echo "**********"
echo "XML"
echo "**********"
EXPECTED="$(echo -e -n "${OUTPUT_XML}")"
ACTUAL="$(echo -e -n "${INPUT}" | csv2dgm -F - -x)"
echo "${INPUT}"
echo "${EXPECTED}"
echo "${ACTUAL}"
if [[ "${EXPECTED}" != "${ACTUAL}" ]]; then
    echo "FAILED!" 1>&2
    XC=1
fi

exit ${XC}
