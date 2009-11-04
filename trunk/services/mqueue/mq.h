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





#include <signal.h>
#include <sys/iofunc.h>

typedef struct mqwait_entry {
	struct mqwait_entry	*next;
	unsigned			 priority;
	int					 rcvid;
	int					 scoid;
	int					 coid;
	unsigned			 xtype;
} MQWAIT;

typedef struct mqmsg_entry {
	struct mqmsg_entry	*next;
	unsigned			 priority;
	int					 nbytes;
	char				 data[1];
} MQMSG;
#define MQ_DATAOFF	(offsetof(MQMSG, data))

typedef struct mqdev_entry {
	iofunc_attr_t		 attr;
	struct mqdev_entry	*link;
	int					 id;
	MQWAIT				*waiting_read;
	MQWAIT				*waiting_write;
	MQMSG				*waiting_msg[2];
	struct mq_attr		 mq_attr;
	iofunc_notify_t		 notify[3];
	char				 name[1];
} MQDEV;

struct ocb {
	iofunc_ocb_t		 ocb;
	struct mqclosemsg	*closemsg;
	int					notify_attached;
};

struct mqclosemsg {
	int			nbytes;
	char		data[1];
};

