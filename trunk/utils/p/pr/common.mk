# This is an automatically generated record.
# Please never delete or relocate it.

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

INSTALLDIR_qnx4 = bin
INSTALLDIR = $(firstword $(INSTALLDIR_$(OS)) usr/bin)

define PINFO
PINFO DESCRIPTION=text pagination and formatting utility
endef

#===== CCFLAGS - add the flags to the C compiler command line. 
CCFLAGS+=-O0 
include $(MKFILES_ROOT)/qtargets.mk
