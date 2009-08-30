ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Change to a new group
endef

#
#  If you type "id" and your group id does not appear in the supplementary
#  group list, then you should compile "newgrp" with "-DFRUGAL" in the
#  in the CFLAGS
#
#  If you don't want "newgrp" to enter events in the syslog, compile
#  "newgrp" with "-DDONTUSESYSLOG" in the CFLAGS.
#
#  If you support limits, then define USELIMITS.
#
CCFLAGS += -DDONTUSESYSLOG -DFRUGAL

FILE_INFO = 0 0 4775

include $(MKFILES_ROOT)/qtargets.mk
