ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
include $(MKFILES_ROOT)/qmacros.mk
-include $(PROJECT_ROOT)/roots.mk

USEFILE=$(PROJECT_ROOT)/seed_hsr.c

INSTALLDIR = sbin

include $(PROJECT_ROOT)/pinfo.mk

include $(MKFILES_ROOT)/qtargets.mk

