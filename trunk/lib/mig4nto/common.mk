ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/lib

define PINFO
PINFO DESCRIPTION=Provides cover functions for many QNX 4 functions that are not available in QNX Neutrino. 
endef

SO_VERSION = 2

EXTRA_INCVPATH=$(PRODUCT_ROOT)

include $(MKFILES_ROOT)/qtargets.mk
