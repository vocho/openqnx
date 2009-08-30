#
# Copyright 2004, QNX Software Systems Ltd. All Rights Reserved.
#
# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.
#
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

EXTRA_INCVPATH_nto=$(PRODUCT_ROOT)/c/public
EXTRA_INCVPATH=$(EXTRA_INCVPATH_$(OS))

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

#
# Major kludge to get things to build
#
CCFLAGS_qnx4= -D_Uint64t=long -D_Int64t=long
CCFLAGS+= $(CCFLAGS_$(OS))

CCFLAGS+= -DELF_TARGET_ALL

define PINFO
PINFO DESCRIPTION = Library for querying/manipulating ELF files
endef

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
