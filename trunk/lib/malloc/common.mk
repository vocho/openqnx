#
# Copyright 2003, QNX Software Systems Ltd. All Rights Reserved.
#
# This source code may contain confidential information of QNX Software
# Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
# modification, disclosure, distribution or transfer of this software,
# or any software which includes or is based upon any of this code, is
# prohibited unless expressly authorized by QSSL by written agreement. For
# more information (including whether this source code file has been
# published) please email licensing@qnx.com.
#

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION=Debugging malloc library
endef

INSTALLDIR_=usr/lib
INSTALLDIR_g=usr/lib/malloc_g
INSTALLDIR=$(INSTALLDIR_$(DBG))

SO_VERSION = 2

DBG = $(filter g, $(VARIANT_LIST))

LIBS=backtrace-lightS

CCFLAGS_g__posix_memalign = -UMALLOC_WRAPPER
CCFLAGS_g__memalign_pc = -UMALLOC_WRAPPER
CCFLAGS_g__memalign = -UMALLOC_WRAPPER
CCFLAGS_g__malloc = -UMALLOC_WRAPPER
CCFLAGS_g__free = -UMALLOC_WRAPPER
CCFLAGS_g__realloc = -UMALLOC_WRAPPER
CCFLAGS_g__malloc_pc = -UMALLOC_WRAPPER
CCFLAGS_g__calloc = -UMALLOC_WRAPPER
CCFLAGS_g_malloc = -UMALLOC_WRAPPER
CCFLAGS_g_band = -UMALLOC_WRAPPER
CCFLAGS_g_compat = -UMALLOC_WRAPPER
CCFLAGS___posix_memalign = -UMALLOC_WRAPPER
CCFLAGS___memalign_pc = -UMALLOC_WRAPPER
CCFLAGS___memalign = -UMALLOC_WRAPPER
CCFLAGS___malloc = -UMALLOC_WRAPPER
CCFLAGS___free = -UMALLOC_WRAPPER
CCFLAGS___realloc = -UMALLOC_WRAPPER
CCFLAGS___malloc_pc = -UMALLOC_WRAPPER
CCFLAGS___calloc = -UMALLOC_WRAPPER
CCFLAGS__malloc = -UMALLOC_WRAPPER
CCFLAGS__band = -UMALLOC_WRAPPER
CCFLAGS__compat = -UMALLOC_WRAPPER
CCFLAGS_g = $(CCFLAGS_g_$(basename $@)) -DMALLOC_DEBUG
CCFLAGS_ = $(CCFLAGS__$(basename $@)) -DMALLOC_GUARD -DMALLOC_PC
CCFLAGS_g += -I$(PROJECT_ROOT)/public/malloc_g
CCFLAGS_ += -I$(PROJECT_ROOT)/public/malloc
CCFLAGS += $(DEBUG) -DMALLOC_WRAPPER -D_LIBMALLOC $(CCFLAGS_$(DBG))
CCFLAGS_g += -O0

EXTRA_SRCVPATH_g = $(PROJECT_ROOT)/dbg
EXTRA_SRCVPATH_ = $(PROJECT_ROOT)/std
EXTRA_SRCVPATH = $(EXTRA_SRCVPATH_$(DBG)) $(PROJECT_ROOT)/common $(PRODUCT_ROOT)/c/alloc 

include $(MKFILES_ROOT)/qmacros.mk

BUILDNAME=$(IMAGE_PREF_$(BUILD_TYPE))$(NAME)$(subst _g,,$(VARIANT_TAG))$(IMAGE_SUFF_$(BUILD_TYPE))
BUILDNAME_SAR=$(IMAGE_PREF_AR)$(NAME)$(subst _g,,$(VARIANT_TAG))S$(IMAGE_SUFF_AR)

define POST_INSTALL_g
	$(LN_HOST) malloc_g/libmallocS.a $(INSTALL_ROOT_SO)/$(INSTALLDIR_)/libmalloc_gS.a
	$(LN_HOST) malloc_g/libmalloc.so $(INSTALL_ROOT_SO)/$(INSTALLDIR_)/libmalloc_g.so
endef

LDOPTS+=-nostdlib++

POST_INSTALL=$(POST_INSTALL_$(DBG))

include $(MKFILES_ROOT)/qtargets.mk
