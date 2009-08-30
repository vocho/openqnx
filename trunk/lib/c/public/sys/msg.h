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
 *  sys/msg.h    UNIX98 Message Queue Structures
 *

 */
#ifndef _MSG_H_INCLUDED
#define _MSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef _IPC_H_INCLUDED
 #include <sys/ipc.h>
#endif

#ifdef __EXT_XOPEN_EX

typedef unsigned long msgqnum_t;
typedef unsigned long msglen_t;

#include <_pack64.h>

#ifdef __EXT_UNIX_MISC

/*
 *  This structure is only here for compatibility.
 */
struct msg {
	struct msg *msg_next;
	long		msg_type;
	ushort_t	msg_ts;
	short		msg_spot;
};

#endif

struct msqid_ds {
	struct ipc_perm		msg_perm;
#ifdef __EXT_UNIX_MISC
	struct msg			*msg_first;
	struct msg			*msg_last;
#else
	void				*msg_first;
	void				*msg_last;
#endif
	msglen_t			msg_cbytes;
	msgqnum_t			msg_qnum;
	msglen_t			msg_qbytes;
	pid_t				msg_lspid;
	pid_t				msg_lrpid;
	time_t				msg_stime;
	long				msg_pad1;
	time_t				msg_rtime;
	long				msg_pad2;
	time_t				msg_ctime;
	long				msg_pad3;
	long				msg_pad4[4];
};

#include <_packpop.h>

/*
 * Message operation flags
 */
#define MSG_NOERROR		010000

/*
 * UNIX98 Prototypes.
 */
__BEGIN_DECLS

#if defined(__NYI)
extern int msgctl(int __msqid, int __cmd, struct msqid_ds *__buf);
extern int msgget(key_t __key, int __msgflg);
extern int msgrcv(int __msqid, void *__msgp, size_t __msgsz, long __msgtyp, int __msgflg);
extern int msgsnd(int __msgid, __const void *__msgp, size_t __msgsz, int __msgflg);
#endif

__END_DECLS

#endif

#endif

/* __SRCVERSION("msg.h $Rev: 153052 $"); */
