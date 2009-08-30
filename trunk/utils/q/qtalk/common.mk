ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Talk over a communications line
endef

LIBS+=util qnx43
USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

