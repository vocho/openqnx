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

INSTALLDIR=usr/lib
define PINFO
PINFO DESCRIPTION=Ncurses lib
endef

LDFLAGS_ncursespp_qnx4 =-v11.0
CCFLAGS_ncursespp_qnx4 =-v11.0

LDFLAGS += $(LDFLAGS_$(subst ++,pp,$(SECTION))_$(OS))
CCFLAGS += $(CCFLAGS_$(subst ++,pp,$(SECTION))_$(OS))

CCFLAGS+=-DHAVE_CONFIG_H
NAME=$(SECTION)
EXTRA_SILENT_VARIANTS+=$(SECTION)

PUBLIC_FLIST = 	\
	form.h		\
	eti.h		\
	menu.h		\
	curses.h	\
	term.h		\
	unctrl.h	\
	cursesapp.h	\
	cursesf.h	\
	cursesm.h	\
	cursesp.h	\
	cursesw.h	\
	cursslk.h	\
	etip.h		\
	panel.h

include $(MKFILES_ROOT)/qmacros.mk

#
# Doesn't work as a VARIANT define
#
VARIANT_LIST:=$(filter-out ncurses++, $(VARIANT_LIST))

PUBLIC_INCVPATH=$(PROJECT_ROOT)/include

define TARGET_HINSTALL
	$(PRE_HINSTALL)
	@-$(foreach file, $(PUBLIC_FLIST), $(CP_HOST) $(PUBLIC_INCVPATH)/$(file) $(INSTALL_ROOT_HDR)/$(file);)
	$(POST_HINSTALL)
endef

include $(MKFILES_ROOT)/qtargets.mk
