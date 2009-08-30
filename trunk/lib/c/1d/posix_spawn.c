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

#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include "posix_spawnattr.h"
#include "posix_spawn_file_actions.h"


pthread_once_t once = PTHREAD_ONCE_INIT;

/*
 * posix_spawn
 * 
 * <path> must be the full path to the executable otherwise ENOENT will be
 * returned. Use posix_spawnp() if you want to embark on a search for the
 * executable to spawn.
 * 
 * Returns:
 * 		EOK		- on success
 * 		EIO		- an internal error occurred in the library
 * 		EINVAL	- an invalid argument was provided including an improperly
 * 				  initialized 'posix_spawnattr_t' or 'posix_spawn_file_actions_t'
 * 				  object
 * 		ENOENT	- the <path> argument could not be found
 * 		ENOMEM	- the memory required to create the message to send to
 * 				  procnto could not be allocated
 * 				- memory to create the new process and its associated data
 * 				  structures could not be allocated
 * 		errno	- any error returned by a stat() on <path>
 *
 * The following error codes can be returned when Adaptive Partitioning modules
 * are include in the image
 * 
 * 		EACCES	- the spawned program does not have permission to associate
 * 				  with the specified partitions 
 * 		ENOMEM	- the 'posix_spawnattr_t' object specifies a memory partition
 * 				  that does not exist.
 * 				- the new process was unable to associate with 1 or more memory
 * 				  partitions to be inherited from the parent process.
 * 		EEXIST	- an attempt to associate with more than one partition of a
 * 				  given memory class. This typically occurs when you create a
 * 				  group name partition with more than one pseudo partition of the
 * 				  same memory class and then an attempt to spawn a process and
 * 				  associate with that group name partition.
 *		EEXIST	- an attempt to associate with more than one scheduling partition.
 * 				  This typically occurs when you create a group name partition
 * 				  with more than one scheduler pseudo partition and then an attempt
 * 				  to spawn a process and associate with that group name partition.
 * 		
*/

/*
 * for our 2.95.3 compiler (and C++) _Restrict expands to __restrict however
 * 2.95.3 compiler (and C++) does not like argv[__restrict] and envp[__restrict].
 * Once we no longer support 2.95.3 compiler, this can be reduced to __cplusplus
 * (see spawn.h and posix_spawnp.c also)
*/
int posix_spawn(pid_t *_Restrict pid,
				const char *_Restrict path,
				const posix_spawn_file_actions_t *file_actions,
				const posix_spawnattr_t *_Restrict attrp,
#if (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus)
				 char *const argv[],
				 char *const envp[])
#else	/* (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus) */
				char *const argv[_Restrict],
				char *const envp[_Restrict])
#endif	/* (__GNUC__ == 2 && __GNUC_MINOR__ >= 9) || defined(__cplusplus) */
{
	proc_posixspawn_t				msg;
	iov_t							*iov;
	unsigned						num_iovs = 0;
	char							*const *arg;
	char							*src;
	char							*dst, *data;
	pid_t							_pid;
	int								coid;
	_posix_spawnattr_t				*_attrp;
	_posix_spawnattr_partition_t	*partition_attr;
	_posix_spawn_file_actions_t		*_factp = NULL;
	int r;
	unsigned						factp_open_iovs = 0;
	struct stat						dummy;


	if (pthread_once(&once, posix_spawnattr_t_once_init) != EOK) return EIO;

	if ((path == NULL) || (*path == '\0')) return EINVAL;
	if (*path != '/') return ENOENT;
	if (stat(path, &dummy) == -1) return errno;
	if (pid == NULL) pid = &_pid;

	/*
	 * 2 conditions exist that require the use of a default 'posix_spawnattr_t'.
	 * Either <attrp> == NULL, or <attrp> is not NULL but no _setxxx() operations
	 * have been done on the object, hence there is no "actual" process specific
	 * copy of the default 'posix_spawnattr_t' object required and therefore
	 * get_attrp() will return NULL. In either of these cases, we make sure we
	 * get the _attp variable is set up before proceeding
	*/
again:
	if (attrp == NULL) attrp = &default_posix_spawnattr_t;
	if (!valid_attrp(attrp)) return EINVAL;
	if ((_attrp = get_attrp(*attrp, KEY_MASK)) == NULL) {
		/* if <attrp> has already been set to the default, got to be an error */
		if (attrp == &default_posix_spawnattr_t) return EIO;
		attrp = &default_posix_spawnattr_t;
		goto again;
	}

	partition_attr = (_posix_spawnattr_partition_t *)&_attrp->partition;

	if (file_actions != NULL) {
		_factp = get_factp(*file_actions, KEY_MASK);
		/* _fact can be NULL if a valid, unset posix_spawn_file_actions_t object is passed in */
	}

	/* start filling in the msg */
	msg.i.type = _PROC_POSIX_SPAWN;
	msg.i.subtype = _PROC_SPAWN_START;
	msg.i.pathlen = strlen(path) + 1;
	msg.i.attr_bytes = (sizeof(*_attrp) - sizeof(_attrp->partition)) + partition_attr->size;
	/* this next line ensures that we see a partition_attr->size == 0 in procnto */
	if (partition_attr->size == 0) msg.i.attr_bytes += sizeof(_attrp->partition);

	if ((iov = calloc(num_iovs = 6, sizeof(*iov))) == NULL) return ENOMEM;

	/* process any file actions */
	if (_factp != NULL) {
		unsigned i;
		factp_open_iovs = 0;
		msg.i.filact_bytes = sizeof(_factp->num_entries) + (_factp->num_entries * sizeof(_factp->action[0]));
		msg.i.filact_obytes = 0;
		for (i=0; i<_factp->num_entries; i++) {
			if (_factp->action[i].type == posix_file_action_type_OPEN) {
				++factp_open_iovs; 
				if ((iov = realloc(iov, ++num_iovs * sizeof(*iov))) == NULL) return ENOMEM;
				SETIOV(&iov[2 + factp_open_iovs], _factp->action[i]._type.open, _factp->action[i]._type.open->size);
				msg.i.filact_obytes += _factp->action[i]._type.open->size;
			}
		}
	} else {
		msg.i.filact_bytes = msg.i.filact_obytes = 0;
	}

	if ((_attrp->flags & SPAWN_SETND) && ND_NODE_CMP(_attrp->node, ND_LOCAL_NODE) != 0) {
		/* remote spawn */
		msg.i.subtype = _PROC_SPAWN_REMOTE;
	}

	msg.i.argenv_bytes = msg.i.nargv = msg.i.narge = 0;
	
	/* process argv[] */
	if(argv) {
		for(arg = argv; *arg; msg.i.argenv_bytes += (strlen(*arg++) + 1), msg.i.nargv++) {
			/* nothing to do */
		}
	}
	
	/* process envp[] */
	if(envp == NULL) envp = environ;
	if(envp) {
		for(arg = envp; *arg; msg.i.argenv_bytes += (strlen(*arg++) + 1), msg.i.narge++) {
			/* nothing to do */
		}
	}

	coid = PROCMGR_COID;

	if(msg.i.subtype == _PROC_SPAWN_REMOTE) 
	{
		spawn_remote_t *premote;
		unsigned len = (msg.i.argenv_bytes + sizeof(int))&~(sizeof(int) - 1);
free(iov);
return ENOSYS;	// FIX ME - not handled yet
#if 0		
		if(!(dst = data = malloc(len + SPAWN_REMOTE_MSGBUF_SIZE))) {
			return ENOMEM;
		}
		premote = (spawn_remote_t *)(data + len);
		SETIOV(iov + 0, &msg.i, sizeof msg.i);
		SETIOV(iov + 1, _factp, msg.i.filact_bytes);
		SETIOV(iov + 2 + factp_open_iovs, &msg.i, sizeof msg.i);
		SETIOV(iov + 3 + factp_open_iovs, premote, SPAWN_REMOTE_MSGBUF_SIZE);
		if(MsgSendvnc(coid, iov + 0, 2, iov + 2, 2) == -1) {
			free(data);
			return errno;
		}
		if((coid = ConnectAttach(premote->nd, premote->pid, premote->chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
			free(data);
			return errno;
		}
		SETIOV(iov + 5 + factp_open_iovs, (char*)premote + sizeof(*premote), premote->size);
		msg.i.subtype = _PROC_SPAWN_START;
#endif
	}
	else
	{
		if(!(dst = data = malloc(msg.i.argenv_bytes))) {
			free(iov);
			return ENOMEM;
		}
		SETIOV(iov + 5 + factp_open_iovs, data, 0);	// unused
	}

	/* copy the argument variables */
	if(argv) {
		for(arg = argv; (src = *arg); arg++) {
			while((*dst++ = *src++)) {
				/* nothing to do */
			}
		}
	}

	/* copy the environment variables */
	if(envp) {
		for(arg = envp; (src = *arg); arg++) {
			while((*dst++ = *src++)) {
				/* nothing to do */
			}
		}
	}

	SETIOV(iov + 0, &msg, sizeof msg);
	SETIOV(iov + 1, _attrp, msg.i.attr_bytes);
	SETIOV(iov + 2, _factp, msg.i.filact_bytes);
	SETIOV(iov + 3 + factp_open_iovs, path, msg.i.pathlen);
	SETIOV(iov + 4 + factp_open_iovs, data, msg.i.argenv_bytes);

	*pid = MsgSendvnc(coid, iov + 0, num_iovs, 0, 0);
	r = (*pid == -1) ? errno : EOK;
/*
if (*pid == -1)
	printf("MsgSendvnc() failed with errno = %d\n", errno);
else
	printf("pid = %d\n", *pid);
*/
	if(coid != PROCMGR_COID) {
		ConnectDetach(coid);
	}

	free(data);
	free(iov);
	return r;
}

__SRCVERSION("posix_spawn.c $Rev: 200568 $");


#endif	/* _POSIX_SPAWN */

