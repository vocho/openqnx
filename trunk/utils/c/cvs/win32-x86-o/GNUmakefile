ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

AM_CFLAGS=-D"SYSTEM_INITIALIZE(a,b)={extern(_fmode);_fmode=_O_BINARY;}"
include $(MKFILES_ROOT)/qmake-cfg.mk
