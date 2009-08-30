#
# This flag sets the threshold for short data to 256, so we can stuff  as much as we can in there.
#
CC_nto_mips_gcc += -G256
AS_nto_mips_gcc += -G256
CC_nto_mips_gcc_qcc += -Wcal,-G256
AS_nto_mips_gcc_qcc += -Wcal,-G256

CCFLAGS += -D_PADDR_BITS=64

#
# enable TX79 context structures
#
CCVFLAG_tx79 = -DTX79_SUPPORT
