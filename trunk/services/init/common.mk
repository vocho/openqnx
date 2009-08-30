ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=sbin

define PINFO
PINFO DESCRIPTION=Process control initialization
endef

include $(MKFILES_ROOT)/qtargets.mk

CCFLAGS+=-Dlint -D__aconst=
