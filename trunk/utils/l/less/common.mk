ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=paginate files
endef

LINKS_nto = more
LINKS = $(LINKS_$(OS))

USEFILE=$(PROJECT_ROOT)/$(NAME).use

LIBS_wcc = unix3r
LIBS=ncurses $(LIBS_$(COMPILER_TYPE))

include $(MKFILES_ROOT)/qtargets.mk
