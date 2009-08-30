ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Set or get date from realtime clock
endef

include $(MKFILES_ROOT)/qtargets.mk
-include $(PROJECT_ROOT)/roots.mk

INSTALLDIR=sbin

EXTRA_INCVPATH = $(PROJECT_ROOT)/../../../lib/util/public
EXTRA_INCVPATH += $(PROJECT_ROOT)/../../../hardware/startup/lib/public

DEFLIB_VARIANT = $(subst $(space),.,$(strip a $(filter wcc be le, $(VARIANTS))))
LIB_VARIANT = $(firstword $(subst ./,,$(dir $(bind))) $(DEFLIB_VARIANT))
EXTRA_LIBVPATH += $(PROJECT_ROOT)/../../../lib/util/$(OS)/$(CPU)/$(LIB_VARIANT)
LIBS+=util

USEFILE=$(PROJECT_ROOT)/$(NAME).use

LD_qnx4_x86_wcc += -T0
