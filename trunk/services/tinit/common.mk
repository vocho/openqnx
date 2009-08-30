ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Terminal initialization
endef

INSTALLDIR = sbin

define PINFO
PINFO DESCRIPTION=terminal initialization program
endef

USEFILE=$(PROJECT_ROOT)/tinit.c

include $(MKFILES_ROOT)/qtargets.mk
