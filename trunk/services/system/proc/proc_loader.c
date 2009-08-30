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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <share.h>
#include <sys/dcmd_all.h>
#include <sys/stat.h>
#include "externs.h"
#include <termios.h>

static void set_signal(int signo, void (*func)(int)) {
	struct sigaction act;

	act.sa_sigaction = act.sa_handler = func;
	act.sa_mask.__bits[0] = act.sa_mask.__bits[1] = 0;
	act.sa_flags = 0;
	(void)SignalAction(0, 0, signo, &act, 0);
}

void loader_exit(resmgr_context_t *ctp, union proc_msg_union *msg, PROCESS *prp) {
	struct loader_context	*lcp;
	int						r;

	if((lcp = prp->lcp)) {
		if(lcp->rcvid != -1) {
			switch(prp->siginfo.si_signo) {
			case SIGCHLD:	
				r = prp->siginfo.si_status;
				break;
			case SIGBUS:
				r = (lcp->fault_errno != EOK) ? lcp->fault_errno : ENOMEM;
				break;
			default:		
				r = EINTR;
				break;
			}
			MsgError(lcp->rcvid, r);
		}
		procmgr_context_free(lcp);
		prp->lcp = 0;
	}
}

/*
	In local case, this function gets the fdlist (need to be duped) either by reading
	the fd list area in *lcp when a fd array is passed in by spawn or by walking the
	fd list of its parents when inheritant case. Then,  ConnectServerInfo is called
	to get the fd infos and dup messages are sent to the servers to get them duped.
	When any fds are connected to other network nodes, a message is sent to proc to query.

	In remote case, at least the first 10 fds' info are passed in by the remote
	node and stored in *lcp. The function sends dup messages to servers based
	on those info. If the fd list is longer than stored (which is very rare),
	a message is sent to the remote node's proc to make queries.
*/
static int loader_fd(struct loader_context *lcp) {
	struct _server_info info;
	unsigned flags;
	int *pfd, fd2, fd, nfds, i_nfds;
	enum {
		_FD_SPAWN_LIST,
		_FD_FORK,
		_FD_SPAWN_NOCLOEXC,
		_FD_SPAWN_REMOTE,
		_FD_SPAWN_QUERY
	}	type;
	io_dup_t						msgdup;
	struct _proc_spawn_fd_info		*pfdinfo;
	struct _proc_spawn_fd_info		temp_buff[32];
	int	coid, size;

	type = _FD_FORK;
	coid = SYSMGR_COID;
	size = 0;
	/* the following three lines are used to kill warnings */
	flags = 0;
	pfdinfo = NULL;
	pfd = NULL;
	if((lcp->state & LC_STATE_MASK) != LC_FORK) {
		nfds = i_nfds = lcp->msg.spawn.i.nfds;

		if(ND_NODE_CMP(lcp->pnode, ND_LOCAL_NODE) == 0) {
			type = _FD_SPAWN_NOCLOEXC;
			if(nfds != 0) {
				type = _FD_SPAWN_LIST;
				pfd = (int*)((char *)&lcp->msg.spawn.i + sizeof lcp->msg.spawn.i);
			}
		} else {
			proc_spawn_remote_t *p;
			 
			p = (proc_spawn_remote_t*) ((char*)&lcp->msg.spawn.i + lcp->remote_off);
			if(lcp->state & LC_CROSS_ENDIAN) {
				/*
				 * Put here by _PROC_SPAWN_REMOTE, skipped by lcp->remote_off?
				 * ENDIAN_SWAP32(&remote.nd);
				 * ENDIAN_SWAP32(&remote.pid);
				 * ENDIAN_SWAP32(&remote.chid);
				 * ENDIAN_SWAP32(&remote.size);
				 */
				ENDIAN_SWAP32(&p->key);
				ENDIAN_SWAP32(&p->umask);
				ENDIAN_SWAP16(&p->nfds);
				ENDIAN_SWAP16(&p->root_len);
				ENDIAN_SWAP16(&p->cwd_len);
				ENDIAN_SWAP16(&p->flags);
			}

			info.flags = 0;
			type = _FD_SPAWN_REMOTE;
			nfds = p->nfds;
			flags = p->flags;
			pfdinfo = (struct _proc_spawn_fd_info*) (((uintptr_t)p + sizeof(*p) + p->root_len + p->cwd_len + sizeof(_Int32t) - 1) & ~(sizeof(_Int32t)-1));
		}
	} else {
		nfds = i_nfds = 0;
	}

	fd = -1;
	errno = EMFILE;
	msgdup.i.type = _IO_DUP;
	msgdup.i.combine_len = sizeof msgdup.i;
	msgdup.i.info.pid = lcp->ppid;
	msgdup.i.info.tid = 0;
	msgdup.i.info.nd = ND_LOCAL_NODE;
	
	while(1) {
		switch(type) {
			case _FD_SPAWN_LIST:
				if(++fd >= nfds) {
					return EOK;
				}
				CRASHCHECK(pfd == NULL);
				if((fd2 = *(pfd++)) == SPAWN_FDCLOSED) {
					continue;
				}
				if((ConnectServerInfo(lcp->ppid, fd2, &info) != fd2) || (info.flags & _NTO_COF_DEAD)) {
					return EBADF;
				}
				info.flags = 0;
				if(ND_NODE_CMP(info.nd, ND_LOCAL_NODE) != 0) {
					type = _FD_SPAWN_QUERY;
					nfds = 0;
					fd--;
					continue;
				}
			break;

			case _FD_FORK:
			case _FD_SPAWN_NOCLOEXC:
				if(ConnectServerInfo(lcp->ppid, ++fd, &info) == -1 || ((fd = info.coid) & _NTO_SIDE_CHANNEL)) {
					return EOK;
				}
				if(info.flags & _NTO_COF_DEAD) {
					continue;
				}
				if((info.flags & _NTO_COF_CLOEXEC) && (type != _FD_FORK)) {
					continue;
				}
				if(ND_NODE_CMP(info.nd, ND_LOCAL_NODE) != 0) {
					type = _FD_SPAWN_QUERY;
					fd--;
					continue;
				}
			break;

			case _FD_SPAWN_QUERY:
			case _FD_SPAWN_REMOTE:
				if(nfds-- <= 0) {
					if(flags & _PROC_SPAWN_REMOTE_FLAGS_FDALLIN) {
						/* finish */
						if(coid != SYSMGR_COID) {
							ConnectDetach(coid);
						}
						return EOK;
					}

					/* read additional fd info */
					{
						proc_spawn_fd_t		spawnfd;

						fd++;

						if(type == _FD_SPAWN_REMOTE) {
							proc_spawn_remote_t *p;

							p = (proc_spawn_remote_t*) ((char*)&lcp->msg.spawn.i + lcp->remote_off);
	
							if(coid == SYSMGR_COID) {
								/* make a connection to remote proc */
								if((coid = ConnectAttach(lcp->pnode, PROCMGR_PID, PROCMGR_CHID, _NTO_SIDE_CHANNEL, 0)) == -1) {
									return errno;
								}
							}
			 
							pfdinfo = (struct _proc_spawn_fd_info*) ((char*)p + sizeof(*p));
							nfds = (SPAWN_REMOTE_REMOTEBUF_SIZE - sizeof(*p))/sizeof(*pfdinfo);
						} else {
							size = sizeof(lcp->msg.spawn.i) + i_nfds * sizeof(_Int32t);
							pfdinfo = (struct _proc_spawn_fd_info*) ((char*)&lcp->msg.spawn.i + size);
							size = sizeof(lcp->msg) - size;
							if(size < sizeof(temp_buff)) {
								size = sizeof(temp_buff);
								pfdinfo = temp_buff;
							} 
							nfds = size/(int)sizeof(*pfdinfo);
						}

						SETIOV(lcp->iov + 0, &spawnfd.i, sizeof spawnfd.i);
						SETIOV(lcp->iov + 2, &spawnfd.o, sizeof spawnfd.o);
						SETIOV(lcp->iov + 3, pfdinfo, nfds * sizeof(*pfdinfo));

						spawnfd.i.type = _PROC_SPAWN;
						spawnfd.i.subtype = _PROC_SPAWN_FD;
						spawnfd.i.base = fd;
						spawnfd.i.ppid = lcp->ppid;

						spawnfd.i.flags = 0;
						if(i_nfds) {
							spawnfd.i.nfds = min(nfds, i_nfds - fd);
							SETIOV(lcp->iov + 1, (char *)&lcp->msg.spawn.i + sizeof lcp->msg.spawn.i + fd * sizeof(_Int32t), spawnfd.i.nfds * sizeof(_Int32t));
							spawnfd.i.flags |= _PROC_SPAWN_FD_LIST;
						} else {
							spawnfd.i.nfds = nfds;
							SETIOV(lcp->iov + 1, lcp->iov[3].iov_base, 0);
							if((lcp->state & LC_STATE_MASK) != LC_FORK) {
								spawnfd.i.flags |= _PROC_SPAWN_FD_NOCLOEXC;
							}
						}

						if(MsgSendv(coid, lcp->iov + 0, 2, lcp->iov + 2, 2) == -1) {
							if(coid != SYSMGR_COID) {
								ConnectDetach(coid);
							}
							return errno;
						}

						nfds = spawnfd.o.nfds;
						if(nfds-- == 0) {
							/* finish */
							if(coid != SYSMGR_COID) {
								ConnectDetach(coid);
							}
							return EOK;
						}
						if(i_nfds) {
							if((pfdinfo + nfds)->fd + 1 >= i_nfds) {
								/* set finish flag */
								flags |= _PROC_SPAWN_REMOTE_FLAGS_FDALLIN;
							}
						} else {
							if(!(spawnfd.o.flags & _PROC_SPAWN_FDREPLY_MORE)) {
								/* set finish flag */
								flags |= _PROC_SPAWN_REMOTE_FLAGS_FDALLIN;
							}
						}
					}
				}
				CRASHCHECK(pfdinfo == NULL);
				if(lcp->state & LC_CROSS_ENDIAN) {
					ENDIAN_SWAP32(&pfdinfo->fd);
					ENDIAN_SWAP32(&pfdinfo->nd);
					ENDIAN_SWAP32(&pfdinfo->pid);
					ENDIAN_SWAP32(&pfdinfo->chid);
					ENDIAN_SWAP32(&pfdinfo->scoid);
					ENDIAN_SWAP32(&pfdinfo->coid);
					ENDIAN_SWAP32(&pfdinfo->srcnd);
				}
				fd = pfdinfo->fd;
				info.nd = pfdinfo->nd;
				info.pid = pfdinfo->pid;
				info.chid = pfdinfo->chid;
				info.scoid = pfdinfo->scoid;
				info.coid = pfdinfo->coid;
				msgdup.i.info.nd = pfdinfo->srcnd;
				pfdinfo ++;
				break;
			default:
				crash();	
		}

		if(ConnectAttach(info.nd, info.pid, info.chid, fd, info.flags) != fd) {
			if(coid != SYSMGR_COID) {
				ConnectDetach(coid);
			}
			return errno;
		}

		if(!(info.chid & _NTO_GLOBAL_CHANNEL)) {
			msgdup.i.info.scoid = info.scoid;
			msgdup.i.info.coid = info.coid;
			if(MsgSend(fd, &msgdup.i, sizeof msgdup.i, 0, 0) == -1) {
				if(coid != SYSMGR_COID) {
					ConnectDetach(coid);
				}
				return errno;
			}
		}
	}
}

void *loader_fork(void *parm) {
	struct loader_context			*lcp = parm;
	PROCESS							*prp;
	int								fd, status = 0;
	int								sig;
	THREAD							*thp = lcp->process->valid_thp;
	sigset_t						mask;

	if(!thp) crash();

	lcp->tid = pthread_self();
	SignalProcmask(0, 0, SIG_SETMASK, &lcp->mask, 0);

	// Block all signals to avoid deadlock
	thp->sig_blocked.__bits[0] = thp->sig_blocked.__bits[1] = ~0;

	if((fd = ConnectAttach(0, PROCMGR_PID, PROCMGR_CHID, PROCMGR_COID, 0)) == -1) {
		ThreadDestroy(-1, -1, (void *)errno);
	} else if(fd != PROCMGR_COID) {
		ThreadDestroy(-1, -1, (void *)EL3HLT);
	}

	// Lookup the parent
	prp = proc_lookup_pid(lcp->ppid);
	if(proc_rlock_adp(prp) == -1) {
		ThreadDestroy(-1, -1, (void *)EL2HLT);
	}

	if(lcp->flags & _FORK_ASPACE) {
		if((proc_wlock_adp(lcp->process) == -1) || (status = memmgr.dup(prp, lcp->process))) {
			if(lcp->process->memory) {
				proc_unlock_adp(lcp->process);
			}
			proc_unlock_adp(prp);
			ThreadDestroy(-1, -1, (void *)status);
		}
		proc_unlock_adp(lcp->process);
	} else {
		lcp->start.stackaddr = (uintptr_t)lcp->msg.fork.i.frame;
		if(lcp->process->memory) {
			proc_wlock_adp(sysmgr_prp);
			while(ProcessShutdown(0, 0) == -1 && errno == EAGAIN) {
				/* nothing to do */
			}
			proc_unlock_adp(sysmgr_prp);
		}
		lcp->process->flags |= _NTO_PF_VFORKED;
		lcp->process->memory = prp->memory;
		ProcessBind(lcp->process->pid);
	}
	proc_unlock_adp(prp);

	if(prp->debug_name) {
		lcp->process->debug_name = strdup(prp->debug_name);
	}
	lcp->process->initial_esp = prp->initial_esp;
	lcp->process->base_addr = prp->base_addr;
	lcp->process->pls = prp->pls;
	lcp->process->umask = prp->umask;

	// Copy the signal handlers...
	mask = lcp->mask;
	for(sig = 1; sig < NSIG; sig++) {
		struct sigaction		act;

		if(SignalAction(lcp->ppid, 0, sig, 0, &act) != -1) {
			(void)SignalAction(lcp->process->pid, prp->sigstub, sig, &act, 0);
			sigaddset(&mask, sig);
		}
	}

	// Only enable signals that don't have a handler
	SignalProcmask(0, 0, SIG_SETMASK, &mask, 0);

	if(!(lcp->flags & _FORK_NOFDS)) {
		if((status = loader_fd(lcp)) != 0) {
			ThreadDestroy(-1, -1, (void *)status);
		}
	}

	lcp->msg.spawn_done.i.type = _PROC_SPAWN;
	lcp->msg.spawn_done.i.subtype = _PROC_SPAWN_DONE;
	SETIOV(lcp->iov + 0, &lcp->msg.spawn_done.i, sizeof lcp->msg.spawn_done.i);
	MsgSendv(PROCMGR_COID, lcp->iov, 1, 0, 0);
	ThreadDestroy(-1, -1, (void *)errno);
	return 0;
}

static int is_exec(const char *path) {
	struct stat 	st;

	if(stat(path, &st) == -1) {
		return -1;
	} else if(!S_ISREG(st.st_mode) && !S_ISNAM(st.st_mode)) {
		errno = EACCES;
		return -1;
	} else if(access(path, X_OK) == -1) {
		return -1;
	}
	return 0;
}

static int searchpath(const char *name, const char *path, char *buffer, int bufsize) {
	char				*b;
	int					n, trailing;


	if (name == NULL || *name == '\0' || path == NULL || *path == '\0') {
		errno = ENOENT;
	}
	else {

#ifndef NDEBUG
		/* assert proper args */
		if ((buffer == NULL) || (bufsize < strlen(name))) crash();
#endif

		bufsize -= strlen(name);
		do {
			for (n = bufsize, *(b = buffer) = 0; *path && *path != ':'; n--, path++) {
				if (n > 0) {
					*b++ = *path;
				}
			}
			if (n > 0) {
				if (*buffer && b[-1] != '/') {
					*b++ = '/';
				}
				STRLCPY(b, name, bufsize - (b - buffer));
				if (is_exec(buffer) != -1) {
					return 0;
				}
			}
			else {
				errno = ENAMETOOLONG;
				break;
			}
			trailing = (*path != ':') ? 0 : !*++path;	
		} while (*path || trailing);
	}
	*buffer = 0;
	return -1;
}

static void loader_args(struct loader_context *lcp, const char *p, const char *exefile,
						char *interp_name, char *interp_arg) {
	char							*aux_exefile;
	const char						**arg, *orig_exefile;
	char							*strings;
	int								naux;
	auxv_t							*auxv;
	int								exelen;
	int 							arg_bytes;
	int 							ninterp = 0, interplen = 0,
									interp_name_len = 0, interp_arg_len = 0;

	int skip_arg0 = 0;

	p += strlen(p) + 1;

	orig_exefile = NULL;
	if (interp_name) {
		orig_exefile = exefile;
		exefile = interp_name;
		interplen = interp_name_len = strlen(interp_name) + 1;
		interplen += strlen(orig_exefile) + 1;
		ninterp = 2;
	}

	if (interp_arg) {
		interp_arg_len = strlen(interp_arg) + 1;
		interplen += interp_arg_len;
		ninterp++;
	}

	arg_bytes  = lcp->msg.spawn.i.nbytes;
    arg_bytes += interplen;

#ifdef STACK_GROWS_UP
#error STACK_GROWS_UP not supported
#endif
	// strings is also top of stack
	exelen = strlen(exefile) + 1;
	aux_exefile = (char *)lcp->start.esp - exelen;
	strcpy(aux_exefile, exefile);
	if(exefile[0] != '/') {
		int	cwd_len;

		// not a full path, tack the CWD at the front
		cwd_len = pathmgr_node2fullpath(lcp->process->cwd, NULL, 0);
		if(cwd_len > 0) {
			int real_len;

			aux_exefile -= cwd_len;
			real_len = pathmgr_node2fullpath(lcp->process->cwd, aux_exefile, cwd_len);
			if(real_len > 0) {
				if(real_len < cwd_len) {
					// The first pathmgr_node2fullpath() can overestimate
					// the length of the CWD due to some qnet issues.
					// If that happens, we'll just slide things up.
					memmove(aux_exefile+1, aux_exefile, real_len);
					++aux_exefile;
				}
				// Add a path separator between the CWD and exefile
				aux_exefile[real_len-1] = '/'; 
			}
		}
	}
	strings = aux_exefile - arg_bytes;

	if (interp_name) {
		strcpy(strings, interp_name);
		if (interp_arg) {
			strcpy(strings+interp_name_len, interp_arg);
		}
		CRASHCHECK(orig_exefile == NULL);
		strcpy(strings+interp_name_len+interp_arg_len, orig_exefile);
	}
	// can only modify "p" and "nbytes"
	if(lcp->msgsize - (p - lcp->msg.filler) > lcp->msg.spawn.i.nbytes) {
		memcpy(strings + interplen, p, lcp->msg.spawn.i.nbytes);
	} else {
		proc_spawn_args_t				args;

		args.i.type = _PROC_SPAWN;
		args.i.subtype = _PROC_SPAWN_ARGS;
		args.i.nbytes = lcp->msg.spawn.i.nbytes;
		args.i.offset = (uintptr_t)p - (uintptr_t)&lcp->msg;
		args.i.zero = 0;
		if(MsgSend(PROCMGR_COID, &args.i, sizeof args.i, 
				   strings + interplen, args.i.nbytes) == -1) {
			ThreadDestroy(-1, -1, (void *)errno);
		}
	}

	// count the size of the aux vector
	for(naux = 1, auxv = lcp->start.aux; auxv->a_type != AT_NULL; auxv++, naux++) {
		/* nothing to do */
	}
	auxv[1] = auxv[0];
	auxv->a_type = AT_EXEFILE;
	auxv->a_un.a_ptr = aux_exefile;
	naux++;

	// find start of args, must include argc and 2 terminating null pointers
	arg = (const char **)((((uintptr_t)strings & ~(sizeof *arg - 1)) -
			(ninterp + lcp->msg.spawn.i.nargv + lcp->msg.spawn.i.narge +
			naux * (sizeof *lcp->start.aux / sizeof *arg) + 3) * sizeof *arg) &
			~(STACK_ALIGNMENT - 1));
	lcp->start.esp = (uintptr_t)arg;

	if((*(unsigned *)arg = lcp->msg.spawn.i.nargv) == 0) {
		ThreadDestroy(-1, -1, (void *)EINVAL);
	}

	/* account for the interpreter and its arg if they are present */
	if (ninterp) {
		*(unsigned *)arg += (ninterp - 1);   /* -1 is because we skip argv[0] */
	}
	arg++;

	for(p = strings;;) {
		*arg++ = p;

		if(lcp->msg.spawn.i.nargv) {
			if (ninterp) {
				ninterp--;
				if (ninterp == 0) {
					/* we have to skip argv[0] because it isn't the
					 * correct argv[0].  the correct argv[0] (i.e.
					 * with a full path) is stored in orig_exefile
					 * and is accounted for by "ninterp".
					 */
					skip_arg0 = 1;
					lcp->msg.spawn.i.nargv--;
				}
			} else {
				lcp->msg.spawn.i.nargv--;
			}

			if (lcp->msg.spawn.i.nargv == 0) {
				*arg++ = 0;
			}
		} 

		if(lcp->msg.spawn.i.nargv == 0) {
			if(lcp->msg.spawn.i.narge == 0) {
				*arg++ = 0;
				break;
			} else {
				lcp->msg.spawn.i.narge--;
			}
		}

		redo:
		while(*p++ != '\0') {
			if(p > aux_exefile) {
				ThreadDestroy(-1, -1, (void *)EINVAL);
			}
		}
		if((char *)arg > strings) {
			ThreadDestroy(-1, -1, (void *)EINVAL);
		}
		if (skip_arg0) {     /* this makes us skip over two strings */
			skip_arg0 = 0;
			goto redo;
		}
	}
	memcpy(arg, lcp->start.aux, sizeof *lcp->start.aux * naux);
}

int
try_hash_bang(int fd, char **pname, char **arg, char *buf, int buflen)
{
	int numread;
	char *ptr = buf;
	
	*pname = NULL;      /* initialize these two */
	*arg   = NULL;

	numread = proc_read(fd, buf, buflen, 0);
	if (numread <= 3) {   /* need at least #! characters */
		return ENOEXEC;
	}
	
	if (ptr[0] != '#' || ptr[1] != '!') {
		return ENOEXEC;
	}
	ptr += 2;
	
	/* skip whitespace after the #! */
	while ((*ptr == ' ' || *ptr == '\t') && ptr < (buf + numread)) {
		ptr++;
	}

	/* 
	 * If this condition is true it means the first line was just "#!\n".
	 * The spec says this is not valid.  We could be nice and interpret
	 * it to be the same as "#!/bin/sh" but we won't do that for now.
	 */
	if (ptr >= (buf+numread) || *ptr == '\n') {
		return ENOEXEC;
	}

	/* interpreter name starts here */
	*pname = ptr;

	/* find the end of the interpeter name */
	while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n' && ptr < (buf+numread)) {
		ptr++;
	}
	
	if (ptr >= (buf+numread)) {  /* hmmm, interpreter name was too long... */
		return ENOEXEC;
	}
	
	if (*ptr == '\n') {         /* we're at the end of the line */
		*ptr = '\0';
		return EOK;
	} else {
		*ptr++ = '\0';          /* null terminate but continue processing */
	}

	/* skip whitespace after the #! */
	while ((*ptr == ' ' || *ptr == '\t') && ptr < (buf+numread)) {
		ptr++;
	}
	
	if (ptr >= (buf+numread) || *ptr == '\n') {   /* there was nothing else */
		return EOK;
	}

	*arg = ptr;
	/* find the end of the interpeter name */
	while (*ptr != '\n' && ptr < (buf+numread)) {
		ptr++;
	}

	if (ptr >= (buf+numread)) {
		ptr--;
	}
    if (*ptr == '\n' || *ptr == ' ' || *ptr == '\t') {
		ptr--;
		while(*ptr == ' ' || *ptr == '\t') {
		  ptr--;
		}
		ptr++;
	}

	*ptr = '\0';                  /* null terminate the arg */

	return EOK;
}

static int
open_exe(const char *name, struct stat *statp) {
	int	fd, flags;

	/*
	 * Try to open the file using the real uid/gid, which will be actual
	 * uid & gid of the new process. If that fails, try the open with
	 * the effective uid/guid, which will be root. The first will work
	 * 99.9% of the time, the latter is needed for files that have execute
	 * but not read permission. This whole song and dance is to handle
	 * NFS root squashing.
	 */
	fd = sopen(name, O_RDONLY | O_REALIDS, SH_DENYWR);
	if((fd == -1) && (errno == EACCES)) {
		fd = sopen(name, O_RDONLY, SH_DENYWR);
	}
	if((fd == -1) || (fstat(fd, statp) == -1)) {
		if(errno == EBUSY) {
			errno = ETXTBSY;
		}
		ThreadDestroy(-1, -1, (void *)errno);
	}
	/*
	 * Honour the MOUNT_NOSUID qualifier by knocking down these bits
	 * (affects local/internal processing only).  If filesystem does
	 * not support this probe, assume that MOUNT_NOSUID is disabled.
	 */
	if((statp->st_mode & (S_ISUID | S_ISGID))
		&& _devctl(fd, DCMD_ALL_GETMOUNTFLAGS, &flags, sizeof(flags), NULL) == EOK
		&& (flags & _MOUNT_NOSUID)) {
		statp->st_mode &= ~(S_ISUID | S_ISGID);
	}
	return fd;
}

void *loader_load(void *parm) {
	struct loader_context			*lcp = parm;
	const char						*p;
	int								fd;
	int								status;
	uid_t							uid;
	struct stat						statl;
	char							buffer[_POSIX_PATH_MAX + 1];
	void							*base;
	size_t							size;
	char 							*interp_name=NULL, *interp_arg=NULL;
	char 							linebuf[128];  /* for the '#!' test */
	int								needs_secure;
	unsigned						prealloc;

	buffer[0] = '\0';
	lcp->tid = pthread_self();

	SignalProcmask(0, 0, SIG_SETMASK, &lcp->mask, 0);

	switch(lcp->msg.spawn.i.parms.flags & SPAWN_ALIGN_MASK) {
	case SPAWN_ALIGN_DEFAULT:
		status = 0;
		break;
	case SPAWN_ALIGN_FAULT:
		status = 1;
		break;
	case SPAWN_ALIGN_NOFAULT:
		status = -1;
		break;
	default:
		ThreadDestroy(-1, -1, (void *)EINVAL);
	}
	(void)ThreadCtl(_NTO_TCTL_ALIGN_FAULT, &status);

	if(lcp->msg.spawn.i.parms.flags & SPAWN_SETSIGDEF) {
		int								sig;

		for(sig = _SIGMIN; sig <= _SIGMAX; sig++) {
			if(sigismember(&lcp->msg.spawn.i.parms.sigdefault, sig)) {
				set_signal(sig, SIG_DFL);
			}
		}
	}

	if(lcp->msg.spawn.i.parms.flags & SPAWN_SETSIGIGN) {
		int								sig;

		for(sig = _SIGMIN; sig <= _SIGMAX; sig++) {
			if(sigismember(&lcp->msg.spawn.i.parms.sigignore, sig)) {
				set_signal(sig, SIG_IGN);
			}
		}
	}

	if((fd = ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, PROCMGR_COID, 0)) == -1) {
		ThreadDestroy(-1, -1, (void *)errno);
	} else if(fd != PROCMGR_COID) {
		ThreadDestroy(-1, -1, (void *)EL3HLT);
	}

	if(lcp->msg.spawn.i.parms.flags & SPAWN_SETSID) {
		if(setsid() == -1) {
			ThreadDestroy(-1, -1, (void *)errno);
		}			
	}

	if(lcp->msg.spawn.i.parms.flags & SPAWN_EXPLICIT_CPU) {
		unsigned runmask = lcp->msg.spawn.i.parms.runmask;

		// Verify specified runmask is valid
		if((runmask & LEGAL_CPU_BITMASK) == 0) {
			ThreadDestroy(-1, -1, (void *)EINVAL);
		}
	}

	if(lcp->msg.spawn.i.parms.flags & SPAWN_SETGROUP) {
		if(setpgid(getpid(), lcp->msg.spawn.i.parms.pgroup) == -1) {
			ThreadDestroy(-1, -1, (void *)errno);
		}			
	}

	// proc_spawn set saved_user_id to 0, so this will work...
	uid = getuid();
	(void)setreuid(geteuid(), 0);
	(void)setregid(getegid(), getgid());

	/* set root and cwd. 
	 * Note: netmgr_xxx() can't be called from loader thread (PR#21182), so
	 * we already munged the premote value in procmgr_spawn().
	 */
	if(ND_NODE_CMP(lcp->pnode, ND_LOCAL_NODE) != 0) {
		proc_spawn_remote_t *premote;

		premote = (proc_spawn_remote_t *)((char*)&lcp->msg.spawn.i + lcp->remote_off);

		(void)chroot((char*)premote + sizeof(*premote));
		(void)chdir((char*)premote + sizeof(*premote) + premote->root_len);
	}

	p = (const char *)&lcp->msg.spawn.i + (sizeof lcp->msg.spawn.i + lcp->msg.spawn.i.nfds * sizeof(long));
	if(lcp->msg.spawn.i.parms.flags & SPAWN_SEARCH_PATH) {
		if(searchpath(p + lcp->msg.spawn.i.searchlen, p, buffer, sizeof buffer) == -1) {
			ThreadDestroy(-1, -1, (void *)errno);
		}
		p += lcp->msg.spawn.i.searchlen;
	} else {
		if(is_exec(p) == -1) {
			ThreadDestroy(-1, -1, (void *)errno);
		} else if (memccpy(buffer, p, '\0', sizeof(buffer)) == NULL) {
			ThreadDestroy(-1, -1, (void *)ENAMETOOLONG);
		}
	}

	fd = open_exe(buffer, &statl);

	lcp->start.aux[0].a_type = AT_NULL;
	lcp->start.aux[0].a_un.a_val = 0;
	needs_secure = 0;
	if((statl.st_mode & S_ISUID) && (statl.st_uid != getuid())) {
		needs_secure = 1;
	}
	if((statl.st_mode & S_ISGID) && (statl.st_gid != getgid())) {
		needs_secure = 1;
	}
	if(needs_secure) {
		lcp->start.aux[0].a_type = AT_LIBPATH;
		lcp->start.aux[1].a_type = AT_NULL;
		lcp->start.aux[1].a_un.a_val = 0;
	}
	if((status = elf_load_hook(fd, buffer, &lcp->start, &statl, &lcp->msg.spawn.i.parms)) == -1) {
		/*
		 * fd is not an ELF executable - see if we should exec interpreter
		 */
		status = try_hash_bang(fd, &interp_name, &interp_arg, linebuf, sizeof(linebuf));
		if(status == ENOEXEC && lcp->msg.spawn.i.parms.flags & SPAWN_CHECK_SCRIPT) {
			/*
			 * POSIX 1003.1-2001 requires exec[vl]p to execute a shell as
			 * the interpreter for ENOEXEC errors.
			 */
			strcpy(linebuf, shell_path);
			interp_name = linebuf;
			interp_arg = 0;
			status = EOK;
		} 
		close(fd);

		if (status == EOK) {
			if(is_exec(interp_name) == -1) {
				ThreadDestroy(-1, -1, (void *)errno);
			}
			fd = open_exe(interp_name, &statl);
			status = elf_load_hook(fd, interp_name, &lcp->start, &statl, &lcp->msg.spawn.i.parms);
		}

		if (status == -1) {
			status = ENOEXEC;
		}
	}
	if(status != EOK) {
		close(fd);
		ThreadDestroy(-1, -1, (void *)status);
	}
	close(fd);

	if(lcp->msg.spawn.i.parms.flags & SPAWN_SETSTACKMAX) {
		lcp->start.stacksize = lcp->msg.spawn.i.parms.stack_max;
	}

	if(lcp->start.stacksize < lcp->start.stackalloc) {
		lcp->start.stacksize = lcp->start.stackalloc;
	}
	if(lcp->flags & PROC_LF_LAZYSTACK) {
		prealloc = 0;
	} else {
		prealloc = lcp->start.stackalloc;
	}
	if(_mmap2((void *)lcp->start.stackaddr, lcp->start.stacksize + guardpagesize,
			PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_STACK|MAP_LAZY, 
			NOFD, guardpagesize, __PAGESIZE, prealloc, &base, &size) == MAP_FAILED) {
		ThreadDestroy(-1, -1, (void *)ENOMEM);
	}
	if((lcp->start.esp = (uintptr_t)ThreadTLS(0, (uintptr_t)base, size, guardpagesize, (uintptr_t)base + size)) == (uintptr_t)-1) {
		ThreadDestroy(-1, -1, (void *)(errno == EFAULT ? ENOMEM : errno));
	}

	loader_args(lcp, p, buffer, interp_name, interp_arg);

	if(!(lcp->flags & SPAWN_EXEC)) {
		status = loader_fd(lcp);
		if(status != EOK) {
			ThreadDestroy(-1, -1, (void *)status);
		}
	}

	(void)setregid(getegid(), (statl.st_mode & S_ISGID) ? statl.st_gid : getgid());
	(void)setreuid(uid, (statl.st_mode & S_ISUID) ? statl.st_uid : getuid());

	if(!(lcp->flags & SPAWN_EXEC)) {
		if((lcp->msg.spawn.i.parms.flags & SPAWN_TCSETPGROUP) && isatty(STDIN_FILENO) > 0) {
			if(lcp->msg.spawn.i.parms.flags & SPAWN_SETSID) {
				(void)tcsetsid(STDIN_FILENO, getpgrp());
			}
			(void)tcsetpgrp(STDIN_FILENO, getpgrp());
		}
	}

	lcp->msg.spawn_done.i.type = _PROC_SPAWN;
	lcp->msg.spawn_done.i.subtype = (lcp->flags & SPAWN_EXEC) ? _PROC_SPAWN_EXEC : _PROC_SPAWN_DONE;
	SETIOV(lcp->iov + 0, &lcp->msg.spawn_done.i, sizeof lcp->msg.spawn_done.i);
	MsgSendv(PROCMGR_COID, lcp->iov, 1, 0, 0);
	ThreadDestroy(-1, -1, (void *)errno);
	return 0;
}

__SRCVERSION("proc_loader.c $Rev: 199544 $");
