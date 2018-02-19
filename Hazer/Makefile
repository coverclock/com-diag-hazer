# vi: set ts=4 shiftwidth=4:
# Copyright 2017-2018 Digital Aggregates Corporation
# Licensed under the terms in README.h
# author:Chip Overclock
# mailto:coverclock@diag.com
# http://www.diag.com/navigation/downloads/Hazer.html
# "Chip Overclock" is a registered trademark.
# "Digital Aggregates Corporation" is a registered trademark.
#
#	make all TARGET=<target>		# Build libraries, unit tests, executable programs.
#
#	. out/host/bin/setup			# Sets up PATH and LD_LIBRARY_PATH after build.
#

.PHONY:	default

default:	all

########## Customizations

TITLE				:=	Hazer

MAJOR				:=	1# API changes that may require that applications be modified.
MINOR				:=	1# Only functionality or features added with no legacy API changes.
BUILD				:=	2# Only bugs fixed with no API changes or new functionality.

# Disclaimer: the only target that I routinely build and test for is "host", my
# X86_64 Ubuntu build server.

TARGET				:=	host# Build for the x86_64 running Ubuntu 14.04 LTS.

# Some certification, defense, or intelligence agencies (e.g. the U.S. Federal
# Aviation Administration or FAA) require that software builds for safety
# critical or national security applications generate exactly the same binary
# images bit for bit if the source code has not changed. (This is theoretically
# a more stringent requirement than requiring that checksums or cryptographic
# hashes are the same, although in practice it is the same thing.) This allows
# agency inspectors to verify the integrity of the binary software images. This
# makes embedding build timestamps inside compiled translation units problematic.
# If your application has this requirement, you can pass in any fixed string
# for the value of the VINTAGE make variable, and only use the value of this
# symbol as a build time stamp, and you should be able to generate identical
# images with subsequent builds of Hazer. This string is embedded inside the
# Hazer "vintage" application. The default build time stamp is an ISO-8601
# complaint string with the time specified in UTC with nanosecond resolution.

# This stuff all gets embedded in the vintage application.
COPYRIGHT			:=	2017-2018 Digital Aggregates Corporation
LICENSE				:=	GNU Lesser General Public License 2.1
CONTACT				:=	coverclock@diag.com
HOMEPAGE			:=	http://www.diag.com/navigation/downloads/$(TITLE).html
HOST				:=	$(shell hostname -s)
BRANCH				:=	$(shell git rev-parse --abbrev-ref HEAD)
REVISION			:=	$(shell git rev-parse HEAD)
MODIFIED			:=	$(shell date -u -d @$(shell git log -1 --format="%at") +%Y-%m-%dT%H:%M:%S.%N%z)
ROOT				:=	$(shell git rev-parse --show-toplevel)
VINTAGE				:=	$(shell date -u +%Y-%m-%dT%H:%M:%S.%N%z)

# This is where I store collateral associated with projects that I have
# downloaded off the web and use without alteration. Examples: Linux kernel
# sources, toolchains, etc.
HOME_DIR			:=	$(HOME)/Projects

########## Directories

ARC_DIR				:=	arc# Archive files for static linking
BIN_DIR				:=	bin# Utility source files or stripped executable binaries
CFG_DIR				:=	cfg# Build configuration files
DEP_DIR				:=	dep# Generated dependencies and other make files
DOC_DIR				:=	doc# Documentation
ETC_DIR				:=	etc# Miscellaneous files
GEN_DIR				:=	gen# Generated source files
INC_DIR				:=	inc# Header files
LIB_DIR				:=	lib# Shared objects for dynamic linking
MOD_DIR				:=	mod# Loadable user modules
OBC_DIR				:=	obc# C object modules
OUT_DIR				:=	out# Build artifacts
SRC_DIR				:=	src# Library source files
SYM_DIR				:=	sym# Unstripped executable binaries
TGZ_DIR				:=	tgz# Compressed tarballs
TST_DIR				:=	tst# Unit tests

########## Configuration

PROJECT				:=	$(shell echo $(TITLE) | tr '[A-Z]' '[a-z]')
SYMBOL				:=	$(shell echo $(TITLE) | tr '[a-z]' '[A-Z]')

HERE				:=	$(shell pwd)
THERE				:=	$(shell realpath ../..)

OUT					:=	$(OUT_DIR)/$(TARGET)

TEMP_DIR			:=	/tmp
ROOT_DIR			:=	$(HOME_DIR)/$(TARGET)

GITURL				:=	https://github.com/coverclock/com-diag-$(PROJECT).git

GENERATED			:=	vintage generate setup
SYNTHESIZED			:=	$(PROJECT)_release.h $(PROJECT)_vintage.h $(PROJECT)_revision.h

ALIASES				:=	

NEW					:=	dummy
OLD					:=	dummy

PACKAGE				:=	$(OUT)/$(TGZ_DIR)/$(PROJECT)-$(TARGET)

MANIFEST			:=	$(ARC_DIR) $(BIN_DIR) $(INC_DIR) $(LIB_DIR) $(MOD_DIR) $(TST_DIR)

DISTRIBUTION		:=	$(OUT)/$(TGZ_DIR)/$(PROJECT)-$(MAJOR).$(MINOR).$(BUILD)

TARBALL				:=	$(OUT)/$(TGZ_DIR)/$(PROJECT)

SO					:=	so

A2SOARCH			:=	-shared -Wl,-soname,lib$(PROJECT).$(SO).$(MAJOR).$(MINOR)

########## Configuration Makefile

TARGETMAKEFILE		:= $(CFG_DIR)/$(TARGET).mk

include $(TARGETMAKEFILE)

########## Diminuto Configuration

DIMINUTO_TITLE		:=	Diminuto

DIMINUTO_PROJECT	:=	$(shell echo $(DIMINUTO_TITLE) | tr '[A-Z]' '[a-z]')
DIMINUTO_SYMBOL		:=	$(shell echo $(DIMINUTO_TITLE) | tr '[a-z]' '[A-Z]')

DIMINUTO_ROOT		:=	$(THERE)/com-diag-$(DIMINUTO_PROJECT)/$(DIMINUTO_TITLE)
DIMINUTO_HEADERS	:=	$(DIMINUTO_ROOT)/$(INC_DIR)
DIMINUTO_LIBRARIES	:=	$(DIMINUTO_ROOT)/$(OUT)/$(LIB_DIR)

DIMINUTO_CPPFLAGS	:=	-iquote $(DIMINUTO_HEADERS)
DIMINUTO_LDFLAGS	:=	-L$(DIMINUTO_LIBRARIES) -l$(DIMINUTO_PROJECT) -lpthread -lrt -ldl

########## Commands and Option Flags

PROJECT_A			:=	lib$(PROJECT).a
PROJECT_SO			:=	lib$(PROJECT).$(SO)

CROSS_COMPILE		:=	$(TOOLCHAIN)

CC					:=	$(CROSS_COMPILE)gcc
AR					:=	$(CROSS_COMPILE)ar
RANLIB				:=	$(CROSS_COMPILE)ranlib
STRIP				:=	$(CROSS_COMPILE)strip

CDEFINES			:=

ARFLAGS				:=	crsv
CPPFLAGS			:=	$(CDEFINES) -iquote $(SRC_DIR) -iquote $(INC_DIR) -iquote $(OUT)/$(INC_DIR) $(DIMINUTO_CPPFLAGS) $(CPPARCH)
CFLAGS				:=	$(CARCH) -g
CPFLAGS				:=	-i
MVFLAGS				:=	-i
LDFLAGS				:=	$(LDARCH) -l$(PROJECT) $(LDLIBRARIES) $(DIMINUTO_LDFLAGS)
MOFLAGS				:=	$(MOARCH) -l$(PROJECT) $(LDLIBRARIES)
SOFLAGS				:=	$(SOARCH) $(LDLIBRARIES)

#BROWSER			:=	firefox
#BROWSER			:=	epiphany
BROWSER				:=	evince

########## Build Artifacts

TARGETOBJECTS		:=	$(addprefix $(OUT)/$(OBC_DIR)/,$(addsuffix .o,$(basename $(wildcard $(SRC_DIR)/*.c))))
TARGETSCRIPTS		:=	$(addprefix $(OUT)/,$(basename $(wildcard $(BIN_DIR)/*.sh)))
TARGETBINARIES		:=	$(addprefix $(OUT)/,$(basename $(wildcard $(BIN_DIR)/*.c)))
TARGETGENERATED		:=	$(addprefix $(OUT)/$(BIN_DIR)/,$(GENERATED)) $(addprefix $(OUT)/$(SYM_DIR)/,$(GENERATED))
TARGETSYNTHESIZED	:=	$(addprefix $(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/,$(SYNTHESIZED))

TARGETALIASES		:=	$(addprefix $(OUT)/$(BIN_DIR)/,$(ALIASES))
TARGETUNITTESTS		:=	$(addprefix $(OUT)/,$(basename $(wildcard $(TST_DIR)/*.c)))
TARGETUNITTESTS		+=	$(addprefix $(OUT)/,$(basename $(wildcard $(TST_DIR)/*.sh)))

TARGETARCHIVE		:=	$(OUT)/$(ARC_DIR)/$(PROJECT_A)
TARGETSHARED		:=	$(OUT)/$(LIB_DIR)/$(PROJECT_SO).$(MAJOR).$(MINOR)
TARGETSHARED		+=	$(OUT)/$(LIB_DIR)/$(PROJECT_SO).$(MAJOR)
TARGETSHARED		+=	$(OUT)/$(LIB_DIR)/$(PROJECT_SO)

TARGETLIBRARIES		:=	$(TARGETARCHIVE) $(TARGETSHARED)
TARGETPROGRAMS		:=	$(TARGETBINARIES) $(TARGETALIASES) $(TARGETUNITTESTS) $(TARGETGENERATED) $(TARGETSCRIPTS)
TARGETALL			:=	$(TARGETLIBRARIES) $(TARGETPROGRAMS)

########## Main Entry Points

.PHONY:	all dist clean pristine clobber

all:	$(TARGETALL)

dist:	distribution

clean:
	rm -rf $(OUT)
	rm -rf *.gcda *.gcno *.gcov

pristine:	clean
	rm -rf $(OUT_DIR)

clobber:	pristine
	rm -f .cscope.lst .cscope.out .cscope.out.in .cscope.out.po

########## Packaging and Distribution

# Useful for copying the executables over to another target for which they were
# cross compiled.

.PHONY:	package

package $(PACKAGE).tgz:
	D=`dirname $(PACKAGE)`; mkdir -p $$D; \
	for M in $(MANIFEST); do mkdir -p $(OUT)/$$M; done; \
	T=`mktemp -d "$(TEMP_DIR)/$(PROJECT).XXXXXXXXXX"`; \
	B=`basename $(PACKAGE)`; mkdir -p $$T/$$B; \
	tar -C $(OUT) --exclude-vcs -cvf - $(MANIFEST) | tar -C $$T/$$B -xvf -; \
	tar --exclude-vcs -cvf - $(INC_DIR) | tar -C $$T/$$B -xvf -; \
	tar -C $$T --exclude-vcs -cvzf - $$B > $(PACKAGE).tgz; \
	rm -rf $$T

# Useful for given someone a tarball of the actual source distribution that
# is guaranteed to at least build.

.PHONY:	distribution distro

distribution distro $(DISTRIBUTION).tgz:
	D=`dirname $(DISTRIBUTION)`; mkdir -p $$D; \
	T=`mktemp -d "$(TEMP_DIR)/$(PROJECT).XXXXXXXXXX"`; \
	B=`basename $(DISTRIBUTION)`; \
	( cd $$T; git clone $(GITURL) $$B ); \
	tar -C $$T --exclude-vcs -cvzf - ./$$B > $(DISTRIBUTION).tgz; \
	rm -rf $$T/$$B; \
	tar -C $$T -xvzf - < $(DISTRIBUTION).tgz; \
	( cd $$T/$$B/$(TITLE); make all TARGET=host OUT=out/host && $(OUT)/bin/generate > ./setup && . ./setup && vintage ); \
	rm -rf $$T

# Useful for copying a tarball of the current development source base to a
# target for which there is no cross compiler toolchain.

.PHONY:	tarball

tarball $(TARBALL).tgz:
	D=`dirname $(TARBALL)`; mkdir -p $$D; \
	T=`pwd`; \
	B=`basename $$T`; \
	( tar -C .. --exclude-vcs --exclude=$(OUT_DIR) --exclude=.??* -cvzf - $$B ) > $(TARBALL).tgz; \

# Useful for backing a complete backup before doing something that may turn
# out to be profoundly stupid, like running a transformative script across all
# the source files. Sure, your local and remote repos should save you, but what
# if they didn't?

.PHONY:	backup

backup:
	tar --exclude=$(OUT_DIR) -cvzf - . > $(HOME)/$(PROJECT)-$(shell date -u +%Y%m%d%H%M%S%N%Z).tgz

########## Target C Libraries

$(OUT)/$(ARC_DIR)/$(PROJECT_A):	$(TARGETOBJECTS)
	D=`dirname $@`; mkdir -p $$D
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

$(OUT)/$(LIB_DIR)/lib$(PROJECT).$(SO).$(MAJOR).$(MINOR):	$(TARGETOBJECTS)
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CFLAGS) $(A2SOARCH) -o $@ $(SOFLAGS) $(TARGETOBJECTS)

$(OUT)/$(LIB_DIR)/lib$(PROJECT).$(SO).$(MAJOR):	$(OUT)/$(LIB_DIR)/lib$(PROJECT).$(SO).$(MAJOR).$(MINOR)
	D=`dirname $<`; F=`basename $<`; T=`basename $@`; ( cd $$D; ln -s -f $$F $$T ) 

$(OUT)/$(LIB_DIR)/lib$(PROJECT).$(SO):	$(OUT)/$(LIB_DIR)/lib$(PROJECT).$(SO).$(MAJOR)
	D=`dirname $<`; F=`basename $<`; T=`basename $@`; ( cd $$D; ln -s -f $$F $$T ) 

########## Target Unstripped Binaries

$(OUT)/$(SYM_DIR)/%:	$(OUT)/$(OBC_DIR)/$(BIN_DIR)/%.o $(TARGETLIBRARIES)
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

########## Target Aliases

$(OUT)/$(BIN_DIR)/hex $(OUT)/$(BIN_DIR)/oct $(OUT)/$(BIN_DIR)/ntohs $(OUT)/$(BIN_DIR)/htons $(OUT)/$(BIN_DIR)/ntohl $(OUT)/$(BIN_DIR)/htonl:	$(OUT)/$(BIN_DIR)/dec
	ln -f $< $@

########## Unit Tests

$(OUT)/$(TST_DIR)/%:	$(OUT)/$(OBC_DIR)/$(TST_DIR)/%.o $(TARGETLIBRARIES)
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

########## Generated Source Files

# For embedding in a system where it can be executed from a shell.
#
# The major.minor.build is emitted to standard error, a bunch more
# metadata to standard output. Hence, they can be redirected to separate
# files.
#
# Some of the information in this binary executable may be sensitive, for
# example, the Directory or the User. That's why it's in a form that doesn't
# have to be distributed with the product, unlike the shared objects.
#
# This program also serves as a unit test, so be careful about removing stuff
# from it that looks redundant or unused.
#
# The stdout stream from vintage is designed so that you can source it
# into a variety of tools including bash. e.g. eval $(vintage 2> /dev/null) .
#
# NOTE: in the generated C code below, whether you should have \\n or \n in the
# echo statements seems to depend on what version of make you are running.

SLASHES=2

ifeq ($(SLASHES), 1)

$(OUT)/$(GEN_DIR)/vintage.c:	Makefile
	@echo MAKE_VERSION=$(MAKE_VERSION) SLASHES=$(SLASHES)
	D=`dirname $@`; mkdir -p $$D
	echo '/* GENERATED FILE! DO NOT EDIT! */' > $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_release.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_release.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_vintage.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_vintage.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_revision.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_revision.h"' >> $@
	echo '#include <stdio.h>' >> $@
	echo '#include <assert.h>' >> $@
	echo 'static const char METADATA[] =' >> $@
	echo '    "Arch=\"$(ARCH)\"\n"' >> $@
	echo '    "Branch=\"$(BRANCH)\"\n"' >> $@
	echo '    "Contact=\"$(CONTACT)\"\n"' >> $@
	echo '    "Copyright=\"$(COPYRIGHT)\"\n"' >> $@
	echo '    "Homepage=\"$(HOMEPAGE)\"\n"' >> $@
	echo '    "Repository=\"$(GITURL)\"\n"' >> $@
	echo '    "Host=\"$(HOST)\"\n"' >> $@
	echo '    "Kernel=\"$(KERNEL_REV)\"\n"' >> $@
	echo '    "License=\"$(LICENSE)\"\n"' >> $@
	echo '    "Modified=\"$(MODIFIED)\"\n"' >> $@
	echo '    "Os=\"$(OS)\"\n"' >> $@
	echo '    "Release=\"" COM_DIAG_$(SYMBOL)_RELEASE "\"\n"' >> $@
	echo '    "Revision=\"" COM_DIAG_$(SYMBOL)_REVISION "\"\n"' >> $@
	echo '    "Root=\"$(ROOT)\"\n"' >> $@
	echo '    "Target=\"$(TARGET)\"\n"' >> $@
	echo '    "Title=\"$(TITLE)\"\n"' >> $@
	echo '    "Toolchain=\"$(TOOLCHAIN)\"\n"' >> $@
	echo '    "User=\"$(USER)\"\n"' >> $@
	echo '    "Vintage=\"" COM_DIAG_$(SYMBOL)_VINTAGE "\"\n"' >> $@
	echo ';' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_RELEASE_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_RELEASE_VALUE;' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_VINTAGE_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_VINTAGE_VALUE;' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_REVISION_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_REVISION_VALUE;' >> $@
	echo 'int main(void) {' >> $@
	echo '    const char * release_keyword = (const char *)0;' >> $@
	echo '    const char * release_value = (const char *)0;' >> $@
	echo '    const char * vintage_keyword = (const char *)0;' >> $@
	echo '    const char * vintage_value = (const char *)0;' >> $@
	echo '    const char * revision_keyword = (const char *)0;' >> $@
	echo '    const char * revision_value = (const char *)0;' >> $@
	echo '    fputs(METADATA, stdout);' >> $@
	echo '    fputs("$(MAJOR).$(MINOR).$(BUILD)\n", stderr);' >> $@
	echo '    release_keyword = COM_DIAG_$(SYMBOL)_RELEASE_KEYWORD;' >> $@
	echo '    release_value = COM_DIAG_$(SYMBOL)_RELEASE_VALUE;' >> $@
	echo '    vintage_keyword = COM_DIAG_$(SYMBOL)_VINTAGE_KEYWORD;' >> $@
	echo '    vintage_value = COM_DIAG_$(SYMBOL)_VINTAGE_VALUE;' >> $@
	echo '    revision_keyword = COM_DIAG_$(SYMBOL)_REVISION_KEYWORD;' >> $@
	echo '    revision_value = COM_DIAG_$(SYMBOL)_REVISION_VALUE;' >> $@
	echo '    assert(release_keyword != (const char *)0);' >> $@
	echo '    assert(release_value != (const char *)0);' >> $@
	echo '    assert(vintage_keyword != (const char *)0);' >> $@
	echo '    assert(vintage_value != (const char *)0);' >> $@
	echo '    assert(release_keyword != (const char *)0);' >> $@
	echo '    assert(release_value != (const char *)0);' >> $@
	echo '    return 0;' >> $@
	echo '}' >> $@

endif

ifeq ($(SLASHES), 2)

$(OUT)/$(GEN_DIR)/vintage.c:	Makefile
	@echo MAKE_VERSION=$(MAKE_VERSION) SLASHES=$(SLASHES)
	D=`dirname $@`; mkdir -p $$D
	echo '/* GENERATED FILE! DO NOT EDIT! */' > $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_release.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_release.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_vintage.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_vintage.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_revision.h"' >> $@
	echo '#include "com/diag/$(PROJECT)/$(PROJECT)_revision.h"' >> $@
	echo '#include <stdio.h>' >> $@
	echo '#include <assert.h>' >> $@
	echo 'static const char METADATA[] =' >> $@
	echo '    "Arch=\\"$(ARCH)\\"\\n"' >> $@
	echo '    "Branch=\\"$(BRANCH)\\"\\n"' >> $@
	echo '    "Contact=\\"$(CONTACT)\\"\\n"' >> $@
	echo '    "Copyright=\\"$(COPYRIGHT)\\"\\n"' >> $@
	echo '    "Homepage=\\"$(HOMEPAGE)\\"\\n"' >> $@
	echo '    "Repository=\\"$(GITURL)\\"\\n"' >> $@
	echo '    "Host=\\"$(HOST)\\"\\n"' >> $@
	echo '    "Kernel=\\"$(KERNEL_REV)\\"\\n"' >> $@
	echo '    "License=\\"$(LICENSE)\\"\\n"' >> $@
	echo '    "Modified=\\"$(MODIFIED)\\"\\n"' >> $@
	echo '    "Os=\\"$(OS)\\"\\n"' >> $@
	echo '    "Release=\\"" COM_DIAG_$(SYMBOL)_RELEASE "\\"\\n"' >> $@
	echo '    "Revision=\\"" COM_DIAG_$(SYMBOL)_REVISION "\\"\\n"' >> $@
	echo '    "Root=\\"$(ROOT)\\"\\n"' >> $@
	echo '    "Target=\\"$(TARGET)\\"\\n"' >> $@
	echo '    "Title=\\"$(TITLE)\\"\\n"' >> $@
	echo '    "Toolchain=\\"$(TOOLCHAIN)\\"\\n"' >> $@
	echo '    "User=\\"$(USER)\\"\\n"' >> $@
	echo '    "Vintage=\\"" COM_DIAG_$(SYMBOL)_VINTAGE "\\"\\n"' >> $@
	echo ';' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_RELEASE_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_RELEASE_VALUE;' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_VINTAGE_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_VINTAGE_VALUE;' >> $@
	echo 'extern const char COM_DIAG_$(SYMBOL)_REVISION_KEYWORD[];' >> $@
	echo 'extern const char * COM_DIAG_$(SYMBOL)_REVISION_VALUE;' >> $@
	echo 'int main(void) {' >> $@
	echo '    const char * release_keyword = (const char *)0;' >> $@
	echo '    const char * release_value = (const char *)0;' >> $@
	echo '    const char * vintage_keyword = (const char *)0;' >> $@
	echo '    const char * vintage_value = (const char *)0;' >> $@
	echo '    const char * revision_keyword = (const char *)0;' >> $@
	echo '    const char * revision_value = (const char *)0;' >> $@
	echo '    fputs(METADATA, stdout);' >> $@
	echo '    fputs("$(MAJOR).$(MINOR).$(BUILD)\\n", stderr);' >> $@
	echo '    release_keyword = COM_DIAG_$(SYMBOL)_RELEASE_KEYWORD;' >> $@
	echo '    release_value = COM_DIAG_$(SYMBOL)_RELEASE_VALUE;' >> $@
	echo '    vintage_keyword = COM_DIAG_$(SYMBOL)_VINTAGE_KEYWORD;' >> $@
	echo '    vintage_value = COM_DIAG_$(SYMBOL)_VINTAGE_VALUE;' >> $@
	echo '    revision_keyword = COM_DIAG_$(SYMBOL)_REVISION_KEYWORD;' >> $@
	echo '    revision_value = COM_DIAG_$(SYMBOL)_REVISION_VALUE;' >> $@
	echo '    assert(release_keyword != (const char *)0);' >> $@
	echo '    assert(release_value != (const char *)0);' >> $@
	echo '    assert(vintage_keyword != (const char *)0);' >> $@
	echo '    assert(vintage_value != (const char *)0);' >> $@
	echo '    assert(revision_keyword != (const char *)0);' >> $@
	echo '    assert(revision_value != (const char *)0);' >> $@
	echo '    return 0;' >> $@
	echo '}' >> $@

endif

# For embedding in an application where it can be interrogated or displayed.
$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_release.h:	Makefile
	D=`dirname $@`; mkdir -p $$D
	echo '/* GENERATED FILE! DO NOT EDIT! */' > $@
	echo '#ifndef _H_COM_DIAG_$(SYMBOL)_RELEASE_' >> $@
	echo '#define _H_COM_DIAG_$(SYMBOL)_RELEASE_' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_RELEASE_MAJOR $(MAJOR)' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_RELEASE_MINOR $(MINOR)' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_RELEASE_BUILD $(BUILD)' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_RELEASE "$(MAJOR).$(MINOR).$(BUILD)"' >> $@
	echo '#endif' >> $@

# For embedding in the library archive and shared object.
$(OUT)/$(OBC_DIR)/$(SRC_DIR)/$(PROJECT)_release.o:	$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_release.h

# For embedding in an application where it can be interrogated or displayed.
$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_vintage.h:	Makefile
	D=`dirname $@`; mkdir -p $$D
	echo '/* GENERATED FILE! DO NOT EDIT! */' > $@
	echo '#ifndef _H_COM_DIAG_$(SYMBOL)_VINTAGE_' >> $@
	echo '#define _H_COM_DIAG_$(SYMBOL)_VINTAGE_' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_VINTAGE "$(VINTAGE)"' >> $@
	echo '#endif' >> $@

# For embedding in the library archive and shared object.
$(OUT)/$(OBC_DIR)/$(SRC_DIR)/$(PROJECT)_vintage.o:	$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_vintage.h

# For embedding in an application where it can be interrogated or displayed.
$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_revision.h:	Makefile
	D=`dirname $@`; mkdir -p $$D
	echo '/* GENERATED FILE! DO NOT EDIT! */' > $@
	echo '#ifndef _H_COM_DIAG_$(SYMBOL)_REVISION_' >> $@
	echo '#define _H_COM_DIAG_$(SYMBOL)_REVISION_' >> $@
	echo '#define COM_DIAG_$(SYMBOL)_REVISION "$(REVISION)"' >> $@
	echo '#endif' >> $@

# For embedding in the library archive and shared object.
$(OUT)/$(OBC_DIR)/$(SRC_DIR)/$(PROJECT)_revision.o:	$(OUT)/$(INC_DIR)/com/diag/$(PROJECT)/$(PROJECT)_revision.h

# For executing from the command line during testing.
$(OUT)/$(SYM_DIR)/vintage:	$(OUT)/$(GEN_DIR)/vintage.c
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

# For generating a setup script for a bash shell (for example, "bash generate > setup").
# (Because the Android bash doesn't seem to implement BASH_ARGV.)
$(OUT)/$(BIN_DIR)/generate:	Makefile
	D=`dirname $@`; mkdir -p $$D
	echo 'COM_DIAG_$(SYMBOL)_PATH=`dirname $$0`; COM_DIAG_$(SYMBOL)_ROOT=`cd $$COM_DIAG_$(SYMBOL)_PATH; pwd`' > $@
	echo 'echo export PATH=\\$$PATH:$$COM_DIAG_$(SYMBOL)_ROOT/../$(BIN_DIR):$$COM_DIAG_$(SYMBOL)_ROOT/../$(TST_DIR)' >> $@
	echo 'echo export LD_LIBRARY_PATH=\\$$LD_LIBRARY_PATH:$$COM_DIAG_$(SYMBOL)_ROOT/../$(LIB_DIR)' >> $@
	echo 'echo . $(DIMINUTO_ROOT)/$(OUT)/$(BIN_DIR)/setup' >> $@
	chmod 755 $@

# For generating a setup script for a bash shell (for example, "bash generate > setup").
# (Because the Android bash doesn't seem to implement BASH_ARGV.)
$(OUT)/$(SYM_DIR)/generate:	Makefile
	D=`dirname $@`; mkdir -p $$D
	echo 'COM_DIAG_$(SYMBOL)_PATH=`dirname $$0`; COM_DIAG_$(SYMBOL)_ROOT=`cd $$COM_DIAG_$(SYMBOL)_PATH; pwd`' > $@
	echo 'echo export PATH=\\$$PATH:$$COM_DIAG_$(SYMBOL)_ROOT/../$(SYM_DIR):$$COM_DIAG_$(SYMBOL)_ROOT/../$(BIN_DIR):$$COM_DIAG_$(SYMBOL)_ROOT/../$(TST_DIR)' >> $@
	echo 'echo export LD_LIBRARY_PATH=\\$$LD_LIBRARY_PATH:$$COM_DIAG_$(SYMBOL)_ROOT/../$(LIB_DIR)' >> $@
	echo 'echo . $(DIMINUTO_ROOT)/$(OUT)/$(SYM_DIR)/setup' >> $@
	chmod 755 $@

# For sourcing into a bash shell (for example, ". out/host/bin/setup").
$(OUT)/$(BIN_DIR)/setup:	$(OUT)/$(BIN_DIR)/generate
	$< > $@
	chmod 644 $@

# For sourcing into a bash shell (for example, ". out/host/sym/setup").
$(OUT)/$(SYM_DIR)/setup:	$(OUT)/$(SYM_DIR)/generate
	$< > $@
	chmod 644 $@

########## User-Space Loadable Modules

LDWHOLEARCHIVES := # These archives will be linked into the shared object in their entirety.

$(OUT)/$(MOD_DIR)/%.$(SO):	$(MOD_DIR)/%.c
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CPPFLAGS) $(CFLAGS) -shared -o $@ $< $(MOFLAGS) -Wl,--whole-archive $(LDWHOLEARCHIVES) -Wl,--no-whole-archive

########## Helpers

.PHONY:	makeversion gccversion implicit defines iquotes isystems

makeversion:
	@echo $(MAKE_VERSION)

gccversion:
	@$(CC) -x c $(CPPFLAGS) $(CFLAGS) -E -v - < /dev/null

implicit:
	@$(CC) $(CFLAGS) -dM -E - < /dev/null
	
defines:
	@$(CC) $(CPPFLAGS) $(CFLAGS) -dM -E - < /dev/null

iquotes:
	@$(CC) -x c $(CPPFLAGS) $(CFLAGS) -E -v - < /dev/null 2>&1 | awk 'BEGIN { S=0; } ($$0=="#include \"...\" search starts here:") { S=1; next; } ($$0=="#include <...> search starts here:") { S=0; next; } (S!=0) { print $$1; } { next; }'

isystems:
	@$(CC) -x c $(CPPFLAGS) $(CFLAGS) -E -v - < /dev/null 2>&1 | awk 'BEGIN { S=0; } ($$0=="#include <...> search starts here:") { S=1; next; } ($$0=="End of search list.") { S=0; next; } (S!=0) { print $$1; } { next; }'

########## Documentation

.PHONY:	documentation browse refman manpages

DOCCOOKED := $(shell echo $(OUT)/$(DOC_DIR) | sed 's/\//\\\//g')

documentation $(OUT)/$(DOC_DIR)/latex $(OUT)/$(DOC_DIR)/man $(OUT)/$(DOC_DIR)/pdf:
	mkdir -p $(OUT)/$(DOC_DIR)/pdf
	cat doxygen.cf | \
		sed -e "s/\\\$$PROJECT_NUMBER\\\$$/$(MAJOR).$(MINOR).$(BUILD)/" | \
		sed -e "s/\\\$$OUTPUT_DIRECTORY\\\$$/$(DOCCOOKED)/" | \
		cat > $(OUT)/$(DOC_DIR)/doxygen-local.cf
	doxygen $(OUT)/$(DOC_DIR)/doxygen-local.cf

# When I upgraded to Ubuntu 14.04, a pile of latex macros used by the Doxygen
# PDF generation were missing. I was never able to figure out what (if any)
# Ubuntu package they might have been in (although I did install a boatload
# of packages for TeX and Latex). In the end I just kept rerunning the make
# step below, examining the refman.log output file in the output directory,
# and figuring out what Latex macro packages from CTAN to manually download,
# install in a subdirectory of /usr/share/texmf/tex/latex, and then run texhash.
# It was very tedius. Here's a list of what I installed.
# https://www.ctan.org/tex-archive/macros/latex/contrib/xtab
# https://www.ctan.org/tex-archive/macros/latex/contrib/multirow
# https://www.ctan.org/pkg/sectsty
# https://www.ctan.org/tex-archive/macros/latex/contrib/tocloft
documentation-ancillary:	$(OUT)/$(DOC_DIR)/latex $(OUT)/$(DOC_DIR)/man $(OUT)/$(DOC_DIR)/pdf
	$(MAKE) -C $(OUT)/$(DOC_DIR)/latex refman.pdf || exit 0
	cp $(OUT)/$(DOC_DIR)/latex/refman.pdf $(OUT)/$(DOC_DIR)/pdf
	cat $(OUT)/$(DOC_DIR)/man/man3/*.3 | groff -man -Tps - > $(OUT)/$(DOC_DIR)/pdf/manpages.ps
	ps2pdf $(OUT)/$(DOC_DIR)/pdf/manpages.ps $(OUT)/$(DOC_DIR)/pdf/manpages.pdf

browse:
	$(BROWSER) file:$(OUT)/$(DOC_DIR)/html/index.html

refman:
	$(BROWSER) file:$(OUT)/$(DOC_DIR)/pdf/refman.pdf

manpages:
	$(BROWSER) file:$(OUT)/$(DOC_DIR)/pdf/manpages.pdf

########## Diffs and Patches

.PHONY:	patch

patch:	$(OLD) $(NEW)
	diff -purN $(OLD) $(NEW)

########## Rules

$(OUT)/$(OBC_DIR)/%.txt:	%.c $(TARGETSYNTHESIZED)
	D=`dirname $@`; mkdir -p $$D
	$(CC) -E $(CPPFLAGS) -c $< > $@

$(OUT)/$(OBC_DIR)/%.o:	%.c $(TARGETSYNTHESIZED)
	D=`dirname $@`; mkdir -p $$D
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

$(OUT)/%:	%.sh
	D=`dirname $@`; mkdir -p $$D
	cp $< $@
	chmod 755 $@
	
.SECONDARY:

$(OUT)/$(BIN_DIR)/%:	$(OUT)/$(SYM_DIR)/%
	D=`dirname $@`; mkdir -p $$D
	$(STRIP) -o $@ $<

########## Dependencies

.PHONY:	depend

DEPENDENCIES := $(OUT)/$(DEP_DIR)/dependencies.mk

depend:	$(TARGETSYNTHESIZED)
	M=`dirname $(DEPENDENCIES)`; mkdir -p $$M
	cp /dev/null $(DEPENDENCIES)
	for S in $(BIN_DIR) $(MOD_DIR) $(SRC_DIR) $(TST_DIR); do \
		if [ -d $$S ]; then \
			for F in $$S/*.c; do \
				D=`dirname $$F`; \
				T=`mktemp "$(TEMP_DIR)/$(PROJECT).XXXXXXXXXX"`; \
				echo -n "$(OUT)/$(OBC_DIR)/$$D/" > $$T; \
				$(CC) $(CPPFLAGS) -MM -MG $$F >> $$T && cat $$T >> $(DEPENDENCIES); \
				rm -f $$T; \
			done; \
		fi; \
	done

-include $(DEPENDENCIES)

########## Installation

.PHONY:	install install-bin install-lib install-include

INSTALL_DIR := /usr/local
INSTALL_BIN := $(INSTALL_DIR)/bin
INSTALL_LIB := $(INSTALL_DIR)/lib
INSTALL_INC := $(INSTALL_DIR)/include

install:	install-bin install-lib install-include

install-bin:
	mkdir -p $(INSTALL_BIN)
	for B in $(OUT)/$(BIN_DIR)/*; do \
		install $$B $(INSTALL_BIN); \
	done

install-lib:
	mkdir -p $(INSTALL_LIB)
	for F in $(OUT)/$(LIB_DIR)/*.so; do \
		O=`basename $$F`; \
		cp $(OUT)/$(LIB_DIR)/$$O.$(MAJOR).$(MINOR) $(INSTALL_LIB); \
		( cd $(INSTALL_LIB); ln -s -f $$O.$(MAJOR).$(MINOR) $$O.$(MAJOR) ); \
		( cd $(INSTALL_LIB); ln -s -f $$O.$(MAJOR) $$O ); \
	done
	ldconfig -v $(INSTALL_LIB)

install-include:
	mkdir -p $(INSTALL_INC)
	tar -C $(OUT)/$(INC_DIR) -cvf - . | tar -C $(INSTALL_INC) -xvf -
	tar -C $(INC_DIR) -cvf - . | tar -C $(INSTALL_INC) -xvf -
