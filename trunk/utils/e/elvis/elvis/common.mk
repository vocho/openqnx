
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=text editor of the 'vi' persuasion
endef

LINKS_nto = view
LINKS = ex vi $(LINKS_$(OS))

EXTRA_SRCVPATH=../../../../common
LIBS+=socket ncurses

include $(MKFILES_ROOT)/qtargets.mk



