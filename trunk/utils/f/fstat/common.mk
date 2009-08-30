ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=$(PROJECT_ROOT)/$(NAME).c

define PINFO
PINFO DESCRIPTION=Show file\'s stat structure
endef

include $(MKFILES_ROOT)/qtargets.mk
