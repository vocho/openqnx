# Copyright 2002, QNX Software Systems Ltd. Unpublished Work All Rights
# Reserved.
#
#
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

define PINFO
PINFO DESCRIPTION=LZO compression library
endef

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

EXCLUDE_OBJS = example.o

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
