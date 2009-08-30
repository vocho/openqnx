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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/neutrino.h>

#include <sys/procfs.h>

/**
 * This function provides one stop shopping for thread name access.
 * The value of newname/newname_len indicates the operation that
 * is performed with regard to setting the name.  
 *  newname_len > 0  -> Name pointed to newname is set
 *  newname_len = 0  -> Name is cleared (newname may be NULL)
 *  newname_len = -1 -> Name is unchanged (used for name retrieval)
 *
 * The values of prevname/prevname_len indicates the operation that
 * is performed with regard to retreiving the name.  If the name is
 * being set at the same time, then the previously set name is returned.
 *  prevname_len > 0  -> Null terminated name, up to prevname_len -1 is returned
 *  prevname_len <= 0 -> No name information is returned
 **/
int __getset_thread_name(pid_t pid, pthread_t tid, 
						 const char *newname, int newname_len, 
						 char *prevname, int prevname_len) {
	int 				ret, islocal, fd;
	struct _thread_name *tn;
	procfs_threadctl 	tctl;

	islocal = (pid == 0 || pid == getpid()) && (tid == 0 || tid == pthread_self());

	if(newname_len < 0) {
		newname_len = -1;
	}
	if(prevname_len < 0) {
 		prevname_len = 0;
	}
	if(newname_len > _NTO_THREAD_NAME_MAX) {
		return E2BIG;
	}
	if(!islocal && tid == 0) {
		return EINVAL;
	}
	if(pid == 0) {
		pid = getpid();
	}

	//Local thread access, use kernel call directly
	if(islocal) {
		int 				 onstack = 1;
		int 				 namelen;

		//Allocate the maximum of newname/prevname for storage
		namelen = (newname_len > prevname_len) ? newname_len : prevname_len;

		//Try for the devctl reuse, then the stack, then dynamic allocation
		if(sizeof(tctl.data) > (sizeof(*tn) + namelen)) {
			tn = (struct _thread_name *)&tctl.data;
		} else if((tn = alloca(sizeof(*tn) + namelen)) == NULL) {
			onstack = 0;
			tn = malloc(sizeof(*tn) + namelen);
			if(tn == NULL) {
				return errno;
			}
		}

		tn->name_buf_len = namelen;
		tn->new_name_len = newname_len;
		if(newname_len > 0) {
			memcpy(tn->name_buf, newname, newname_len);
		}
	
		ret = ThreadCtl_r(_NTO_TCTL_NAME, tn);
		if(ret == EOK && prevname_len > 0) { 
			strncpy(prevname, tn->name_buf, prevname_len - 1);
			prevname[prevname_len - 1] = '\0';		//Guarantee null return
		}

		if(!onstack) {
			free(tn);
		}

		return ret;
	}

	//Non-local thread/process access, use the devctl

	//@@@ Why do we have to open up AS instead of just /proc/<pid>
	snprintf(tctl.data, sizeof(tctl.data), "/proc/%d/as", pid);
	fd = open(tctl.data, (newname_len >= 0) ? O_RDWR : O_RDONLY);
	if(fd == -1) {
		return errno;
	}
	memset(&tctl, 0, sizeof(tctl));

	tctl.tid = tid;
	tctl.cmd = _NTO_TCTL_NAME;

	tn = (struct _thread_name *)(&tctl.data);
	tn->name_buf_len = sizeof(tctl.data) - sizeof(*tn);

	//We can only communicate a maximum buffer size via devctl
	if(newname_len > tn->name_buf_len || prevname_len > tn->name_buf_len) {
		return E2BIG;
	}

	tn->new_name_len = newname_len;
	if(newname_len > 0) {
		memcpy(tn->name_buf, newname, newname_len);
	}

	ret = devctl(fd, DCMD_PROC_THREADCTL, &tctl, sizeof(tctl), NULL); 

	if(ret == EOK && prevname_len > 0) {
		strncpy(prevname, tn->name_buf, prevname_len - 1);
		prevname[prevname_len - 1] = '\0';		//Guarantee null return
	}

	close(fd);

	return ret;
}



__SRCVERSION("__getset_thread_name.c $Rev: 153052 $");
