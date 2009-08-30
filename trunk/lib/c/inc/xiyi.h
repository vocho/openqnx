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




#include <_pack1.h>

struct __xiyi_init {
	unsigned char	type;				// 0 - near which is what we always are
	unsigned char	prio;				// 0 .. 255
	void			(*init)(void);		// function address
};

#include <_packpop.h>

extern void __xiyi(const struct __xiyi_init *beg, const struct __xiyi_init *end, int low, int high);
extern struct __xiyi_init __xibeg, __xiend, __yibeg, __yiend;

#define INIT_PRIORITY_THREAD		1	// priority for thread data init
#define INIT_PRIORITY_FPU			2	// priority for FPU/EMU init
#define INIT_PRIORITY_USERTHREAD	3	// priority for user thread data init
#define INIT_PRIORITY_RUNTIME		10	// priority for run/time initialization
#define INIT_PRIORITY_IOSTREAM		20	// priority for IOSTREAM
#define INIT_PRIORITY_LIBRARY		32	// default library-initialization priority
#define INIT_PRIORITY_PROGRAM		64	// default program-initialization priority
#define DTOR_PRIORITY				40	// priority for module DTOR

/* __SRCVERSION("xiyi.h $Rev: 153052 $"); */
