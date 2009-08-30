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

/*==============================================================================
 * 
 * apmgr_open
 * 
 * /proc/<pid>/partition resource manager open()
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

/*
 * dummy_apmgr_attr
 * 
 * this resource manager does not require an iofunc_attr_t but we need
 * something to attach to the ocb
*/
iofunc_attr_t  dummy_apmgr_attr;
static bool dummy_attr_init = bool_t_FALSE;

static int init_ocb(apmgr_ocb_t *ocb, pid_t pid, const char *path);

/*
 * apmgr_proc_open
 * 
 * The 'apmgr' resource manager must neccesarily sit in the "/proc/" namespace
 * however we need to differentiate accesses through "/proc/<pid>/partition/"
 * and accesses through either the memory or scheduling partition resource manger
 * (which are redirected from the <pid> list for the partition back through /proc).
 * 
 * Because this resource manager is registered under "/proc/", we are only
 * concerned with a msg->connect.path which represents a pid_t. If that is
 * found, we will respond to a readdir on /proc/<pid>/ with "./partition/". A
 * subsequent open on "/proc/<pid>/partition/" will defer to the 'apmmgr'
 * code to provide the list of partitions associated with that <pid> 
*/
int apmgr_proc_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved)
{
	pid_t  pid;
	char  *pid_extra;

	/* one time initialization */
	if (!dummy_attr_init)
	{
		dummy_attr_init = bool_t_TRUE;
		iofunc_attr_init(&dummy_apmgr_attr, 0444 | S_IFDIR, NULL, NULL);
	}

	if ((msg->connect.path == NULL) || (*msg->connect.path == '\0'))
		return ENOSYS;

	if ((nametoolong(msg->connect.path, PATH_MAX, (void *)apmgr_devno)) ||
		(nametoolong(msg->connect.path, NAME_MAX, (void *)apmgr_devno)))
		return ENAMETOOLONG;

	/*
	 * check the path name for the string "self". If the caller is attempting
	 * to open("/proc/self"), we will reconstruct the message and replace
	 * "/proc/self/..." with "/proc/<pid>/..." (pid being the callers pid string)
	*/
	if (memcmp(msg->connect.path, "self", sizeof("self") - 1) == 0)
	{
		int n;
		struct {
			io_open_t	open;
			uint8_t		path[PATH_MAX+1];
		} new_msg;

		memcpy(&new_msg.open.connect, &msg->connect, sizeof(new_msg.open.connect));
		n = ksprintf(&new_msg.open.connect.path[0], "%d", ctp->info.pid);
		if (msg->connect.path[sizeof("self") - 1] != '\0')
		{
			STRLCPY(&new_msg.open.connect.path[n], &msg->connect.path[sizeof("self") - 1],
					strlen(&msg->connect.path[sizeof("self") - 1]) + 1);
		}
		new_msg.open.connect.path_len = strlen(new_msg.open.connect.path) + 1;
		new_msg.open.connect.reply_max = sizeof(new_msg);
		return apmgr_proc_open(ctp, &new_msg.open, extra, reserved);
	}

	/*
	 * at this point, the first '/' delimited character sequence MUST represent
	 * a valid process id or it is not handled by this resource manager
	*/
	if (!isProcessPath(msg->connect.path, &pid))
		return ENOSYS;

	/*
	 * furthermore, we handle nothing above <pid>/partition but need to reply
	 * positively to an open on the <pid> directory in order to be queried
	 * about contents we wish to 'union in' (ie ./partition)
	 * 
	 * NOTE: 'meminfo' is a hidden name. It will never show up in a /proc/pid/
	 * 		listing. Its intended use is to allow the 'ap' command to display
	 * 		per process per memory class memory usage via the
	 * 		APMGR_GET_PROC_MEMINFO devctl() on an fd opened as "/proc/<pid>/meminfo".
	 * 		Note that this information is only retrievable with the 'apmgr' module
	 * 		included in the image. The 'apm' module is not required.
	*/
	if (((pid_extra = strchr(msg->connect.path, '/')) != NULL) &&
		(memcmp(pid_extra, "/partition", sizeof("/partition")-1) != 0) &&
		(memcmp(pid_extra, "/meminfo", sizeof("/meminfo")-1) != 0))
	{
//		kprintf("%s is something other than <pid>/partition or <pid>/meminfo\n", msg->connect.path);
		return ENOSYS;
	}

	/*
	 * at this point we are being asked for <pid>, <pid>/partition or
	 * <pid>/partition/...
	 * pid_extra will point to the remainder after <pid> or it will be
	 * NULL
	*/
	switch (msg->connect.subtype)
	{
		case _IO_CONNECT_COMBINE_CLOSE:
		case _IO_CONNECT_OPEN:
		case _IO_CONNECT_COMBINE:
		{
			apmgr_ocb_t *ocb;
			int  r;

			/*
			 * we do not carry an attr structure for access to the /proc/<pid>/
			 * namespace for this memory manager since we do not control the 
			 * existence of /proc/<pid> anyway. All we do here is make sure that
			 * the process exists and if it does allow the access, otherwise
			 * we return ENOENT. Note that this must be done for every access
			 * through the ocb returned on a successful open.
			 * FIX ME - more properly, procfs should be modified to handle the
			 * open of /proc/<pid>/partition/... and this resource manager
			 * called to do the work at that point then none of this would
			 * matter however the current implementation relies on this resource
			 * manager listening on /proc/<pid> as well. This is partly due to
			 * limitations in procfs and partly due to a desire to not modify
			 * procfs to much
			*/
			if (!isProcessPath(msg->connect.path, &pid) || (proc_lookup_pid(pid) == NULL))
				return ENOENT;

			if ((ocb = calloc(1, sizeof(*ocb))) == NULL) return ENOMEM;
			if ((r = init_ocb(ocb, pid, pid_extra)) != EOK)
			{
				free(ocb);
				return r;
			}
			
			/*
			 * if we are opening a real memory or scheduler partition through the
			 * proc/<pid>/ namespace, we do some additional permissions checking
			*/
			if ((ocb->attr != NULL) &&
				((ocb->attr->type == part_type_MEMPART_REAL) || (ocb->attr->type == part_type_SCHEDPART_REAL)))
			{
				struct _client_info  ci;
				unsigned ioflag = msg->connect.ioflag & (~_IO_FLAG_MASK | msg->connect.access);
				mode_t  check_mode = ((ioflag & _IO_FLAG_RD) ? S_IREAD : 0) | ((ioflag & _IO_FLAG_WR) ? S_IWRITE : 0);
			
				if (check_mode != 0) {
					if (((r = iofunc_client_info(ctp, msg->connect.ioflag, &ci)) != EOK) ||
						((r = check_access_perms(ctp, (apxmgr_attr_t *)ocb->attr, check_mode, &ci, bool_t_TRUE)) != EOK)) {
						free(ocb);
						return r;
					}
				}
			}
			if ((r = iofunc_ocb_attach(ctp, msg, &ocb->ocb, &dummy_apmgr_attr, &apmgr_io_funcs)) != EOK)
			{
				free(ocb);
				return r;
			}

			return EOK;
		}
		default:
			crash();
			return EINVAL;
	}
}

/*
 * init_ocb
 * 
 * fill in the private portion of the OCB structure based on what is being opened
 * Recall that when a open on <pid> is first done, we allocate and initialize
 * the 'pid_attr_t' structure. This is the only attr structure will really need
 * and its pretty much so that we can retieve the pid_t in any I/O calls.
 * For the OCB's, we will attach to the 'pid_attr_t' and then depending on whether
 * we have selected (via the open) a specific resource manager (ie. mem or sched)
 * the ocb->attr and ocb->apmgr_type will be filled in.
 * 
 * For example,
 * 		if <path> == "1"
 * 			ocb->apmgr_type = apmgr_type_NONE, ocb->attr = NULL
 * 		if <path> == "1/partition"
 * 			ocb->apmgr_type = apmgr_type_PROC_PART, ocb->attr = NULL
 * 		if <path> == "1/partition/mem"
 * 			ocb->apmgr_type = apmgr_type_MEM, ocb->attr = APMMGR_GETATTR(..)
 *  	if <path> == "1/partition/sched"
 * 			ocb->apmgr_type = apmgr_type_SCHED, ocb->attr = APSMGR_GETATTR(..)
 * 
 * What the APMMGR_GETATTR() and APSMGR_GETATTR() functions do is return
 * the attribute structure corresponding to namespace they manage. This is entered
 * into the <ocb> and can be used along with the 'pid_attr_t' to satisfy I/O
 * requests via the '/proc/<pid>/partition/... namespaces directly calling the
 * repective partioning manager functions.
 *  
*/
static int init_ocb(apmgr_ocb_t *ocb, pid_t pid, const char *path)
{
	CRASHCHECK(ocb == NULL);

	ocb->attr = NULL;
	ocb->apmgr_type = apmgr_type_NONE;
	ocb->pid = pid;

	if ((path == NULL) || (*path == '\0') ||
		((*path == '/') && (*++path == '\0')))
		return EOK;
	
	/* check for an exact match to 'partition' (or the hidden 'meminfo' see note above) */
	if ((path[sizeof("partition")-1] == '\0') ||
		(path[sizeof("meminfo")-1] == '\0'))
	{
		ocb->apmgr_type = apmgr_type_PROC_PART;
		return EOK;
	}
	
	/*
	 * at this point, we have either 'partition/mem' or 'partition/sched'
	 * (if its anything else, it means some other resource manager has responded
	 * with something other than mem or sched or a ./partition/ readdir)
	*/
	path += (sizeof("partition") - 1);
	if (memcmp(path, "/mem", sizeof("/mem")-1) == 0)
	{
		ocb->apmgr_type = apmgr_type_MEM;
		ocb->attr = (apmgr_attr_t *)APMMGR_GETATTR((char *)path+sizeof("/mem")-1);
		if ((ocb->attr == NULL) ||
			((ocb->attr->type == part_type_MEMPART_REAL) &&
				(find_pid_in_mpart_hierarchy(pid, (apmmgr_attr_t *)ocb->attr) == NULL)))
			return ENOENT;
		else
			return EOK;
	}
	else if (memcmp(path, "/sched", sizeof("/sched")-1) == 0)
	{
		ocb->apmgr_type = apmgr_type_SCHED;
		ocb->attr = (apmgr_attr_t *)APSMGR_GETATTR((char *)path+sizeof("/sched")-1);
		if ((ocb->attr == NULL) ||
			((ocb->attr->type == part_type_SCHEDPART_REAL) &&
				(find_pid_in_spart_hierarchy(pid, (apsmgr_attr_t *)ocb->attr) == NULL)))
			return ENOENT;
		else
			return EOK;
	}
	else
		return EINVAL;
}




__SRCVERSION("$IQ: apmgr_open.c,v 1.23 $");

