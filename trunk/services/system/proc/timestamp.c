/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */

#include <kernel/nto.h>

// "timestamp.h" is automaticly created by the common.mk
// and has the definition of the "timestamp" variable.
// This way, the timestamp always indicates the date that we
// linked procnto ("timestamp.c" is re-compiled before every link).
#include "timestamp.h"

const char os_version_string[] = {
	(_NTO_VERSION / 100) % 10 + '0', '.',
	(_NTO_VERSION /  10) % 10 + '0', '.',
	(_NTO_VERSION /   1) % 10 + '0', '\0',
};

__SRCVERSION("timestamp.c $Rev: 153052 $");
