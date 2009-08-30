# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.

# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.


ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Support library for tcpip utils. Misc functions
endef

INSTALLDIR=usr/lib

INCVPATH += $(PROJECT_ROOT)/

CCF_nto = -Du_int32_t=uint32_t -Du_int16_t=uint16_t -Du_int8_t=uint8_t -D'_DIAGASSERT(x)=((void)0)'

#EXCLUDE_nto = err.o

#EXCLUDE_OBJS += $(EXCLUDE_$(OS))

CCFLAGS += $(CCF_$(OS))

# Note that md5.h from lib/util/public will conflict with md5.h from
# lib/misc/public.  For now they are identical, so this isn't a problem.
# The md5 functionality should be moved from lib lib/misc to lib/util.

include $(MKFILES_ROOT)/qtargets.mk

