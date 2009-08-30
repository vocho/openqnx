ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=compresses files
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use
LINKS=melt fcat

include $(MKFILES_ROOT)/qtargets.mk

