ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_nto = bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

define PINFO
PINFO DESCRIPTION=print text to stdout
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
