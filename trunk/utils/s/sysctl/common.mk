# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.

# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.


ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_qnx4=usr/ucb
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) sbin)

define PINFO
PINFO DESCRIPTION=Get or set socket manager operating parameters
endef

CCF_qnx4 =-Dunix -zp1 -DINET_ONLY -DLITTLE_ENDIAN=1234 -DBYTE_ORDER=LITTLE_ENDIAN -g -DEXTERN=extern -DLONGLONG=__int64 -v11.0b -D_POSIX_SOURCE -fi=unix.h 
CCF_nto = -DINET6 -DIPSEC 
CCFLAGS += $(CCF_$(OS)) 


#INCVPATH += $(PROJECT_ROOT)/../include/

#LIB_VARIANT=a
#EXTRA_LIBVPATH += $(PROJECT_ROOT)/../lib/misc/$(OS)/$(CPU)/$(LIB_VARIANT)
#EXTRA_LIBVPATH += $(PROJECT_ROOT)/../lib/socket/$(OS)/$(CPU)/$(LIB_VARIANT)

LIBS += misc socket

EXCLUDE_OBJS = pathconf.o

include $(MKFILES_ROOT)/qtargets.mk
