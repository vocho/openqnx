ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Utility to display tracelogger output
endef

LIBS=traceparser compat

USEFILE=$(PROJECT_ROOT)/traceprinter.c

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
