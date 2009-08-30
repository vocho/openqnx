# Copyright 2001, QNX Software Systems Ltd. Unpublished Work All Rights
# Reserved.

 
# This source code contains confidential information of QNX Software Systems
# Ltd. (QSSL). Any use, reproduction, modification, disclosure, distribution
# or transfer of this software, or any software which includes or is based
# upon any of this code, is only permitted under the terms of the QNX
# Confidential Source License version 1.0 (see licensing.qnx.com for details)
# or as otherwise expressly authorized by a written license agreement from
# QSSL. For more information, please email licensing@qnx.com.

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=print cpu utilization
endef

CCFLAGS += -D_FILE_OFFSET_BITS=64 -D_IOFUNC_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE=1

USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

