ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_win32 = usr/bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) bin)

define PINFO
PINFO DESCRIPTION=posix pax utility
endef

NAME=pax

LIBS_win32=compat
LIBS=$(LIBS_$(OS))

CCFLAGS+=-D_POSIX_SOURCE
LINKS = cpio

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
