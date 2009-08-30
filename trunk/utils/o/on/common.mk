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

define PINFO
PINFO DESCRIPTION=Execute a command on another tty
endef

INSTALLDIR_qnx4 = usr/bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) bin)

LINKS = waitfor

LIBS_nto+=login
LIBS=$(LIBS_$(OS))

USEFILE=$(OS_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

