ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Change the login password or create new user names
endef

#USEFILE=$(PROJECT_ROOT)/$(OS)/$(NAME).use

LIBS_qnx4=login unix3r
LIBS_nto=login
LIBS=$(LIBS_$(OS))

CCFLAGS_qnx4=-DBROKEN_CHROOT

FILE_INFO = 0 0 4775

include $(MKFILES_ROOT)/qtargets.mk

