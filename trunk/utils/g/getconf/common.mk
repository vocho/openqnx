ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=get/set configuration information
endef

LINKS = setconf

include $(MKFILES_ROOT)/qtargets.mk

