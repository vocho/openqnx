ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=QNX 4 to QNX Neutrino migration kit process manager.
endef

EXTRA_INCVPATH=$(PRODUCT_ROOT) $(PRODUCT_ROOT)/mig4nto/public
LIBS = mig4nto

USEFILE=$(PROJECT_ROOT)/mig4nto-procmgr.use

include $(MKFILES_ROOT)/qtargets.mk
