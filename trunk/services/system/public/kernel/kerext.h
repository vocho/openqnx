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

//
// Covers for ker_ring0 calls
//

//
// Kernel calls which are installed by the process manager
//
int 	CredGet(pid_t pid, struct _cred_info *cip);
int 	CredSet(pid_t pid, struct _cred_info *cip);
int		CredDirty(pid_t pid);
void	DebugInstall(int code);
int		DebugAttach(pid_t pid, unsigned flags);
int		DebugDetach(pid_t pid);
int		DebugProcess(enum nto_debug_request request, pid_t pid, int tid, union nto_debug_data *data);
int		DebugChannel(CHANNEL *chp, debug_channel_t *data);
int		PageFaultWait(struct fault_info *);
int		PageWait(uintptr_t vaddr, unsigned flags, pid_t pid, int code);
int 	PageCont(pid_t pid, int tid, int err);
int 	PageContErrno(pid_t pid, int tid, int sigcode);
uintptr_t PageWaitInfo(union sigval value, pid_t *ppid, int *ptid, pid_t *paspace_pid, unsigned *pflags);
PROCESS *ProcessCreate(pid_t parent_pid, void *lcp, proc_create_attr_t *extra);
int 	ProcessDestroy(pid_t pid);
int 	ProcessDestroyAll(pid_t pid);
int		ProcessAssociate(PROCESS *prp, MPART_ID mpid, mempart_dcmd_flags_t flags);
int		ProcessReassociate(PROCESS *prp, MPART_ID mpid, mempart_dcmd_flags_t flags);
int		ProcessDisassociate(PROCESS *prp, MPART_ID mpid);
void	MemclassPidUse(pid_t pid, part_id_t mpart_id, memsize_t size);
void	MemclassPidFree(pid_t pid, part_id_t mpart_id, memsize_t size);
int 	ProcessExec(void *lcp, int *prcvid, int rcvid, int ppid);
int 	ProcessSwap(PROCESS *parent, PROCESS *child);
int 	ProcessShutdown(PROCESS *prp, int exec);
void	ProcessRestore(int rcvid, struct _thread_local_storage *tsp, void *frame, void *frame_base, unsigned frame_size);
int		ProcessBind(pid_t pid);
void 	*ProcessStartup(pid_t pid, int start);
void 	*QueryObject(int type, unsigned index1, int subtype, unsigned index2, unsigned *next, void *objbuf, int objsize);
void	QueryObjectDone(void *);
struct _thread_local_storage *ThreadTLS(struct _thread_local_storage *tls, uintptr_t stackaddr, unsigned stacksize, unsigned guardsize, uintptr_t esp);
#define V86Enter(swi) __Ring0(kerext_v86_enter, (void *)swi)
void	kerext_v86_enter(void *data);
void	KerextLock(void);
void	KerextUnlock(void);
int		KerextAmInKernel(void);
int		KerextNeedPreempt(void);
int		KerextSyncOwner(pid_t, int);
void	KerextStatus(THREAD *thp, int status);
void	RebootSystem(int);
int		SysCpumode(int);
int		CacheControl(void *base, size_t len, int flags);
int		StackCont(pid_t pid, int tid, int err, struct _thread_attr *attr);
int		StackWaitInfo(union sigval value, pid_t *ppid, int *ptid, struct _thread_attr *attr);
uintptr_t GetSp(THREAD *thp);
int		KerextThreadCtl(PROCESS *prp, int tid, int32_t cmd, void *data );
void	MemobjDestroyed(OBJECT *obp, off_t start, off_t end, PROCESS *prp, void *vaddr);
int 	KerextAddTraceEvent(int ev_class, int ev_code, void *data, int datalen);
int 	KerextAddTraceEventIOV(int ev_class, int ev_code, IOV *iovs, int iovlen);
int		KerextSlogf( int opcode, int severity, const char *fmt, ... );
int		ValidateMpartId(MPART_ID mpid);
//
// Kernel calls used by the idle thread
//
void        kerext_idle(void *);

/* __SRCVERSION("kerext.h $Rev: 168445 $"); */
