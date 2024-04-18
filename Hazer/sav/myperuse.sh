#!/bin/bash
# ssh coverclock@plutonium /home/coverclock/bin/myperuse neof10tHinitpp out 38
cd ${HOME}/src/com-diag-hazer/Hazer
. out/host/bin/setup
peruse ${*}
