ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/sbin

define PINFO
PINFO DESCRIPTION=system logging utility
endef

CCFLAGS=-DUSE_TERMIO -DDIAGNOSTICS

include $(MKFILES_ROOT)/qtargets.mk
