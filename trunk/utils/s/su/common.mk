ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Switch user ID
endef

LIBS_qnx4=unix3r
LIBS=$(LIBS_$(OS)) login

FILE_INFO = 0 0 4775

include $(MKFILES_ROOT)/qtargets.mk

