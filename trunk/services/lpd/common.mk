# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.

# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.


ifndef	QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR_qnx4=usr/usb
INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/bin)

NAME=$(SECTION)
EXTRA_SILENT_VARIANTS+=$(SECTION)
USEFILE=$(SECTION_ROOT)/$(NAME).use

PDESC_lpd=BSD line printer spooler daemon
PDESC_lpr=BSD spool to line printer command
PDESC_lprc=BSD line printer control program
PDESC_lprq=BSD line printer spool queue querying program
PDESC_lprrm=BSD line printer spooler job removal program

define PINFO
PINFO DESCRIPTION=$(PDESC_$(NAME))
endef

EXTRA_INCVPATH=$(PROJECT_ROOT)/common_source

LIBS = socket

include $(MKFILES_ROOT)/qmacros.mk

#
# Do this after the include of qmacros so that not all files are brought
# in.
#
SRCVPATH += $(PROJECT_ROOT)/common_source

include $(SECTION_ROOT)/pinfo.mk

include $(MKFILES_ROOT)/qtargets.mk
