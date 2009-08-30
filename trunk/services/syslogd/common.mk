# 
# Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
#  
# This source code has been published by QNX Software Systems Ltd. (QSSL).
# However, any use, reproduction, modification, distribution or transfer of
# this software, or any software which includes or is based upon any of this
# code, is only permitted under the terms of the QNX Open Community License
# version 1.0 (see licensing.qnx.com for details) or as otherwise expressly
# authorized by a written license agreement from QSSL. For more information,
# please email licensing@qnx.com.
#  
ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_qnx4=usr/ucb
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/sbin)

define PINFO
PINFO DESCRIPTION=System Logger daemon
endef

#OLD QNX4 lib line LIBS_qnx4=unix3r ncurses3r telnet socket_s
LIBS_qnx4 = misc 
LIBS=socket $(LIBS_$(OS))

EXCLUDE_OBJS = daemon.o

include $(MKFILES_ROOT)/qtargets.mk
