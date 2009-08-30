ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = bin

define PINFO
PINFO DESCRIPTION=copy files
endef

LIBS_qnx4=qnx43

LIBS_win32=compat

LIBS+=util $(LIBS_$(OS))
USEFILE=$(PROJECT_ROOT)/$(NAME).c

include $(MKFILES_ROOT)/qtargets.mk

