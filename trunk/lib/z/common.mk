
# Copyright 2003-2004, QNX Software Systems Ltd. All Rights Reserved.

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

SO_VERSION = 2

define PINFO
PINFO DESCRIPTION=Compression/decompression library
endef

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

# PR/15688 appears to be a -O1 codegen bug in GCC in "infcodes.c" line 200-201
CCOPTS_nto_infcodes.c=-Wc,-O0
override CCOPTS+=$(CCOPTS_$(OS)_$(<F))

EXCLUDE_OBJS = example.o

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw

