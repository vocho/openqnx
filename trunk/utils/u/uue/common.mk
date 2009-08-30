ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=uue
endef

USEFILE=$(PROJECT_ROOT)/$(NAME).c

LIBS=		socket
LIBPREF_socket=	-Bstatic

LINKS=	uuencode

include $(MKFILES_ROOT)/qtargets.mk

