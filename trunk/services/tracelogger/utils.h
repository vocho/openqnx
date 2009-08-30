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


/*
 * utils.h
 */
 
#ifndef __UTILS_H_
#define __UTILS_H_

#include <sys/cdefs.h>

enum verbosity_level {
	VERBOSITY_NONE = 0,
	VERBOSITY_INFO,
	VERBOSITY_DEBUG,
	VERBOSITY_HACK,
};

__BEGIN_DECLS

struct attributes {
	struct attributes *next;
	char *key;
	char *value;
};

extern struct attributes *g_extra_attributes;

extern int info( const char *fmt, ... );
extern int debug( const char *fmt, ... );
extern int hack( const char *fmt, ... );

extern unsigned discover_num_colours(void);
extern int add_user_attribute(const char *str);
__END_DECLS

#endif /* __UTILS_H_ */

/* __SRCVERSION("utils.h $Rev: 153052 $"); */
