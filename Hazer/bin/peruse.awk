# Copyright 2021 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
BEGIN   { inp="INP [  0]"; out="OUT [  0]"; arm=1; }
/^INP / { inp=substr($0,0,79); arm=1; next; }
/^OUT / { out=substr($0,0,79); arm=1; next; }
        { if (arm!=0) { print inp; print out; arm=0; } print $0; next; }
END     { if (arm!=0) { print inp; print out; arm=0; } }
