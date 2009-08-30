ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=script
endef


USEFILE=$(PROJECT_ROOT)/$(NAME).use

include $(MKFILES_ROOT)/qtargets.mk

