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
#include <sys/perfregs.h>

struct kerargs_debug_process {
	pid_t							pid;
	int								tid;
	union nto_debug_data			*data;
	enum nto_debug_request			request;
};

static void
kerext_debug_process(void *args) {
	PROCESS							*prp;
	THREAD							*thp, *act = actives[KERNCPU];
	DEBUG							*dep;
	struct kerargs_debug_process	*kap = args;
	int								status;
	int								stopped;
	int								tid;

	if(!(prp = lookup_pid(kap->pid))) {
		kererr(act, ESRCH);
		return;
	}

	dep = prp->debugger;

	if(kap->request == NTO_DEBUG_PROCESS_INFO || (prp->flags & _NTO_PF_TERMING)) {
		tid = 0;
	} else if(dep && kap->tid == 0) {
		tid = dep->tid + 1;
	} else {
		tid = kap->tid;
	}

	thp = 0;
	if(tid > 0 && !(thp = vector_search(&prp->threads, tid - 1,
			(kap->request == NTO_DEBUG_THREAD_INFO ||
			kap->request == NTO_DEBUG_STOP) ? (unsigned *)&tid : 0))) {
		kererr(act, ESRCH);
		return;
	}

	stopped = 0;
	if(thp && ((thp->flags & _NTO_TF_TO_BE_STOPPED) ||
			(thp->state != STATE_RUNNING && thp->state != STATE_READY))) {
		stopped = 1;
	}

	status = EINVAL;
	switch(kap->request) {
	case NTO_DEBUG_PROCESS_INFO:		// pid:na:debug_process_t
		status =  debug_process(prp, &kap->data->process);
		break;

	case NTO_DEBUG_THREAD_INFO:			// pid:tid:debug_thread_t
		status = debug_thread(prp, thp, &kap->data->thread);
		break;

	case NTO_DEBUG_GET_GREG:			// pid:tid:debug_greg_t
		if(thp) {
			memcpy(&kap->data->greg, &thp->reg, sizeof thp->reg);
			status = EOK;
		}
		break;

	case NTO_DEBUG_SET_GREG:			// pid:tid:debug_greg_t
		if(stopped) {
			lock_kernel();
			cpu_greg_load(thp, (CPU_REGISTERS *)&kap->data->greg);
			status = EOK;
		}
		break;

	case NTO_DEBUG_GET_FPREG:			// pid:tid:debug_fpreg_t
		if(thp) {
			FPU_REGISTERS	*fpudata = FPUDATA_PTR(thp->fpudata);
			int				cpu = FPUDATA_CPU(thp->fpudata);

			status = ENXIO;
			if(fpudata) {
				if(FPUDATA_INUSE(thp->fpudata) && cpu != KERNCPU) {
					// In use on another CPU; send ipi, restart kernel call
					SENDIPI(cpu, IPI_CONTEXT_SAVE);
					KERCALL_RESTART(act);
					return;
				}
				if(actives_fpu[thp->runcpu] == thp) {
					if(KERNCPU == thp->runcpu) {
						cpu_force_fpu_save(thp);
						actives_fpu[KERNCPU] = NULL;
					} else {
						// We should not get here
						crash();
					}
				}
				memcpy(&kap->data->fpreg, fpudata, sizeof *fpudata);
				status = EOK;
			} else if(thp->un.lcl.tls && thp->un.lcl.tls->__fpuemu_data) {
// @@@ NEED TO FIND PROPER SIZE OF EMULATOR DATA
				memcpy(&kap->data->fpreg, thp->un.lcl.tls->__fpuemu_data, sizeof(*fpudata) + 256);
				status = EOK;
			}
		}
		break;
			
	case NTO_DEBUG_SET_FPREG:			// pid:tid:debug_fpreg_t
		if(thp && stopped) {
			FPU_REGISTERS	*fpudata = FPUDATA_PTR(thp->fpudata);
			int				cpu = FPUDATA_CPU(thp->fpudata);

			status = ENXIO;
			if(thp->fpudata) {
				if(FPUDATA_INUSE(thp->fpudata) && cpu != KERNCPU) {
					// In use on another CPU; send ipi, restart kernel call
					SENDIPI(cpu, IPI_CONTEXT_SAVE);
					KERCALL_RESTART(act);
					return;
				}
				if(actives_fpu[thp->runcpu] == thp) {
					if(KERNCPU == thp->runcpu) {
						cpu_force_fpu_save(thp);
						actives_fpu[KERNCPU] = NULL;
					} else {
						// We should not get here
						crash();
					}
				}
				memcpy(fpudata, &kap->data->fpreg, sizeof *fpudata);
				status = EOK;
			} else if(thp->un.lcl.tls && thp->un.lcl.tls->__fpuemu_data) {
// @@@ NEED TO FIND PROPER SIZE OF EMULATOR DATA
				memcpy(thp->un.lcl.tls->__fpuemu_data, &kap->data->fpreg, sizeof(*fpudata) + 256);
				status = EOK;
			}
		}
		break;

	case NTO_DEBUG_STOP:				// pid:na:na
		if(dep) {
			status =  debug_stop(prp);
		}
		break;

	case NTO_DEBUG_RUN:					// pid:tid:debug_run_t
		if(dep && stopped) {
			status = debug_run(prp, &kap->data->run);
		}
		break;

	case NTO_DEBUG_CURTHREAD:			// pid:tid:NULL
		if(dep) {
			lock_kernel();
			SETKSTATUS(act, dep->tid + 1);
			if(thp) {
				dep->tid = thp->tid;
			}
			return;
		}
		break;

	case NTO_DEBUG_FREEZE:				// pid:tid:NULL
		if(thp == NULL){
			status = EINVAL;
			break;
		}
		if(stopped) {
			lock_kernel();
			thp->flags |= _NTO_TF_FROZEN;
		}
		break;

	case NTO_DEBUG_THAW:				// pid:tid:NULL
		if(thp == NULL){
			status = EINVAL;
			break;
		}
		if(stopped) {
			lock_kernel();
			thp->flags &= ~_NTO_TF_FROZEN;
		}
		break;

	case NTO_DEBUG_BREAK:				// pid:na:debug_break_t
		if(dep && stopped) {
			status = debug_break(prp, &kap->data->brk);
		}
		break;

	case NTO_DEBUG_GET_BREAKLIST:		// pid:na:debug_breaklist_t
		status = debug_break_list(prp, &kap->data->brklist);
		break;

	case NTO_DEBUG_SET_FLAG:			// pid:na:uint32_t
		if(dep && !(kap->data->flags & ~_DEBUG_FLAG_MASK)) {
			lock_kernel();
			dep->flags |= kap->data->flags;
		}
		break;

	case NTO_DEBUG_CLEAR_FLAG:			// pid:na:uint32_t
		if(dep && !(kap->data->flags & ~_DEBUG_FLAG_MASK)) {
			lock_kernel();
			dep->flags &= ~kap->data->flags;
		}
		break;

	case NTO_DEBUG_GET_ALTREG:			// pid:tid:debug_altreg_t
		if(thp) {
			status = cpu_debug_get_altregs(thp, &kap->data->altreg);
		}
		break;

	case NTO_DEBUG_SET_ALTREG:			// pid:tid:debug_altreg_t
		if(thp) {
			status = cpu_debug_set_altregs(thp, &kap->data->altreg);
		}
		break;

	case NTO_DEBUG_GET_PERFREG:
		if ( thp ) {
			status = cpu_debug_get_perfregs(thp, &kap->data->perfreg);
		}
		break;

	case NTO_DEBUG_SET_PERFREG:
		if ( thp && !stopped )
			status = EINVAL;
		else {
			if ( (kap->data->flags & ~PERFREGS_ENABLED_FLAG) == cpu_perfreg_id() ) {
				status = cpu_debug_set_perfregs(thp, &kap->data->perfreg);
			}
			else
				status = ENODEV;
		}
		break;
	}

	if(status != EOK) {
		kererr(act, status);
	} else {
		lock_kernel();
		SETKSTATUS(act, 0);
	}
}

int DebugProcess(enum nto_debug_request request, pid_t pid, int tid, union nto_debug_data *data) {
	struct kerargs_debug_process	args;

	args.request = request;
	args.pid = pid;
	args.tid = tid;
	args.data = data;

	return __Ring0(kerext_debug_process, &args);
}
		
struct kerargs_debug_channel {
	CHANNEL				*chp;
	debug_channel_t		*data;
	struct pril_update	up;
	enum {
		DC_INIT,
		DC_COUNT_RECEIVE,
		DC_COUNT_REPLY,
		DC_COUNT_SEND_START,
		DC_COUNT_SEND_CONTINUE,
		DC_DONE
	}					state;
};

static void kerext_debug_channel(void *args) {
	THREAD								*tp;
	struct kerargs_debug_channel		*kap = args;
	CHANNEL								*chp = kap->chp;
	debug_channel_t						*p = kap->data;
	THREAD								*act = actives[KERNCPU];

	switch(kap->state) {
	case DC_INIT:	
		p->chid = chp->chid;
		p->type = chp->type;
		p->zero = chp->zero;
		p->flags = chp->flags;

		kap->state = DC_COUNT_RECEIVE;
		// fall through
	case DC_COUNT_RECEIVE:
		p->receive_queue_depth = 0;
		for(tp = chp->receive_queue; tp != NULL; tp = tp->next.thread) {
			p->receive_queue_depth++;
		}
		kap->state = DC_COUNT_REPLY;
		// fall through
	case DC_COUNT_REPLY:
		p->reply_queue_depth = 0;
		for(tp = chp->reply_queue; tp != NULL; tp = tp->next.thread) {
			p->reply_queue_depth++;
		}
		kap->state = DC_COUNT_SEND_START;
		// fall through
	case DC_COUNT_SEND_START:
		lock_kernel();
		p->send_queue_depth = 0;
		p->pulse_queue_depth = 0;
		kap->up.pril = pril_first(&chp->send_queue);
		pril_update_register(&chp->send_queue, &kap->up);
		kap->state = DC_COUNT_SEND_CONTINUE;
		// fall through
	case DC_COUNT_SEND_CONTINUE:
		lock_kernel();
		pril_update_unregister(&chp->send_queue, &kap->up);
		for(tp = (THREAD *)kap->up.pril; tp != NULL; tp = pril_next(tp)) {
			switch(TYPE_MASK(tp->type)) {
			case TYPE_PULSE:		
			case TYPE_VPULSE:	
				p->pulse_queue_depth += ((PULSE *)tp)->count;
				break;
			default:
				p->send_queue_depth++;
				break;
			} 
			if(NEED_PREEMPT(act)) {
				// Tell the pril routines to update the up.pril pointer
				// if somebody deletes the entry while we're preempted
				kap->up.pril = pril_next(tp);
				pril_update_register(&chp->send_queue, &kap->up);
				return;
			}
		}
		kap->state = DC_DONE;
		break;
	default:
		crash();
	}
}

int DebugChannel(CHANNEL *chp, debug_channel_t *data) {
	struct kerargs_debug_channel	args;

	args.chp = chp;
	args.data = data;
	args.state = DC_INIT;

	do {
		__Ring0(kerext_debug_channel, &args);
	} while(args.state != DC_DONE);

	return EOK;
}
	
struct kerargs_debug_attach {
	pid_t							pid;
	unsigned						flags;
};

static void kerext_debug_attach(void *args) {
	struct kerargs_debug_attach		*kap = args;
	PROCESS							*prp;
	THREAD							*act = actives[KERNCPU];
	DEBUG							*dep;

	if(!(prp = lookup_pid(kap->pid))) {
		kererr(act, ESRCH);
		return;
	}

	if(prp->pid == SYSMGR_PID) {
		kererr(act, EINVAL);
		return;
	}

	if(prp->debugger) {
		kererr(act, EBUSY);
		return;
	}

	lock_kernel();
	if(!(dep = prp->debugger = object_alloc(prp, &debug_souls))) {
		kererr(act, ENOMEM);
		return;
	}
	dep->process = prp;
	dep->flags = kap->flags;
	SETKSTATUS(act, 0);
}

int DebugAttach(pid_t pid, unsigned flags) {
	struct kerargs_debug_attach		args;

	args.pid = pid;
	args.flags = flags;

	return __Ring0(kerext_debug_attach, &args);
}
	
struct kerargs_debug_detach {
	pid_t							pid;
};

static void kerext_debug_detach(void *args) {
	struct kerargs_debug_detach		*kap = args;
	PROCESS							*prp;
	DEBUG							*dep;
	THREAD							*act = actives[KERNCPU];

	if(!(prp = lookup_pid(kap->pid))) {
		kererr(act, ESRCH);
		return;
	}

	dep = prp->debugger;
	lock_kernel();
	prp->debugger = 0;
	if(prp->flags & _NTO_PF_DEBUG_STOPPED) {
		prp->flags &= ~_NTO_PF_DEBUG_STOPPED;
		if(prp->flags & _NTO_PF_TERMING) {
			if(procmgr.process_threads_destroyed) {
				struct	sigevent	ev;
				
				(*procmgr.process_threads_destroyed)(prp, &ev);
				sigevent_proc(&ev);
			}
		} else if(!dep || (dep->flags & _DEBUG_FLAG_RLC)) {
			cont_threads(prp, 0);
		} else if(dep->flags & _DEBUG_FLAG_KLC) {
			signal_kill_process(prp, SIGKILL, 0, 0, SYSMGR_PID, 0);
		}
	}
	if(dep) {
		object_free(prp, &debug_souls, dep);
	}
	SETKSTATUS(act, 0);
}

int DebugDetach(pid_t pid) {
	struct kerargs_debug_detach		args;

	args.pid = pid;

	return __Ring0(kerext_debug_detach, &args);
}

struct kerargs_tctl_args {
	PROCESS	*prp;
	int		tid;
	struct kerargs_thread_ctl tctl;
};

void kerext_thread_ctl(void *data)
{
THREAD *thp, *act = actives[KERNCPU];
struct kerargs_tctl_args *kap = (void *)data;
int err;

	// Verify the specified thread exists.
	if((thp = vector_lookup2(&kap->prp->threads, kap->tid-1)) == NULL) {
		kererr(act, ESRCH);
		return;
	}

	err = kerop_thread_ctl(act, thp, &kap->tctl);
	if ( err != ENOERROR ) {
		if ( err != EOK ) {
			kererr(act, err);
		} else {
			lock_kernel();
			SETKSTATUS(act, 0);
		}
	}
}

int KerextThreadCtl(PROCESS *prp, int tid, int32_t cmd, void *data )
{
struct kerargs_tctl_args args;

	args.prp = prp;
	args.tid = tid;
	args.tctl.cmd = cmd;
	args.tctl.data = data;
	/* Sigh - adjust for RUNMASK */
	if ( cmd == _NTO_TCTL_RUNMASK )
		memcpy( &args.tctl.data, data, sizeof(args.tctl.data) );
	return __Ring0(kerext_thread_ctl, (void *)&args);
}


__SRCVERSION("kerext_debug.c $Rev: 200383 $");
