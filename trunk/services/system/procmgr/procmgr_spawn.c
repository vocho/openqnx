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


static int get_fdinfo(pid_t pid, int nd, int base, int fdinfo_max, struct _proc_spawn_fd_info *pfdinfo, int nfds, int* pfd, int flags) {
		struct _server_info sinfo;
		int max = fdinfo_max;
		int fd;

		if(nfds != 0) {
			/* given fd list */
			for(; nfds-- && max > 0; pfd++, base++) {
				if((fd = *pfd) != SPAWN_FDCLOSED) {
					if(ConnectServerInfo(pid, fd, &sinfo) != fd || (sinfo.flags & _NTO_COF_DEAD)) {
						errno = EBADF;
						return -1;
					}

					pfdinfo->fd = base;
					if(proc_thread_pool_reserve() != 0) {
						errno = EAGAIN;
						return -1;
					}
					/* the node descriptor of the server as viewed by the client */
					if((pfdinfo->nd = netmgr_remote_nd(nd, sinfo.nd)) == (unsigned)-1) {
						proc_thread_pool_reserve_done();
						return -1;
					}
					if((pfdinfo->srcnd = netmgr_remote_nd(sinfo.nd, ND_LOCAL_NODE)) == (unsigned)-1) {
						proc_thread_pool_reserve_done();
						return -1;
					}
					proc_thread_pool_reserve_done();
					pfdinfo->pid = sinfo.pid;
					pfdinfo->chid = sinfo.chid;
					pfdinfo->scoid = sinfo.scoid;
					pfdinfo->coid = sinfo.coid;
					pfdinfo ++;
					max --;
				}
			}
		} else {
			while(ConnectServerInfo(pid, base, &sinfo) != -1 && !(sinfo.coid & _NTO_SIDE_CHANNEL)) {
				if(!((sinfo.flags & _NTO_COF_DEAD) || ((sinfo.flags & _NTO_COF_CLOEXEC) && (flags & _PROC_SPAWN_FD_NOCLOEXC)))) {
					pfdinfo->fd = sinfo.coid;
					if(proc_thread_pool_reserve() != 0) {
						errno = EAGAIN;
						return -1;
					}
					/* the node descriptor of the server as viewed by the client */
					if((pfdinfo->nd = netmgr_remote_nd(nd, sinfo.nd)) == (unsigned)-1) {
						proc_thread_pool_reserve_done();
						return -1;
					}
					if((pfdinfo->srcnd = netmgr_remote_nd(sinfo.nd, ND_LOCAL_NODE)) == (unsigned)-1) {
						proc_thread_pool_reserve_done();
						return -1;
					}
					proc_thread_pool_reserve_done();
					pfdinfo->pid = sinfo.pid;
					pfdinfo->chid = sinfo.chid;
					pfdinfo->coid = sinfo.coid;
					pfdinfo->scoid = sinfo.scoid;
					pfdinfo ++;
					if(--max <= 0) {
						break;
					}
				}
				base = sinfo.coid + 1;
			}
		}

		return fdinfo_max - max;
}

int procmgr_spawn(resmgr_context_t *ctp, void *vmsg, proc_create_attr_t *partlist) {
	struct loader_context				*lcp;
	struct _thread_attr					attr;
	int									sig;
	int									status;
	PROCESS								*prp;
	unsigned							msgsize;
	pid_t								ppid = ctp->info.pid;
	union msg_s {
		proc_spawn_t						spawn;
		proc_spawn_args_t					spawn_args;
		proc_spawn_debug_t					spawn_debug;
		proc_spawn_fd_t						spawn_fd;
		proc_spawn_done_t					spawn_done;
		proc_spawn_dir_t					spawn_dir;
		proc_fork_t							fork;
	}									*msg = vmsg;
	PROCESS								*pprp = NULL;
	proc_create_attr_t					_partlist;

	/* always make a local copy of <partlist> so we don't alter incoming parameter */
	if (partlist != NULL) {
		_partlist = *partlist;
	} else {
		_partlist.mpart_list = NULL;
		_partlist.spart_list = NULL;
	}
	partlist = &_partlist;

	/*
	 * if remote spawn, need to use a local parent process for partition
	 * associations
	*/
	if(ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) != 0) {
		/* set ppid to netmgr's pid in network spawn case */
		ppid = pathmgr_netmgr_pid();
		if(ppid == 0) {
			ppid = SYSMGR_PID;
		}
	}

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->spawn.i.subtype);
		switch(msg->spawn.i.subtype) {
		case _PROC_SPAWN_START:
			ENDIAN_SWAP16(&msg->spawn.i.searchlen);
			ENDIAN_SWAP16(&msg->spawn.i.pathlen);
			ENDIAN_SWAP32(&msg->spawn.i.parms.flags);
			ENDIAN_SWAP32(&msg->spawn.i.parms.pgroup);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigmask.__bits[0]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigmask.__bits[1]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigdefault.__bits[0]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigdefault.__bits[1]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigignore.__bits[0]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.sigignore.__bits[1]);
			ENDIAN_SWAP32(&msg->spawn.i.parms.stack_max);
			ENDIAN_SWAP32(&msg->spawn.i.parms.policy);
			ENDIAN_SWAP32(&msg->spawn.i.parms.nd);
			ENDIAN_SWAP32(&msg->spawn.i.parms.runmask);
			ENDIAN_SWAP32(&msg->spawn.i.parms.param.sched_priority);
			ENDIAN_SWAP32(&msg->spawn.i.parms.param.sched_curpriority);
			ENDIAN_SWAP16(&msg->spawn.i.nfds);
			ENDIAN_SWAP16(&msg->spawn.i.nargv);
			ENDIAN_SWAP16(&msg->spawn.i.narge);
			ENDIAN_SWAP32(&msg->spawn.i.nbytes);
			break;
		case _PROC_SPAWN_FD:
			ENDIAN_SWAP16(&msg->spawn_fd.i.flags);
			ENDIAN_SWAP16(&msg->spawn_fd.i.nfds);
			ENDIAN_SWAP32(&msg->spawn_fd.i.base);
			ENDIAN_SWAP32(&msg->spawn_fd.i.ppid);
			break;
		default:
			return EENDIAN;
		}
	}

	/* if 'mempart_list' == NULL, set up the associated partition list from the parent */
	if (partlist->mpart_list == NULL)
	{
		/*
		 * note that for speed, we pre-allocate the 'mempart_list' using alloca()
		 * instead of having inherit_mempart_list() malloc() the memory for us. This
		 * requires that the number of associated partitions for <ppid> be determined
		 * first
		*/ 
		int  num_parts, r;
		mempart_flags_t getlist_flags = mempart_flags_t_GETLIST_INHERITABLE | mempart_flags_t_GETLIST_CREDENTIALS;
		struct _cred_info cred;
		
		pprp = proc_lock_pid(ppid);
		CRASHCHECK(pprp == NULL);

		cred = pprp->cred->info;

		num_parts = MEMPART_GETLIST(pprp, NULL, 0, getlist_flags, &cred);
		if ((num_parts < 0) || ((partlist->mpart_list = alloca(PART_LIST_T_SIZE(num_parts))) == NULL))
		{
			proc_unlock(pprp);
			/* if no partitions returned, process likely has not got proper permissions */
			return (num_parts < 0) ? EACCES : ENOMEM;
		}
		
		partlist->mpart_list->num_entries = num_parts;
		if ((r = inherit_mempart_list(pprp, &partlist->mpart_list)) != EOK) {
			proc_unlock(pprp);
			return r;
		}

		/* 
		 * By default, the mempart_list->i[].flags are inherited from the
		 * parent. However, in the case of the the initial processes created
		 * by procnto, we do not want this inheritance. Instead we want to use
		 * the 'default_mempart_flags'. These flags will apply only to the
		 * partition of the system memory class since that is (currently) the
		 * only partition procnto is associated with but we loop anyway
		*/
		if (pprp == procnto_prp)
		{
			unsigned i;
			for (i=0; i<partlist->mpart_list->num_entries; i++) {
				partlist->mpart_list->i[i].flags = default_mempart_flags;
			}
		}
		proc_unlock(pprp);
	}

	/* if 'schedpart_list' == NULL, set up the associated partition list from the parent */
	if (SCHEDPART_INSTALLED() && (partlist->spart_list == NULL))
	{
		int  num_parts, r;
		schedpart_flags_t getlist_flags = schedpart_flags_t_GETLIST_INHERITABLE | schedpart_flags_t_GETLIST_CREDENTIALS;
		struct _cred_info cred;
		
		pprp = proc_lock_pid(ppid);
		CRASHCHECK(pprp == NULL);

		cred = pprp->cred->info;
		num_parts = SCHEDPART_GETLIST(pprp, NULL, 0, getlist_flags, &cred);
		if ((num_parts < 0) || ((partlist->spart_list = alloca(PART_LIST_T_SIZE(num_parts))) == NULL))
		{
			proc_unlock(pprp);
			/* if no partitions returned, process likely has not got proper permissions */
			return (num_parts < 0) ? EACCES : ENOMEM;
		}

		partlist->spart_list->num_entries = num_parts;
		if ((r = inherit_schedpart_list(pprp, &partlist->spart_list)) != EOK) {
			proc_unlock(pprp);
			return r;
		}

		/* 
		 * By default, the schedpart_list->i[].flags are inherited from the
		 * parent. However, in the case of the the initial processes created
		 * by procnto, we do not want this inheritance. Instead we want to use
		 * the 'default_schedpart_flags'
		*/
		if (pprp == procnto_prp)
		{
			unsigned i;
			for (i=0; i<partlist->spart_list->num_entries; i++) {
				partlist->spart_list->i[i].flags = default_schedpart_flags;
			}
		}
		proc_unlock(pprp);	
	}

	switch(msg->spawn.i.subtype) {
	case _PROC_SPAWN_START:
	{
		unsigned off, len;
		unsigned int i;
		/*
		 * make sure a scheduling partition is available either through inheritance
		 * or explicitly specified. If it was not specified in a posix_spawn()
		 * call, it will be caught at above so we can assume that a if a scheduler
		 * partition is not found here it is because of access permissions
		 * (ie. SCHEDPART_GETLIST() returned an empty set)
		*/
		if (SCHEDPART_INSTALLED()) {
			if (partlist->spart_list->num_entries < 1)
				return EACCES;
		}

		/*
		 * make sure a sysram partition is available either through inheritance
		 * or explicitly specified. We can assume that a if a sysram memory class
		 * partition is not found here it is because of access permissions
		*/
		for (i=0; i<partlist->mpart_list->num_entries; i++)
			if (mempart_get_classid(partlist->mpart_list->i[i].id) == sys_memclass_id)
				break;
		if (i >= partlist->mpart_list->num_entries)
			return EACCES;

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
		CRASHCHECK(mempart_get_classid(partlist->mpart_list->i[0].id) != sys_memclass_id);

		off = msgsize = sizeof(struct _proc_spawn)
					+ msg->spawn.i.searchlen
					+ msg->spawn.i.pathlen
					+ msg->spawn.i.nfds * sizeof(int32_t)
					+ msg->spawn.i.nbytes;

		/* most time one msg can hold full args */
		if(msgsize > sizeof(lcp->msg)) {
			msgsize -= msg->spawn.i.nbytes;
			if(msgsize < sizeof(lcp->msg)) {
				msgsize = sizeof(lcp->msg);
			}
		}

		len = 0; /* to get rid of a warning */
		if(ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) != 0) {
			/* remote spawn, need extra buffer */
			if(ctp->info.srcmsglen > off + SPAWN_REMOTE_REMOTEBUF_SIZE) {
				return EINVAL;
			}
			/* make the length int32 aligned */
			msgsize = (msgsize + sizeof(_Int32t) -1) & ~(sizeof(_Int32t) -1);
			msgsize += (len = ctp->info.srcmsglen - off);
		}

		if(!(lcp = procmgr_context_alloc(msgsize, LC_SPAWN | (((ctp->info.flags & _NTO_MI_ENDIAN_DIFF)) ? LC_CROSS_ENDIAN : 0)))) {
			return ENOMEM;
		}
			
		lcp->flags = msg->spawn.i.parms.flags;
		lcp->pnode = ctp->info.nd;
		ppid = lcp->ppid = ctp->info.pid;
		if(ND_NODE_CMP(lcp->pnode, ND_LOCAL_NODE) != 0) {

			/* set ppid to netmgr's pid in network spawn case */
			ppid = pathmgr_netmgr_pid();
			if(ppid == 0) {
				ppid = SYSMGR_PID;
			}
			msgsize -= len;
			lcp->remote_off = msgsize;
			if(msgsize + len <= ctp->info.msglen) {
				memcpy((char*)&lcp->msg + lcp->remote_off, (char*)msg + off, len);
			} else {
				(void)MsgRead(ctp->rcvid, (char*)&lcp->msg + lcp->remote_off, len, off);
			}
		} 
		if(msgsize <= ctp->info.msglen) {
			memcpy(&lcp->msg, msg, msgsize);
		} else {
			memcpy(&lcp->msg, msg, (len = ctp->info.msglen));
			(void)MsgRead(ctp->rcvid, (char*)&lcp->msg + len, msgsize - len, len);
		}
		lcp->rcvid = ctp->rcvid;

		if(msg->spawn.i.parms.flags & SPAWN_SETSIGMASK) {
			lcp->mask = msg->spawn.i.parms.sigmask;
		} else {
			SignalProcmask(ppid, ctp->info.tid, 0, 0, &lcp->mask);
		}

#ifndef NDEBUG
		if (MEMPART_INSTALLED() && (partlist->mpart_list == NULL))
		{
			kprintf("ins: %d\n", MEMPART_INSTALLED());
			crash();
		}
#endif	/* NDEBUG */

		proc_wlock_adp(sysmgr_prp);
		prp = lcp->process = ProcessCreate(ppid, lcp, partlist);
		proc_unlock_adp(sysmgr_prp);
		if(prp == (void *)-1) {
			procmgr_context_free(lcp);
			return errno;
		}
		if(ND_NODE_CMP(lcp->pnode, ND_LOCAL_NODE) != 0) {
			struct _client_info info;
			
			// network spawn case
			ConnectClientInfo(ctp->info.scoid, &info, NGROUPS_MAX);
			info.cred.sgid = info.cred.egid;
			info.cred.suid = 0; // will be set by the loader
			CredSet(prp->pid, &info.cred);
		}

		lcp->pid = prp->pid;

		if(!(lcp->msg.spawn.i.parms.flags & SPAWN_SETSIGDEF)) {
			sigemptyset(&lcp->msg.spawn.i.parms.sigdefault);
		}
		lcp->msg.spawn.i.parms.flags |= SPAWN_SETSIGDEF;
		for(sig = _SIGMIN; sig <= _SIGMAX; sig++) {
			if((lcp->msg.spawn.i.parms.flags & SPAWN_SETSIGIGN) && sigismember(&lcp->msg.spawn.i.parms.sigignore, sig)) {
				sigdelset(&lcp->msg.spawn.i.parms.sigdefault, sig);
			} else if(!sigismember(&lcp->process->sig_ignore, sig)) {
				sigaddset(&lcp->msg.spawn.i.parms.sigdefault, sig);
			}
		}

		if((prp = proc_lock_pid(ppid))) {
			lcp->process->root = pathmgr_node_clone(prp->root);
			lcp->process->cwd = pathmgr_node_clone(prp->cwd);

			proc_unlock(prp);
		}
		
		/* For remote spawn, "root directory" was generated on another node.
		 * Check if it starts with the network path to the local node (e.g. "on -f") 
		 * and if it does remove it.  Do it here since netmgr_xxx() functions can't 
		 * be called by the loader thread while in loader_load (PR21182)
		 */
		if(ND_NODE_CMP(lcp->pnode, ND_LOCAL_NODE) != 0) {
			proc_spawn_remote_t *premote;
			char *p0, *p1;
			
			premote = (proc_spawn_remote_t*)((char*)&lcp->msg.spawn.i + lcp->remote_off);
			
			p0 = (char*)premote + sizeof(*premote);
			if(proc_thread_pool_reserve() != 0) {
				return EAGAIN;
			}
			// Test whether the root directory is local.
			if( netmgr_strtond(p0, &p1) == ND_LOCAL_NODE ) {
				// It is local.  Strip the network identifier off the root path.
				unsigned lenp1 = strlen(p1);
				if (lenp1==0) {
					// There's nothing after the network identifier, so root is "/".
					p0[0]='/';
					p0[1]='\0';
				} else {
					// Copy everything after the network name to the start of premote->root
					memmove(p0, p1-1, min(lenp1,premote->root_len));
				}
			}
			proc_thread_pool_reserve_done(); 
		}


		procmgr_thread_attr(&attr, lcp, ctp);
		if(msg->spawn.i.parms.flags & SPAWN_EXPLICIT_SCHED) {
			memcpy(&attr.__param, &msg->spawn.i.parms.param, sizeof attr.__param);
			if (msg->spawn.i.parms.policy != SCHED_NOCHANGE) {
				attr.__policy = msg->spawn.i.parms.policy;
			}
		}

		if((status = ThreadCreate_r((prp = lcp->process)->pid, loader_load, lcp, &attr)) < 0) {
			// Turn terming flag on, we're not doing a thread_destroy
			prp->flags |= _NTO_PF_TERMING;
			(void)MsgSendPulse_r(PROCMGR_COID, attr.__param.__sched_priority, PROC_CODE_TERM, prp->pid);
			return -status;
		}
		return _RESMGR_NOREPLY;
	}

	case _PROC_SPAWN_DEBUG:
		if(ctp->info.nd != ND_LOCAL_NODE || ProcessStartup(ctp->info.pid, 0) == (void *)-1) {
			break;
		}
		if((prp = proc_lock_pid(ctp->info.pid))) {
			size_t  slen = strlen(msg->spawn_debug.i.name) + 1;
			if(prp->debug_name) {
				free(prp->debug_name);
			}
			if(!(prp->debug_name = malloc(slen))) {
				return proc_error(ENOMEM, prp);
			}
			memcpy(prp->debug_name, msg->spawn_debug.i.name, slen);	// '\0' is guaranteed to be copied
			prp->base_addr = msg->spawn_debug.i.text_addr;
#ifndef NKDEBUG
			memset(&prp->kdebug, 0x00, sizeof prp->kdebug);
			prp->kdebug.ptr = prp;
			prp->kdebug.type = KDEBUG_TYPE_PROCESS;
			prp->kdebug.text_addr = msg->spawn_debug.i.text_addr;
			prp->kdebug.text_size = msg->spawn_debug.i.text_size;
			prp->kdebug.text_reloc = msg->spawn_debug.i.text_reloc;
			prp->kdebug.data_addr = msg->spawn_debug.i.data_addr;
			prp->kdebug.data_size = msg->spawn_debug.i.data_size;
			prp->kdebug.data_reloc = msg->spawn_debug.i.data_reloc;

			(void)kdebug_attach(&prp->kdebug, 0);
#endif
			return proc_error(EOK, prp);
		}
		break;

	/* In remote spawn, when the fd map is too big to be passed at one shot
	by remote spawn, the remote loader will send this msg to ask for more fd
	map. In local spawn, when fds are from another network node, the loader
	will send this message to get fdinfo. */
	case _PROC_SPAWN_FD: {
		int nfds, max;

		nfds = 0;
		max = msg->spawn_fd.i.nfds;
		ctp->iov[1].iov_base = (char *)&msg->spawn_fd.i + sizeof msg->spawn_fd.i;
		if(msg->spawn_fd.i.flags & _PROC_SPAWN_FD_LIST) {
			nfds = max;
			ctp->iov[1].iov_base = (char*)ctp->iov[1].iov_base + nfds * sizeof(_Int32t);
		}
		max = min(max, (ctp->msg_max_size - ((uintptr_t)ctp->iov[1].iov_base - (uintptr_t)&msg->spawn_fd))/sizeof(struct _proc_spawn_fd_info));
		nfds = get_fdinfo(msg->spawn_fd.i.ppid, ctp->info.nd, msg->spawn_fd.i.base, max, ctp->iov[1].iov_base, nfds, (int *)((char *)&msg->spawn_fd + sizeof msg->spawn_fd.i), msg->spawn_fd.i.flags);
		if(nfds == -1) {
			return errno;
		}
		msg->spawn_fd.o.flags = 0;
		if(nfds >= max) {
			msg->spawn_fd.o.flags = _PROC_SPAWN_FDREPLY_MORE;
		}
		msg->spawn_fd.o.nfds = nfds;

		ctp->iov[0].iov_base = (caddr_t)&msg->spawn_fd.o;
		ctp->iov[0].iov_len = sizeof msg->spawn_fd.o;
		ctp->iov[1].iov_len = nfds * sizeof(struct _proc_spawn_fd_info);
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAP16(&msg->spawn_fd.o.flags);
			ENDIAN_SWAP16(&msg->spawn_fd.o.nfds);
		}
		return _RESMGR_NPARTS(2);
	}

	case _PROC_SPAWN_REMOTE: {
		char *buff, *ptr, *p;
		spawn_remote_t remote;
		proc_spawn_remote_t remote2;
		struct inheritance *parms;
		int size, len, max, nfds;

		parms = &msg->spawn.i.parms;
		if(!((parms->flags & SPAWN_SETND) && ND_NODE_CMP(parms->nd, ND_LOCAL_NODE) != 0)) {
			return EINVAL;
		}
		parms->flags |= (SPAWN_SETSID | SPAWN_TCSETPGROUP);

		ptr = buff = (char*)_smalloc(size = SPAWN_REMOTE_REMOTEBUF_SIZE - sizeof(remote2));
		if(ptr == NULL) {
			return ENOMEM;
		}
		remote.nd = parms->nd;
		remote.pid = PROCMGR_PID;
		remote.chid = PROCMGR_CHID;
		remote2.key = 0;

		/* get umask, root, cwd and params */
		if(!(parms->flags & SPAWN_SETSIGMASK)) {
			parms->flags |= SPAWN_SETSIGMASK;
			SignalProcmask(ctp->info.pid, ctp->info.tid, 0, 0, &parms->sigmask);
		}

		if(!(parms->flags & SPAWN_EXPLICIT_SCHED)) {
			parms->flags |= SPAWN_EXPLICIT_SCHED;
			parms->policy = SchedGet(ctp->info.pid, ctp->info.tid, &parms->param);
		}

		if((prp = proc_lock_pid(ctp->info.pid))) {
			if(!(parms->flags & SPAWN_SETSIGIGN)) {
				parms->flags |= SPAWN_SETSIGIGN;
				parms->sigignore = prp->sig_ignore;
			}

			if(!(parms->flags & SPAWN_SETSIGDEF)) {
				parms->flags |= SPAWN_SETSIGDEF;
				sigemptyset(&parms->sigdefault);
				for(sig = _SIGMIN; sig <= _SIGMAX; sig ++) {
					if(!sigismember(&parms->sigignore, sig)) {
						sigaddset(&parms->sigdefault, sig);
					}
				}
			}
	
			remote2.umask = prp->umask;

			max = 2 * PATH_MAX;
			len = pathmgr_node2fullpath(prp->root, ptr, max);
			if(len == -1) {
				_sfree(buff, size);
				return proc_error(errno, prp);
			} else if(len > max) {
				_sfree(buff, size);
				return proc_error(ENAMETOOLONG, prp);
			}

			if(proc_thread_pool_reserve() != 0) {
				return proc_error(EAGAIN, prp);
			}
			if(netmgr_strtond(ptr,&p) != -1) {
				p--;
				if(netmgr_strtond(p,NULL) != -1) {
					len = strlen(p)+1;
					STRLCPY(ptr, p, size);
				}
			}
			proc_thread_pool_reserve_done();

			remote2.root_len = len;
			max -= len;
			ptr += len;
			len = pathmgr_node2fullpath(prp->cwd, ptr, max);
			if(len == -1) {
				_sfree(buff, size);
				return proc_error(errno, prp);
			} else if(len > max) {
				_sfree(buff, size);
				return proc_error(ENAMETOOLONG, prp);
			}

			proc_unlock(prp);
			remote2.cwd_len = len;
			ptr += len;
			/* make ptr int aligned */
			max = (uintptr_t)ptr & (sizeof(_Uint32t) - 1);
			if(max) {
				ptr += sizeof(_Uint32t) - max;
				len += sizeof(_Uint32t) - max;
			}
			max = (size - len) - remote2.root_len;
		} else {
			_sfree(buff, size);
			return ESRCH;
		}

		/* get fd array */
		max = max / (int)sizeof(struct _proc_spawn_fd_info);
		nfds = get_fdinfo(ctp->info.pid, parms->nd, 0, max, (struct _proc_spawn_fd_info*)ptr, msg->spawn.i.nfds, (int*)((char*)&msg->spawn.i + sizeof msg->spawn.i), 0);
		if(nfds == -1) {
			_sfree(buff, size);
			return errno;
		}
		remote2.nfds = nfds;
		if(max > nfds) {
			remote2.flags |= _PROC_SPAWN_REMOTE_FLAGS_FDALLIN;
		}

		ctp->iov[0].iov_base = &msg->spawn.i;
		ctp->iov[0].iov_len = sizeof msg->spawn.i;
		ctp->iov[1].iov_base = &remote;
		ctp->iov[1].iov_len = sizeof remote;
		ctp->iov[2].iov_base = &remote2;
		ctp->iov[2].iov_len = sizeof remote2;
		ctp->iov[3].iov_base = buff;
		ctp->iov[3].iov_len = ptr + nfds * sizeof(struct _proc_spawn_fd_info) - buff;
		remote.size = ctp->iov[3].iov_len + sizeof remote2;
		if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
			ENDIAN_SWAP32(&remote.nd);
			ENDIAN_SWAP32(&remote.pid);
			ENDIAN_SWAP32(&remote.chid);
			ENDIAN_SWAP32(&remote.size);
			ENDIAN_SWAP32(&remote2.key);
			ENDIAN_SWAP32(&remote2.umask);
			ENDIAN_SWAP16(&remote2.nfds);
			ENDIAN_SWAP16(&remote2.root_len);
			ENDIAN_SWAP16(&remote2.cwd_len);
			ENDIAN_SWAP16(&remote2.flags);
		}
		MsgReplyv(ctp->rcvid, EOK, &ctp->iov[0], 4);
		
		_sfree(buff, size);
		return _RESMGR_NOREPLY;
	}

	case _PROC_SPAWN_ARGS:
		if(ctp->info.nd == ND_LOCAL_NODE && (lcp = ProcessStartup(ctp->info.pid, 0)) != (void *)-1) {
			void		*buff = &msg->spawn_args.i + sizeof msg->spawn_args.i;
			unsigned	off = 0;
			unsigned	len;

			while(msg->spawn_args.i.nbytes) {
				len = min(msg->spawn_args.i.nbytes, 512); //ctp->ctrl->msg_max_size - sizeof msg->spawn_args.i);
				if(MsgRead(lcp->rcvid, buff, len, msg->spawn_args.i.offset) == -1) {
					return errno;
				}
				if(MsgWrite(ctp->rcvid, buff, len, off) == -1) {
					return errno;
				}
				off += len;
				msg->spawn_args.i.offset += len;
				msg->spawn_args.i.nbytes -= len;
			}
			return EOK;
		}
		break;

	case _PROC_SPAWN_EXEC:
		if(ctp->info.nd != ND_LOCAL_NODE || (lcp = ProcessStartup(ctp->info.pid, 0)) == (void *)-1) {
			break;
		}
		lcp->state = LC_EXEC_SWAP;
		if (lcp->flags & SPAWN_NOZOMBIE) {
			PROCESS *zprp;
			zprp = proc_lock_pid(lcp->ppid);
			if (zprp) {
				procmgr_nozombie(zprp, EOK);
				proc_unlock(zprp);
			}
		}
		{
			int rcvid = lcp->rcvid;

			if (ProcessExec(lcp, &lcp->rcvid, ctp->rcvid, lcp->ppid) == -1) {
				MsgError(ctp->rcvid, errno);
				return _RESMGR_NOREPLY;
			}

			MsgError(rcvid, EOK);
		}
		return _RESMGR_NOREPLY;

	case _PROC_SPAWN_DONE:
		if(ctp->info.nd != ND_LOCAL_NODE) {
			break;
		}
		ProcessBind(ctp->info.pid);
		lcp = ProcessStartup(ctp->info.pid, 1);
		ProcessBind(0);
		if(lcp == (void *)-1) {
			break;
		}
		if((lcp->state & LC_STATE_MASK) == LC_FORK) {
			SignalProcmask(ctp->info.pid, 1, SIG_SETMASK, &lcp->mask, 0);
		} 
		if(((lcp->state & LC_STATE_MASK) != LC_FORK) || (lcp->flags & _FORK_ASPACE)) {
			MsgReplyv(lcp->rcvid, ctp->info.pid, 0, 0);
		}
		procmgr_context_free(lcp);
		return EOK;

	default:
		break;
	}
	return ENOSYS;
}

struct loader_context *
procmgr_exec(resmgr_context_t *ctp, void *msg, PROCESS *prp) {
	PROCESS					*child = NULL;
	struct loader_context	*lcp = prp->lcp;
	struct sched_param		params;

	// Need to access child through lcp->pid, lcp->process may be invalid
	if(MsgInfo(lcp->rcvid, &ctp->info) == -1 || !prp->child 
			|| (prp->pid != lcp->ppid) || !(child = proc_lock_pid(lcp->pid)) 
			|| (child->parent != prp) || (child->flags & (_NTO_PF_TERMING | _NTO_PF_ZOMBIE))) {
		// Child started terming or is gone; don't try to swap
		// Need to term again without LC_EXEC_SWAP set
		lcp->state = LC_TERM;
		prp->flags |= _NTO_PF_TERMING;
		(void)MsgSendPulse_r(PROCMGR_COID, -1, PROC_CODE_TERM, prp->pid);
		if(child) {
			proc_unlock(child);
		}
		if(ctp->info.pid == lcp->pid) {
			 MsgReplyv(lcp->rcvid, 0, 0, 0);
		} else {
			// no reply
		}
		return NULL; // Still need loader context
	} else {

if(child->lcp) crash();
		prp->lcp = NULL;
		child->lcp = lcp;

		//Run the termination thread at a slightly higher priority than
		//that of the child. This way the termination will complete
		//before the child begins to run - gets the loader context
		//and such released. All in all, saves on system resources.
		params.sched_priority = child->process_priority + 1;
		(void)SchedSet(0, 0, SCHED_NOCHANGE, &params);

		(void)ProcessSwap(prp, child);
		ProcessBind(child->pid);
		(void)ProcessStartup(child->pid, 1);
		ProcessBind(0);
		proc_unlock(child);
		MsgReplyv(lcp->rcvid, 0, 0, 0);
		return lcp;	// termer can get rid of loader context
	}
}


__SRCVERSION("procmgr_spawn.c $Rev: 198777 $");
