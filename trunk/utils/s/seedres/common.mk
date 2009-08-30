ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

USEFILE=$(PROJECT_ROOT)/seedres.use

INSTALLDIR = sbin

#CCFLAGS += -O0 -g
LDFLAGS += -M

include $(MKFILES_ROOT)/qmacros.mk
include $(PROJECT_ROOT)/pinfo.mk
include $(MKFILES_ROOT)/qtargets.mk

