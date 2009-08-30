ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_win32 = usr/bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) bin)

define PINFO
PINFO DESCRIPTION=Remove files
endef

LIBS_qnx4 = util 
LIBS_win32 = compat
LIBS = $(LIBS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
