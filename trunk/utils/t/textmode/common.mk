ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin
include $(MKFILES_ROOT)/qmacros.mk
define PINFO
PINFO DESCRIPTION=Used to try to switch back to text mode using Video BIOS and unlock the console 
endef

LIBS=disputil

#EXTRA_INCVPATH=$(USE_ROOT_nto)/usr/include/graphics

include $(MKFILES_ROOT)/qtargets.mk

