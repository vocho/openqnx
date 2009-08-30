ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_nto = bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

define PINFO
PINFO NAME=split
PINFO DESCRIPTION=split a text file into smaller files
endef

#USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS=compat
LIBS_win32 = regex

LIBS+=$(LIBS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
