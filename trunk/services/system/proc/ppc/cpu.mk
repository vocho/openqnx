#
# This particular little kludge is to stop GCC from using F.P. instructions
# to move 8 byte quantities around. ktest has to be compiled with F.P.
# enabled so that it can call printf.
#
CC_nto_ppc_gcc += -msoft-float
CC_nto_ppc_gcc_qcc += -Wc,-msoft-float
FLAGS_ktest_ppc = -Wc,-mhard-float -Wc,-G0

#
# Use short data (off r13) on PPC, and set threshold to 256 bytes
#
CC_nto_ppc_gcc += -msdata=sysv -G256
AS_nto_ppc_gcc += -msdata=sysv -G256 -Wa,-maltivec
CC_nto_ppc_gcc_qcc += -Wcl,-msdata=sysv -Wcl,-G256
AS_nto_ppc_gcc_qcc += -Wcl,-msdata=sysv -Wcl,-G256 -Wa,-maltivec

#
# Set the current PPC family that we're compiling.
#
FAMILY_NAMES = 400 600 800 booke 900

PWD := $(shell $(PWD_HOST))
CURRENT_FAMILY := $(filter $(FAMILY_NAMES), $(subst ., ,$(notdir $(PWD))))

CCFLAGS_600 = -D_PADDR_BITS=64
CCFLAGS_booke = -D_PADDR_BITS=64
CCFLAGS_900 = -D_PADDR_BITS=64

CCFLAGS += -DPPC_FAMILY_MINE=PPC_FAMILY_$(CURRENT_FAMILY) $(CCFLAGS_$(CURRENT_FAMILY))

EXTRA_DIR_ker_600=600-900
EXTRA_DIR_ker_900=600-900
EXTRA_DIR_memmgr_600=600-900
EXTRA_DIR_memmgr_900=600-900

# The "$(1)" macro is being expanded in the module.mk file and is the
# current module name being built
SRC_VARIANTS += $(EXTRA_DIR_$(1)_$(CURRENT_FAMILY))

CPU_TYPE_600=600/700/7000
CPU_TYPE=ppc $(if $(CPU_TYPE_$(CURRENT_FAMILY)), $(CPU_TYPE_$(CURRENT_FAMILY)), $(CURRENT_FAMILY)) family
