ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=get the base filename from a path
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw