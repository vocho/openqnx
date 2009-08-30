ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

INSTALLDIR=bin


define PINFO
PINFO DESCRIPTION=Embedded shell
endef

USEFILE=$(PROJECT_ROOT)/esh.c

EXTRA_CCDEPS = esh.c

SHELLS:=$(basename $(notdir $(wildcard ../../../*.c)))

ICLEAN=$(SHELLS)
ALL_DEPENDENCIES=$(SHELLS)

include $(MKFILES_ROOT)/qtargets.mk

#
# It has been decided that 'sh' should link to 'ksh'
#
#LINKS_nto = sh

define TARGET_INSTALL
	-$(foreach bin,$(SHELLS), $(CP_HOST) $(bin) $(INSTALL_ROOT_EX)/bin/$(bin)$(VARIANT_TAG);)
	-$(foreach link, $(LINKS_$(OS)), $(LN_HOST) $(IMAGE_PREF_$(BUILD_TYPE))esh$(VARIANT_TAG)$(IMAGE_SUFF_$(BUILD_TYPE)) $(INSTALL_DIRECTORY)/$(IMAGE_PREF_$(BUILD_TYPE))$(link)$(VARIANT_TAG)$(IMAGE_SUFF_$(BUILD_TYPE));)
endef

INSTALLNAME=$(INSTALL_DIRECTORY)/$(IMAGE_PREF_$(BUILD_TYPE))$(@)$(VARIANT_TAG)$(IMAGE_SUFF_$(BUILD_TYPE))$(VERSION_TAG_$(BUILD_TYPE))

ifneq ($(filter socketpair, $(VARIANTS)),)
#
# this variant is for internal use only
#
ADD_PINFO=true
ADD_USAGE=true
LIBS+=socket
endif

$(SHELLS): %: %.o 
	$(TARGET_BUILD)
