ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = sbin

define PINFO
PINFO DESCRIPTION=standard unix getty - listens on tty for login
endef

include $(MKFILES_ROOT)/qtargets.mk
