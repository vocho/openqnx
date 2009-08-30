ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)


CPU_TYPE=$(CPU)
define PINFO
PINFO DESCRIPTION=Microkernel and process manager for $(CPU_TYPE) processors
endef

INSTALLDIR=boot/sys
LINKER_TYPE=BOOTSTRAP

NAME = procnto
EXTRA_SILENT_VARIANTS+=$(SECTION)

ifndef IDSTAMPER
#IDSTAMPER=date +%Y/%m/%d-%T%Z-$$(id -un)
IDSTAMPER=date +%Y/%m/%d-%T%Z
endif

EXTRA_MODULES_ion=ker/cisco memmgr/cisco pathmgr/cisco proc/cisco
EXTRA_MODULES_iox=ker/cisco memmgr/cisco pathmgr/cisco proc/cisco

ADDON_MODULE_DIRS=ker/aps ker/apmgr memmgr/apm apmgr $(EXTRA_MODULES_$(BUILDENV)) $(EXTRA_MODULES)
MODULE_DIRS=ker memmgr $(ADDON_MODULE_DIRS)

EXCLUDE_OBJS += timestamp.o

.PHONY: first all iclean clean spotless nto install qinstall ninstall hinstall

#force a recompile if the timestamp files are updated
EXTRA_CCDEPS += $(PROJECT_ROOT)/clean.timestamp $(PROJECT_ROOT)/proc/$(CPU)/clean.timestamp
EXTRA_ASDEPS += $(EXTRA_CCDEPS)

DEFFILE = asmoff.def

SRC_VARIANTS = $(filter-out be le o g, $(VARIANTS))

PROC_SECTIONS = $(filter-out CVS doc $(MODULE_DIRS) proc public common.mk cvsignore %.h $(EXCLUDE_SECTIONS), $(notdir $(wildcard $(PROJECT_ROOT)/*)))

EXTRA_SRCVPATH = $(foreach sect, $(PROC_SECTIONS), $(addprefix $(PROJECT_ROOT)/$(sect)/$(CPU)/, $(SRC_VARIANTS)) $(PROJECT_ROOT)/$(sect)/$(CPU) $(PROJECT_ROOT)/$(sect))

INCVPATH=$(PROJECT_ROOT)

include $(MKFILES_ROOT)/qmacros.mk

## NYI Kludge fix - PPC/MIPS/ARM/SH code should be updated to handle this
ifeq ($(filter ppc mips arm sh, $(CPU)),)
CCFLAGS += -DSMP_MSGOPT
ASFLAGS += -DSMP_MSGOPT=1
endif

MODULE_NAMES:=$(sort $(foreach module,$(MODULE_DIRS), $(notdir $(module))))

BUILD_LIST=$(FULLNAME) $(MODULE_LIBS)

first: all

-include ../cpu.mk

ACTUAL_MODULE_NAMES:=

ifdef EVAL_WORKS
# Need fix to $(eval ...) expansion bug propagated before we can use this...
# Need to update DO_MODULE to match current module.mk before enabling...
#define DO_MODULE
#SRCVPATH_$(notdir $(1)):=$(addprefix $(PROJECT_ROOT)/$(1)/$(CPU)/, $(SRC_VARIANTS)) $$(PROJECT_ROOT)/$(1)/$(CPU) $(addprefix $(PROJECT_ROOT)/$(1)/, $(SRC_VARIANTS)) $(PROJECT_ROOT)/$(1)
#METASRCS_$(notdir $(1)) := $$(foreach dir, $$(SRCVPATH_$(notdir $(1))), $$(addprefix $$(dir)/, *.s *.S *.c))
#SRCS_$(notdir $(1)) := $$(wildcard $$(METASRCS_$(notdir $(1))))
#OBJS_$(notdir $(1)) := $$(sort $$(addsuffix .o, $$(filter-out asmoff ktest, $$(basename $$(notdir $$(SRCS_$(notdir $(1))))))))
#
#$(PWD)/libmod_$(notdir $(1)).a : $$(OBJS_$(notdir $(1))) $$(EXTRA_DEPS)
#	$$(RM_HOST) $$@
#	$$(CREATE_AR)
#endef


$(foreach module,$(MODULE_NAMES),$(eval $(call DO_MODULE,$(module))))
else
modules:=$(MODULE_NAMES)
include $(foreach module,$(MODULE_NAMES), $(PROJECT_ROOT)/module.mk)
endif

MODULE_LIBS:=$(foreach module,$(ACTUAL_MODULE_NAMES), $(PWD)/libmod_$(module).a)

SRCVPATH += $(foreach module,$(ACTUAL_MODULE_NAMES), $(SRCVPATH_$(module)))

# check for unresolved symbols in procnto.  Of course, there are always
# exceptions to every rule, and some versions are actually allowed some
# unresolved symbols!
#
# Allowed unresolves, by CPU
# all: module_list_start, module_list_end
# MIPS: _gp
# ARM: _end and _btext
# PPC: _SDA_BASE_ and _SDA2_BASE_

ifeq ($(CHECK_FOR_UNDEFS),)
define CHECK_FOR_UNDEFS
	u=$$(nto$(CPU)-nm -u $@ | grep -Ev '_gp|_end|_btext|_SDA_BASE_|_SDA2_BASE_|module_list_start|module_list_end'); \
      test -z "$$u" || { $(ECHO_HOST)  "Unresolved symbols in $@:"; $(ECHO_HOST) "$$u"; $(RM_HOST) $@; exit 1; }
endef
endif

#
# Lint Configuration
#

FLINTCONFIG_default += $(PROJECT_ROOT)/kernel.lnt
FLINTCONFIG_default += $(PROJECT_ROOT)/kernel-default.lnt

FLINTCONFIG_WO790585 += $(PROJECT_ROOT)/kernel.lnt
FLINTCONFIG_WO790585 += $(PROJECT_ROOT)/kernel-WO790585.lnt

#
# Rules
#

all: $(BUILD_LIST)

include $(MKFILES_ROOT)/qrules.mk

EXCLUDE_OBJS += gcov_begin.o gcov_end.o
ifeq ($(filter gcov, $(VARIANT_LIST)),gcov)
CCFLAGS += -ftest-coverage -fprofile-arcs
GCOV_FIRST=gcov_begin.o
GCOV_LAST=gcov_end.o
endif

ifndef _native_path
# On 6.3.2 or earlier, _native_path isn't defined.
_native_path=$(1)
endif
SRC_SECTION=$(firstword $(subst /,$(space),$(subst $(PROJECT_ROOT)/,,$(call _native_path,$<))))
oldinc:=$(filter-out ., $(INCVPATH))
INCVPATH=. $(addprefix $(PROJECT_ROOT)/$(SRC_SECTION)/, $(addprefix $(CPU)/, $(SRC_VARIANTS)) $(CPU) .) $(oldinc)

CCFLAGS_ion = -DNO_INLINE_BLOCKANDREADY
CCFLAGS_iox = -DNO_INLINE_BLOCKANDREADY -DEVENT_REVIVE_LATER

CCF_gcc_ = -fno-common -fno-strict-aliasing
CCF_gcc_qcc = -Wc,-fno-common -Wc,-fno-strict-aliasing
CCFLAGS += $(CCF_$(COMPILER_TYPE)_$(COMPILER_DRIVER)) \
				$(CCFLAGS_$(SRC_SECTION)_$(CPU))	\
				$(CCFLAGS_$(BUILDENV)) \
				$(CCFLAGS_$(basename $@)_$(CPU)) \
				$(if $(filter $(foreach f, $(ADDON_MODULE_DIRS), $(PROJECT_ROOT)/$(f)/%), $<),-DCOMPILING_MODULE)
CCFLAGS += -D_BASE_FILE_=$(basename $@)

LDF_gcc_qcc = -M
LDFLAGS += timestamp.o $(LDF_$(COMPILER_TYPE)_$(COMPILER_DRIVER)) \
				$(LDFLAGS_$(SRC_SECTION)_$(CPU))	\
				$(LDFLAGS_$(basename $@)_$(CPU))

#
# Include nto.h into timestamp and compile with -g; this allows the
# kernel symbol file to contain enough debug info on the main kernel data
# structures for hardware tool vendors to provide OS awareness. On
# the X86, force the compiler type to be "gcc" so that we have a consistant
# debugging format for all platforms. 
#
TIMESTAMP_CCPREF=$(CCPREF)
TIMESTAMP_COMPILER_TYPE=$(COMPILER_TYPE)
ifeq ($(CPU)$(COMPILER_TYPE)$(filter g, $(VARIANTS)),x86wcc)
TIMESTAMP_CCPREF=qcc -Vgcc_ntox86 -c
TIMESTAMP_COMPILER_TYPE=gcc
endif

ifneq ($(filter 6.4.%, $(VERSION_REL)),)
ifeq ($(TIMESTAMP_COMPILER_TYPE),gcc)
TIMESTAMP_CCPREF+=-fno-eliminate-unused-debug-types -fno-eliminate-unused-debug-symbols
endif
endif

$(FULLNAME): $(GCOV_FIRST) $(OBJS) $(EXTRA_DEPS) $(PWD)/libmod_memmgr.a $(PWD)/libmod_ker.a $(GCOV_LAST)
	$(RM_HOST) $@
	echo "const char timestamp[] =\"`$(IDSTAMPER)`\";" >timestamp.h
	$(TIMESTAMP_CCPREF) $(CCFLAGS) $(FLAGS) $(CCVFLAGS) $(CCOPTS) -g ../../timestamp.c $(CCPOST)
	$(RM_HOST) timestamp.h
	$(CREATE_EX)
	$(CHECK_FOR_UNDEFS)
	$(ADD_PINFO)

nto: $(PWD)/libmod_ker.a

tnto: ktest.o nto
	$(RM_HOST) $@
	$(CREATE_EX)

asmoff.def: $(PROJECT_ROOT)/ker/asmoff.c \
			$(PROJECT_ROOT)/public/kernel/objects.h \
			$(PROJECT_ROOT)/public/kernel/cpu_$(CPU).h \
			$(PROJECT_ROOT)/ker/$(CPU)/kercpu.h \
			$(PROJECT_ROOT)/ker/$(CPU)/cpu_asmoff.h \
			$(PROJECT_ROOT)/public/sys/neutrino.h
kexterns.o: $(PROJECT_ROOT)/ker/kexterns.c $(PROJECT_ROOT)/ker/externs.h
externs.o:  $(PROJECT_ROOT)/proc/externs.c $(PROJECT_ROOT)/externs.h

EXTRA_CLEAN += timestamp.c

clean spotless:
	$(TARGET_CLEAN)

ICLEAN = procnto* tnto* $(MODULE_LIBS)

iclean:
	$(TARGET_ICLEAN)

PROC_INSTDIR = $(INSTALL_ROOT_nto)/$(CPUDIR)/$(INSTALLDIR)/

# Only install modules from one directory for each architecture (two if BE/LE)	
ifeq ($(strip $(filter-out o be le v4 600, $(subst -, , $(subst ., ,$(VARIANT1))))),)
INST_MODULES = $(strip $(filter-out $(PWD)/libmod_ker.a $(PWD)/libmod_memmgr.a, $(MODULE_LIBS)))
endif	

define TARGET_INSTALL
	$(CP_HOST) $(FULLNAME) $(PROC_INSTDIR)
	$(if $(INST_MODULES),$(CP_HOST) $(INST_MODULES) $(PROC_INSTDIR))
endef

# Re-enable if/when we start shipping a standalone kernel product
#define TARGET_NINSTALL
#	-$(CP_HOST) libmod_ker.a $(INSTALL_ROOT_nto)/$(CPUDIR)/lib/libnto$(VARIANT_TAG).a
#endef

install: all
	$(TARGET_HINSTALL)
	$(TARGET_NINSTALL)
	-$(TARGET_INSTALL)

qinstall: 
	$(TARGET_HINSTALL)
	$(TARGET_NINSTALL)
	-$(TARGET_INSTALL)

ninstall:
	$(TARGET_NINSTALL)

hinstall:
	$(TARGET_HINSTALL)

SRCS += $(foreach m, $(MODULE_NAMES),$(SRCS_$(m)))
