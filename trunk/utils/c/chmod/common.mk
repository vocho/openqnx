ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO NAME=chmod
PINFO DESCRIPTION=change file permissions
endef

INSTALLDIR = bin

define PINFO
PINFO DESCRIPTION=change file permissions
endef

#USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

