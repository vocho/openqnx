# This is an automatically generated record.
# Please never delete or relocate it.


ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCIPTION=ed
endef

#===== LIBS - a space-separated list of library items to be included in the link.
LIBS+=compat
include $(MKFILES_ROOT)/qtargets.mk


#QNX internal start
ifeq ($(filter g, $(VARIANT_LIST)),g)
DEBUG_SUFFIX=_g
LIB_SUFFIX=_g
else
DEBUG_SUFFIX=_r
endif

LIBS_D = $(LIBS_$(CPUDIR)$(DEBUG_SUFFIX)) $(LIBS$(DEBUG_SUFFIX))

EXTRA_LIBVPATH := $(EXTRA_LIBVPATH_$(CPUDIR)$(DEBUG_SUFFIX)) $(EXTRA_LIBVPATH$(DEBUG_SUFFIX)) \
                 $(EXTRA_LIBVPATH_$(CPUDIR)) $(EXTRA_LIBVPATH)

EXTRA_INCVPATH := $(EXTRA_INCVPATH_$(CPUDIR)$(DEBUG_SUFFIX)) $(EXTRA_INCVPATH$(DEBUG_SUFFIX)) \
                 $(EXTRA_INCVPATH_$(CPUDIR)) $(EXTRA_INCVPATH)

EXTRA_SRCVPATH := $(EXTRA_SRCVPATH_$(CPUDIR)$(DEBUG_SUFFIX)) $(EXTRA_SRCVPATH$(DEBUG_SUFFIX)) \
				  $(EXTRA_SRCVPATH_$(CPUDIR)) $(EXTRA_SRCVPATH)

CCFLAGS_D = $(CCFLAGS$(DEBUG_SUFFIX)) $(CCFLAGS_$(CPUDIR)$(DEBUG_SUFFIX)) \
			$(CCFLAGS_$(basename $@)$(DEBUG_SUFFIX)) 					  \
			$(CCFLAGS_$(CPUDIR)_$(basename $@)$(DEBUG_SUFFIX)) 
LDFLAGS_D = $(LDFLAGS$(DEBUG_SUFFIX)) $(LDFLAGS_$(CPUDIR)$(DEBUG_SUFFIX))

CCFLAGS += $(CCFLAGS_$(CPUDIR))  $(CCFLAGS_$(basename $@)) 				  \
		   $(CCFLAGS_$(CPUDIR)_$(basename $@))  $(CCFLAGS_D)

LDFLAGS += $(LDFLAGS_$(CPUDIR)) $(LDFLAGS_D)

LIBS := $(foreach token, $(LIBS_D) $(LIBS_$(CPUDIR)) $(LIBS), $(if $(findstring ^, $(token)), $(subst ^,,$(token))$(LIB_SUFFIX), $(token)))

libnames:= $(subst lib-Bdynamic.a, ,$(subst lib-Bstatic.a, , $(libnames)))
libopts := $(subst -l-B,-B, $(libopts))
#QNX internal end

OPTIMIZE_TYPE+=$(OPTIMIZE_TYPE_$(filter g, $(VARIANTS)))
OPTIMIZE_TYPE_g+=none
