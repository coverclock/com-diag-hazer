#!/bin/bash
# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
#
# ABSTRACT
#
# Tests the Bash sourcing files by sourcing them and checking the results.
#
# USAGE
#
# unittest-sourcing
#

XC=1

BINDIR=$(readlink -e $(dirname ${0})/../bin)
SYMDIR=$(readlink -e $(dirname ${0})/../sym)

for DD in ${BINDIR} ${SYMDIR}; do
    FF=${DD}/hazer
    SS=${DD}/setup
    VV=${DD}/vintage
    if [ -r ${FF} -a -r ${SS} -a -x ${VV} ]; then
        (
            . ${FF}
            echo ${FF}: Arch=${Arch}
            echo ${FF}: Branch=${Branch}
            echo ${FF}: Cc=${Cc}
            echo ${FF}: Contact=${Contact}
            echo ${FF}: Copyright=${Copyright}
            echo ${FF}: Distro=${Distro}
            echo ${FF}: Homepage=${Homepage}
            echo ${FF}: Host=${Host}
            echo ${FF}: Kernel=${Kernel}
            echo ${FF}: Libc=${Libc}
            echo ${FF}: License=${License}
            echo ${FF}: Make=${Make}
            echo ${FF}: Modified=${Modified}
            echo ${FF}: Os=${Os}
            echo ${FF}: Platform=${Platform}
            echo ${FF}: Release=${Release}
            echo ${FF}: Repository=${Repository}
            echo ${FF}: Revision=${Revision}
            echo ${FF}: Root=${Root}
            echo ${FF}: Target=${Target}
            echo ${FF}: Title=${Title}
            echo ${FF}: Toolchain=${Toolchain}
            echo ${FF}: User=${User}
            echo ${FF}: Vintage=${Vintage}
            . ${SS}
            ${VV} 2> /dev/null | (
                while read LL; do
                    echo ${VV}: ${LL}
                    eval LL_${LL}
                done
                test "${Arch}" = "${LL_Arch}" || exit 2
                test "${Branch}" = "${LL_Branch}" || exit 2
                test "${Cc}" = "${LL_Cc}" || exit 2
                test "${Contact}" = "${LL_Contact}" || exit 2
                test "${Copyright}" = "${LL_Copyright}" || exit 2
                test "${Distro}" = "${LL_Distro}" || exit 2
                test "${Homepage}" = "${LL_Homepage}" || exit 2
                test "${Host}" = "${LL_Host}" || exit 2
                test "${Kernel}" = "${LL_Kernel}" || exit 2
                test "${Libc}" = "${LL_Libc}" || exit 2
                test "${License}" = "${LL_License}" || exit 2
                test "${Make}" = "${LL_Make}" || exit 2
                test "${Modified}" = "${LL_Modified}" || exit 2
                test "${Os}" = "${LL_Os}" || exit 2
                test "${Platform}" = "${LL_Platform}" || exit 2
                test "${Release}" = "${LL_Release}" || exit 2
                test "${Repository}" = "${LL_Repository}" || exit 2
                test "${Revision}" = "${LL_Revision}" || exit 2
                test "${Root}" = "${LL_Root}" || exit 2
                test "${Target}" = "${LL_Target}" || exit 2
                test "${Title}" = "${LL_Title}" || exit 2
                test "${Toolchain}" = "${LL_Toolchain}" || exit 2
                test "${User}" = "${LL_User}" || exit 2
                test "${Vintage}" = "${LL_Vintage}" || exit 2
            ) || exit 3
        ) || exit 4
        XC=0
    fi
done

exit ${XC}
