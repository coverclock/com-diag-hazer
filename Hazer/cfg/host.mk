# Copyright 2017 Digital Aggregates Corporation
# Licensed under the terms in README.hz
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Hazer.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

# sudo apt-get install linux-headers-$(uname -r)

# host: most Linux/GNU systems hosting the native toolchain.

ARCH				:=	x86_64
OS					:=	linux
TOOLCHAIN			:=
KERNELCHAIN			:=
KERNEL_REV			:=	$(shell uname -r)
KERNEL_DIR			:=	/lib/modules/$(KERNEL_REV)/build
CPPARCH				:=	-isystem /usr/src/linux-headers-$(KERNEL_REV)
CARCH				:=	-rdynamic -fPIC
CXXARCH				:=	$(CARCH)
LDARCH				:=	-L$(OUT)/$(LIB_DIR)
MOARCH				:=	-L$(OUT)/$(LIB_DIR)
SOARCH				:=
SOXXARCH			:=	-L$(OUT)/$(LIB_DIR) -l$(PROJECT)
KERNELARCH			:=
LDLIBRARIES			:=	-lm
LDXXLIBRARIES			:=	$(LDLIBRARIES)
