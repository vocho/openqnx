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

#define MK_KERTABLE(pref)	\
	pref##_nop,				\
	pref##_trace_event,		\
	pref##_ring0,			\
	pref##_bad,				\
	pref##_bad,				\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_sys_cpupage_get,	\
	pref##_sys_cpupage_set,	\
	pref##_bad,				\
							\
	pref##_msg_current,		\
	pref##_msg_sendv,		\
	pref##_msg_sendv,		\
	pref##_msg_error,		\
	pref##_msg_receivev,	\
	pref##_msg_replyv,		\
	pref##_msg_readv,		\
	pref##_msg_writev,		\
	pref##_msg_readwritev,	\
	pref##_msg_info,		\
	pref##_msg_sendpulse,	\
	pref##_msg_deliver_event,\
	pref##_msg_keydata,		\
	pref##_msg_readiov,		\
	pref##_msg_receivev,	\
	pref##_msg_verify_event,\
							\
	pref##_signal_kill,		\
	pref##_signal_return,	\
	pref##_signal_fault,	\
	pref##_signal_action,	\
	pref##_signal_procmask,	\
	pref##_signal_suspend,	\
	pref##_signal_waitinfo,	\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_channel_create,	\
	pref##_channel_destroy,	\
	pref##_channel_connect_attrs,	\
	pref##_bad,				\
							\
	pref##_connect_attach,	\
	pref##_connect_detach,	\
	pref##_connect_server_info,\
	pref##_connect_client_info,\
	pref##_connect_flags,	\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_thread_create,	\
	pref##_thread_destroy,	\
	pref##_thread_destroyall,\
	pref##_thread_detach,	\
	pref##_thread_join,		\
	pref##_thread_cancel,	\
	pref##_thread_ctl,		\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_interrupt_attach,\
	pref##_interrupt_detach_func,\
	pref##_interrupt_detach,\
	pref##_interrupt_wait,	\
	pref##_interrupt_mask,	\
	pref##_interrupt_unmask,\
	pref##_bad,				\
	pref##_bad,				\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_clock_time,		\
	pref##_clock_adjust,	\
	pref##_clock_period,	\
	pref##_clock_id,		\
	pref##_bad,				\
							\
	pref##_timer_create,	\
	pref##_timer_destroy,	\
	pref##_timer_settime,	\
	pref##_timer_info,		\
	pref##_timer_alarm,		\
	pref##_timer_timeout,	\
	pref##_bad,				\
	pref##_bad,				\
							\
	pref##_sync_create,		\
	pref##_sync_destroy,	\
	pref##_sync_mutex_lock,	\
	pref##_sync_mutex_unlock,\
	pref##_sync_condvar_wait,\
	pref##_sync_condvar_signal,\
	pref##_sync_sem_post,	\
	pref##_sync_sem_wait,	\
	pref##_sync_ctl,		\
	pref##_sync_mutex_revive,\
							\
	pref##_sched_get,		\
	pref##_sched_set,		\
	pref##_sched_yield,		\
	pref##_sched_info,		\
	pref##_bad,		\
							\
	pref##_net_cred,		\
	pref##_net_vtid,		\
	pref##_net_unblock,		\
	pref##_net_infoscoid,	\
	pref##_net_signal_kill,	\
							\
	pref##_bad,				\
	pref##_bad,             \
                                                        \
        pref##_mt_ctl         \

int kdecl (* ker_call_table[])() ={
	MK_KERTABLE(ker)
};
//#define VARIANT_instr	/* PDB TODO HACK ALERT: to be removed !!! */
#if defined(VARIANT_instr)	/* PDB : contition is true */


#include <unistd.h>

//
// syscall bits
//
// 15  14    13:12 11   10   9     8    7:0
// CEX IPCEX   ^   EXIT ARGS ENTER EXIT KERCALL
//             |
//            1 1 retval ENOERROR, use KSTATUS()
//            0 1 retval is ERRNO
//            0 0 not returnval
//
// States/arguments control macros
#define _TRACE_ENTERCALL(c,n) (c|=0x200)
#define _TRACE_EXITCALL(c)    (c&=~(0xf00|_TRACE_COMM_IPC_EXIT))
#define _TRACE_OUTSTATE(c)    (c|=(0x1<<8))
#define _TRACE_GETSTATE(c)    (((c)&0x300)>>8)
#define _TRACE_SETEXIT(c)     (c|=(0x1<<11))
#define _TRACE_GETEXIT(c)     ((c)&(0x1<<11))

#define _TRACE_TESTARGRET(c)  ((((c)->syscall&(0x3<<12))==(0x3<<12))) /* kercall returned ENOERROR, KSTATUS() should be valid */
#define _TRACE_TESTARGSTAT(c) ((((c)->syscall&(0x3<<12))!=(0x1<<12))) /* kercall returned ENOERROR or EOK */
#define _TRACE_TESTARGERR(c)  (!((c)->flags&_NTO_TF_KERERR_SET)&&_TRACE_TESTARGSTAT(c)) /* kercall DIDN'T fail */
#define _TRACE_GETRETVAL(c)   (((c)->flags&_NTO_TF_KERERR_SET)?(-1):(_TRACE_TESTARGRET(c)? \
                              (KSTATUS(c)):((c)->syscall&(0x3<<12)?(-1):(0))))
#define _TRACE_SETRETSTAT(r,a) \
	r = (*((int kdecl(*)(THREAD*, void*))ker_call_table[num]))(a, (void*) kap); \
	if ((r)==ENOERROR) { \
		(a)->syscall |= (0x3<<12); \
	} else if (r) { \
		(a)->syscall |= (0x1<<12); \
		(a)->syscall &= ~(0x1<<13); \
	} else { \
		(a)->syscall &= ~(0x3<<12); \
	}

// Emitting macro definitions used/valid only within
// intercepting ker-call macro-functions: F-fast, W-wide
#define _TRACE_IN_F_0PTR(n,a,b) \
	return (_trace_emit_in_f(act, kap, n, a, b))
#define _TRACE_IN_W_0PTR(n,R,S) \
	return (_trace_emit_in_w(act, kap, n, R, S))
#define _TRACE_IN_F_1PTR(n,a,b,p) \
	(void) _TRACE_VER_PTR(p); \
	return (_trace_emit_in_f(act, kap, n, a, b))
#define _TRACE_IN_W_1PTR(n,R,S,p) \
	(void) _TRACE_VER_PTR(p); \
	return (_trace_emit_in_w(act, kap, n, R, S))
#define _TRACE_IN_F_2PTR(n,a,b,p,q) \
	(void) _TRACE_VER_PTR(p); \
	(void) _TRACE_VER_PTR(q); \
	return (_trace_emit_in_f(act, kap, n, a, b))
#define _TRACE_IN_W_2PTR(n,R,S,p,q) \
	(void) _TRACE_VER_PTR(p); \
	(void) _TRACE_VER_PTR(q); \
	return (_trace_emit_in_w(act, kap, n, R, S))
#define _TRACE_OUT_W(n,R,S)  _trace_emit_out_w(act, n, R, S);
#define _TRACE_OUT_F(n,r)    _trace_emit_out_f(act, n, _TRACE_TESTARGERR(act)?(r):(NULL))

// Other supporting macro definitions
#define _TRACE_CHK_PTID(a,n)    (!trace_masks.ker_call_pid[n]||trace_masks.ker_call_pid[n]== \
                                (uint32_t)_TRACE_DER_PTR((a)->process,pid)&& \
                                (!trace_masks.ker_call_tid[n]||trace_masks.ker_call_tid[n]== \
                                ((uint32_t)(a)->tid+1)))
#define _TRACE_CHK_ENTRY(a,m,n) (m&trace_masks.ker_call_masks[n]&&_TRACE_CHK_PTID(a,n))
#define _TRACE_IN_BOUNDRY(thp, p, size)	(WITHIN_BOUNDRY((uintptr_t)(p),(uintptr_t)(p)+(size),(thp)->process->boundry_addr))
#define _TRACE_EXE_PTR(p)       ((*(int*)(p))=(*(int*)(p)))
#define _TRACE_VER_PTR(p)       ((p)?(_TRACE_EXE_PTR(p),(p)):NULL)
#define _TRACE_PARTS(n)           (((int32_t)(n)<0)?(1):(n))

// Supporting function definitions and structures
static void fill_12B_IOV(THREAD *thp, IOV* msg, int32_t parts, uint32_t* out_buff)
{
	if(msg) {
		uint32_t len;
		void*    buff;

		if(parts<0) {
			len  = (-parts);
			buff = msg;
		} else if(parts) {
			len  = GETIOVLEN(msg);
			buff = GETIOVBASE(msg);
		} else {
			(void) memset((void*) out_buff, 0, (size_t) 12);

			return;
		}
		if (!_TRACE_IN_BOUNDRY(thp,buff,len)) {
			(void) memset((void*) out_buff, 0, (size_t) 12);
			return;
		}
		if(len>12) {
			(void) memcpy((void*) out_buff, buff, (size_t) 12);
		} else {
			(void) memset((void*) out_buff, 0, (size_t) 12);
			(void) memcpy((void*) out_buff, buff, (size_t) len);
		}
	} else {
		(void) memset((void*) out_buff, 0, (size_t) 12);
	}

	return;
}
static uint32_t read_4B_IOV(THREAD *thp, IOV* msg, int32_t parts)
{
	if(msg) {
		uint32_t len;
		void*    buff;

		if(parts<0) {
			len  = (-parts);
			buff = msg;
		} else if(parts) {
			len  = GETIOVLEN(msg);
			buff = GETIOVBASE(msg);
			if(buff == NULL) return 0;
		} else {
			return 0;
		}
		if (!_TRACE_IN_BOUNDRY(thp,buff,len)) {
			return 0;
		}
		if(len>4) {
			(void) memcpy((void*) (&len), buff, (size_t) 4);
		} else {
			(void) memcpy((void*) (&len), buff, (size_t) len);
		}

		return (len);
	} else {
		return 0;
	}
}
static void fill_52B_msg_info(THREAD *thp, struct _msg_info* info_p, uint32_t* out_buff)
{
	if(info_p && _TRACE_IN_BOUNDRY(thp,info_p,sizeof *info_p)) {
		(void) memcpy
		(
		 (void*) out_buff,
		 (void*) info_p,
		 offsetof(struct _msg_info, priority)
		);
		out_buff[10] = info_p->priority;
		out_buff[11] = info_p->flags;
		out_buff[12] = info_p->reserved;
	} else {
		(void) memset((void*) out_buff, 0, (size_t) 52);
	}

	return;
}
static void fill_16B_itimer(THREAD *thp, struct _itimer* it_p, uint32_t* out_buff)
{
	if(it_p && _TRACE_IN_BOUNDRY(thp,it_p,sizeof(*it_p))) {
		out_buff[0] = _TRACE_SEC (it_p->nsec);
		out_buff[1] = _TRACE_NSEC(it_p->nsec);
		out_buff[2] = _TRACE_SEC (it_p->interval_nsec);
		out_buff[3] = _TRACE_NSEC(it_p->interval_nsec);
	} else {
		(void) memset((void*) out_buff, 0, 4*sizeof(uint32_t));
	}

	return;
}
static void fill_8B_sync_t(THREAD *thp, sync_t* s_p, uint32_t* out_buff)
{
	if(s_p && _TRACE_IN_BOUNDRY(thp,s_p,sizeof(*s_p))) {
		out_buff[0] = s_p->__count;
		out_buff[1] = s_p->__owner;
	} else {
		out_buff[0] = NULL;
		out_buff[1] = NULL;
	}

	return;
}
static void fill_mem(THREAD *thp, void* d, const void* s, size_t l)
{
	if(s && _TRACE_IN_BOUNDRY(thp,s,l)) {
		(void) memcpy(d, s, l);
	} else {
		(void) memset(d, 0, l);
	}

	return;
}
static int _trace_emit_in_f(THREAD *act, void* kap, uint32_t num, uint32_t arg_1, uint32_t arg_2)
{
	int h_r_v=1;

	_TRACE_ENTERCALL(act->syscall, num);
	if(_TRACE_GETSTATE(act->syscall)==2&&_TRACE_CHK_ENTRY(act, _TRACE_ENTER_CALL, num)) {
		uint32_t header=_TRACE_MAKE_CODE(
		                                 RUNCPU,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_KER_CALL_C,
		                                 num
		                                );

		if(trace_masks.class_ker_call_enter_ehd_p) {
			h_r_v = exe_pt_event_h(
			                       trace_masks.class_ker_call_enter_ehd_p,
			                       header,
			                       _TRACE_DER_PTR(act->process,pid),
			                       act->tid+1,
			                       arg_1,
			                       arg_2
			                      );
		}
		if(trace_masks.ker_call_enter_ehd_p[num]&&h_r_v) {
			h_r_v = h_r_v && exe_pt_event_h(
			                                trace_masks.ker_call_enter_ehd_p[num],
			                                header,
			                                _TRACE_DER_PTR(act->process,pid),
			                                act->tid+1,
			                                arg_1,
			                                arg_2
			                               );
		}
		if(h_r_v) {
			(void) add_trace_event(header, NULL, arg_1, arg_2);
		}
	}
	if(h_r_v==2) _TRACE_SETEXIT(act->syscall);
	_TRACE_OUTSTATE(act->syscall);
	_TRACE_SETRETSTAT(h_r_v, act);

	return (h_r_v);
}
static void _trace_emit_out_f(THREAD *act, uint32_t num, uint32_t arg_2)
{
	int h_r_v      =1;
	int arg_1      =_TRACE_GETRETVAL(act);
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_KER_CALL_C,
	                                 _TRACE_MAX_KER_CALL_NUM+num
	                                );

	if(!_TRACE_TESTARGERR(act)) {
		arg_2 = KSTATUS(act);
	}

	if(trace_masks.class_ker_call_exit_ehd_p&&!_TRACE_GETEXIT(act->syscall)) {
		h_r_v = exe_pt_event_h(
		                       trace_masks.class_ker_call_exit_ehd_p,
		                       header,
		                       _TRACE_DER_PTR(act->process,pid),
		                       act->tid+1,
		                       arg_1,
		                       arg_2
		                      );
	}
	if(trace_masks.ker_call_exit_ehd_p[num]&&!_TRACE_GETEXIT(act->syscall)&&h_r_v) {
		h_r_v = h_r_v && exe_pt_event_h(
		                                trace_masks.ker_call_exit_ehd_p[num],
		                                header,
		                                _TRACE_DER_PTR(act->process,pid),
		                                act->tid+1,
		                                arg_1,
		                                arg_2
		                               );
	}
	if(h_r_v) {
		(void) add_trace_event(header, NULL, arg_1, arg_2);
	}

	return;
}
static int _trace_emit_in_w(THREAD *act, void* kap, uint32_t num, uint32_t* arg_arr, uint32_t len)
{
	int h_r_v=1;

	_TRACE_ENTERCALL(act->syscall, num);
	if(_TRACE_GETSTATE(act->syscall)==2&&_TRACE_CHK_ENTRY(act, _TRACE_ENTER_CALL, num)) {
		uint32_t header=_TRACE_MAKE_CODE(
		                                 RUNCPU,
		                                 NULL,
		                                 _TRACE_KER_CALL_C,
		                                 num
		                                );

		if(trace_masks.class_ker_call_enter_ehd_p) {
			h_r_v = exe_pt_event_h_buff(
			                            trace_masks.class_ker_call_enter_ehd_p,
			                            header,
			                            _TRACE_DER_PTR(act->process,pid),
			                            act->tid+1,
			                            (void*) arg_arr,
			                            sizeof(uint32_t)*len
			                           );
		}
		if(trace_masks.ker_call_enter_ehd_p[num]&&h_r_v) {
			h_r_v = h_r_v && exe_pt_event_h_buff(
			                                     trace_masks.ker_call_enter_ehd_p[num],
			                                     header,
			                                     _TRACE_DER_PTR(act->process,pid),
			                                     act->tid+1,
			                                     (void*) arg_arr,
			                                     sizeof(uint32_t)*len
			                                    );
		}
		if(h_r_v) {
			add_trace_buffer(header, arg_arr, len);
		}
	}
	if(h_r_v==2) _TRACE_SETEXIT(act->syscall);
	_TRACE_OUTSTATE(act->syscall);
	_TRACE_SETRETSTAT(h_r_v, act);

	return (h_r_v);
}
static void _trace_emit_out_w(THREAD *act, int num, uint32_t* arg_arr, uint32_t len)
{
	int h_r_v      =1;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 NULL,
	                                 _TRACE_KER_CALL_C,
	                                 _TRACE_MAX_KER_CALL_NUM+num
	                                );

	arg_arr[0] = _TRACE_GETRETVAL(act);
	if(!_TRACE_TESTARGERR(act)) {
		arg_arr[1] = KSTATUS(act);
		len = 2;
	}
	if(trace_masks.class_ker_call_exit_ehd_p&&!_TRACE_GETEXIT(act->syscall)) {
		h_r_v = exe_pt_event_h_buff(
		                            trace_masks.class_ker_call_exit_ehd_p,
		                            header,
		                            _TRACE_DER_PTR(act->process,pid),
		                            act->tid+1,
		                            (void*) arg_arr,
		                            sizeof(uint32_t)*len
		                           );
	}
	if(trace_masks.ker_call_exit_ehd_p[num]&&!_TRACE_GETEXIT(act->syscall)&&h_r_v) {
		h_r_v = h_r_v && exe_pt_event_h_buff(
		                                     trace_masks.ker_call_exit_ehd_p[num],
		                                     header,
		                                     _TRACE_DER_PTR(act->process,pid),
		                                     act->tid+1,
		                                     (void*) arg_arr,
		                                     sizeof(uint32_t)*len
		                                                            );
	}
	if(h_r_v) {
		add_trace_buffer(header, arg_arr, len);
	}

	return;
}
static void xfer_trace_fault_handler(THREAD* act, CPU_REGISTERS* regs, unsigned flags) {
	_TRACE_OUT_F(_TRACE_GETSYSCALL(act->syscall), NULL);
	_TRACE_EXITCALL(act->syscall);
	__ker_exit();
}

const static struct fault_handlers xfer_trace_str_fault_handler = {
	xfer_trace_fault_handler, NULL
};

// Intercepting ker-call macro-functions. For efficiency
// reasons, the following assumptions have been made:
//
//       i) sizeof(int)      == sizeof(_int32)  == 4
//      ii) sizeof(unsigned) == sizeof(_uint32) == 4
//

int kdecl _trace_ker_empty(THREAD *act,struct kerargs_null *kap)
{
	int n=KTYPE(act);

	_TRACE_IN_F_0PTR(n, NULL, NULL);
}
int kdecl _trace_ker_trace_event(THREAD *act, struct kerargs_trace_event *kap)
{
	uint32_t len;

	/* inserting user trace events shouldn't show the kernel entry/exit */
	switch(_TRACE_GET_FLAG(*((unsigned *)kap->data))) {
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSCLASSEVENT):
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCCLASSEVENT):
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSUSEREVENT):
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCUSEREVENT):
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTUSRSTREVENT):
	case _TRACE_GET_FLAG(_NTO_TRACE_INSERTEVENT):
		return ker_trace_event( act, kap );
	default:
		break;
	}

	if(kap->data) {
		len = *kap->data>>28;
	} else {
		len = 0;
	}

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TRACE_EVENT]) {
		uint32_t arg_arr[10];

		if ( len > 0 ) {
			len = min(len,NUM_ELTS(arg_arr));
			memcpy( &arg_arr[0], kap->data, len * sizeof(uint32_t) );
		}

		_TRACE_IN_W_0PTR(__KER_TRACE_EVENT, arg_arr, len );
	} else {
		uint32_t arg_1=NULL;
		uint32_t arg_2=NULL;

		if(len>1) {
			arg_1 = *kap->data;
			arg_2 = *(kap->data+1);
		} else if(len) {
			arg_1 = *kap->data;
		}
		_TRACE_IN_F_0PTR(__KER_TRACE_EVENT, arg_1, arg_2);
	}
}
int kdecl _trace_ker_ring0(THREAD *act, struct kerargs_ring0 *kap)
{
#ifdef VARIANT_ring0
	_TRACE_IN_F_0PTR(__KER_RING0, (uint32_t) kap->func, (uint32_t) kap->arg);
#else
	return ker_ring0(act,kap);
#endif
}

int kdecl _trace_ker_sys_cpupage_get(THREAD *act, struct kerargs_sys_cpupage_get *kap)
{
	_TRACE_IN_F_0PTR(__KER_SYS_CPUPAGE_GET, kap->index, NULL);
}
int kdecl _trace_ker_sys_cpupage_set(THREAD *act, struct kerargs_sys_cpupage_set *kap)
{
	_TRACE_IN_F_0PTR(__KER_SYS_CPUPAGE_SET, kap->index, kap->value);
}

int kdecl _trace_ker_msg_current(THREAD *act, struct kerargs_msg_current *kap)
{
	_TRACE_IN_F_0PTR(__KER_MSG_CURRENT, kap->rcvid, NULL);
}
int kdecl _trace_ker_msg_sendv(THREAD *act, struct kerargs_msg_sendv *kap)
{
	int n=KTYPE(act);

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[n]) {
		uint32_t arg_arr[6];

		arg_arr[0] = kap->coid;
		arg_arr[1] = _TRACE_PARTS(kap->sparts);
		arg_arr[2] = _TRACE_PARTS(kap->rparts);

		fill_12B_IOV(act, kap->smsg, kap->sparts, (arg_arr+3));
		_TRACE_IN_W_0PTR(n, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(n, kap->coid, read_4B_IOV(act, kap->smsg, kap->sparts));
	}
}
int kdecl _trace_ker_msg_error(THREAD *act, struct kerargs_msg_error *kap)
{
	_TRACE_IN_F_0PTR(__KER_MSG_ERROR, kap->rcvid, kap->err);
}
int kdecl _trace_ker_msg_receivev(THREAD *act, struct kerargs_msg_receivev *kap)
{
	int n=KTYPE(act);

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[n]) {
		_TRACE_IN_F_2PTR(n, kap->chid, _TRACE_PARTS(kap->rparts), kap->rmsg, kap->info);
	} else {
		_TRACE_IN_F_1PTR(n, kap->chid, _TRACE_PARTS(kap->rparts), kap->rmsg);
	}
}
int kdecl _trace_ker_msg_replyv(THREAD *act, struct kerargs_msg_replyv *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_REPLYV]) {
		uint32_t arg_arr[6];

		arg_arr[0] = kap->rcvid;
		arg_arr[1] = _TRACE_PARTS(kap->sparts);
		arg_arr[2] = kap->status;

		fill_12B_IOV(act, kap->smsg, kap->sparts, (arg_arr+3));
		_TRACE_IN_W_0PTR(__KER_MSG_REPLYV, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_MSG_REPLYV, kap->rcvid, kap->status);
	}
}
int kdecl _trace_ker_msg_readv(THREAD *act, struct kerargs_msg_readv *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_READV]) {
		uint32_t arg_arr[4];

		arg_arr[0] = (uint32_t) kap->rcvid;
		arg_arr[1] = (uint32_t) kap->rmsg;
		arg_arr[2] = (uint32_t) _TRACE_PARTS(kap->rparts);
		arg_arr[3] = (uint32_t) kap->offset;

		_TRACE_IN_W_1PTR(__KER_MSG_READV, arg_arr,  NUM_ELTS(arg_arr), kap->rmsg);
	} else {
		_TRACE_IN_F_1PTR(__KER_MSG_READV, kap->rcvid, kap->offset, kap->rmsg);
	}
}
int kdecl _trace_ker_msg_writev(THREAD *act, struct kerargs_msg_writev *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_WRITEV]) {
		uint32_t arg_arr[6];

		arg_arr[0] = kap->rcvid;
		arg_arr[1] = _TRACE_PARTS(kap->sparts);
		arg_arr[2] = kap->offset;

		fill_12B_IOV(act, kap->smsg, kap->sparts, (arg_arr+3));
		_TRACE_IN_W_0PTR(__KER_MSG_WRITEV, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_MSG_WRITEV, kap->rcvid, kap->offset);
	}
}
int kdecl _trace_ker_msg_readwritev(THREAD *act, struct kerargs_msg_readwritev *kap)
{
	_TRACE_IN_F_0PTR(__KER_MSG_READWRITEV, kap->src_rcvid, kap->dst_rcvid);
}
int kdecl _trace_ker_msg_info(THREAD *act, struct kerargs_msg_info *kap)
{
	_TRACE_IN_F_1PTR(__KER_MSG_INFO, kap->rcvid, (uint32_t) kap->info, kap->info);
}
int kdecl _trace_ker_msg_sendpulse(THREAD *act, struct kerargs_msg_sendpulse *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_SEND_PULSE]) {
		uint32_t arg_arr[4];

		arg_arr[0] = (uint32_t) kap->coid;
		arg_arr[1] = (uint32_t) kap->priority;
		arg_arr[2] = (uint32_t) kap->code;
		arg_arr[3] = (uint32_t) kap->value;

		_TRACE_IN_W_0PTR(__KER_MSG_SEND_PULSE, arg_arr, 4);
	} else {
		_TRACE_IN_F_0PTR(__KER_MSG_SEND_PULSE, kap->coid, kap->code);
	}
}
int kdecl _trace_ker_msg_deliver_event(THREAD *act, struct kerargs_msg_deliver_event *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_DELIVER_EVENT]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct sigevent))];

		arg_arr[0] = kap->rcvid;
		fill_mem(act, (void*) (arg_arr+1), (void*) kap->event, sizeof(struct sigevent));
		_TRACE_IN_W_0PTR(__KER_MSG_DELIVER_EVENT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_MSG_DELIVER_EVENT,
		 (uint32_t) kap->rcvid,
		 _TRACE_DER_PTR(kap->event, sigev_notify)
		);
	}
}
int kdecl _trace_ker_msg_keydata(THREAD *act, struct kerargs_msg_keydata *kap)
{
	_TRACE_IN_F_1PTR(__KER_MSG_KEYDATA, kap->rcvid, kap->op, kap->newkey);
}
int kdecl _trace_ker_msg_readiov(THREAD *act, struct kerargs_msg_readiov *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_READIOV]) {
		uint32_t arg_arr[4];

		arg_arr[0] = kap->rcvid;
		arg_arr[1] = kap->parts;
		arg_arr[2] = kap->offset;
		arg_arr[3] = kap->flags;

		_TRACE_IN_W_1PTR(__KER_MSG_READIOV, arg_arr, NUM_ELTS(arg_arr), kap->iov);
	} else {
		_TRACE_IN_F_1PTR(__KER_MSG_READIOV, kap->rcvid, kap->offset, kap->iov);
	}
}
int kdecl _trace_ker_msg_verify_event(THREAD *act, struct kerargs_msg_verify_event *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_VERIFY_EVENT]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct sigevent))];

		arg_arr[0] = kap->rcvid;

		fill_mem(act, (void*) (arg_arr+1), (void*) kap->event, sizeof(struct sigevent));
		_TRACE_IN_W_0PTR(__KER_MSG_VERIFY_EVENT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_MSG_VERIFY_EVENT,
		 (uint32_t) kap->rcvid,
		 _TRACE_DER_PTR(kap->event, sigev_notify)
		);
	}
}

int kdecl _trace_ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_KILL]) {
		uint32_t arg_arr[6];

		arg_arr[0] =            kap->nd;
		arg_arr[1] = (uint32_t) kap->pid;
		arg_arr[2] = (uint32_t) kap->tid;
		arg_arr[3] = (uint32_t) kap->signo;
		arg_arr[4] = (uint32_t) kap->code;
		arg_arr[5] = (uint32_t) kap->value;

		_TRACE_IN_W_0PTR(__KER_SIGNAL_KILL, arg_arr, 6);
	} else {
		_TRACE_IN_F_0PTR(__KER_SIGNAL_KILL, kap->pid, kap->signo);
	}
}
int kdecl _trace_ker_signal_return(THREAD *act, struct kerargs_signal_return *kap)
{
	_TRACE_IN_F_0PTR(__KER_SIGNAL_RETURN, (uint32_t) kap->s, NULL);
}
int kdecl _trace_ker_signal_fault(THREAD *act, struct kerargs_signal_fault *kap)
{
	_TRACE_IN_F_1PTR(__KER_SIGNAL_FAULT, kap->sigcode, kap->addr, (uint32_t*) kap->regs);
}
int kdecl _trace_ker_signal_action(THREAD *act, struct kerargs_signal_action *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_ACTION]) {
		uint32_t arg_arr[7];

		arg_arr[0] = kap->pid;
		arg_arr[1] = (uint32_t) kap->sigstub;
		arg_arr[2] = kap->signo;

		fill_mem(act, (void*) (arg_arr+3), (void*) kap->act, (size_t) 16);
		_TRACE_IN_W_1PTR(__KER_SIGNAL_ACTION, arg_arr, NUM_ELTS(arg_arr), kap->oact);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_SIGNAL_ACTION,
		 kap->signo,
		 (uint32_t) _TRACE_DER_PTR(kap->act, sa_handler),
		 kap->oact
		);
	}
}
int kdecl _trace_ker_signal_procmask(THREAD *act, struct kerargs_signal_procmask *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_PROCMASK]) {
		uint32_t arg_arr[5];

		arg_arr[0] = kap->pid;
		arg_arr[1] = kap->tid;
		arg_arr[2] = kap->how;

		fill_mem(act, (void*) (arg_arr+3), (void*) kap->sig_blocked, (size_t) 8);
		_TRACE_IN_W_1PTR(__KER_SIGNAL_PROCMASK, arg_arr, NUM_ELTS(arg_arr), kap->old_sig_blocked);
	} else {
		_TRACE_IN_F_1PTR(__KER_SIGNAL_PROCMASK, kap->pid, kap->tid, kap->old_sig_blocked);
	}
}
int kdecl _trace_ker_signal_suspend(THREAD *act, struct kerargs_signal_suspend *kap)
{
	if(kap->sig_blocked) {
		_TRACE_IN_F_0PTR
		(
		 __KER_SIGNAL_SUSPEND,
		 kap->sig_blocked->__bits[0],
		 kap->sig_blocked->__bits[1]
		);
	} else {
		_TRACE_IN_F_0PTR(__KER_SIGNAL_SUSPEND, NULL, NULL);
	}
}
int kdecl _trace_ker_signal_waitinfo(THREAD *act, struct kerargs_signal_wait *kap)
{
	if(kap->sig_wait) {
		_TRACE_IN_F_1PTR
		(
		 __KER_SIGNAL_WAITINFO,
		 (uint32_t) kap->sig_wait->__bits[0],
		 (uint32_t) kap->sig_wait->__bits[1],
		 kap->sig_info
		);
	} else {
		_TRACE_IN_F_1PTR(__KER_SIGNAL_WAITINFO, NULL, NULL, kap->sig_info);
	}
}

int kdecl _trace_ker_channel_create(THREAD *act, struct kerargs_channel_create *kap)
{
	_TRACE_IN_F_0PTR(__KER_CHANNEL_CREATE, kap->flags, NULL);
}
int kdecl _trace_ker_channel_destroy(THREAD *act, struct kerargs_channel_destroy *kap)
{
	_TRACE_IN_F_0PTR(__KER_CHANNEL_DESTROY, kap->chid, NULL);
}
int kdecl _trace_ker_channel_connect_attrs(THREAD *act, struct kerargs_channel_connect_attr *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CHANCON_ATTR]) {
		uint32_t arg_arr[2 + _TRACE_ROUND_UP(sizeof(union _channel_connect_attr))];

		arg_arr[0] = kap->id;
		arg_arr[1] = kap->flags;

		if ( kap->new_attrs ) {
			fill_mem(act, (void*) (arg_arr+2), (void*) kap->new_attrs, sizeof(union _channel_connect_attr));
		} else {
			memset((void*) (arg_arr+2), 0, sizeof(union _channel_connect_attr));
		}
		_TRACE_IN_W_0PTR(__KER_CHANCON_ATTR, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_CHANCON_ATTR, kap->id, kap->flags );
	}
}

int kdecl _trace_ker_connect_attach(THREAD *act, struct kerargs_connect_attach *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CONNECT_ATTACH]) {
		uint32_t arg_arr[5];

		arg_arr[0] =            kap->nd;
		arg_arr[1] = (uint32_t) kap->pid;
		arg_arr[2] = (uint32_t) kap->chid;
		arg_arr[3] = (uint32_t) kap->index;
		arg_arr[4] = (uint32_t) kap->flags;

		_TRACE_IN_W_0PTR(__KER_CONNECT_ATTACH, arg_arr, 5);
	} else {
		_TRACE_IN_F_0PTR(__KER_CONNECT_ATTACH, kap->nd, kap->pid);
	}
}
int kdecl _trace_ker_connect_detach(THREAD *act, struct kerargs_connect_detach *kap)
{
	_TRACE_IN_F_0PTR(__KER_CONNECT_DETACH, kap->coid, NULL);
}
int kdecl _trace_ker_connect_server_info(THREAD *act, struct kerargs_connect_server_info *kap)
{
	_TRACE_IN_F_1PTR(__KER_CONNECT_SERVER_INFO, kap->pid, kap->coid, kap->info); }
int kdecl _trace_ker_connect_client_info(THREAD *act, struct kerargs_connect_client_info *kap)
{
	_TRACE_IN_F_1PTR(__KER_CONNECT_CLIENT_INFO, kap->scoid, kap->ngroups, kap->info);
}
int kdecl _trace_ker_connect_flags(THREAD *act, struct kerargs_connect_flags *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CONNECT_FLAGS]) {
		uint32_t arg_arr[4];

		arg_arr[0] = (uint32_t) kap->pid;
		arg_arr[1] = (uint32_t) kap->coid;
		arg_arr[2] = (uint32_t) kap->mask;
		arg_arr[3] = (uint32_t) kap->bits;

		_TRACE_IN_W_0PTR(__KER_CONNECT_FLAGS, arg_arr, 4);
	} else {
		_TRACE_IN_F_0PTR(__KER_CONNECT_FLAGS, kap->coid, kap->bits);
	}
}

int kdecl _trace_ker_thread_create(THREAD *act, struct kerargs_thread_create *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_THREAD_CREATE]) {
		uint32_t arg_arr[3 + _TRACE_ROUND_UP(sizeof(struct _thread_attr))];

		arg_arr[0] = kap->pid;
		arg_arr[1] = (uint32_t) kap->func;
		arg_arr[2] = (uint32_t) kap->arg;

		fill_mem(act, (void*) (arg_arr+3), (void*) kap->attr, sizeof(struct _thread_attr));
		_TRACE_IN_W_0PTR(__KER_THREAD_CREATE, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_THREAD_CREATE, (uint32_t) kap->func, (uint32_t) kap->arg);
	}
}
int kdecl _trace_ker_thread_destroy(THREAD *act, struct kerargs_thread_destroy *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_THREAD_DESTROY]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->tid;
		arg_arr[1] = (uint32_t) kap->priority;
		arg_arr[2] = (uint32_t) kap->status;

		_TRACE_IN_W_0PTR(__KER_THREAD_DESTROY, arg_arr, 3);
	} else {
		_TRACE_IN_F_0PTR(__KER_THREAD_DESTROY, kap->tid, (uint32_t) kap->status);
	}
}
int kdecl _trace_ker_thread_destroyall(THREAD *act, struct kerargs_null *kap)
{
	_TRACE_IN_F_0PTR(__KER_THREAD_DESTROYALL, NULL, NULL);
}
int kdecl _trace_ker_thread_detach(THREAD *act, struct kerargs_thread_detach *kap)
{
	_TRACE_IN_F_0PTR(__KER_THREAD_DETACH, kap->tid, NULL);
}
int kdecl _trace_ker_thread_join(THREAD *act, struct kerargs_thread_join *kap)
{ _TRACE_IN_F_1PTR(__KER_THREAD_JOIN, kap->tid, (uint32_t) kap->status, kap->status); }
int kdecl _trace_ker_thread_cancel(THREAD *act, struct kerargs_thread_cancel *kap)
{
	_TRACE_IN_F_0PTR(__KER_THREAD_CANCEL, kap->tid, (uint32_t) kap->canstub);
}
int kdecl _trace_ker_thread_ctl(THREAD *act, struct kerargs_thread_ctl *kap)
{
	_TRACE_IN_F_0PTR(__KER_THREAD_CTL, kap->cmd, (uint32_t) kap->data);
}

int kdecl _trace_ker_interrupt_attach(THREAD *act, struct kerargs_interrupt_attach *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_INTERRUPT_ATTACH]) {
		uint32_t arg_arr[5];
		arg_arr[0] = (uint32_t)kap->intr;
		arg_arr[1] = (uint32_t)kap->handler;
		arg_arr[2] = (uint32_t)kap->area;
		arg_arr[3] = (uint32_t)kap->areasize;
		arg_arr[4] = (uint32_t)kap->flags;
		_TRACE_IN_W_0PTR(__KER_INTERRUPT_ATTACH, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_INTERRUPT_ATTACH, kap->intr, kap->flags);
	}
}
int kdecl _trace_ker_interrupt_detach_func(THREAD *act, struct kerargs_interrupt_detach_func *kap)
{
	_TRACE_IN_F_0PTR(__KER_INTERRUPT_DETACH_FUNC, kap->intr, (uint32_t) kap->handler);
}
int kdecl _trace_ker_interrupt_detach(THREAD *act, struct kerargs_interrupt_detach *kap)
{
	_TRACE_IN_F_0PTR(__KER_INTERRUPT_DETACH, kap->id, NULL);
}
int kdecl _trace_ker_interrupt_wait(THREAD *act, struct kerargs_interrupt_wait *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_INTERRUPT_WAIT]) {
		uint32_t arg_arr[3];

		arg_arr[0] = kap->flags;
		if(kap->timeout) {
			arg_arr[1] = _TRACE_SEC (*kap->timeout);
			arg_arr[2] = _TRACE_NSEC(*kap->timeout);
		} else {
			arg_arr[1] = NULL;
			arg_arr[2] = NULL;
		}
		_TRACE_IN_W_0PTR(__KER_INTERRUPT_WAIT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
	 	(
		 __KER_INTERRUPT_WAIT,
		 kap->flags,
		 kap->timeout?_TRACE_SEC(*kap->timeout):NULL
		);
	}
}
int kdecl _trace_ker_interrupt_mask(THREAD *act, struct kerargs_interrupt_mask *kap)
{
	_TRACE_IN_F_0PTR(__KER_INTERRUPT_MASK, kap->intr, kap->id);
}
int kdecl _trace_ker_interrupt_unmask(THREAD *act, struct kerargs_interrupt_unmask *kap)
{
	_TRACE_IN_F_0PTR(__KER_INTERRUPT_UNMASK, kap->intr, kap->id);
}

int kdecl _trace_ker_clock_time(THREAD *act, struct kerargs_clock_time *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_TIME]) {
		uint32_t arg_arr[3];

		arg_arr [0] = kap->id;
		if(kap->new) {
			arg_arr[1] = _TRACE_SEC (*kap->new);
			arg_arr[2] = _TRACE_NSEC(*kap->new);
		} else {
			arg_arr[1] = NULL;
			arg_arr[2] = NULL;
		}
		_TRACE_IN_W_1PTR(__KER_CLOCK_TIME, arg_arr, NUM_ELTS(arg_arr), kap->old);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_CLOCK_TIME,
		 kap->id,
		 kap->new?_TRACE_SEC(*kap->new):NULL,
		 kap->old
		);
	}
}
int kdecl _trace_ker_clock_adjust(THREAD *act, struct kerargs_clock_adjust *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_ADJUST]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _clockadjust))];

		arg_arr[0] = kap->id;
		fill_mem(act, (void*) (arg_arr+1), (void*) kap->new, sizeof(struct _clockadjust));
		_TRACE_IN_W_1PTR(__KER_CLOCK_ADJUST, arg_arr, NUM_ELTS(arg_arr), kap->old);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_CLOCK_ADJUST,
		 kap->id,
		 _TRACE_DER_PTR(kap->new,tick_count),
		 kap->old
		);
	}
}
int kdecl _trace_ker_clock_period(THREAD *act, struct kerargs_clock_period *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_PERIOD]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _clockperiod))];

		arg_arr[0] = kap->id;
		fill_mem(act, (void*) (arg_arr+1), (void*) kap->new, sizeof(struct _clockperiod));
		_TRACE_IN_W_1PTR(__KER_CLOCK_PERIOD, arg_arr, NUM_ELTS(arg_arr), kap->old);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_CLOCK_PERIOD,
		 kap->id,
		 _TRACE_DER_PTR(kap->new, nsec),
		 kap->old
		);
	}
}
int kdecl _trace_ker_clock_id(THREAD *act, struct kerargs_clock_id *kap)
{
	_TRACE_IN_F_0PTR(__KER_CLOCK_ID, kap->pid, kap->tid);
}

int kdecl _trace_ker_timer_create(THREAD *act, struct kerargs_timer_create *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_CREATE]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct sigevent))];

		arg_arr[0] = kap->id;
		fill_mem(act, (void*) (arg_arr+1), (void*) kap->event, sizeof(struct sigevent));
		_TRACE_IN_W_0PTR(__KER_TIMER_CREATE, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_TIMER_CREATE,
		 (uint32_t) kap->id,
		 (uint32_t) _TRACE_DER_PTR(kap->event, sigev_notify)
		);
	}
}
int kdecl _trace_ker_timer_destroy(THREAD *act, struct kerargs_timer_destroy *kap)
{
	_TRACE_IN_F_0PTR(__KER_TIMER_DESTROY, kap->id, NULL);
}
int kdecl _trace_ker_timer_settime(THREAD *act, struct kerargs_timer_settime *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_SETTIME]) {
		uint32_t arg_arr[6];

		arg_arr[0] = kap->id;
		arg_arr[1] = kap->flags;
		fill_16B_itimer(act, kap->itime, arg_arr+2);
		_TRACE_IN_W_1PTR(__KER_TIMER_SETTIME, arg_arr, NUM_ELTS(arg_arr), kap->oitime);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_TIMER_SETTIME,
		 kap->id,
		 kap->itime?_TRACE_SEC(kap->itime->nsec):NULL,
		 kap->oitime
		);
	}
}
int kdecl _trace_ker_timer_info(THREAD *act, struct kerargs_timer_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_INFO]) {
		uint32_t arg_arr[4];

		arg_arr[0] = kap->pid;
		arg_arr[1] = kap->id;
		arg_arr[2] = kap->flags;
		arg_arr[3] = (uint32_t) kap->info;

		_TRACE_IN_W_1PTR(__KER_TIMER_INFO, arg_arr, 4, kap->info);
	} else {
		_TRACE_IN_F_1PTR(__KER_TIMER_INFO, kap->pid, kap->id, kap->info);
	}
}
int kdecl _trace_ker_timer_alarm(THREAD *act, struct kerargs_timer_alarm *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_ALARM]) {
		uint32_t arg_arr[5];

		arg_arr[0] = kap->id;
		fill_16B_itimer(act, kap->itime, arg_arr+1);
		_TRACE_IN_W_1PTR(__KER_TIMER_ALARM, arg_arr, NUM_ELTS(arg_arr), kap->otime);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_TIMER_ALARM,
		 kap->id,
		 kap->itime?_TRACE_SEC(kap->itime->nsec):NULL,
		 kap->otime
		);
	}
}
int kdecl _trace_ker_timer_timeout(THREAD *act, struct kerargs_timer_timeout *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_TIMEOUT]) {
		uint32_t arg_arr[4 + _TRACE_ROUND_UP(sizeof(struct sigevent))];

		arg_arr[0] = kap->id;
		arg_arr[1] = kap->timeout_flags;
		if(kap->ntime) {
			arg_arr[2] = _TRACE_SEC (*kap->ntime);
			arg_arr[3] = _TRACE_NSEC(*kap->ntime);
		} else {
			arg_arr[2] = NULL;
			arg_arr[3] = NULL;
		}
		fill_mem(act, (void*) (arg_arr+4), (void*) kap->event, sizeof(struct sigevent));
		_TRACE_IN_W_1PTR(__KER_TIMER_TIMEOUT, arg_arr, NUM_ELTS(arg_arr), kap->otime);
	} else {
		_TRACE_IN_F_1PTR
		(
		 __KER_TIMER_TIMEOUT,
		 kap->timeout_flags,
		 kap->ntime?_TRACE_SEC(*kap->ntime):NULL,
		 kap->otime
		);
	}
}

int kdecl _trace_ker_sync_create(THREAD *act, struct kerargs_sync_create *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_CREATE]) {
		uint32_t arg_arr[8];

		arg_arr[0] = kap->type;
		arg_arr[1] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+2);
		fill_mem(act, (void*) (arg_arr+4), (void*) kap->attr, (size_t) 16);
		_TRACE_IN_W_0PTR(__KER_SYNC_CREATE, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_SYNC_CREATE, kap->type, (uint32_t) kap->sync);
	}
}
int kdecl _trace_ker_sync_destroy(THREAD *act, struct kerargs_sync_destroy *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_DESTROY]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+1);
		_TRACE_IN_W_0PTR(__KER_SYNC_DESTROY, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_DESTROY,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __owner)
		);
	}
}
int kdecl _trace_ker_sync_mutex_lock(THREAD *act, struct kerargs_sync_mutex_lock *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_MUTEX_LOCK]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+1);
		_TRACE_IN_W_0PTR(__KER_SYNC_MUTEX_LOCK, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_MUTEX_LOCK,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __owner)
		);
	}
}
int kdecl _trace_ker_sync_mutex_unlock(THREAD *act, struct kerargs_sync_mutex_unlock *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_MUTEX_UNLOCK]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+1);
		_TRACE_IN_W_0PTR(__KER_SYNC_MUTEX_UNLOCK, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_MUTEX_UNLOCK,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __owner)
		);
	}
}
int kdecl _trace_ker_sync_condvar_wait(THREAD *act, struct kerargs_sync_condvar_wait *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_CONDVAR_WAIT]) {
		uint32_t arg_arr[6];

		arg_arr[0] = (uint32_t) kap->sync;
		arg_arr[1] = (uint32_t) kap->mutex;
		fill_8B_sync_t(act, kap->sync, arg_arr+2);
		fill_8B_sync_t(act, kap->mutex, arg_arr+4);
		_TRACE_IN_W_0PTR(__KER_SYNC_CONDVAR_WAIT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_CONDVAR_WAIT,
		 (uint32_t) kap->sync,
		 (uint32_t) kap->mutex
		);
	}
}
int kdecl _trace_ker_sync_condvar_signal(THREAD *act, struct kerargs_sync_condvar_signal *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_CONDVAR_SIGNAL]) {
		uint32_t arg_arr[4];

		arg_arr[0] = (uint32_t) kap->sync;
		arg_arr[1] = kap->all;
		fill_8B_sync_t(act, kap->sync, arg_arr+2);
		_TRACE_IN_W_0PTR(__KER_SYNC_CONDVAR_SIGNAL, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_SYNC_CONDVAR_SIGNAL, (uint32_t) kap->sync, kap->all);
	}
}
int kdecl _trace_ker_sync_sem_post(THREAD *act, struct kerargs_sync_sem_post *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_SEM_POST]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+1);
		_TRACE_IN_W_0PTR(__KER_SYNC_SEM_POST, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_SEM_POST,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __count)
		);
	}
}
int kdecl _trace_ker_sync_sem_wait(THREAD *act, struct kerargs_sync_sem_wait *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_SEM_WAIT]) {
		uint32_t arg_arr[4];

		arg_arr[0] = (uint32_t) kap->sync;
		arg_arr[1] = kap->try;
		fill_8B_sync_t(act, kap->sync, arg_arr+2);
		_TRACE_IN_W_0PTR(__KER_SYNC_SEM_WAIT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_SEM_WAIT,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __count)
		);
	}
}
int kdecl _trace_ker_sync_ctl(THREAD *act, struct kerargs_sync_ctl *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_CTL]) {
		uint32_t arg_arr[5];

		arg_arr[0] = kap->cmd;
		arg_arr[1] = (uint32_t) kap->sync;
		arg_arr[2] = (uint32_t) kap->data;
		fill_8B_sync_t(act, kap->sync, arg_arr+3);
		_TRACE_IN_W_0PTR(__KER_SYNC_CTL, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_SYNC_CTL, kap->cmd, (uint32_t) kap->sync);
	}
}
int kdecl _trace_ker_sync_mutex_revive(THREAD *act, struct kerargs_sync_mutex_revive *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SYNC_MUTEX_REVIVE]) {
		uint32_t arg_arr[3];

		arg_arr[0] = (uint32_t) kap->sync;
		fill_8B_sync_t(act, kap->sync, arg_arr+1);
		_TRACE_IN_W_0PTR(__KER_SYNC_MUTEX_REVIVE, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SYNC_MUTEX_REVIVE,
		 (uint32_t) kap->sync,
		 _TRACE_DER_PTR(kap->sync, __owner)
		);
	}
}

int kdecl _trace_ker_sched_get(THREAD *act, struct kerargs_sched_get *kap)
{
	_TRACE_IN_F_1PTR(__KER_SCHED_GET, kap->pid, kap->tid, kap->param);
}
int kdecl _trace_ker_sched_set(THREAD *act, struct kerargs_sched_set *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SCHED_SET]) {
		uint32_t arg_arr[3 + _TRACE_ROUND_UP(sizeof(struct sched_param))];

		arg_arr[0] = kap->pid;
		arg_arr[1] = kap->tid;
		arg_arr[2] = kap->policy;

		fill_mem(act, (void*) (arg_arr+3), (void*) kap->param, sizeof(struct sched_param));
		_TRACE_IN_W_0PTR(__KER_SCHED_SET, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR
		(
		 __KER_SCHED_SET,
		 kap->pid,
		_TRACE_DER_PTR(kap->param, sched_priority)
		);
	}
}
int kdecl _trace_ker_sched_yield(THREAD *act, struct kerargs_null *kap)
{
	_TRACE_IN_F_0PTR(__KER_SCHED_YIELD, NULL, NULL);
}
int kdecl _trace_ker_sched_info(THREAD *act, struct kerargs_sched_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SCHED_INFO]) {
		_TRACE_IN_F_1PTR(__KER_SCHED_INFO, kap->pid, kap->policy, kap->info);
	} else {
		_TRACE_IN_F_0PTR(__KER_SCHED_INFO, kap->pid, kap->policy);
	}
}

int kdecl _trace_ker_net_cred(THREAD *act, struct kerargs_net_cred *kap)
{
	_TRACE_IN_F_1PTR(__KER_NET_CRED, kap->coid, (uint32_t) kap->info, kap->info);
}
int kdecl _trace_ker_net_vtid(THREAD *act, struct kerargs_net_vtid *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_NET_VTID]) {
		uint32_t arg_arr[2 + _TRACE_ROUND_UP(sizeof(struct _vtid_info))];

		arg_arr[0] = kap->vtid;
		arg_arr[1] = (uint32_t) kap->info;
		fill_mem(act, (void*) (arg_arr+2), (void*) kap->info, sizeof(struct _vtid_info));
		_TRACE_IN_W_0PTR(__KER_NET_VTID, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_IN_F_0PTR(__KER_NET_VTID, (uint32_t) kap->vtid, (uint32_t) kap->info);
	}
}
int kdecl _trace_ker_net_unblock(THREAD *act, struct kerargs_net_unblock *kap)
{
	_TRACE_IN_F_0PTR(__KER_NET_UNBLOCK, kap->vtid, NULL);
}
int kdecl _trace_ker_net_infoscoid(THREAD *act, struct kerargs_net_infoscoid *kap)
{
	_TRACE_IN_F_0PTR(__KER_NET_INFOSCOID, kap->scoid, kap->infoscoid);
}
int kdecl _trace_ker_net_signal_kill(THREAD *act, struct kerargs_net_signal_kill *kap)
{
	struct signal_info {
		unsigned nd;
		pid_t    pid;
		int32_t  tid;
		int32_t  signo;
		int32_t  code;
		int32_t  value;
	}* info=kap->sigdata;

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_NET_SIGNAL_KILL]) {
		uint32_t arg_arr[8];

		if(act->process!=net.prp) {
			arg_arr[0] = kap->cred->ruid;
			arg_arr[1] = kap->cred->euid;
			arg_arr[2] = info->nd;
			arg_arr[3] = info->pid;
			arg_arr[4] = info->tid;
			arg_arr[5] = info->signo;
			arg_arr[6] = info->code;
			arg_arr[7] = info->value;
		}
		_TRACE_IN_W_0PTR(__KER_NET_SIGNAL_KILL, arg_arr, NUM_ELTS(arg_arr));
	} else {
		if(act->process!=net.prp) {
			_TRACE_IN_F_0PTR(__KER_NET_SIGNAL_KILL, info->pid, info->signo);
		} else {
			_TRACE_IN_F_0PTR(__KER_NET_SIGNAL_KILL, NULL, NULL);
		}
	}
}

int kdecl _trace_ker_mt_ctl(THREAD *act, struct kerargs_mt_ctl *kap)
{
}

// Intercepting return values/"input arguments"
static void _trace_ex_ker_empty(THREAD *act, struct kerargs_null *kap)
{
	_TRACE_OUT_F(_TRACE_GETSYSCALL(act->syscall), NULL);
}
static void _trace_ex_ker_trace_event(THREAD *act, struct kerargs_trace_event *kap)
{
		_TRACE_OUT_F(__KER_TRACE_EVENT, NULL);
}
static void _trace_ex_ker_ring0(THREAD *act, struct kerargs_ring0 *kap)
{
	_TRACE_OUT_F(__KER_RING0, NULL);
}

static void _trace_ex_ker_sys_cpupage_get(THREAD *act, struct kerargs_sys_cpupage_get *kap)
{
	_TRACE_OUT_F(__KER_SYS_CPUPAGE_GET, NULL);
}
static void _trace_ex_ker_sys_cpupage_set(THREAD *act, struct kerargs_sys_cpupage_set *kap)
{
	_TRACE_OUT_F(__KER_SYS_CPUPAGE_SET, NULL);
}

static void _trace_ex_ker_msg_current(THREAD *act, struct kerargs_msg_current *kap)
{
	_TRACE_OUT_F(__KER_MSG_CURRENT, NULL);
}
static void _trace_ex_ker_msg_sendv(THREAD *act, struct kerargs_msg_sendv *kap)
{
	int n=_TRACE_GETSYSCALL(act->syscall);

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[n]) {
		uint32_t arg_arr[4];

		if(act->process!=net.prp) {
			if(_TRACE_TESTARGERR(act)) {
				fill_12B_IOV(act, kap->rmsg, kap->rparts, (arg_arr+1));
			}
			_TRACE_OUT_W(n, arg_arr, 4);
		} else {
			_TRACE_OUT_F(n, kap->rparts);
		}
	} else {
		if(act->process!=net.prp) {
			_TRACE_OUT_F(n, read_4B_IOV(act, kap->rmsg, kap->rparts));
		} else {
			_TRACE_OUT_F(n, kap->rparts);
		}
	}
}
static void _trace_ex_ker_msg_error(THREAD *act, struct kerargs_msg_error *kap)
{
	_TRACE_OUT_F(__KER_MSG_ERROR, NULL);
}
static void _trace_ex_ker_msg_receivev(THREAD *act, struct kerargs_msg_receivev *kap)
{
	int n=_TRACE_GETSYSCALL(act->syscall);

	if((trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG]&_TRACE_ENTER_COMM ||
	    trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) &&
	   act->syscall&_TRACE_COMM_IPC_EXIT) {
		comm_em(act, NULL, 0U, _NTO_TRACE_COMM_RMSG);
		SET_XFER_HANDLER(&xfer_trace_str_fault_handler);
	}

	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[n]) {
		uint32_t arg_arr[17];

		if(_TRACE_TESTARGERR(act)) {
			fill_12B_IOV(act, kap->rmsg, kap->rparts, (arg_arr+1));
			fill_52B_msg_info(act, kap->info, (arg_arr+4));
		}
		_TRACE_OUT_W(n, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(n, read_4B_IOV(act, kap->rmsg, kap->rparts));
	}
	SET_XFER_HANDLER(NULL);
}
static void _trace_ex_ker_msg_replyv(THREAD *act, struct kerargs_msg_replyv *kap)
{
	_TRACE_OUT_F(__KER_MSG_REPLYV, NULL);
}
static void _trace_ex_ker_msg_readv(THREAD *act, struct kerargs_msg_readv *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_READV]) {
		uint32_t arg_arr[4];

		if(_TRACE_TESTARGERR(act)) {
			fill_12B_IOV(act, kap->rmsg, kap->rparts, (arg_arr+1));
		}
		_TRACE_OUT_W(__KER_MSG_READV, arg_arr, 4);
	} else {
		_TRACE_OUT_F(__KER_MSG_READV, read_4B_IOV(act, kap->rmsg, kap->rparts));
	}
}
static void _trace_ex_ker_msg_writev(THREAD *act, struct kerargs_msg_writev *kap)
{
	_TRACE_OUT_F(__KER_MSG_WRITEV, NULL);
}
static void _trace_ex_ker_msg_readwritev(THREAD *act, struct kerargs_msg_readwritev *kap)
{
	_TRACE_OUT_F(__KER_MSG_READWRITEV, NULL);
}
static void _trace_ex_ker_msg_info(THREAD *act, struct kerargs_msg_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_INFO]) {
		uint32_t arg_arr[14];

		if(_TRACE_TESTARGERR(act)) {
			fill_52B_msg_info(act, kap->info, (arg_arr+1));
		}
		_TRACE_OUT_W(__KER_MSG_INFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_MSG_INFO, _TRACE_DER_PTR(kap->info, nd));
	}
}
static void _trace_ex_ker_msg_sendpulse(THREAD *act, struct kerargs_msg_sendpulse *kap)
{
	_TRACE_OUT_F(__KER_MSG_SEND_PULSE, NULL);
}
static void _trace_ex_ker_msg_deliver_event(THREAD *act, struct kerargs_msg_deliver_event *kap)
{
	_TRACE_OUT_F(__KER_MSG_DELIVER_EVENT, (uint32_t) kap->event);
}
static void _trace_ex_ker_msg_keydata(THREAD *act, struct kerargs_msg_keydata *kap)
{
	_TRACE_OUT_F(__KER_MSG_KEYDATA, kap->newkey?*kap->newkey:NULL);
}
static void _trace_ex_ker_msg_readiov(THREAD *act, struct kerargs_msg_readiov *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_MSG_READIOV]) {
		uint32_t arg_arr[4];

		if(_TRACE_TESTARGERR(act)) {
			fill_12B_IOV(act, kap->iov, kap->parts, (arg_arr+1));
		}
		_TRACE_OUT_W(__KER_MSG_READIOV, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_MSG_READIOV, read_4B_IOV(act, kap->iov, kap->parts));
	}
}
static void _trace_ex_ker_msg_verify_event(THREAD *act, struct kerargs_msg_verify_event *kap)
{
	_TRACE_OUT_F(__KER_MSG_VERIFY_EVENT, NULL);
}

static void _trace_ex_ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap)
{
	_TRACE_OUT_F(__KER_SIGNAL_KILL, NULL);
}
static void _trace_ex_ker_signal_return(THREAD *act, struct kerargs_signal_return *kap)
{
	_TRACE_OUT_F(__KER_SIGNAL_RETURN, NULL);
}
static void _trace_ex_ker_signal_fault(THREAD *act, struct kerargs_signal_fault *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_KILL]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(CPU_REGISTERS))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), kap->regs, sizeof(CPU_REGISTERS));
		}
		_TRACE_OUT_W(__KER_SIGNAL_FAULT, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_SIGNAL_FAULT, kap->regs?*(uint32_t*)kap->regs:NULL);
	}
}
static void _trace_ex_ker_signal_action(THREAD *act, struct kerargs_signal_action *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_ACTION]) {
		uint32_t arg_arr[5];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), kap->oact, (size_t) 16);
		}
		_TRACE_OUT_W(__KER_SIGNAL_ACTION, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_SIGNAL_ACTION, (uint32_t) _TRACE_DER_PTR(kap->oact, sa_handler));
	}
}
static void _trace_ex_ker_signal_procmask(THREAD *act, struct kerargs_signal_procmask *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_PROCMASK]) {
		uint32_t arg_arr[3];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), (void*) kap->old_sig_blocked, (size_t) 8);
		}
		_TRACE_OUT_W(__KER_SIGNAL_PROCMASK, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_SIGNAL_PROCMASK, _TRACE_DER_PTR(kap->old_sig_blocked, __bits[0]));
	}
}
static void _trace_ex_ker_signal_suspend(THREAD *act, struct kerargs_signal_suspend *kap)
{
	_TRACE_OUT_F(__KER_SIGNAL_SUSPEND, (uint32_t) kap->sig_blocked);
}
static void _trace_ex_ker_signal_waitinfo(THREAD *act, struct kerargs_signal_wait *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SIGNAL_WAITINFO]) {
		uint32_t arg_arr[11];

		////////////////////////////////////////////////////////////////////
		//
		//   FIX_ME!!
		//
		//   For some reasons there is an uncontrolled change of the address
		//   space that causes kernel crash.
		//
		////////////////////////////////////////////////////////////////////

		//fill_mem(act, (void*) (arg_arr+1), (void*) kap->sig_info, sizeof(siginfo_t));
		_TRACE_OUT_W(__KER_SIGNAL_WAITINFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		//_TRACE_OUT_F(__KER_SIGNAL_WAITINFO, _TRACE_DER_PTR(kap->sig_info, si_code));
		_TRACE_OUT_F(__KER_SIGNAL_WAITINFO, NULL);
	}
}

static void _trace_ex_ker_channel_create(THREAD *act, struct kerargs_channel_create *kap)
{
	_TRACE_OUT_F(__KER_CHANNEL_CREATE, NULL);
}
static void _trace_ex_ker_channel_destroy(THREAD *act, struct kerargs_channel_destroy *kap)
{
	_TRACE_OUT_F(__KER_CHANNEL_DESTROY, NULL);
}
static void _trace_ex_ker_channel_connect_attrs(THREAD *act, struct kerargs_channel_connect_attr *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CHANCON_ATTR]) {
		uint32_t arg_arr[2 + _TRACE_ROUND_UP(sizeof(union _channel_connect_attr))];

		arg_arr[0] = kap->id;
		arg_arr[1] = kap->flags;

		if( kap->old_attrs && _TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+2), (void*) kap->old_attrs, sizeof(union _channel_connect_attr));
		} else {
			memset((void*) (arg_arr+2), 0, sizeof(union _channel_connect_attr));
		}
		_TRACE_OUT_W(__KER_CHANCON_ATTR, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CHANCON_ATTR, NULL);
	}
}

static void _trace_ex_ker_connect_attach(THREAD *act, struct kerargs_connect_attach *kap)
{
	_TRACE_OUT_F(__KER_CONNECT_ATTACH, NULL);
}
static void _trace_ex_ker_connect_detach(THREAD *act, struct kerargs_connect_detach *kap)
{
	_TRACE_OUT_F(__KER_CONNECT_DETACH, NULL);
}
static void _trace_ex_ker_connect_server_info(THREAD *act, struct kerargs_connect_server_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CONNECT_SERVER_INFO]) {
		uint32_t arg_arr[14];

		if(_TRACE_TESTARGERR(act)) {
			fill_52B_msg_info(act, kap->info, (arg_arr+1));
		}
		_TRACE_OUT_W(__KER_CONNECT_SERVER_INFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CONNECT_SERVER_INFO, _TRACE_DER_PTR(kap->info, nd));
	}
}
static void _trace_ex_ker_connect_client_info(THREAD *act, struct kerargs_connect_client_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CONNECT_CLIENT_INFO]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _client_info))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), (void*) kap->info, sizeof(struct _client_info));
		}
		_TRACE_OUT_W(__KER_CONNECT_CLIENT_INFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CONNECT_CLIENT_INFO, _TRACE_DER_PTR(kap->info, nd));
	}
}
static void _trace_ex_ker_connect_flags(THREAD *act, struct kerargs_connect_flags *kap)
{
	_TRACE_OUT_F(__KER_CONNECT_FLAGS, NULL);
}

static void _trace_ex_ker_thread_create(THREAD *act, struct kerargs_thread_create *kap)
{
	_TRACE_OUT_F(__KER_THREAD_CREATE, SYNC_OWNER(act));
}
static void _trace_ex_ker_thread_destroy(THREAD *act, struct kerargs_thread_destroy *kap)
{
	_TRACE_OUT_F(__KER_THREAD_DESTROY, NULL);
}
static void _trace_ex_ker_thread_destroyall(THREAD *act, struct kerargs_null *kap)
{
	_TRACE_OUT_F(__KER_THREAD_DESTROYALL, NULL);
}
static void _trace_ex_ker_thread_detach(THREAD *act, struct kerargs_thread_detach *kap)
{
	_TRACE_OUT_F(__KER_THREAD_DETACH, NULL);
}
static void _trace_ex_ker_thread_join(THREAD *act, struct kerargs_thread_join *kap)
{
	_TRACE_OUT_F(__KER_THREAD_JOIN, kap->status?*(uint32_t*) kap->status:NULL);
}
static void _trace_ex_ker_thread_cancel(THREAD *act, struct kerargs_thread_cancel *kap)
{
	_TRACE_OUT_F(__KER_THREAD_CANCEL, NULL);
}
static void _trace_ex_ker_thread_ctl(THREAD *act, struct kerargs_thread_ctl *kap)
{
	_TRACE_OUT_F(__KER_THREAD_CTL, NULL);
}

static void _trace_ex_ker_interrupt_attach(THREAD *act, struct kerargs_interrupt_attach *kap)
{
	_TRACE_OUT_F(__KER_INTERRUPT_ATTACH, NULL);
}
static void _trace_ex_ker_interrupt_detach_func(THREAD *act, struct kerargs_interrupt_detach_func *kap)
{
	_TRACE_OUT_F(__KER_INTERRUPT_DETACH_FUNC, NULL);
}
static void _trace_ex_ker_interrupt_detach(THREAD *act, struct kerargs_interrupt_detach *kap)
{
	_TRACE_OUT_F(__KER_INTERRUPT_DETACH, NULL);
}
static void _trace_ex_ker_interrupt_wait(THREAD *act, struct kerargs_interrupt_wait *kap)
{
	_TRACE_OUT_F (__KER_INTERRUPT_WAIT, (uint32_t) kap->timeout);
}
static void _trace_ex_ker_interrupt_mask(THREAD *act, struct kerargs_interrupt_mask *kap)
{
	_TRACE_OUT_F(__KER_INTERRUPT_MASK, NULL);
}
static void _trace_ex_ker_interrupt_unmask(THREAD *act, struct kerargs_interrupt_unmask *kap)
{
	_TRACE_OUT_F(__KER_INTERRUPT_UNMASK, NULL);
}

static void _trace_ex_ker_clock_time(THREAD *act, struct kerargs_clock_time *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_TIME]) {
		uint32_t arg_arr[3];

		if(kap->old&&_TRACE_TESTARGERR(act)) {
				arg_arr[1] = _TRACE_SEC (*kap->old);
				arg_arr[2] = _TRACE_NSEC(*kap->old);
		} else {
				arg_arr[1] = NULL;
				arg_arr[2] = NULL;
		}
		_TRACE_OUT_W(__KER_CLOCK_TIME, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CLOCK_TIME, kap->old?_TRACE_SEC(*kap->old):NULL);
	}
}
static void _trace_ex_ker_clock_adjust(THREAD *act, struct kerargs_clock_adjust *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_ADJUST]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _clockadjust))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), (void*) kap->old, sizeof(struct _clockadjust));
		}
		_TRACE_OUT_W(__KER_CLOCK_ADJUST, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CLOCK_ADJUST, _TRACE_DER_PTR(kap->old,tick_count));
	}
}
static void _trace_ex_ker_clock_period(THREAD *act, struct kerargs_clock_period *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_CLOCK_PERIOD]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _clockperiod))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), (void*) kap->old, sizeof(struct _clockperiod));
		}
		_TRACE_OUT_W(__KER_CLOCK_PERIOD, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_CLOCK_PERIOD, _TRACE_DER_PTR(kap->old, nsec));
	}
}
static void _trace_ex_ker_clock_id(THREAD *act, struct kerargs_clock_id *kap)
{
	_TRACE_OUT_F(__KER_CLOCK_ID, NULL);
}

static void _trace_ex_ker_timer_create(THREAD *act, struct kerargs_timer_create *kap)
{
	_TRACE_OUT_F(__KER_TIMER_CREATE, NULL);
}
static void _trace_ex_ker_timer_destroy(THREAD *act, struct kerargs_timer_destroy *kap)
{
	_TRACE_OUT_F(__KER_TIMER_DESTROY, NULL);
}
static void _trace_ex_ker_timer_settime(THREAD *act, struct kerargs_timer_settime *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_SETTIME]) {
		uint32_t arg_arr[6];

		if(_TRACE_TESTARGERR(act)) {
			fill_16B_itimer(act, kap->oitime, arg_arr+1);
		}
		_TRACE_OUT_W(__KER_TIMER_SETTIME, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_TIMER_SETTIME, kap->oitime?_TRACE_SEC(kap->oitime->nsec):NULL);
	}
}
static void _trace_ex_ker_timer_info(THREAD *act, struct kerargs_timer_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_INFO]) {
		uint32_t arg_arr[1+_TRACE_ROUND_UP(sizeof(struct _timer_info))];

		if(kap->info&&_TRACE_TESTARGERR(act)) {
				fill_16B_itimer(act, &kap->info->itime, arg_arr+1);
				fill_16B_itimer(act, &kap->info->otime, arg_arr+5);
				(void) memcpy
				(
				 (void*) (arg_arr+9),
				 (void*) ((size_t)kap->info+(sizeof(uint32_t)*8)),
				 sizeof(struct _timer_info)-(sizeof(uint32_t)*8)
				);
		} else {
				(void) memset
				(
				 (void*) (arg_arr+1),
				 0,
				 sizeof(struct _timer_info)
				);
		}
		_TRACE_OUT_W(__KER_TIMER_INFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_TIMER_INFO, kap->info?_TRACE_SEC(kap->info->itime.nsec):NULL);
	}
}
static void _trace_ex_ker_timer_alarm(THREAD *act, struct kerargs_timer_alarm *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_ALARM]) {
		uint32_t arg_arr[5];

		if(_TRACE_TESTARGERR(act)) {
			fill_16B_itimer(act, kap->otime, arg_arr+1);
		}
		_TRACE_OUT_W(__KER_TIMER_ALARM, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_TIMER_ALARM, kap->otime?_TRACE_SEC(kap->otime->nsec):NULL);
	}
}
static void _trace_ex_ker_timer_timeout(THREAD *act, struct kerargs_timer_timeout *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_TIMER_TIMEOUT]) {
		uint32_t arg_arr[4 + _TRACE_ROUND_UP(sizeof(struct sigevent))];

		if(kap->otime&&_TRACE_TESTARGERR(act)) {
				arg_arr[1] = _TRACE_SEC (*kap->otime);
				arg_arr[2] = _TRACE_NSEC(*kap->otime);
		} else {
				arg_arr[1] = NULL;
				arg_arr[2] = NULL;
		}
		_TRACE_OUT_W(__KER_TIMER_TIMEOUT, arg_arr, 3);
	} else {
		_TRACE_OUT_F(__KER_TIMER_TIMEOUT, kap->otime?_TRACE_SEC(*kap->otime):NULL);
	}
}

static void _trace_ex_ker_sync_create(THREAD *act, struct kerargs_sync_create *kap)
{
	_TRACE_OUT_F(__KER_SYNC_CREATE, NULL);
}
static void _trace_ex_ker_sync_destroy(THREAD *act, struct kerargs_sync_destroy *kap)
{
	_TRACE_OUT_F(__KER_SYNC_DESTROY, NULL);
}
static void _trace_ex_ker_sync_mutex_lock(THREAD *act, struct kerargs_sync_mutex_lock *kap)
{
	_TRACE_OUT_F(__KER_SYNC_MUTEX_LOCK, NULL);
}
static void _trace_ex_ker_sync_mutex_unlock(THREAD *act, struct kerargs_sync_mutex_unlock *kap)
{
	_TRACE_OUT_F(__KER_SYNC_MUTEX_UNLOCK, NULL);
}
static void _trace_ex_ker_sync_condvar_wait(THREAD *act, struct kerargs_sync_condvar_wait *kap)
{
	_TRACE_OUT_F(__KER_SYNC_CONDVAR_WAIT, NULL);
}
static void _trace_ex_ker_sync_condvar_signal(THREAD *act, struct kerargs_sync_condvar_signal *kap)
{
	_TRACE_OUT_F(__KER_SYNC_CONDVAR_SIGNAL, NULL);
}
static void _trace_ex_ker_sync_sem_post(THREAD *act, struct kerargs_sync_sem_post *kap)
{
	_TRACE_OUT_F(__KER_SYNC_SEM_POST, NULL);
}
static void _trace_ex_ker_sync_sem_wait(THREAD *act, struct kerargs_sync_sem_wait *kap)
{
	_TRACE_OUT_F(__KER_SYNC_SEM_WAIT, NULL);
}
static void _trace_ex_ker_sync_ctl(THREAD *act, struct kerargs_sync_ctl *kap)
{
	_TRACE_OUT_F(__KER_SYNC_CTL, NULL);
}
static void _trace_ex_ker_sync_mutex_revive(THREAD *act, struct kerargs_sync_mutex_revive *kap)
{
	_TRACE_OUT_F(__KER_SYNC_MUTEX_REVIVE, NULL);
}

static void _trace_ex_ker_sched_get(THREAD *act, struct kerargs_sched_get *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SCHED_GET]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct sched_param))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), kap->param, sizeof(struct sched_param));
		}
		_TRACE_OUT_W(__KER_SCHED_GET, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_SCHED_GET, _TRACE_DER_PTR(kap->param, sched_priority));
	}
}
static void _trace_ex_ker_sched_set(THREAD *act, struct kerargs_sched_set *kap)
{
	_TRACE_OUT_F(__KER_SCHED_SET, NULL);
}
static void _trace_ex_ker_sched_yield(THREAD *act, struct kerargs_null *kap)
{
	_TRACE_OUT_F(__KER_SCHED_YIELD, NULL);
}
static void _trace_ex_ker_sched_info(THREAD *act, struct kerargs_sched_info *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_SCHED_INFO]) {
		uint32_t arg_arr[6];

		if(kap->info&&_TRACE_TESTARGERR(act)) {
				arg_arr[1] = kap->info->priority_min;
				arg_arr[2] = kap->info->priority_max;
				arg_arr[3] = _TRACE_SEC (kap->info->interval);
				arg_arr[4] = _TRACE_NSEC(kap->info->interval);
				arg_arr[5] = kap->info->priority_priv;
		} else {
				(void) memset((void*) arg_arr, 0, sizeof(arg_arr));
		}
		_TRACE_OUT_W(__KER_SCHED_INFO, arg_arr, NUM_ELTS(arg_arr));
	} else {
		if(kap->info&&_TRACE_TESTARGERR(act)) {
			_TRACE_OUT_F(__KER_SCHED_INFO, (uint32_t) kap->info->priority_max);
		} else {
			_TRACE_OUT_F(__KER_SCHED_INFO, (uint32_t) 0);
		}
	}
}

static void _trace_ex_ker_net_cred(THREAD *act, struct kerargs_net_cred *kap)
{
	if(_TRACE_CALL_ARG_WIDE&trace_masks.ker_call_masks[__KER_NET_CRED]) {
		uint32_t arg_arr[1 + _TRACE_ROUND_UP(sizeof(struct _client_info))];

		if(_TRACE_TESTARGERR(act)) {
			fill_mem(act, (void*) (arg_arr+1), kap->info, sizeof(struct _client_info));
		}
		_TRACE_OUT_W(__KER_NET_CRED, arg_arr, NUM_ELTS(arg_arr));
	} else {
		_TRACE_OUT_F(__KER_NET_CRED, _TRACE_DER_PTR(kap->info, nd));
	}
}
static void _trace_ex_ker_net_vtid(THREAD *act, struct kerargs_net_vtid *kap)
{
	_TRACE_OUT_F(__KER_NET_VTID, NULL);
}
static void _trace_ex_ker_net_unblock(THREAD *act, struct kerargs_net_unblock *kap)
{
	_TRACE_OUT_F(__KER_NET_UNBLOCK, NULL);
}
static void _trace_ex_ker_net_infoscoid(THREAD *act, struct kerargs_net_infoscoid *kap)
{
	_TRACE_OUT_F(__KER_NET_INFOSCOID, NULL);
}
static void _trace_ex_ker_net_signal_kill(THREAD *act, struct kerargs_net_signal_kill *kap)
{
	_TRACE_OUT_F(__KER_NET_SIGNAL_KILL, NULL);
}

static void _trace_ex_ker_mt_ctl(THREAD *act, struct kerargs_mt_ctl *kap)
{
        _TRACE_OUT_F(__KER_MT_CTL, NULL);
}

#define _trace_ex_ker_nop	_trace_ex_ker_empty
#define _trace_ex_ker_bad	_trace_ex_ker_empty

void (* const _trace_ex_ker_table[])() ={
	MK_KERTABLE(_trace_ex_ker)
};

void _trace_ker_exit(THREAD* act)
{
	uint32_t n=_TRACE_GETSYSCALL(act->syscall);

	if(_TRACE_GETSTATE(act->syscall)>1) {
		if(_TRACE_CHK_ENTRY(act, _TRACE_EXIT_CALL, n)) {
			if(_TRACE_GETARGSFLAG(act)) {
				specret_kernel();
				unlock_kernel();
				SET_XFER_HANDLER(&xfer_trace_str_fault_handler);
				(*_trace_ex_ker_table[n])(act, (void*) _TRACE_ARGPTR(act));
				SET_XFER_HANDLER(NULL);
				lock_kernel();
				unspecret_kernel();
			} else {
				(void) add_trace_event
				(
				 _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, _TRACE_KER_CALL_C, (2*_TRACE_MAX_KER_CALL_NUM)+n),
				 NULL,
				 NULL,
				 NULL
				);
			}
		}
		_TRACE_EXITCALL(act->syscall);
		act->syscall |= _TRACE_COMM_CALL_EXIT;
	} else if(n==__KER_MSG_RECEIVEV || n == __KER_MSG_RECEIVEPULSEV) {
		if((trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG]&_TRACE_ENTER_COMM ||
			trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) &&
		    !(act->syscall&_TRACE_COMM_CALL_EXIT)      &&
		    act->syscall&_TRACE_COMM_IPC_EXIT) {
			comm_em(act, NULL, 0U, ~_NTO_TRACE_COMM_RMSG);
		}
	}
 	if ( trace_force_flush ) {
 		(void) trace_flushbuffer();
 	}

	return;
}

#define _trace_ker_nop	_trace_ker_empty
#define _trace_ker_bad	_trace_ker_empty

int kdecl (* _trace_ker_call_table[])() ={
	MK_KERTABLE(_trace_ker)
};


int kdecl (* _trace_call_table[])() ={
	MK_KERTABLE(ker)
};

const int ker_call_entry_num = NUM_ELTS(_trace_call_table);

#endif

__SRCVERSION("ker_call_table.c $Rev: 173881 $");
