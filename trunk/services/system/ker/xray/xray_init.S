#
#------------------------------------------------------------------
#
# xray_init.S   Cisco XX
#
# Copyright (c) 2006 by cisco Systems, Inc.
# All rights reserved.
#------------------------------------------------------------------
#


# This file is sucked in from the .bom file by referencing the xray module.

#include <cisco/vendor.h>

.section ".rodata_mod", "a"


.global init_xray
init_xray:

#if defined(INCLUDE_PROCNTO_MODULE_XRAY)
.extern xray_initialize
	.long xray_initialize,0
#endif
