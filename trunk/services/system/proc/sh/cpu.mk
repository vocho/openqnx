# List of files that we want to compile at specific 
# optimization levels.

# Default for ker is O2
CCFLAGS_ker_sh += -O2

# If I need to specify the other bits to be a certain level
CCFLAGS_memmgr_sh += -O1
CCFLAGS_pathmgr_sh += -O1
CCFLAGS_proc_sh +=  -O1
CCFLAGS_procmgr_sh += -O1

# Certain files don't follow the general rules
CCFLAGS_ker_timer_sh += -O1
CCFLAGS_pathmgr_init_sh += -O2
CCFLAGS_pathmgr_link_sh += -O2
CCFLAGS_pathmgr_node_sh += -O2
CCFLAGS_pathmgr_object_sh += -O2
CCFLAGS_pathmgr_open_sh += -O2

# For SH, we need to pick up a nofpu version of libc
LDPOST += -lc-nofpu

# Make sure the compiler doesn't generate any FPU code when compiling for SH.
# We do this for 4.x versions of the compiler only.  (Once 6.4.0 ships and
# the 2.95.3 compiler isn't supported any more, we can remove the ifneq test
# and always do the CCFLAGS+=)
ifneq ($(filter 4.%, $(GCC_VERSION)),)
CCFLAGS += -m4-nofpu
endif


