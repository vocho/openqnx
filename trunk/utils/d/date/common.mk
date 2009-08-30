ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Display or set the date and time
endef

LIBS+=qnx43 util 
USEFILE=$(PROJECT_ROOT)/$(NAME).c

LINKS_qnx4 = clock
LINKS = $(LINKS_$(OS))

include $(MKFILES_ROOT)/qtargets.mk

LD_qnx4_x86_wcc += -T0
