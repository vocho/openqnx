ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Sort, merge, or sequence-check text files
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use

LIBS_win32 = compat

LIBS += $(LIBS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
