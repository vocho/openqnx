#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#

	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	
.text
	 
EXC_COPY_CODE_START intr_entry_critical
#
# For CPU's that don't have the concept of a critical interrupt.
# Overridden for PPC400's & book E's.
#
EXC_COPY_CODE_END
