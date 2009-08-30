ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=search text for regular expressions
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

LINKS=fgrep egrep

LIBS_win32=regex compat
LIBS+=$(LIBS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw