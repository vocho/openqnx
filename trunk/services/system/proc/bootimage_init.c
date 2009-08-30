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
#include <unistd.h>
#include <fcntl.h>
#include <share.h>
#include <libgen.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/image.h>
#include <sys/pathmgr.h>
#include <sys/sysmgr.h>
#include <sys/sched_aps.h>

#if (SCRIPT_FLAGS_SESSION << 8) != SPAWN_SETSID
#error SCRIPT_FLAGS_SESSION doesnt match SPAWN_SETSID>>8
#endif
#if (SCRIPT_FLAGS_SCHED_SET << 8) != SPAWN_EXPLICIT_SCHED
#error SCRIPT_FLAGS_SCHED_SET doesnt match SPAWN_EXPLICIT_SCHED>>8
#endif
#if (SCRIPT_FLAGS_CPU_SET << 8) != SPAWN_EXPLICIT_CPU
#error SCRIPT_FLAGS_CPU_SET doesnt match SPAWN_EXPLICIT_CPU>>8
#endif
#if (SCRIPT_FLAGS_BACKGROUND << 8) != SPAWN_NOZOMBIE
#error SCRIPT_FLAGS_BACKGROUND doesnt match SPAWN_ZOMBIE>>8
#endif
#if (SCRIPT_FLAGS_KDEBUG << 8) != SPAWN_DEBUG
#error SCRIPT_FLAGS_KDEBUG doesnt match SPAWN_DEBUG>>8
#endif
#if SCRIPT_SCHED_EXT_NONE != SCHED_EXT_NONE
#error SCHED_EXT_NONE doesnt match
#endif
#if SCRIPT_SCHED_EXT_APS != SCHED_EXT_APS
#error SCHED_EXT_NONE doesnt match
#endif

struct boot_data {
	struct image_header		*boot;
	unsigned				proc_offset;
};

#ifndef NKDEBUG
static int 
kdebug_path(struct kdebug_entry *entry, char *buff, int buffsize) {
	char				*p;
	
	switch(entry->type) {
	case KDEBUG_TYPE_PROCESS:
		if(entry->ptr && (p = ((PROCESS *)(entry->ptr))->debug_name)) {
			int					len;

			for(len = 0; buffsize-- && (*buff++ = *p++); len++) {
				/* nothing to do */
			}
			return len;
		}
		*buff = '\0';
		return 0;

	case KDEBUG_TYPE_OBJECT:

	default:
		break;
	}
	return -1;
}
#endif


char *
getenv(const char *name) {
	int len;
	const char **pp, *p;

	len = strlen(name);

	for(pp = (const char **)environ; *pp; ++pp) {
       		if(!strncmp(*pp, name, len)) {
        	        if(*(p = *pp + len) == '=') {
        	                return (char *)++p;
        	        }
        	}
	}
	return NULL;
}

static int check_func(const char *name, void *handle)
{
	struct stat64 sbuf;
	int ret = handle ? stat64( name, &sbuf ) : sopen( name, O_LARGEFILE|O_RDWR, SH_DENYNO );
	if (ret >= 0) {
		return ret;
	}
	switch(errno) {
	case ENOENT:
	case ENOSYS:
	case EAGAIN:
	case EHOSTDOWN:
	case EHOSTUNREACH:
	case ENXIO:
		return WAITFOR_CHECK_CONTINUE;
	default:
		return WAITFOR_CHECK_ABORT;
	}
}

static char *
boot_waitfor_reopen(struct script_waitfor_reopen *scp, int fds[3], char *tty) {
	unsigned					check;

	check = ((scp->checks_lo | (scp->checks_hi << 8))) + 1;
	if(scp->hdr.type == SCRIPT_TYPE_REOPEN ) {
		fds[2] = _waitfor( scp->fname, check * 100, 100, check_func, NULL );
		if ( fds[2] != -1 ) {
			close(fds[0]);
			fds[0] = fds[1] = fds[2];
			tty = scp->fname;
			return tty;
		}
		fds[2] = fds[1];
	} else if(scp->hdr.type == SCRIPT_TYPE_WAITFOR ) {
		if ( _waitfor( scp->fname, check * 100, 100, check_func, (void *)1 ) == 0 ) {
			return tty;
		}
	}
	kprintf("Unable to access \"%s\" (%d)\n", scp->fname, errno);
	return tty;
}


static pid_t 
boot_external(struct script_external *scp, int extsched, int fds[3]) {
	pid_t						pid;
	int							argc, envc;
	char						*args[102];//max 100 args + NULL for each of argv and arge
	char						*cmd, **argv, **arge, **arg;
	char						*p;
	spawn_inheritance_type		inherit;

	argc = scp->argc;
	envc = scp->envc;

	if ((argc + envc) > 100){
		kprintf("warning: some arg/env strings will be lost\n");
	}

	argv = &args[0];
	arge = &args[argc+1]; //account for extra NULL ptr
	
	cmd = p = scp->args;
	while(*p++) {
		/* nothing to do */
	}

	for(arg = argv; argc--; arg++) {
		*arg = p;
		while(*p++) {
			/* nothing to do */
		}
	}
	*arg = 0;

	for(arg = arge; envc--; arg++) {
		*arg = p;
		while(*p++) {
			/* nothing to do */
		}
	}
	*arg = 0;

	memset(&inherit, 0x00, sizeof inherit);
	inherit.flags = (scp->flags & ~SCRIPT_FLAGS_EXTSCHED) << 8;
	inherit.policy = scp->policy;
	inherit.param.sched_priority = scp->priority;
	if(scp->cpu < NUM_PROCESSORS) {
		inherit.runmask = 1 << scp->cpu;
	} else if(inherit.flags & SPAWN_EXPLICIT_CPU) {
		inherit.flags &= ~SPAWN_EXPLICIT_CPU;
		kprintf("Unable to place \"%s\" on specified CPU\n", cmd);
	}

	inherit.flags |= SPAWN_CHECK_SCRIPT;
	if(!strchr(cmd, '/')) inherit.flags |= SPAWN_SEARCH_PATH;

	if(inherit.flags & SPAWN_SETSID) {
		inherit.flags &= ~SPAWN_SETGROUP;
		inherit.flags |= SPAWN_TCSETPGROUP;
	} else {
		inherit.flags |= SPAWN_SETGROUP;
		inherit.pgroup = 0;
	}
	if(!(inherit.flags & SPAWN_SETSIGMASK)) {
		inherit.flags |= SPAWN_SETSIGMASK;
		sigemptyset(&inherit.sigmask);
	}

	switch(extsched) {
	case SCHED_EXT_APS:
		if(scp->flags & SCRIPT_FLAGS_EXTSCHED && scp->extsched.aps.id != APS_SYSTEM_PARTITION_ID) {
		sched_aps_join_parms		aps;
			APS_INIT_DATA(&aps); 
			aps.pid = aps.tid = 0;
			aps.id = scp->extsched.aps.id;
			if(SchedCtl(SCHED_APS_JOIN_PARTITION, &aps, sizeof(aps)) == -1)
				kprintf("Unable to place \"%s\" into APS partition (%d)\n", cmd, errno);
		}
		break;
	default:
		break;
	}

	if((pid = spawn(cmd, 3, fds, &inherit, argv, arge)) == -1) {
		kprintf("Unable to start \"%s\" (%d)\n", cmd, errno);
	}

	switch(extsched) {
	case SCHED_EXT_APS:
		if(scp->flags & SCRIPT_FLAGS_EXTSCHED && scp->extsched.aps.id != APS_SYSTEM_PARTITION_ID) {
		sched_aps_join_parms		aps;
			APS_INIT_DATA(&aps); 
			aps.pid = aps.tid = 0;
			aps.id = APS_SYSTEM_PARTITION_ID;
			(void)SchedCtl(SCHED_APS_JOIN_PARTITION, &aps, sizeof(aps));
		}
		break;
	default:
		break;
	}

	if(inherit.flags & SPAWN_NOZOMBIE) {
		pid = 0;
	}
	return pid;
}


static int 
map_ifs(struct asinfo_entry *as, char *name, void *d) {
	struct image_header				*image;
	struct system_private_entry		*spp;
	struct boot_data				*data = d;

	image = imagefs_mount(as->start, (as->end - as->start) + 1, 0, _RESMGR_FLAG_AFTER, sysmgr_prp->root, 0);

	if((image != (void *)-1) && (((struct image_header *)image)->boot_ino[0])) {
		data->boot = image;
		spp = SYSPAGE_ENTRY(system_private);
		if(image->flags & IMAGE_FLAGS_INO_BITS) {
			data->proc_offset = -1U;
		} else {
			data->proc_offset = spp->boot_pgm[spp->boot_idx].base - as->start;
		}
	}
	return 1;
}


static int 
bootimage_start(message_context_t *ctp, int code, unsigned flags, void *handle) {
	union image_dirent			*dir;
	struct image_file			*pfp;
	int							fds[3];
	int							size;
	pid_t						pid;
	PROCESS						*prp;
	char						*tty;
	char						*tmp1, *tmp2;
#ifdef PROC_BOOTIMAGE_DIR
static char						boot[_POSIX_PATH_MAX + 1] = "/" PROC_BOOTIMAGE_DIR "/";
#endif
	union script_cmd			*scp;
	siginfo_t                   info;
	struct boot_data			data;
	struct image_header			*boot_image;
	struct sched_query			query;
	size_t  slen;

	data.boot = NULL;
	walk_asinfo("imagefs", map_ifs, &data);

	if(data.boot == NULL) {
		crash();
	}
	boot_image = data.boot;

  	if(proc_thread_pool_reserve() != 0)
    		return(-1);
	prp = proc_lock_pid(SYSMGR_PID);
	if(sigismember(&prp->sig_ignore, SIGCHLD)) {
		proc_unlock(prp);
		proc_thread_pool_reserve_done();
		return -1;
	}
	
	prp->flags |= _NTO_PF_LOADING;
	proc_unlock(prp);

	dir = (union image_dirent *)((char *)boot_image + boot_image->dir_offset);
	for(pfp = 0, scp = 0; dir->attr.size; dir = (union image_dirent *)((unsigned)dir + dir->attr.size)) {
		switch(dir->attr.mode & S_IFMT) {
#ifdef PROC_BOOTIMAGE_DIR
		case S_IFLNK:
			pathmgr_symlink(&dir->symlink.path[dir->symlink.sym_offset], dir->symlink.path);
			break;
		case S_IFIFO:
#endif
		case S_IFREG:
			if((boot_image->flags & IMAGE_FLAGS_INO_BITS) && (dir->attr.ino & IFS_INO_BOOTSTRAP_EXE)) {
				//If we have the INO flag bits, proc is the last bootstrap
				//executable.
				pfp = &dir->file;
			} else if(dir->file.offset == data.proc_offset) {
				pfp = &dir->file;
			} else if(dir->file.attr.ino == boot_image->script_ino) {
				scp = (union script_cmd *)((char *)boot_image + dir->file.offset);
			}
#ifdef PROC_BOOTIMAGE_DIR
			if(strncmp(dir->file.path, boot + 1, sizeof PROC_BOOTIMAGE_DIR) ||
					strchr(dir->file.path + sizeof PROC_BOOTIMAGE_DIR + 1, '/')) {
static			char					link[_POSIX_PATH_MAX + 1] = "/";
				char					*p1, *p2;

				STRLCPY(&link[1], dir->file.path, sizeof(link) - 1);

				p1 = &boot[sizeof PROC_BOOTIMAGE_DIR + 1];
				*p1++ = '.';
				for(p2 = dir->file.path; *p1++ = (*p2 == '/' ? '.' : *p2); p2++);
				pathmgr_symlink(boot, link);
			}
#endif
			break;
		default:
			break;
		}
	}
	if(!scp) {
		kprintf("Unable to find startup script\n");
		proc_thread_pool_reserve_done();
		return -1;
	}
	if(!pfp) {
		crash();
	}
	slen = strlen(pfp->path) + 2;
	tmp1 = alloca(slen);
	if(!tmp1) {
		crash();
	}
	tmp1[0] = '/';
	memcpy(tmp1 + 1, pfp->path, slen - 1);	// '\0' guaranteed to be copied
	tmp1 = dirname(tmp1);

	sysmgr_confstr_set(0, _CS_SYSNAME, "QNX");
	sysmgr_confstr_set(0, _CS_PATH, (tmp2 = getenv("PATH")) ? tmp2 : tmp1);
	sysmgr_confstr_set(0, _CS_LIBPATH, (tmp2 = getenv("LD_LIBRARY_PATH")) ? tmp2 : tmp1);

	/*
	 * Get the default path for shell for scripts without a #! interpreter
	 */
	shell_path = getenv("SHELL");
	if (shell_path == NULL) {
		shell_path = "/bin/sh";
	}

	if((fds[0] = sopen(pfp->path, O_RDONLY, SH_DENYNO)) == -1) {
		kprintf("bootimage: Unable to open '%s' (%d)\n", pfp->path, errno);
		proc_thread_pool_reserve_done();
		return -1;
	}
	if(elf_load(fds[0], pfp->path, 0, 0, 0) != EOK) {
		kprintf("bootimage: Unable to load '%s'(%d)\n", pfp->path, errno);
		proc_thread_pool_reserve_done();
		return -1;
	}
	close(fds[0]);

#ifndef NKDEBUG
	kdebug_init(kdebug_path);
	(void)kdebug_attach(&prp->kdebug, 1);
#endif

	prp->flags &= ~_NTO_PF_LOADING;

	fds[0] = fds[1] = fds[2] = sopen(tty = "/dev/text", O_RDWR, SH_DENYNO);

	if(SchedCtl(SCHED_QUERY_SCHED_EXT, &query, sizeof(query)) == -1) query.extsched = SCHED_EXT_NONE;

	for(pid = 0; (size = scp->hdr.size_lo | (scp->hdr.size_hi << 8)); scp = (union script_cmd *)((char *)scp + ((size + 3) & ~3))) {
		if(pid) {
			(void)waitid(P_PID, pid, &info, WEXITED|WTRAPPED);
			pid = 0;
		}

		switch(scp->hdr.type) {
		case SCRIPT_TYPE_DISPLAY_MSG:
			write(fds[0], scp->display_msg.msg, strlen(scp->display_msg.msg));
			break;

		case SCRIPT_TYPE_WAITFOR:
		case SCRIPT_TYPE_REOPEN:
			tty = boot_waitfor_reopen(&scp->waitfor_reopen, fds, tty);
			break;

		case SCRIPT_TYPE_PROCMGR_SYMLINK:
			tmp1 = scp->procmgr_symlink.src_dest;
			(void)pathmgr_symlink(tmp1, &tmp1[strlen(tmp1) + 1]);
			break;

		case SCRIPT_TYPE_EXTSCHED_APS:
			if(query.extsched == SCHED_EXT_APS) {
			sched_aps_create_parms		aps;
				APS_INIT_DATA(&aps); 
				aps.name = scp->extsched_aps.pname;
				aps.budget_percent = scp->extsched_aps.budget;
				aps.critical_budget_ms = scp->extsched_aps.critical_hi << 8 | scp->extsched_aps.critical_lo;
				if(SchedCtl(SCHED_APS_CREATE_PARTITION, &aps, sizeof(aps)))
					kprintf("Unable to create APS '%s' (%d)\n", aps.name, errno);
				else if(aps.id != scp->extsched_aps.id)
					kprintf("Unexpected ID for APS '%s'\n", aps.name);
			} else {
				kprintf("Ignoring APS schedulling specification\n");
			}
			break;

		case SCRIPT_TYPE_EXTERNAL:
			pid = boot_external(&scp->external, query.extsched, fds);
			break;

		default:
			kprintf("Unknown type %d\n", scp->hdr.type);
			break;
		}
	}

	prp = proc_lock_pid(SYSMGR_PID);
	sigaddset(&prp->sig_ignore, SIGCHLD);
	proc_unlock(prp);

	while(waitid(P_ALL, 0, &info, WNOHANG|WEXITED|WTRAPPED) > 0) {
		// Collect any grandchildren that might have been retargeted to proc
		// while we were running the boot image script file
	}

	close(fds[0]);

	proc_thread_pool_reserve_done();
	return 0;
}


void 
bootimage_init(void) {
	int								code;

	if(imagefs_mount_mounter() == -1) {
		crash();
	}

	code = pulse_attach(dpp, MSG_FLAG_ALLOC_PULSE, 0, bootimage_start, NULL);
	if(code == -1) {
		crash();
	}

	MsgSendPulse(PROCMGR_COID, PROC_INIT_PRIORITY, code, 0);
}

__SRCVERSION("bootimage_init.c $Rev: 172534 $");
