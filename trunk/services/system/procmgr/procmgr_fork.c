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

#include "externs.h"
#include "procmgr_internal.h"
#include "apm.h"

int procmgr_fork(resmgr_context_t *ctp, proc_fork_t *msg) {
	struct loader_context				*lcp;
	struct _thread_attr					attr;
	PROCESS								*prp;
	proc_create_attr_t					extra = {NULL};
	part_list_t							*mempart_list;
	part_list_t							*schedpart_list = NULL;
	
	if(!(lcp = procmgr_context_alloc(0, LC_FORK))) {
		return ENOMEM;
	}
	
	/* memory partitions */
	{
		unsigned i, r;
		PROCESS *pprp = proc_lock_pid(ctp->info.pid);
		int  num_parts;
		mempart_flags_t getlist_flags = mempart_flags_t_GETLIST_INHERITABLE | mempart_flags_t_GETLIST_CREDENTIALS;
		struct _cred_info cred;

		CRASHCHECK(pprp == NULL);

		cred = pprp->cred->info;
		num_parts = MEMPART_GETLIST(pprp, NULL, 0, getlist_flags, &cred);
		if ((num_parts <= 0) || ((mempart_list = alloca(PART_LIST_T_SIZE(num_parts))) == NULL))
		{
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return ENOMEM;
		}
		
		mempart_list->num_entries = num_parts;
		if ((r = inherit_mempart_list(pprp, &mempart_list)) != EOK) {
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return r;
		}

		/* make sure a sysram partition is inheritable */
		for (i=0; i<mempart_list->num_entries; i++)
			if (mempart_get_classid(mempart_list->i[i].id) == sys_memclass_id)
				break;
		if (i >= mempart_list->num_entries)
		{
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return EACCES;	// no permission for parents sysram partition
		}

		/* 
		 * By default, the mempart_list->i[].flags are inherited from the
		 * parent. However, in the case of the the initial processes created
		 * by procnto, we do not want this inheritance. Instead we want to use
		 * the 'default_mempart_flags'. These flags will apply only to the
		 * partition of the system memory class since that is (currently) the
		 * only partition procnto is associated with.
		*/
		if (pprp == procnto_prp)
		{
			unsigned j;
			for (j=0; j<mempart_list->num_entries; j++) {
				mempart_list->i[j].flags = default_mempart_flags;
			}
		}
		proc_unlock(pprp);
	}
	
	/* scheduler partitions */
	if (SCHEDPART_INSTALLED())
	{
		PROCESS *pprp = proc_lock_pid(ctp->info.pid);
		int  num_parts, r;
		schedpart_flags_t getlist_flags = schedpart_flags_t_GETLIST_INHERITABLE | schedpart_flags_t_GETLIST_CREDENTIALS;
		struct _cred_info cred;

		CRASHCHECK(pprp == NULL);

		cred = pprp->cred->info;
		num_parts = SCHEDPART_GETLIST(pprp, NULL, 0, getlist_flags, &cred);
		if ((num_parts <= 0) || ((schedpart_list = alloca(PART_LIST_T_SIZE(num_parts))) == NULL))
		{
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return ENOMEM;
		}
		
		schedpart_list->num_entries = num_parts;
		if ((r = inherit_schedpart_list(pprp, &schedpart_list)) != EOK) {
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return r;
		}
		/* make sure a scheduling partition is available */
		if (schedpart_list->num_entries < 1) {
			proc_unlock(pprp);
			procmgr_context_free(lcp);
			return EACCES;
		}
		/* 
		 * By default, the schedpart_list->i[].flags are inherited from the
		 * parent. However, in the case of the the initial processes created
		 * by procnto, we do not want this inheritance. Instead we want to use
		 * the 'default_schedpart_flags'
		*/
		if (pprp == procnto_prp)
		{
			unsigned j;
			for (j=0; j<schedpart_list->num_entries; j++) {
				schedpart_list->i[j].flags = default_schedpart_flags;
			}
		}
		proc_unlock(pprp);
	}

	/*
	 * force the first partition entry to be for the sysram memory class. This
	 * is done first of all because kerext_process_create() requires a sysram
	 * memory class partition in order to perform the first association and
	 * secondly it ensures the fastest possible access to the sysram partition
	 * for a process (ie. first list entry).
	 * The only time that this assertion should go off is when an explicit
	 * partition list is provided by the user which does not have a sysram
	 * partition as the first entry and list reordering has not occured. 
	*/ 
	CRASHCHECK(mempart_get_classid(mempart_list->i[0].id) != sys_memclass_id);

	lcp->flags = msg->i.flags;
	lcp->ppid = ctp->info.pid;
	lcp->rcvid = ctp->rcvid;
	memcpy(&lcp->msg, msg, min(sizeof lcp->msg, sizeof *msg));
	SignalProcmask(lcp->ppid, ctp->info.tid, 0, 0, &lcp->mask);

#ifndef NDEBUG
	if (MEMPART_INSTALLED() && (mempart_list == NULL))
	{
		kprintf("ins: %d\n", MEMPART_INSTALLED());
		crash();
	}
#endif	/* NDEBUG */
	
	extra.mpart_list = mempart_list;
	extra.spart_list = schedpart_list;
	proc_wlock_adp(sysmgr_prp);
	lcp->process = ProcessCreate(lcp->ppid, lcp, &extra);
	proc_unlock_adp(sysmgr_prp);
	if(lcp->process == (void *)-1) {
		procmgr_context_free(lcp);
		return errno;
	}

	if((prp = proc_lock_pid(lcp->ppid))) {
		lcp->process->root = pathmgr_node_clone(prp->root);
		lcp->process->cwd = pathmgr_node_clone(prp->cwd);

		proc_unlock(prp);
	}

	procmgr_thread_attr(&attr, lcp, ctp);

	if(ThreadCreate(lcp->process->pid, loader_fork, lcp, &attr) == -1) {
		// Turn terming flag on, we're not doing a thread_destroy
		lcp->process->flags |= _NTO_PF_TERMING;
		MsgSendPulse(PROCMGR_COID, attr.__param.__sched_priority, PROC_CODE_TERM, lcp->process->pid);
		return ENOMEM;
	}
	return _RESMGR_NOREPLY;
}

__SRCVERSION("procmgr_fork.c $Rev: 198175 $");
