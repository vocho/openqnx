#
# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.
#
# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.
#

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=build embeddable filesystems
endef

INSTALLDIR_nto = bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

NAME=$(SECTION)
EXTRA_SILENT_VARIANTS+=$(SECTION)
USEFILE=$(PROJECT_ROOT)/$(SECTION)/$(NAME).c

LINKS_mkxfs = mkifs mkefs 

# Since we're not shipping etfs with 6.3.0, supress the link
# on the 6.3.0 build machine. The ifneq can be removed later...
ifneq ($(VERSION_REL), 6.3.0)
LINKS_mkxfs += mketfs
endif

LINKS = $(LINKS_$(SECTION))

CCFLAGS += -DELF_TARGET_ALL

LDOPTS_dumpifs += -lutil
LDOPTS += -lmisc $(LDOPTS_$(SECTION))

LIB_SOCKET_win32=wsock32 iphlpapi
LIB_SOCKET_solaris=socket nsl
LIB_SOCKET_nto=socket
LIB_SOCKET_linux=pthread
LIB_SOCKET=$(LIB_SOCKET_$(OS))

LIBS += $(LIBS_$(SECTION)) compat z lzo ucl

include $(MKFILES_ROOT)/qmacros.mk

ifneq ($(call FIND_HDR_DIR,nto,usr/include,sys/f3s_spec.h),)
CCFLAGS += -DHAVE_F3S
endif

ifneq ($(call FIND_HDR_DIR,nto,usr/include,fs/f3s_spec.h),)
CCFLAGS += -DHAVE_F3S_V3
endif

ifneq ($(call FIND_HDR_DIR,nto,usr/include,fs/etfs.h),)
CCFLAGS += -DHAVE_ETFS
endif

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
