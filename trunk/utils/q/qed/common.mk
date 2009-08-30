ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=QNX fullscreen Editor
endef

INSTALLDIR_qnx4 = usr/bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

LIBS=qnxterm
CCFLAGS_gcc = -Wc,-funsigned-char
CCFLAGS = $(CCFLAGS_$(COMPILER_TYPE))

include $(MKFILES_ROOT)/qtargets.mk

