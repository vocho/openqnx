ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO NAME=cron
PINFO DESCRIPTION=Clock server
endef

INSTALLDIR = usr/sbin

define PINFO
PINFO DESCRIPTION=cron scheduling daemon
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use

include $(MKFILES_ROOT)/qtargets.mk

