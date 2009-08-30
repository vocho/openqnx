ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_qnx4=lib
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib)

define PINFO
PINFO DESCRIPTION = Internal Utility Library
endef

# The integer type definitions are needed by the md5 code
CCFLAGS += -DELF_TARGET_ALL -Du_int32_t=uint32_t -Du_int16_t=uint16_t -Du_int8_t=uint8_t -D'_DIAGASSERT(x)=((void)0)'

# Note that md5.h from lib/util/public will conflict with md5.h from 
# lib/misc/public.  For now they are identical, so this isn't a problem.  
# The md5 functionality should be moved from lib lib/misc to lib/util.

EXTRA_INCVPATH+=$(PROJECT_ROOT)/h $(PROJECT_ROOT)/public/util
include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
