#!/bin/bash
# Copyright 2023 Digital Aggregates Corporation, Arvada Colorado USA.
# https://github.com/coverclock/com-diag-hazer
# mailto:coverclock@diag.com
# Convenience script to start multiple apps in windows for field test.
# Goes in ${HOME}/bin.
APPNAM=${1:-"spartan"}
${HOME}/src/com-diag-hazer/Hazer/out/host/bin/windows ${APPNAM}
