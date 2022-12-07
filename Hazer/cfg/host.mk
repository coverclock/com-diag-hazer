# vi: set ts=4 shiftwidth=4:
# Copyright 2017-2022 Digital Aggregates Corporation
# Licensed under the terms in LICENSE.txt
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Hazer.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.
#
# host: most Linux/GNU systems hosting the native toolchain.
#
# Adapted from kernel.org:linux-5.15.11/scripts/subarch.include .

MACHINE				:=	$(shell uname -m)
ARCH				:=	$(shell uname -m | sed -e 's/i.86/x86/' -e 's/x86_64/x86/' -e 's/sun4u/sparc64/' -e 's/arm.*/arm/' -e 's/sa110/arm/' -e 's/s390x/s390/' -e 's/ppc.*/powerpc/' -e 's/mips.*/mips/' -e 's/sh[234].*/sh/' -e 's/aarch64.*/arm64/' -e 's/riscv.*/riscv/')
OS					:=	$(shell uname -o)
TOOLCHAIN			:=
KERNELCHAIN			:=
KERNEL_REV			:=	$(shell uname -r)
KERNEL_DIR			:=	/lib/modules/$(KERNEL_REV)/build
GNUARCH				:=	-D_USE_GNU -D_GNU_SOURCE
# sudo apt-get install linux-headers-$(uname -r)
CPPARCH				:=	-isystem /usr/src/linux-headers-$(KERNEL_REV) $(GNUARCH)
CARCH				:=	-rdynamic -fPIC -Wall
CXXARCH				:=	$(CARCH)
LDARCH				:=	-L$(OUT)/$(LIB_DIR)
MOARCH				:=	-L$(OUT)/$(LIB_DIR)
SOARCH				:=
SOXXARCH			:=	-L$(OUT)/$(LIB_DIR) -l$(PROJECT)
KERNELARCH			:=
LDLIBRARIES			:=	-lm
LDXXLIBRARIES		:=	$(LDLIBRARIES)
