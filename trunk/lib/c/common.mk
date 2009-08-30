# Copyright 2001, 2008, 2009, QNX Software Systems Ltd. All Rights Reserved
#  
# This source code has been published by QNX Software Systems Ltd. (QSSL).
# However, any use, reproduction, modification, distribution or transfer of
# this software, or any software which includes or is based upon any of this
# code, is only permitted under the terms of the QNX Realtime Plaform End User
# License Agreement (see licensing.qnx.com for details) or as otherwise
# expressly authorized by a written license agreement from QSSL. For more
# information, please email licensing@qnx.com.
#
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

.PHONY: first all iclean clean spotless install qinstall hinstall

first: all

EXTRA_INCVPATH = $(PROJECT_ROOT)/inc $(PROJECT_ROOT)/../../services/system/public $(PROJECT_ROOT)/../m/public $(PROJECT_ROOT)/../m/inc $(PROJECT_ROOT)/../pm/public $(PROJECT_ROOT)/../../services/registry/public

ASSEMBLER_TYPE_x86 = gcc
ASMOFF_FORMAT_x86 = cpp
DEFFILE = asmoff.def

OS=nto

#
# The mcount function is used for profiling, and needs the frame pointer
# to find the caller's adress
#
CCFLAGS_mcount = -fno-omit-frame-pointer

#
# Special compiler/assembler options for startup code
#
ASFLAGS_crt1+=-static
ASFLAGS_mcrt1+=-static
ASFLAGS_startup+=-shared
ASFLAGS_startup_ion +=-DNO_INIT_ARRAY
ASFLAGS_startup_iox +=-DNO_INIT_ARRAY -DNO_CLEAR_ERRNO

CCFLAGS_startup_ppc_gcc_ = -G0
CCFLAGS_startup_ppc_gcc_qcc = -Wc,-G0
CCFLAGS_startup_mips_gcc_ = -G0
CCFLAGS_startup_mips_gcc_qcc = -Wc,-G0
CCFLAGS_startup_sh_gcc_ = -mprefergot
CCFLAGS_startup_sh_gcc_qcc = -mprefergot
CCFLAGS_startup=-fpic -finhibit-size-directive $(CCFLAGS_startup_$(CPU)_$(COMPILER_TYPE)_$(COMPILER_DRIVER))

#
# This shrinks the library on PPC by avoiding unnecessary F.P. instructions
# on vararg functions that never do any floating point.
#
CCFLAGS_execl_ppc = -msoft-float
CCFLAGS_execle_ppc = -msoft-float
CCFLAGS_execlp_ppc = -msoft-float
CCFLAGS_execlpe_ppc = -msoft-float
CCFLAGS_fcntl_ppc = -msoft-float
CCFLAGS_iodir_ppc = -msoft-float
CCFLAGS_open_ppc = -msoft-float
CCFLAGS_mq_open_ppc = -msoft-float
CCFLAGS_sem_open_ppc = -msoft-float
CCFLAGS_typed_mem_open_ppc = -msoft-float
CCFLAGS_traceevent_ppc = -msoft-float
CCFLAGS_hwi_find_item_ppc = -msoft-float
CCFLAGS_sopen_ppc = -msoft-float
CCFLAGS_spawnl_ppc = -msoft-float
CCFLAGS_spawnle_ppc = -msoft-float
CCFLAGS_spawnlp_ppc = -msoft-float
CCFLAGS_spawnlpe_ppc = -msoft-float
CCFLAGS_mount_ppc = -msoft-float
CCFLAGS_ioctl_ppc = -msoft-float
CCFLAGS_open64_ppc = -msoft-float

#
# This works around an optimizer bug in the compiler
#
CCFLAGS_ioctl_sh += -O1

ifeq ($(GCC_VERSION),4.2.1)
# this is a temporary fix to workaround a compiler bug
CCFLAGS_mips += -fno-optimize-sibling-calls
endif

#
# And try to shrink the literal pools...
#
CCFLAGS_sh += -mspace

CCFLAGS_ansi += -D__INLINE_FUNCTIONS__

#
# Don't display gcc warnings in Dinkim libs
#
CCFLAGS_gcc_ansi += -Wno-uninitialized -Wno-switch -Wno-missing-braces -Wno-char-subscripts
CCFLAGS_gcc_stdio += -Wno-uninitialized -Wno-char-subscripts
CCFLAGS_gcc_string += -Wno-uninitialized

include $(MKFILES_ROOT)/qrules.mk

ifndef LIBC_SO_VERSION
include $(PROJECT_ROOT)/lib/soversion.mk
-include $(PRODUCT_ROOT)/cpp/soversion.mk
ifndef LIBCPP_SO_VERSION
LIBCPP_SO_VERSION=2
endif
-include $(PRODUCT_ROOT)/m/soversion.mk
ifndef LIBM_SO_VERISION
LIBM_SO_VERSION=2
endif
endif

#
# If we're compiling the SH nofpu variant with a version 4.x.x compiler, add the 
# appropriate -m flag.  (The nofpu variant will be identical to the regular variant
# if you compile with the 2.x.x or 3.x.x compiler.  The ifneq test can be removed
# once 6.4.0 ships and we no longer support the 2.95.3 compiler.)
ifneq ($(filter 4.%, $(GCC_VERSION)),)
CCVFLAG_nofpu_sh = -m4-nofpu
CCVFLAG_nofpu = $(CCVFLAG_nofpu_$(CPU))
endif

ifeq ($(filter g, $(VARIANTS)),)
CCF_opt_gcc_    = -O2 -fomit-frame-pointer
CCF_opt_gcc_qcc = -O2 -fomit-frame-pointer
CCF_gcc_    = -O2 
CCF_gcc_qcc = -O2 
endif

CCF_opt_gcc_    += -fno-strict-aliasing
CCF_opt_gcc_qcc += -fno-strict-aliasing
CCF_gcc_        += -fno-strict-aliasing
CCF_gcc_        += -Wp,-include -Wp,$(PROJECT_ROOT)/inc/weak.h
CCF_gcc_qcc     += -fno-strict-aliasing
CCF_gcc_qcc     += -Wp,-include -Wp,$(PROJECT_ROOT)/inc/weak.h

# Never want debug information for startup files - causes compiler
# to get confused about the sections.
ifeq ($(SECTION),startup)
override DEBUG=
endif

CCF_wcc_	= -U__INLINE_FUNCTIONS__
CCF_wcc_	+= -fi$(PROJECT_ROOT)/inc/weak.h

# ION wants MALLOC_PC
CCFLAGS_alloc_ion = -DMALLOC_PC
CCFLAGS_malloc_iox= -DMALLOC_ID

#what to do when someone does printf("%s",null). default behavior is to sigsegv
CCFLAGS_stdio_qss = -DSTDIO_OPT_PERCENT_S_NULL=NULL

#ION and IOX want printf("%s",NULL) to not sigsegv. 
CCFLAGS_stdio_ion = -DSTDIO_OPT_PERCENT_S_NULL=\"\(null\)\"
CCFLAGS_stdio_iox = -DSTDIO_OPT_PERCENT_S_NULL=\"\(null\)\" 

ifeq ($(BUILDENV),ion)
CCFLAGS_crtbegin_PIC_gcc__mips = -mabicalls
CCFLAGS_crtend_PIC_gcc__mips = -mabicalls
endif

# We really should fix our source to not require -fno-strict-aliasing
CCFLAGS += $(CCF_$(COMPILER_TYPE)_$(COMPILER_DRIVER)) \
		$(CCFLAGS_$(SECTION))	\
		$(CCFLAGS_$(COMPILER_TYPE)_$(SECTION))	\
		$(CCFLAGS_$(COMPILER_TYPE)_$(CPU)) \
		$(CCFLAGS_$(CPU))	\
		$(CCFLAGS_$(basename $@)) \
		$(CCFLAGS_$(basename $@)_$(CPU)) \
		$(CCFLAGS_$(basename $@)_$(COMPILER_TYPE)_$(COMPILER_DRIVER)_$(CPU)) \
		$(CCFLAGS_$(SECTION)_$(BUILDENV)) \
		-D_LIBC_SO_VERSION=$(LIBC_SO_VERSION) \
		-D_LIBCPP_SO_VERSION=$(LIBCPP_SO_VERSION) \
		-D_LIBM_SO_VERSION=$(LIBM_SO_VERSION)



ASFLAGS += $(ASFLAGS_$(SECTION))	\
		$(ASFLAGS_$(COMPILER_TYPE)_$(CPU)) \
		$(ASFLAGS_$(CPU))	\
		$(ASFLAGS_$(SECTION)_$(BUILDENV))	\
		$(ASFLAGS_$(basename $@)) \
		$(ASFLAGS_$(basename $@)_$(CPU)) 


# Some sections we only want to build in certain environments	
BUILDENV_dllmgr = iox
BUILDENV_malloc = iox
BUILDENV_dllmisc = iox
BUILDENV_alloc = qss ion
BUILDENV_ldd = qss
BUILDENV_dllstubs = iox
BUILDENV_libdl = iox
BUILDENV_miscdll = iox
ifeq ($(BUILDENV),)
BUILDENV=qss
endif

ifeq ($(BUILDENV),iox)
CCVFLAG_dll += -D__DLL -D__SLIB
ASVFLAG_dll += -D__DLL -D__SLIB
endif

ifneq ($(filter $(BUILDENV), $(if $(BUILDENV_$(SECTION)),$(BUILDENV_$(SECTION)),$(BUILDENV))),$(BUILDENV))
OBJS=
endif

ifeq ($(COMPILER),wcc)
wcc_comp:=$(firstword $(CC_nto_x86_wcc))
ifeq ($(wcc_comp)$,)
OBJS=
else	
ifeq ($(wildcard $(wcc_comp)*),)
OBJS=
endif
endif
endif

#
# Lint Configuration
#

FLINTCONFIG_default += $(PROJECT_ROOT)/libc.lnt
FLINTCONFIG_default += $(PROJECT_ROOT)/libc-default.lnt

FLINTCONFIG_WO790585 += $(PROJECT_ROOT)/libc.lnt
FLINTCONFIG_WO790585 += $(PROJECT_ROOT)/libc-WO790585.lnt

#
# Rules
#

all: $(OBJS)

ifeq ($(BUILDENV),iox)
stubs:
ifneq ($(OBJS),)
	$(MKSTUBS) -v -D $(PROJECT_ROOT)/dllstubs -S -d libc.dll lib_$(SECTION).dll.a $(OBJS)
endif
endif

install: all qinstall

qinstall iclean: # Nothing to do

hinstall: # Nothing to do

spotless clean:
	$(RM_HOST) *.o $(DEFFILE) $(EXTRA_CLEAN)

ldd.o: ldd.c $(firstword $(wildcard ../relocs.ci* ../../relocs.ci*))
