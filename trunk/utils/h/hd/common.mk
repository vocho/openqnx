ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=Display files in decimal, hex, octal, or ASCII
endef

LINKS = od

LIBS_win32 = compat
LIBS += $(LIBS_$(OS))

USEFILE=$(PROJECT_ROOT)/$(NAME).use

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
