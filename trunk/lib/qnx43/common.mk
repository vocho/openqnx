ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

EXTRA_SRCVPATH+=$(PROJECT_ROOT)/c
EXTRA_INCVPATH+=$(PROJECT_ROOT)/h
include $(MKFILES_ROOT)/qtargets.mk

define PINFO
PINFO DESCRIPTION=qnx43 library
endef

#did this by placement of corresponding src in variant subdirs
#QNX4ONLYOBJS = console.o fish_time.o fndusr.o fs_mount.o get_disk.o \
#	      getsid.o getut.o nidtostr.o stresc.o ticnames.o qnx_term.o \
#	      report.o proc_link.o 
#   
#NTOONLYOBJS  = fish_time_nto.o fsys_stat.o
#EXCLUDE_OBJS += $(QNX4ONLYOBJS) $(NTOONLYOBJS)


