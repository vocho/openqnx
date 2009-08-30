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





#include <inttypes.h>

#ifndef INTMAX_MAX
#define intmax_t				long
#endif

#define FLAG_TYPE				0x00000003
#define FLAG_END				0x00000000
#define FLAG_CONFSTR			0x00000001
#define FLAG_SYSCONF			0x00000002
#define FLAG_PATHCONF			0x00000003

#define FLAG_NAME				0x00000100
#define FLAG_VALUE				0x00000200
#define FLAG_POSIX_VALUE		0x00000400

struct variable {
	const char			*string;
	unsigned			flags;
	int					name;
	intmax_t			value;
	intmax_t			posix_value;
};

extern struct variable	table[];
