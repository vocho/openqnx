ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Internal library for shared routines in kdebug/kdumper
endef

DEFFILE = asmoff.def

include $(MKFILES_ROOT)/qtargets.mk

#
# This particular little kludge is to stop GCC from using F.P. instructions
# to move 8 byte quantities around. 
#
CC_nto_ppc_gcc += -msoft-float
CC_nto_ppc_gcc_qcc += -Wc,-msoft-float
