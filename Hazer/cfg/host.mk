# Copyright 2017 Digital Aggregates Corporation
# Licensed under the terms in LICENSE.txt
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Hazer.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

# sudo apt-get install linux-headers-$(uname -r)

# host: most Linux/GNU systems hosting the native toolchain.

MACHINE				:=	$(shell uname -m)
ifeq ($(MACHINE),x86_64)
ARCH				:=	x86_64
endif
ifeq ($(MACHINE),armv7l)
ARCH				:=	arm
endif
ifeq ($(MACHINE),aarch64)
ARCH				:=	arm
endif
OS				:=	linux
TOOLCHAIN			:=
KERNELCHAIN			:=
KERNEL_REV			:=	$(shell uname -r)
KERNEL_DIR			:=	/lib/modules/$(KERNEL_REV)/build
GNUARCH				:=	-D_USE_GNU -D_GNU_SOURCE
CPPARCH				:=	-isystem /usr/src/linux-headers-$(KERNEL_REV) $(GNUARCH)
CARCH				:=	-rdynamic -fPIC
CXXARCH				:=	$(CARCH)
LDARCH				:=	-L$(OUT)/$(LIB_DIR)
MOARCH				:=	-L$(OUT)/$(LIB_DIR)
SOARCH				:=
SOXXARCH			:=	-L$(OUT)/$(LIB_DIR) -l$(PROJECT)
KERNELARCH			:=
LDLIBRARIES			:=	-lm
LDXXLIBRARIES			:=	$(LDLIBRARIES)
