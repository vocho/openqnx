ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=usr/bin

define PINFO
PINFO DESCRIPTION=Identifies areas in programs that need attention during the migration process. 
endef

INSTALLDIR_qnx4 = usr/bin

LDFLAGS += -N8000
LEX=lex
LFLAGS=-i -l
EXTRA_INCVPATH=$(PRODUCT_ROOT)
USEFILE=$(PROJECT_ROOT)/$(NAME).c

POST_INSTALL = $(CP_HOST) $(PROJECT_ROOT)/mig4nto.tab $(INSTALL_ROOT_$(OS))/etc/mig4nto.tab

include $(MKFILES_ROOT)/qtargets.mk


