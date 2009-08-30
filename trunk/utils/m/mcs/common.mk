ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=Manage comment sections
endef

LIBS = elf


include $(MKFILES_ROOT)/qtargets.mk

