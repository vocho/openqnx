ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR = bin

define PINFO
PINFO DESCRIPTION=change file ownership
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c
LIBS+=util

LINKS=chgrp

include $(MKFILES_ROOT)/qtargets.mk

