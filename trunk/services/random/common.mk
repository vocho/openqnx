ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/sbin

define PINFO
PINFO DESCRIPTION= /dev/random Service
endef

# CCFLAGS += -O2

LIBS=m z

USEFILE=$(PROJECT_ROOT)/random.use

include $(MKFILES_ROOT)/qtargets.mk
