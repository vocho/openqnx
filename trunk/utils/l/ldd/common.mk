ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=Print shared libraries required by program
endef

include $(MKFILES_ROOT)/qtargets.mk

VFLAG_g=-gstabs+
