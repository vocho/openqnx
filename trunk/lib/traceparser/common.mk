ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/lib

define PINFO
PINFO DESCRIPTION=Library for parsing tracelogger output
endef

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw

