ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=test that ELF binaries are functionally identical
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).use

CCFLAGS_qnx4 =-D__LITTLEENDIAN__

CCVFLAG_a=-DMAKE_LIB
CCVFLAG_so=-DMAKE_LIB
CCVFLAG_dll=-DMAKE_LIB
LIBS=elf
LIBS_win32=compat
LIBS+=$(LIBS_$(OS))


include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

CCFLAGS_nto=
WIN32_ENVIRON=mingw
