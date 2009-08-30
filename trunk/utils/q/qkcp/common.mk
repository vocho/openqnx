ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=quick copy utility using DirectIO method
endef

USEFILE=$(PROJECT_ROOT)/qkcp.use
CCFLAGS+=-w9

INSTALLDIR=usr/bin

include $(MKFILES_ROOT)/qtargets.mk
