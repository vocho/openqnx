# Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
#  
# This source code has been published by QNX Software Systems Ltd. (QSSL).
# However, any use, reproduction, modification, distribution or transfer of
# this software, or any software which includes or is based upon any of this
# code, is only permitted under the terms of the QNX Realtime Plaform End User
# License Agreement (see licensing.qnx.com for details) or as otherwise
# expressly authorized by a written license agreement from QSSL. For more
# information, please email licensing@qnx.com.
#

#
# This file is included by both the C and HA client recovery library
#

#
# This include file actually doesn't have any data in it, it's just used
# to force MAKE to re-read things and regenerate the $(SRCS) macro after 
# 'mkkercalls' has created the kernel entry assembly stubs.
#
-include ../kupdate.mk

MKKERCALLS:=$(MKKERCALLS_DIR)/mkkercalls
	
#
# Don't try to gen the kercalls if the <sys/kercalls.h> header isn't 
# installed yet.
#
KERCALLS_H:=$(firstword $(wildcard $(dir $(TOP))services/system/public/sys/kercalls.h*) $(call FIND_HDR_DIR, nto, usr/include, sys/kercalls.h)sys/kercalls.h)
ifneq ($(KERCALLS_H),)

../kupdate.mk: $(MKKERCALLS) $(MKKERCALLS_DIR)/$(CPU)/template
	ksh $(MKKERCALLS) $(MKKERCALLS_SWITCHES) $(KERCALLS_H) $(CPU) ..
	$(TOUCH_HOST) $@
		
endif

EXTRA_CLEAN=../kupdate.mk ../*.s ../*.S
