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

static int
set_rlimit_resource(rlim_t cur, rlim_t max, uint32_t *rlim_soft, uint32_t *rlim_hard,
					resmgr_context_t *ctp)
{
	rlim_t old_hard = *rlim_hard;
			
	if (max > *rlim_hard) {
		if (!proc_isaccess(0, &ctp->info)) {
			return EPERM;
		}

		if (max == RLIM_INFINITY) {
			*rlim_hard = RLIM_INFINITY;
		} else {
			*rlim_hard = max;
		}
	} else {
		*rlim_hard = max;
	}

	if (cur > *rlim_hard) {
		// reset this just in case it was changed above
		*rlim_hard = old_hard;
		return EINVAL;
	}

	if (cur == RLIM_INFINITY) {
		*rlim_soft = *rlim_hard;
	} else {
		*rlim_soft = cur;
	}

	return 0;
}


int procmgr_msg_resource(resmgr_context_t *ctp, void *vmsg) {
	PROCESS							*prp;
	union {
		struct _proc_resource_hdr		hdr;
		proc_resource_usage_t			usage;
		proc_resource_getlimit_t		getlimit;
		proc_resource_setlimit_t		setlimit;
	}									*msg = vmsg;
	rlim_t                              cur, max;
	int 								ret;

	if(!(prp = proc_lock_pid(msg->hdr.pid ? msg->hdr.pid : ctp->info.pid))) {
		return ESRCH;
	}

	switch(msg->hdr.subtype) {
	case _PROC_RESOURCE_USAGE:
		switch((int)msg->usage.i.who) {
		case RUSAGE_SELF:
		case RUSAGE_CHILDREN: {
			debug_process_t					info;
			uint64_t						utimel, stime;
			int								who;

			if(DebugProcess(NTO_DEBUG_PROCESS_INFO, prp->pid, 0, (union nto_debug_data *)&info) == -1) {
				return proc_error(errno, prp);
			}
			who = msg->usage.i.who;
			memset(&msg->usage.o, 0x00, sizeof msg->usage.o);

			if(who == RUSAGE_SELF) {
				utimel = info.utime;
				stime = info.stime;
			} else {
				utimel = info.cutime;
				stime = info.cstime;
			}
			utimel /= 1000;
			stime /= 1000;
			msg->usage.o.ru_utime.tv_sec = utimel / 1000000;
			msg->usage.o.ru_utime.tv_usec = utimel % 1000000;
			msg->usage.o.ru_stime.tv_sec = stime / 1000000;
			msg->usage.o.ru_stime.tv_usec = stime % 1000000;
			return proc_error(_RESMGR_PTR(ctp, &msg->usage.o, sizeof msg->usage.o), prp);
		}
		default:
			return proc_error(EINVAL, prp);
		}

	case _PROC_RESOURCE_SETLIMIT:
		if(msg->setlimit.i.count != 1) {
			return proc_error(ENOTSUP, prp);
		}

		// have to remap RLIM64_INFINITY values to RLIM_INFINITY
		if (msg->setlimit.i.entry[0].limit.rlim_cur == RLIM64_INFINITY) {
			cur = RLIM_INFINITY;
		} else {
			cur = msg->setlimit.i.entry[0].limit.rlim_cur;
		}
		if (msg->setlimit.i.entry[0].limit.rlim_max == RLIM64_INFINITY) {
			max = RLIM_INFINITY;
		} else {
			max = msg->setlimit.i.entry[0].limit.rlim_max;
		}

		if (cur > max) {
			return proc_error(EINVAL, prp);
		}

		/* pretest */
		switch(msg->setlimit.i.entry[0].resource) {
		case RLIMIT_NOFILE:
			{
				int num_fdcons = prp->fdcons.nentries - prp->fdcons.nfree;
				if ((cur < num_fdcons) || (max < num_fdcons)) {
					/* the limit specified can't be lowered because current 
				 	* usage is already higher than the limit
				 	*/
					return proc_error(EINVAL, prp);
				}	
			};
			break;
		case RLIMIT_CORE:   break;
		case RLIMIT_VMEM:   break;  // VMEM is the same as RLIMIT_AS
		case RLIMIT_STACK:  break;
		case RLIMIT_DATA:   break;
		case RLIMIT_NTHR:   
			{
				int num_threads = prp->threads.nentries - prp->threads.nfree;
				if ((cur < num_threads) || (max < num_threads)) {
					/* the limit specified can't be lowered because current
				 	* usage is already higher than the limit
				 	*/
					return proc_error(EINVAL, prp);
				}
			}
			break;
		case RLIMIT_NPROC:  break;
		case RLIMIT_CPU:    break; 
				    
                /* usupported ones here */ 
		case RLIMIT_FSIZE:
		case RLIMIT_MEMLOCK:
		default:
			return proc_error(EINVAL, prp);
		}
		
		ret = set_rlimit_resource(cur, max, 
			&prp->rlimit_vals_soft[msg->setlimit.i.entry[0].resource],
			&prp->rlimit_vals_hard[msg->setlimit.i.entry[0].resource], 
			ctp);
		if(ret != 0) {
			return proc_error(ret, prp);
		}

		/* post processing */ 
		switch(msg->setlimit.i.entry[0].resource) {
		case RLIMIT_CPU:
			// multiply by 1 billion because internal time vals are in nanoseconds
			prp->max_cpu_time = (uint64_t)prp->rlimit_vals_soft[RLIMIT_CPU] * 1000000000;
			break;
		case RLIMIT_CORE:
			if(cur == 0) {
				prp->flags |= _NTO_PF_NOCOREDUMP;
			} else {
				prp->flags &= ~_NTO_PF_NOCOREDUMP;
			}
			break;
		default:
			break;
		}

		break;

	case _PROC_RESOURCE_GETLIMIT:
		if(msg->getlimit.i.count != 1) {
			return proc_error(ENOTSUP, prp);
		}
		switch(msg->getlimit.i.resource[0]) {
		case RLIMIT_CORE:
		case RLIMIT_NOFILE:
		case RLIMIT_VMEM:     // VMEM is the same as RLIMIT_AS
		case RLIMIT_STACK:
		case RLIMIT_DATA:
		case RLIMIT_NTHR:
		case RLIMIT_NPROC:
		case RLIMIT_CPU: {
			// have to remap RLIM_INFINITY values to RLIM64_INFINITY
			if (prp->rlimit_vals_soft[msg->getlimit.i.resource[0]] == RLIM_INFINITY) {
				msg->getlimit.o[0].rlim_cur = RLIM64_INFINITY;
			} else {
				msg->getlimit.o[0].rlim_cur = prp->rlimit_vals_soft[msg->getlimit.i.resource[0]];
			}
			if (prp->rlimit_vals_hard[msg->getlimit.i.resource[0]] == RLIM_INFINITY) {
				msg->getlimit.o[0].rlim_max = RLIM64_INFINITY;
			} else {
				msg->getlimit.o[0].rlim_max = prp->rlimit_vals_hard[msg->getlimit.i.resource[0]];
			}
			break;
		}

		case RLIMIT_FSIZE:
		case RLIMIT_MEMLOCK:
			msg->getlimit.o[0].rlim_cur = RLIM64_INFINITY;	// Limits ignored
			msg->getlimit.o[0].rlim_max = RLIM64_INFINITY;	// Limits ignored
			break;

		default:
			return proc_error(EINVAL, prp);
		}
		return proc_error(_RESMGR_PTR(ctp, &msg->getlimit.o, sizeof msg->getlimit.o), prp);

	default:
		return proc_error(ENOSYS, prp);
	}

	return proc_error(EOK, prp);
}

__SRCVERSION("procmgr_resource.c $Rev: 153052 $");
