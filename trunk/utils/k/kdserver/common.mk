ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="Kernel Dump Server (for use with GDB)"
endef

USEFILE=$(PROJECT_ROOT)/main.c

include $(MKFILES_ROOT)/qmacros.mk

CPU_FILES:=$(foreach f,$(SRCS),$(if $(findstring /cpu_,$(f)),$(f)))

include $(MKFILES_ROOT)/qtargets.mk
include $(MKFILES_ROOT)/ntoxdev.mk

avail_cpus.gh: $(CPU_FILES) $(PROJECT_ROOT)/common.mk
	$(ECHO_HOST) $(patsubst cpu_%, "CPU(%)", $(basename $(notdir $(CPU_FILES)))) >avail_cpus.gh

avail_cpus.o: avail_cpus.c avail_cpus.gh	

main.o: main.c kdserver.h	

WIN32_ENVIRON=mingw
