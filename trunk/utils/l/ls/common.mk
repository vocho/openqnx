ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
include $(MKFILES_ROOT)/ntoxdev.mk
LIBS_win32=compat

LIBS+=util
USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS+=$(LIBS_$(OS))

LDPOST_qnx4_x86_wcc += -N128K

define PINFO
PINFO DESCRIPTION=List directory contents \(POSIX\)
endef

include $(MKFILES_ROOT)/qtargets.mk

WIN32_ENVIRON=mingw
