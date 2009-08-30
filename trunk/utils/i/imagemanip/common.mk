ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Utility for processing files
endef

USEFILE=$(PROJECT_ROOT)/imgmanip.use

LIBS +=

include $(MKFILES_ROOT)/qtargets.mk
