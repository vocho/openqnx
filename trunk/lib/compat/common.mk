ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

PUBLIC_INCVPATH = $(PROJECT_ROOT)/public $(wildcard $(PROJECT_ROOT)/public_$(OS)_$(HOST_SYSTEM))

define PINFO
PINFO DESCRIPTION = Internal Utility Library
endef

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw

