ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

MFILESDIR	=	/usr/share/misc
MAGIC		=	$(MFILESDIR)/magic

INSTALLDIR = usr/bin

CCFLAGS += -DHAVE_CONFIG_H -DQUICK -DBUILTIN_ELF -DELFCORE 

define PINFO
PINFO DESCRIPTION=determine the type of file
endef

LIBS=z

EXTRA_DEPS = \
	$(PROJECT_ROOT)/config$(MFILESDIR)/magic.mime \
	$(PROJECT_ROOT)/config$(MFILESDIR)/magic 

#	$(PROJECT_ROOT)/config$(MFILESDIR)/magic.mgc

include $(MKFILES_ROOT)/qtargets.mk

MAGIC_FILES :=	$(wildcard $(PROJECT_ROOT)/magic/magdir/[0-9a-z]*)

$(PROJECT_ROOT)/config$(MFILESDIR)/magic.mime: $(PROJECT_ROOT)/magic/magic.mime
		$(CP_HOST) $< $@

$(PROJECT_ROOT)/config$(MFILESDIR)/magic: $(MAGIC_FILES) $(PROJECT_ROOT)/config$(MFILESDIR)/magic.mime
		cat $(MAGIC_FILES) >$@

$(PROJECT_ROOT)/config$(MFILESDIR)/magic.mgc: ./mkmagic
		./mkmagic $(PROJECT_ROOT)/magdir/magic
		$(CP_HOST) $(PROJECT_ROOT)/magdir/magic.mgc $@

./mkmagic: $(PROJECT_ROOT)/apprentice.c $(PROJECT_ROOT)/print.c
		$(CL_HOST) -DHAVE_CONFIG_H -DQUICK -DCOMPILE_ONLY -I$(PROJECT_ROOT) -o $@ $^

EXTRA_CLEAN := mkmagic $(PROJECT_ROOT)/config$(MFILESDIR)/*

