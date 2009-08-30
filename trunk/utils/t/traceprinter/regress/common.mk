# Copyright 2001, QNX Software Systems Ltd. Unpublished Work All Rights
#Reserved.
#
#This source code contains confidential information of QNX Software Systems
#Ltd. (QSSL). Any use, reproduction, modification, disclosure, distribution
#or transfer of this software, or any software which includes or is based
#upon any of this code, is only permitted under the terms of the QNX
#Confidential Source License version 1.0 (see licensing.qnx.com for details)
#or as otherwise expressly authorized by a written license agreement from
#QSSL. For more information, please email licensing@qnx.com.
#

DEFDEBUG=-g
OPTIMIZE_TYPE=NONE
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=
INSTALLDIR=/dev/null

ifndef HOWTOLINK
HOWTOLINK=$(word 1, $(filter static dynamic, $(VARIANTS) dynamic))
endif
LDFLAGS+=-B$(HOWTOLINK)

include $(MKFILES_ROOT)/qmacros.mk
BUILDNAME=ignore

include $(MKFILES_ROOT)/qtargets.mk

EXES:=$(addsuffix  ,$(basename $(notdir $(SRCS))))

all: $(EXES)

$(PWD)/ignore:
	true

$(EXES): % : %.o
	$(RM_HOST) $@
	$(CREATE_$(BUILD_TYPE))
ifneq ($(USEFILE),)
	$(UMPREF) $@ $(USEFILE) $(UMPOST)
endif
	$(MG_HOST) $@


LIBS+=traceparser

