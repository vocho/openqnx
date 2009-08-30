ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/lib
define PINFO
PINFO DESCRIPTION = Run time backtrace library
endef

include $(MKFILES_ROOT)/qtargets.mk

CCFLAGS+=-Werror
CCFLAGS+=$(if $(filter light,$(VARIANT_LIST)),-D_BT_LIGHT)

# get_backtrace.c is special.  It should not be compiled with little
# or no optimisation, because bt_get_backtrace must be able to trace
# back out of itself, and backtracing doesn't like optimisation.
get_backtrace.o : override CCFLAGS+=-O0

