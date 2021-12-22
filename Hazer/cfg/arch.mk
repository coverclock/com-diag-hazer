# vi: set ts=4 shiftwidth=4:
# Copyright 2021 Digital Aggregates Corporation
# Licensed under the terms in LICENSE.txt
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Hazer.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

# (I have yet to find a reliable way to determine under what
# ARCH for which the host Linux kernel was built. You would
# think uname would do this for you.)

ifeq ($(MACHINE),x86_64)
ARCH				:=	x86_64
else
ifeq ($(MACHINE),armv7l)
ARCH				:=	arm
else
ifeq ($(MACHINE),aarch64)
ARCH				:=	arm
else
ARCH				:=	other
$(error MACHINE not recognized for ARCH)
endif
endif
endif
