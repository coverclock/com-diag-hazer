#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

XC=0

# Actual DGNSS corrected measurements taken during testing around a ~1m square.
SE1="39.794439629, -105.153355119"
NE1="39.794448585, -105.153354742"
NW1="39.794448978, -105.153366525"
SW1="39.794440107, -105.153366753"
SE2="39.794439794, -105.153355129"
NE2="39.794448627, -105.153354846"
NW2="39.794448998, -105.153366461"
SW2="39.794440138, -105.153366794"

# My arbitrarily chosen precision of 0.025m = 2.5cm ~ 1in
LIMIT="0.025"

FABS="define abs(i) {
    if (i < 0) return (-i)
    return (i)
}"

##########

echo "Should be 0.0 exactly." 2>&1

ONE=${SE1}
TWO=${SE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NE1}
TWO=${NE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NW1}
TWO=${NW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${SW1}
TWO=${SW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

##########

ONE=${SE2}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NE2}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NW2}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${SW2}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "print (${COMPUTED} == 0.0)" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

##########

echo "Should be 1.0 plus or minus ${LIMIT}." 2>&1

ONE=${SE1}
TWO=${NE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NE1}
TWO=${NW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NW1}
TWO=${SW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${SW1}
TWO=${SE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

##########

ONE=${SE2}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NE2}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NW2}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${SW2}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print ((abs(${COMPUTED} - 1.0) < ${LIMIT}))
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

##########

echo "Should be ${LIMIT} maximum." 2>&1

ONE=${SE1}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print (${COMPUTED} < ${LIMIT})
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NE1}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print (${COMPUTED} < ${LIMIT})
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${NW1}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print (${COMPUTED} < ${LIMIT})
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

ONE=${SW1}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED} 1>&2
EVALUATED=$(echo "
${FABS}
print (${COMPUTED} < ${LIMIT})
" | bc -l)
if [[ ${EVALUATED} -ne 1 ]]; then
    echo "FAILED!" 1>&2
    XC=2
fi

##########

if [[ ${XC} == "0" ]]; then
    echo "PASSED." 1>&2
fi

exit ${XC}
