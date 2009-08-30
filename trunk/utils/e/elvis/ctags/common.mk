ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=ctags
endef

EXTRA_SRCVPATH=../../../../common

include $(MKFILES_ROOT)/qtargets.mk
