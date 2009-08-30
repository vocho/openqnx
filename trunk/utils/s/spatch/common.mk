ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=full screen patch utility
endef

LIBS=qnxterm qnx43 util
USEFILE=$(PROJECT_ROOT)/$(NAME).c
CCFLAGS_gcc = -Wc,-funsigned-char
CCFLAGS = $(CCFLAGS_$(COMPILER_TYPE))

include $(MKFILES_ROOT)/qtargets.mk


