ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Kernel Debugger
endef

LIBS = kdutil

#Only used when compiling w/ watcom
#DEFCOMPILER_TYPE_x86=wcc
ASMOFF_FORMAT_x86 = cpp
ASSEMBLER_TYPE=gcc

DEFFILE=asmoff.def
USEFILE=
NAME=$(SECTION)_$(PROJECT)
EXTRA_SILENT_VARIANTS=$(SECTION)
INSTALLDIR=boot/sys
LINKER_TYPE=BOOTSTRAP
EXTRA_SRCVPATH=$(PROJECT_ROOT)/$(CPU)
EXTRA_CLEAN=asmoff.def

CCFLAGS_x86 += -D_PADDR_BITS=64

CCFLAGS += -DMD_$(CPU) $(CCFLAGS_$(CPU))

include $(MKFILES_ROOT)/qtargets.mk

# Can't add usage information to relocatable objects
define ADD_USAGE
endef

#
# This particular little kludge is to stop GCC from using F.P. instructions
# to move 8 byte quantities around. 
#
CC_nto_ppc_gcc += -msoft-float
CC_nto_ppc_gcc_qcc += -Wc,-msoft-float
