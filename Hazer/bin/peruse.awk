# Copyright 2020 Digital Aggregates Corporation, Colorado, USA
# Licensed under the terms in LICENSE.txt
# Chip Overclock <coverclock@diag.com>
# https://github.com/coverclock/com-diag-hazer
begin   { inp="INP [   ]"; out="OUT [   ]"; arm=1; }
/^INP / { inp=substr($0,0,79); arm=1; next; }
/^OUT / { out=substr($0,0,79); arm=1; next; }
        { if (arm!=0) { print inp; print out; arm=0; } print $0; next; }
end     { if (arm!=0) { print inp; print out; arm=0; } }
