# vi: set ts=4 shiftwidth=4:
# Copyright 2012-2020 Digital Aggregates Corporation
# author:Chip Overclock
# mailto:coverclock@diag.com
# https://github.com/coverclock/com-diag-hazer
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

.PHONY:	all bootstrap release

all:
	$(MAKE) -C Hazer all

bootstrap:

release:	all

.PHONY:	scope

scope:
	mycscope

.PHONY:	rescope

rescope:
	rm -f .cscope*
	mycscope
