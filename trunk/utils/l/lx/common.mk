ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=show extents used by file
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c
LIBS+=util

CCFLAGS_gcc = -Wc,-funsigned-char
CCFLAGS = $(CCFLAGS_$(COMPILER_TYPE))

include $(MKFILES_ROOT)/qtargets.mk


