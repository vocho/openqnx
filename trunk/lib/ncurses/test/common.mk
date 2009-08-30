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
ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

CCFLAGS+=-DHAVE_CONFIG_H
NAME=$(SECTION)
EXTRA_INCVPATH=$(PROJECT_ROOT) $(PROJECT_ROOT)/../include $(wildcard $(PROJECT_ROOT)/../public_*)

LIB_VARIANT_qnx4=a.3r
LIB_VARIANT_nto=a
EXTRA_LIBVPATH=$(foreach i, ncurses form panel menu ncurses++, $(PROJECT_ROOT)/../$i/$(OS)/$(CPU)/$(LIB_VARIANT_$(OS)))
LIBS_qnx4=ncurses3r form3r panel3r menu3r ncurses++3r
LIBS_nto=ncurses form panel menu

LIBS=$(LIBS_$(OS))

USEFILE=$(PROJECT_ROOT)/$(SECTION)/$(NAME).use
INSTALLDIR=/dev/null

include $(MKFILES_ROOT)/qtargets.mk
