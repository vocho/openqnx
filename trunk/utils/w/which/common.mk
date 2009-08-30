ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Locate a program file
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS_win32=compat 
LIBS += $(LIBS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
