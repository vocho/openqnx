1:=$(firstword $(modules))
modules:=$(wordlist 2, 9999, $(modules))
	
dir_list:=$(strip $(foreach chkdir,$(MODULE_DIRS), $(if $(filter $(notdir $(chkdir)), $(1)), $(chkdir))))
SRCVPATH_$(1):=$(strip $(foreach dir, $(dir_list), $(addprefix $(PROJECT_ROOT)/$(dir)/$(CPU)/, $(SRC_VARIANTS)) $(PROJECT_ROOT)/$(dir)/$(CPU) $(addprefix $(PROJECT_ROOT)/$(dir)/, $(SRC_VARIANTS)) $(PROJECT_ROOT)/$(dir)))
METASRCS_$(1) := $(foreach dir, $(SRCVPATH_$(1)), $(addprefix $(dir)/, *.s *.S *.c))
SRCS_$(1) := $(wildcard $(METASRCS_$(1)))
OBJS_$(1) := $(sort $(addsuffix .o, $(filter-out asmoff ktest, $(basename $(notdir $(SRCS_$(1)))))))
ACTUAL_MODULE_NAMES += $(if $(SRCS_$(1)),$(1))	

$(PWD)/libmod_$(1).a : $(OBJS_$(1)) $(EXTRA_DEPS)
	$(RM_HOST) $@
	$(CREATE_AR)
