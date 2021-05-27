#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer

PROGRAM=$(basename ${0})

. $(readlink -e $(dirname ${0})/../bin)/setup

XC=0

SE1="39.794439629, -105.153355119"
NE1="39.794448585, -105.153354742"
NW1="39.794448978, -105.153366525"
SW1="39.794440107, -105.153366753"

SE2="39.794439794, -105.153355129"
NE2="39.794448627, -105.153354846"
NW2="39.794448998, -105.153366461"
SW2="39.794440138, -105.153366794"

##

ONE=${SE1}
TWO=${SE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NE1}
TWO=${NE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NW1}
TWO=${NW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${SW1}
TWO=${SW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

##

ONE=${SE2}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NE2}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NW2}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${SW2}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

##

ONE=${SE1}
TWO=${NE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NE1}
TWO=${NW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NW1}
TWO=${SW1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${SW1}
TWO=${SE1}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

##

ONE=${SE2}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NE2}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NW2}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${SW2}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

##

ONE=${SE1}
TWO=${SE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NE1}
TWO=${NE2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${NW1}
TWO=${NW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

ONE=${SW1}
TWO=${SW2}
COMPUTED=$(geodesic ${ONE} ${TWO})
echo "unittest-geodesic:" "geodesic" ${ONE} ${TWO} "=" ${COMPUTED}

exit 0
