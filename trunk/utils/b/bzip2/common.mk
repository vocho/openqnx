ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=bzip2 compression utility
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use
LINKS=bunzip2 bz2cat

include $(MKFILES_ROOT)/qtargets.mk

