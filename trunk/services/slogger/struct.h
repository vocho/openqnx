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


#define LOGF_FLAG_OSYNC 0x00000001
 

struct waiting {
	struct waiting	*next;
	int				 rcvid;
	int				 priority;
} ;

struct ocbs {
	struct ocbs		*next;
	IOFUNC_OCB_T	*ocb;
} ;

struct slogdev {
	iofunc_attr_t		 attr;
	iofunc_notify_t		 notify[3];
	struct ocbs			*ocbs;
	struct waiting		*waiting;
	int					 cnt;		// Number of ints starting at get
	int					*put;		// Points into buf for putting data
	int					*get;		// Points into buf for getting data
	int					*beg;		// Pointer to begining of buf
	int					*end;		// Pointer to end of buf + 1
	int					 id;		// Contains id of /dev/slog
} ;


// The offset in a ocb is 64 bits. We remap it as follows. The offset
// is on a per open basis. A dup() will share the existing offset for
// the dupped fd.
struct ocbget {
	int		*get;
} ;

#define OCBGET(ocb)		((struct ocbget *)&ocb->offset)->get

/* __SRCVERSION("struct.h $Rev: 157840 $"); */
