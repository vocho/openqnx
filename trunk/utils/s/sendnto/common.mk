ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Send a boot image to an embedded device
endef

INSTALLDIR_nto = bin
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

LIBS_qnx4=socket
LIBS_nto=socket
# Can't add ws2_32 due to makefile bug: LIBS_win32=compat ws2_32
LIBS_win32=compat
LDOPTS_win32=-lws2_32
LIBS_solaris=posix4
LIBS+=$(LIBS_$(OS))
CCFLAGS+= -DELF_TARGET_ALL
LDOPTS+=$(LDOPTS_$(OS))

USEFILE=$(PROJECT_ROOT)/sendnto.c

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

WIN32_ENVIRON=mingw
