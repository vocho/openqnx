ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Mount a filesystem
endef

include $(MKFILES_ROOT)/qtargets.mk
