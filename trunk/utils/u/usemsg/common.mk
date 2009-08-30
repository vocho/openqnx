ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=get or set use messages
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS=compat

CCFLAGS=-DELF_TARGET_ALL

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
