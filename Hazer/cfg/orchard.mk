# Copyright 2014 Digital Aggregates Corporation
# Licensed under the terms in LICENSE.txt
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Diminuto.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.

# orchard: Mac OS X 10.9.5 running on a Mac Mini "late 2012".
# WORK IN PROGRESS!

ARCH				:=	x86_64
OS					:=	darwin
TOOLCHAIN			:=
KERNELCHAIN			:=
KERNEL_REV			:=
KERNEL_DIR			:=
CPPARCH				:=
CARCH				:=	-fPIC
CXXARCH				:=	$(CARCH)
LDARCH				:=	-L$(OUT)/$(LIB_DIR)
MOARCH				:=	-L$(OUT)/$(LIB_DIR)
SOARCH				:=	-L$(OUT)/$(LIB_DIR)
SOXXARCH			:=	-L$(OUT)/$(LIB_DIR) -l$(PROJECT)
KERNELARCH			:=
LDLIBRARIES			:=	-lpthread -ldl -lm
LDXXLIBRARIES		:=	$(LDLIBRARIES)
A2SOARCH			:=	-Wl,-dylib -Wl,-current_version,$(MAJOR).$(MINOR).$(BUILD)
A2SOXXARCH			:=	$(A2SOARCH) -l$(PROJECT)
