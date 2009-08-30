ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION = Shutdown library, a library that implements core system shutdown functionality
endef

include $(MKFILES_ROOT)/qtargets.mk
