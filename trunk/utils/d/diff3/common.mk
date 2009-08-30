ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=Show differences among three files
endef

include $(MKFILES_ROOT)/qtargets.mk

