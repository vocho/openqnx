ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = usr/bin

define PINFO
PINFO DESCRIPTION=edit the cron schedules
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use

FILE_INFO=0 0 4775 

include $(MKFILES_ROOT)/qtargets.mk

