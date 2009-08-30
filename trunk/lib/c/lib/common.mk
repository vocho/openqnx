# Copyright 2001, QNX Software Systems Ltd. All Rights Reserved
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

define PINFO
PINFO DESCRIPTION=C runtime library
endef

NAME=c

# There's nothing to be gained by running commands in this makefile
# in parallel and some things will break if we do.	The following
# special target tells make to be a good boy and leave well enough
# alone.
.NOTPARALLEL:

.PHONY: first all iclean clean spotless install qinstall hinstall cinstall

ICLEAN=lib*.* 

first: all

# The maximum command line length for cygwin and qcc is too small to allow us
# to build the library with one command, so we do it in several chunks.
#
# note we should fix qcc to not need this, see PR54777
#
define reset_objlist
objlist:=
endef
define create_cmdlist
objlist+= $(o)
cmdlist+=$$(if $$(word 800, $$(objlist)), $$(ARPREF) $$(AROPTS) $$@ $$(objlist) $(ARVFLAGS_$(COMPILER_DRIVER)) $$(ARPOST)$$(NEWLINE) $$(eval $(reset_objlist)))
endef
objlist:=
cmdlist:=
define CREATE_AR 
  $(PRECREATE_AR)
  $(foreach o,$(only_objs),$(eval $(create_cmdlist)))
  $(cmdlist)
  $(if $(objlist),$(ARPREF) $(AROPTS) $@ $(objlist) $(ARVFLAGS_$(COMPILER_DRIVER)) $(ARPOST))
  $(POSTCREATE_AR)
endef

include $(MKFILES_ROOT)/qrules.mk

# See NOTE following STATIC_ONLY_OBJS macro definition if version number
# ever changes.
include $(PROJECT_ROOT)/soversion.mk
SO_VERSION = $(LIBC_SO_VERSION)

LIBTYPE:=$(filter a so,$(VARIANTS))

# EXCLUDE_SECTIONS_so = 1g

SECTIONS:=$(filter-out $(foreach sect, lib startup $(EXCLUDE_SECTIONS_$(LIBTYPE)), %/$(sect)), $(sort $(subst /Makefile,,$(wildcard ../../../*/Makefile))))

STARTUP_DIR:=a$(patsubst %.,%,.$(filter be le, $(VARIANTS)))
STDIO_DIR:=so$(patsubst %.,%,.$(filter be le, $(VARIANTS)))


skipit=
ifeq ($(COMPILER),wcc)
wcc_comp:=$(firstword $(CC_nto_x86_wcc))
ifeq ($(wcc_comp)$,)
skipit=1
else
ifeq ($(wildcard $(wcc_comp)*),)
skipit=1
endif
endif
endif

ifneq ($(skipit),)

all qinstall:
#	Nothing to do if we don't have the watcom compiler present


else

all: $(PWD)/libc.$(LIBTYPE)

qinstall: hinstall
	$(CP_HOST) $(wildcard ../../../startup/$(CPU)/nto.link*) ../../../startup/$(CPU)/$(STARTUP_DIR)/*.o $(INSTALL_ROOT_nto)/$(CPUDIR)/lib
	$(CP_HOST) libc.$(LIBTYPE) $(INSTALL_ROOT_nto)/$(CPUDIR)/lib/libc$(VARIANT_TAG).$(LIBTYPE)$(VERSION_TAG_$(BUILD_TYPE))
ifeq ($(LIBTYPE),so)
	$(CP_HOST) libcS.a $(INSTALL_ROOT_nto)/$(CPUDIR)/lib/libc$(VARIANT_TAG)S.a
	$(LN_HOST) libc$(VARIANT_TAG).$(LIBTYPE)$(VERSION_TAG_SO) $(INSTALL_DIRECTORY)/$(IMAGE_PREF_$(BUILD_TYPE))c$(VARIANT_TAG).so
endif

endif

install: all qinstall

#
# So hinstall works properly
#
PUBLIC_INCVPATH=$(dir $(PROJECT_ROOT))public
POST_HINSTALL=$(LN_HOST) sys/poll.h $(INSTALL_ROOT_HDR)/poll.h

hinstall:
	$(TARGET_HINSTALL)
	
clean spotless:
	$(TARGET_CLEAN)
	
iclean:
	$(TARGET_ICLEAN)

OBJS := $(wildcard $(addsuffix /$(CPU)/$(COMPOUND_VARIANT)/*.o,$(SECTIONS)))
STATIC_ONLY_OBJS = \
    __hwi_base.o \
    __hwi_find_string.o \
    _salloc.o \
    cfgopen.o \
    crypt.o \
    ftw.o \
    getlogin.o \
    getpass.o \
    getut.o \
    getw.o \
    getwd.o \
    glob.o \
    gmon.o \
    hwi_find_item.o \
    hwi_find_tag.o \
    hwi_next_item.o \
    hwi_next_tag.o \
    hwi_off2tag.o \
    hwi_tag2off.o \
    inpline.o \
    intr.o \
    mcount.o \
    miniproc_start.o \
    modem_open.o \
    modem_read.o \
    modem_script.o \
    modem_write.o \
    popen.o \
    pty.o \
    putw.o \
    qnx_crypt.o \
    que.o \
    random.o \
    rdchk.o \
    re_comp.o \
    regcomp.o \
    regerror.o \
    regexec.o \
    regfree.o \
    sample.o \
    scandir.o \
	sched_get_priority_adjust.o \
    slog.o \
    strptime.o \
    __waitid_net.o \
    wordexp.o 
SHARED_STATIC_EXCLUSION_OBJS= \
    gmon.o \
    sample.o

	
# NOTE: The following object files used to be on the static only list, 
# but libsocket referenced them and got exported there. Since we're
# going to now restrict what libsocket.so exports via the gnu ld 
# versioning mechanism, we have to put these functions in libc.so to
# avoid breaking programs that use libsocket.so and reference one of
# functions that are defined in these files. If we ever go to a libc.so.3,
# we can contemplate moving the files back to the static only list.
#
#    err.o \
#    errx.o \
#    passwd.o \
#    syslog.o \
#    verr.o \
#    verrx.o \
#    vwarn.o \
#    vwarnx.o \
#    warn.o \
#    warnx.o \
	
ifeq ($(BUILDENV),ion)
EXTRA_LDD_DIR:=$(patsubst %.,%,$(filter a so, $(VARIANTS)))$(patsubst %.,%,.$(filter be le, $(VARIANTS)))
OBJS += $(PROJECT_ROOT)/../../ldd_ion/$(CPU)/$(EXTRA_LDD_DIR)/*.o
endif

ifeq ($(BUILDENV),)
BUILDENV=qss
endif
ifeq ($(BUILDENV),qss)
SHARED_OBJS:=$(filter-out $(addprefix %/,$(STATIC_ONLY_OBJS)), $(OBJS))
else
SHARED_OBJS:=$(OBJS)
endif
SHARED_STATIC_OBJS:=$(filter-out $(addprefix %/,$(SHARED_STATIC_EXCLUSION_OBJS)), $(OBJS))

#
# Some makefile mopery-popery to get libcS.a.pinfo generated properly
#
$(PWD)/libcS.a: INSTALLNAME=$(INSTALL_DIRECTORY)/$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(VARIANT_TAG)S.a
$(PWD)/libcS.a: VERSION_TAG_SO=

$(PWD)/libc.a: $(OBJS)
	$(RM_HOST) $@
	$(CREATE_AR)
	$(ADD_PINFO)

$(PWD)/libcS.a: $(SHARED_STATIC_OBJS)
	$(RM_HOST) $@
	$(CREATE_AR)
	$(ADD_PINFO)

libc_cut.a: $(SHARED_OBJS)
	$(RM_HOST) $@
	$(CREATE_AR)

lint.out:
#there are no source files in here, so a blank target will keep the build from stopping
#when we do a lint run.
	
LDF_    = -Wl,-Bstatic -nostartfiles -e_start_
LDF_qcc = -Wl,-Bstatic -nostartup -Wl,-e_start_
LDFLAGS += $(LDF_$(COMPILER_DRIVER))

OBJPREF_libc_cut.a = -Wl,--whole-archive
OBJPOST_libc_cut.a = -Wl,--no-whole-archive
LIBVPATH = . $(USE_ROOT_LIB)

# Dirty hack time!  Just for the M6 testing
LDPOST_sh_4.2.1 = -Wl,--whole-archive $(shell ntosh-gcc -V2.95.3 -ml -fpic -print-libgcc-file-name) -Wl,--no-whole-archive
LDPOST += $(LDPOST_$(CPU)_$(GCC_VERSION))

USEFILE=	

EXTRA_DEPS = $(PWD)/libcS.a

#
# The ARPREF/RM_HOST for libc.a below is prevent a bad libc from being 
# included in the link accidently. I'd prefer not to link against any libc.a 
# at all, but I can't get qcc to stop that without stopping the link 
# against libgcc.a, which we do need. 
#

$(PWD)/libc$(IMAGE_SUFF_SO): libc_cut.a $(PWD)/libcS.a
	$(ARPREF) $(AROPTS) libc.a ../../../stdio/$(CPU)/$(STDIO_DIR)/printf.o $(ARVFLAGS_$(COMPILER_DRIVER)) $(ARPOST)
	$(TARGET_BUILD)
	$(RM_HOST) libc.a
