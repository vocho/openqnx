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
# $Id: common.mk 153052 2008-08-13 01:17:50Z coreos $

ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=curses applications
endef

INSTALLDIR=usr/bin

CCFLAGS+=-DHAVE_CONFIG_H
EXTRA_INCVPATH=$(PROJECT_ROOT) $(PRODUCT_ROOT)/include $(wildcard $(PRODUCT_ROOT)/public_*)

LIB_VARIANT_qnx4=a.3r
LIB_VARIANT_nto=a
EXTRA_LIBVPATH=$(foreach i, ncurses form panel menu ncurses++, $(PRODUCT_ROOT)/$i/$(OS)/$(CPU)/$(LIB_VARIANT_$(OS)))
LIBS_qnx4=ncurses3r form3r panel3r menu3r ncurses++3r
LIBS_nto=ncurses form panel menu ncurses++

LIBS_qnx4 = ncurses3r unix3r
LIBS_nto  = ncurses
LIBS=$(LIBS_$(OS))

EXCLUDE_OBJS_clear = dump_entry.o
EXCLUDE_OBJS_tput = dump_entry.o
EXCLUDE_OBJS = termsort.o $(EXCLUDE_OBJS_$(SECTION))

NAME=$(SECTION)
EXTRA_SILENT_VARIANTS+=$(SECTION)
USEFILE=$(PROJECT_ROOT)/$(SECTION)/$(NAME).use

include $(MKFILES_ROOT)/qtargets.mk
include $(PROJECT_ROOT)/$(SECTION)/pinfo.mk
