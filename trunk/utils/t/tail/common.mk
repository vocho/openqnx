ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=print last lines of a file
endef

LIBS_qnx4 = qnx43
LIBS_win32=compat

LIBS+=util $(LIBS_$(OS))
USEFILE=$(PROJECT_ROOT)/$(NAME).use

LDPOST_qnx4_x86_wcc += -N32K

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
