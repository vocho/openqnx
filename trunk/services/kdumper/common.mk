ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Kernel Dumper
endef

LIBS = kdutil z

ASMOFF_FORMAT_x86 = cpp
ASSEMBLER_TYPE=gcc

DEFFILE=asmoff.def
USEFILE=
INSTALLDIR=boot/sys
LINKER_TYPE=BOOTSTRAP
EXTRA_CLEAN=asmoff.def

CCFLAGS_ppc+=-D_PADDR_BITS=64
CCFLAGS += $(CCFLAGS_$(CPU))

# Sneak into procnto's source tree so that we can get at the physical
# allocator structures.
EXTRA_INCVPATH = $(PRODUCT_ROOT)/system/memmgr	

EXTRA_CLEAN = *.gh

include $(MKFILES_ROOT)/qmacros.mk

# include procnto's cpu.mk so that this code matches the same size of
# paddr_t that proc is using.	

#always use gcc
NOWATCOM=1				
#use ldbootstrap/ldrel on X86 to work around mkifs bug
NOMODULE_SUPPORT=1		
-include $(PRODUCT_ROOT)/system/proc/$(CPU)/cpu.mk	

WRITER_FILES:=$(foreach f,$(SRCS),$(if $(findstring /write_,$(f)),$(f)))

include $(MKFILES_ROOT)/qtargets.mk

# Can't add usage information to relocatable objects
define ADD_USAGE
endef

# Make sure this writer is at the start of the list, so it's the default
DEFAULT_WRITER = write_uuencode

avail_writers.gh: $(WRITER_FILES) $(PROJECT_ROOT)/common.mk
	$(ECHO_HOST) $(patsubst write_%, "WRITER(%)", $(DEFAULT_WRITER) $(filter-out $(DEFAULT_WRITER), $(basename $(notdir $(WRITER_FILES))))) >avail_writers.gh

avail_writers.o: avail_writers.c avail_writers.gh	
