ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=gaping security hole
endef

include $(MKFILES_ROOT)/qtargets.mk

