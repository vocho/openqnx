# 
# Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
#  
# This source code has been published by QNX Software Systems Ltd. (QSSL).
# However, any use, reproduction, modification, distribution or transfer of
# this software, or any software which includes or is based upon any of this
# code, is only permitted under the terms of the QNX Community License version
# 1.0 (see licensing.qnx.com for details) or as otherwise expressly authorized
# by a written license agreement from QSSL. For more information, please email
# licensing@qnx.com.
#  
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_nto = bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

define PINFO
PINFO DESCRIPTION=Return system name
endef

LIBS_mingw32=compat

LIBS+=$(LIBS_$(OS))

ifeq ($(OSTYPE),cygwin)
  CCFLAGS += -w
  USEFILE=
else
  USEFILE=$(PROJECT_ROOT)/$(OS)/uname.c
endif

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
