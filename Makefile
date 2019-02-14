# vi: set ts=4 shiftwidth=4:
# Copyright 2017 Digital Aggregates Corporation
# author:Chip Overclock
# mailto:coverclock@diag.com
# https://github.com/coverclock/com-diag-hazer
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

.PHONY:	scope

scope:
	mycscope

.PHONY:	rescope

rescope:
	rm -f .cscope*
	mycscope
