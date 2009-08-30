ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=print use message
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS_win32=compat
LIBS=$(LIBS_$(OS))

CCFLAGS+=-DELF_TARGET_ALL

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
