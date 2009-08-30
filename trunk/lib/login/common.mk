ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Support library for login
endef

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

EXTRA_SRCVPATH+=$(PROJECT_ROOT)/c
EXTRA_INCVPATH+=$(PROJECT_ROOT)/h
include $(MKFILES_ROOT)/qtargets.mk
