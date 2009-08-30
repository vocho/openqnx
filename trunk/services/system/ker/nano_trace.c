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

#include <stdarg.h>
#include "externs.h"

#if defined(VARIANT_instr)

#include <unistd.h>
#include <string.h>

// Platform dependent execution of the event/interrupt handler.
#if defined(__MIPS__)
#define _TRACE_EH_INVOKE(handler, area, cpus) \
({                                            \
	uint32_t r_v;                               \
	                                            \
	asm volatile (                              \
	              " .set noreorder  ;"          \
	              " move $28,%3     ;"          \
	              " move $25,%1     ;"          \
	              " jalr %1         ;"          \
	              " move $4,%2      ;"          \
	              " move %0,$2      ;"          \
	              " la   $28,_gp    ;"          \
	              " .set reorder    ;"          \
	              : "=r" (r_v)                  \
	              : "r" (handler), "r" (area), "r" ((uint32_t) cpus.gp) \
	              : "memory"                    \
	             );                             \
	r_v;                                        \
})
#elif defined(__PPC__)
#define _TRACE_EH_INVOKE(handler, area, cpus)      \
({                                                 \
	uint32_t r_v;                                    \
	                                                 \
	asm volatile (                                   \
	              " mtlr %1                       ;" \
	              " mr   %%r13,%3                 ;" \
	              " mr   %%r2,%4                  ;" \
	              " mr   %%r3,%2                  ;" \
	              " blrl                          ;" \
	              " lis  %%r13,_SDA_BASE_@ha      ;" \
	              " addi %%r13,%%r13,_SDA_BASE_@l ;" \
	              " lis  %%r2,_SDA2_BASE_@ha      ;" \
	              " addi %%r2,%%r2,_SDA2_BASE_@l  ;" \
	              " mr   %0,%%r3                  ;" \
	              : "=r" (r_v)                       \
	              : "r" (handler), "r" (area), "r" (cpus.gpr13), "r" (cpus.gpr2) \
	              : "memory"                         \
	             );                                  \
	r_v;                                             \
})
#else
#define _TRACE_EH_INVOKE(handler, area, cpus)  (*(handler))(area)
#endif

//Local macro definitions of conditional execution of the event handler
#define _TRACE_CKH_EXE_PT_EH(e1, e2, a1, a2, a3, a4, a5) \
	((!(e1) || exe_pt_event_h(e1, a1, a2, a3, a4, a5)) &&  \
	 (!(e2) || exe_pt_event_h(e2, a1, a2, a3, a4, a5)))

#define _TRACE_CKH_EXE_EH(e1, e2, a1, a2, a3) \
	((!(e1) || exe_event_h(e1, a1, a2, a3)) &&  \
	 (!(e2) || exe_event_h(e2, a1, a2, a3)))

#define _TRACE_CKH_EXE_PT_EHB(e1, e2, a1, a2, a3, a4, a5)    \
	((!(e1) || exe_pt_event_h_buff(e1, a1, a2, a3, a4, a5)) && \
	 (!(e2) || exe_pt_event_h_buff(e2, a1, a2, a3, a4, a5)))

#define _TRACE_CKH_EXE_EHB(e1, e2, a1, a2, a3)    \
	((!(e1) || exe_event_h_buff(1, e1, a1, a2, a3)) && \
	 (!(e2) || exe_event_h_buff(1, e2, a1, a2, a3)))

#define _TRACE_CKH_EXE_EHBNL(e1, e2, a1, a2, a3)    \
	((!(e1) || exe_event_h_buff(0, e1, a1, a2, a3)) && \
	 (!(e2) || exe_event_h_buff(0, e2, a1, a2, a3)))

#define _TRACE_CKH_EXE_EHNL(e1, e2, a1, a2, a3)      \
	((!(e1) || exe_event_h_no_lock(e1, a1, a2, a3)) && \
	 (!(e2) || exe_event_h_no_lock(e2, a1, a2, a3)))

//Local macro definition clears the buffer
#define _TRACE_CLR_BUFF(b_p) \
{ \
 	dropped_buffers++; \
 	dropped_events+=b_p->h.num_events; \
	b_p->h.tail_ptr   = b_p->h.begin_ptr; \
	b_p->h.num_events = 0U;               \
	b_p->h.flags      = 0U;               \
}

static int dropped_events;
static int dropped_buffers;

// Triggers pseudo-interrupt handler
int em_event(tracebuf_t* t_b_p)
{
	INTERRUPT* itp;

	if ( dropped_events != 0 ) {
		if(ker_verbose > 2) kprintf("INSTR: dropped %d events!\n", dropped_events );
		dropped_events = 0;
	}
	if ( dropped_buffers != 0 ) {
		if(ker_verbose > 2) kprintf("INSTR: dropped %d buffers!\n", dropped_buffers );
		dropped_buffers = 0;
	}
	if((itp = interrupt_level[HOOK_TO_LEVEL(_NTO_HOOK_TRACE)].queue)) {
		PROCESS* pprp;
		uint32_t r_vlocal;
		int      cpu =RUNCPU;
		THREAD*  thp =itp->thread;
		uint32_t locked=_TRACE_GET_LOCK();

		trace_force_flush = 0;

		lock_kernel();

		t_b_p->h.begin_ptr->data[2] = t_b_p->h.num_events;
		pprp  = aspaces_prp[cpu];
		if(thp->aspace_prp&&thp->aspace_prp!=pprp) {
			memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
		}

		// invoke pseudo interrupt handler
		// pass trace buffer and sequence number as first argument
		// ASSUMPTION: kernel trace buffers are contiguous
		if((r_vlocal = _TRACE_EH_INVOKE(
		                          (uint32_t (*)(int))itp->handler,
		                          t_b_p->h.ls13_rb_num|(t_b_p->h.seq_buff_num&0x7ff),
		                          itp->cpu)))
		{
			intrevent_add((const struct sigevent*) r_vlocal, thp, itp);
		}
		if(pprp&&pprp!=aspaces_prp[cpu]) {
			memmgr.aspace(pprp, &aspaces_prp[cpu]);
		}
		if(_TRACE_CHK_UNLOCK(locked)) unlock_kernel();

		return(EOK);
	}
	return(ENOTSUP);
}

// Executes an event handler with two data arguments and pid/tid
uint32_t exe_pt_event_h
(
	ehandler_data_t* stor_ptr,
	uint32_t         header,
	pid_t            pid,
	int              tid,
	uint32_t         d1,
	uint32_t         d2
)
{
	event_data_t* area=stor_ptr->area;
	int           l   =_TRACE_GET_LOCK();
	int           cpu =RUNCPU;
	THREAD*       thp =stor_ptr->thp;
	PROCESS*      pp  =aspaces_prp[cpu];
	uint32_t      r_vlocal;
	int           state;
	void*         save_tls;
	struct cpupage_entry* cpu_p=cpupageptr[cpu];

	lock_kernel();
	InterruptDisable();
	state        = cpu_p->state;
	cpu_p->state = state | 0x2;
	save_tls     = cpu_p->tls;
	cpu_p->tls   = &intr_tls;
	InterruptEnable();
	if(thp->aspace_prp&&thp->aspace_prp!=pp) {
		memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
	}

	area->header       = (__traceentry) header;
	area->el_num       = 2U;
	area->feature_mask = _NTO_TRACE_FMPID|_NTO_TRACE_FMTID;
	((uint32_t*) area->feature)[_NTO_TRACE_FIPID] = (uint32_t) pid;
	((uint32_t*) area->feature)[_NTO_TRACE_FITID] = (uint32_t) tid;
	((uint32_t*) area->data_array)[0] = d1;
	((uint32_t*) area->data_array)[1] = d2;

	r_vlocal = _TRACE_EH_INVOKE(stor_ptr->handler, area, stor_ptr->cpu);
	if(pp&&pp!=aspaces_prp[cpu]) {
		memmgr.aspace(pp, &aspaces_prp[cpu]);
	}
	InterruptDisable();
	cpu_p->state = state;
	cpu_p->tls   = save_tls;
	InterruptEnable();
	if(_TRACE_CHK_UNLOCK(l)) unlock_kernel();

	return (r_vlocal);
}

// Executes an event handler with two data arguments
uint32_t exe_event_h
(
	ehandler_data_t* stor_ptr,
	uint32_t         header,
	uint32_t         d1,
	uint32_t         d2
)
{
	event_data_t* area=stor_ptr->area;
	int           l   =_TRACE_GET_LOCK();
	int           cpu =RUNCPU;
	THREAD*       thp =stor_ptr->thp;
	PROCESS*      pp  =aspaces_prp[cpu];
	uint32_t      r_vlocal;
	int           state;
	void*         save_tls;
	struct cpupage_entry* cpu_p=cpupageptr[cpu];

	lock_kernel();
	InterruptDisable();
	state        = cpu_p->state;
	cpu_p->state = state | 0x2;
	save_tls     = cpu_p->tls;
	cpu_p->tls   = &intr_tls;
	InterruptEnable();
	if(thp->aspace_prp&&thp->aspace_prp!=pp) {
		memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
	}

	area->header       = (__traceentry) header;
	area->el_num       = 2U;
	area->feature_mask = _NTO_TRACE_FMEMPTY;
	((uint32_t*) area->data_array)[0] = d1;
	((uint32_t*) area->data_array)[1] = d2;

	r_vlocal = _TRACE_EH_INVOKE(stor_ptr->handler, area, stor_ptr->cpu);
	if(pp&&pp!=aspaces_prp[cpu]) {
		memmgr.aspace(pp, &aspaces_prp[cpu]);
	}
	InterruptDisable();
	cpu_p->state = state;
	cpu_p->tls   = save_tls;
	InterruptEnable();
	if(_TRACE_CHK_UNLOCK(l)) unlock_kernel();

	return (r_vlocal);
}

// Executes an event handler with buffer type arguments and pid/tid
uint32_t exe_pt_event_h_buff
(
	ehandler_data_t* stor_ptr,
	uint32_t         header,
	pid_t            pid,
	int              tid,
	void*            buff,
	uint32_t         size
)
{
	event_data_t* area=stor_ptr->area;
	int           l   =_TRACE_GET_LOCK();
	int           cpu =RUNCPU;
	THREAD*       thp =stor_ptr->thp;
	PROCESS*      pp  =aspaces_prp[cpu];
	uint32_t      r_vlocal;
	int           state;
	void*         save_tls;
	struct cpupage_entry* cpu_p=cpupageptr[cpu];

	lock_kernel();
	InterruptDisable();
	state        = cpu_p->state;
	cpu_p->state = state | 0x2;
	save_tls     = cpu_p->tls;
	cpu_p->tls   = &intr_tls;
	InterruptEnable();
	if(thp->aspace_prp&&thp->aspace_prp!=pp) {
		memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
	}

	area->header       = (__traceentry) header;
	area->el_num       = (uint32_t) _TRACE_ROUND_UP(size);
	area->feature_mask = _NTO_TRACE_FMPID|_NTO_TRACE_FMTID;
	((uint32_t*) area->feature)[_NTO_TRACE_FIPID] = (uint32_t) pid;
	((uint32_t*) area->feature)[_NTO_TRACE_FITID] = (uint32_t) tid;
	(void) memcpy((void*) area->data_array, buff, (size_t) size);

	r_vlocal = _TRACE_EH_INVOKE(stor_ptr->handler, area, stor_ptr->cpu);
	if(pp&&pp!=aspaces_prp[cpu]) {
		memmgr.aspace(pp, &aspaces_prp[cpu]);
	}
	InterruptDisable();
	cpu_p->state = state;
	cpu_p->tls   = save_tls;
	InterruptEnable();
	if(_TRACE_CHK_UNLOCK(l)) unlock_kernel();

	return (r_vlocal);
}

// Executes an event handler with buffer type arguments
uint32_t exe_event_h_buff
(
 	int				 locked,
	ehandler_data_t* stor_ptr,
	uint32_t         header,
	void*            buff,
	uint32_t         size
)
{
	event_data_t* area=stor_ptr->area;
	int           l = 0;
	int           cpu =RUNCPU;
	THREAD*       thp =stor_ptr->thp;
	PROCESS*      pp  =aspaces_prp[cpu];
	uint32_t      r_vlocal;
	int           state;
	void*         save_tls;
	struct cpupage_entry* cpu_p=cpupageptr[cpu];

	if ( locked ) {
 		l =_TRACE_GET_LOCK();
		lock_kernel();
	}
	InterruptDisable();
	state        = cpu_p->state;
	cpu_p->state = state | 0x2;
	save_tls     = cpu_p->tls;
	cpu_p->tls   = &intr_tls;
	InterruptEnable();
	if(thp->aspace_prp&&thp->aspace_prp!=pp) {
		memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
	}

	area->header       = (__traceentry) header;
	area->el_num       = (uint32_t) _TRACE_ROUND_UP(size);
	area->feature_mask = _NTO_TRACE_FMEMPTY;
	(void) memcpy((void*) area->data_array, buff, (size_t) size);

	r_vlocal = _TRACE_EH_INVOKE(stor_ptr->handler, area, stor_ptr->cpu);
	if(pp&&pp!=aspaces_prp[cpu]) {
		memmgr.aspace(pp, &aspaces_prp[cpu]);
	}
	InterruptDisable();
	cpu_p->state = state;
	cpu_p->tls   = save_tls;
	InterruptEnable();
	if(locked && _TRACE_CHK_UNLOCK(l)) unlock_kernel();

	return (r_vlocal);
}

// Executes an event handler with buffer type arguments and no locking
static uint32_t exe_event_h_no_lock
(
	ehandler_data_t* stor_ptr,
	uint32_t         header,
	uint32_t         d1,
	uint32_t         d2
)
{
	event_data_t* area=stor_ptr->area;
	int           cpu =RUNCPU;
	THREAD*       thp =stor_ptr->thp;
	PROCESS*      pp  =aspaces_prp[cpu];
	uint32_t      r_vlocal;
	int           state;
	void*         save_tls;
	struct cpupage_entry* cpu_p=cpupageptr[cpu];

	InterruptDisable();
	state        = cpu_p->state;
	cpu_p->state = state | 0x2;
	save_tls     = cpu_p->tls;
	cpu_p->tls   = &intr_tls;
	InterruptEnable();
	if(thp->aspace_prp&&thp->aspace_prp!=pp) {
		memmgr.aspace(thp->aspace_prp, &aspaces_prp[cpu]);
	}

	area->header       = (__traceentry) header;
	area->el_num       = 2U;
	area->feature_mask = _NTO_TRACE_FMEMPTY;
	((uint32_t*) area->data_array)[0] = d1;
	((uint32_t*) area->data_array)[1] = d2;

	r_vlocal = _TRACE_EH_INVOKE(stor_ptr->handler, area, stor_ptr->cpu);
	if(pp&&pp!=aspaces_prp[cpu]) {
		memmgr.aspace(pp, &aspaces_prp[cpu]);
	}
	InterruptDisable();
	cpu_p->state = state;
	cpu_p->tls   = save_tls;
	InterruptEnable();

	return (r_vlocal);
}

// Destroys event handler
void destroy_eh(PROCESS* prp)
{
	if (prp) {
		uint32_t i;

		InterruptLock(&trace_masks.eh_spin);
		for(i=0;trace_masks.eh_num&&i<_TRACE_MAX_EV_HANDLER_NUM;++i) {
			if(trace_masks.eh_storage[i].process==prp) {
				*trace_masks.eh_storage[i].location = NULL;
				trace_masks.eh_storage[i].location  = NULL;
				trace_masks.eh_storage[i].area      = NULL;
				trace_masks.eh_storage[i].thp       = NULL;
				trace_masks.eh_storage[i].process   = NULL;
				trace_masks.eh_storage[i].handler   = NULL;
				--trace_masks.eh_num;
			}
		}
		InterruptUnlock(&trace_masks.eh_spin);
	}

	return;
}

// Emits event when an APS partition is named. Assumes you dont want the event if name is null.
void aps_name_em(uint32_t partition_id, char *name)
{
	struct buf {
		uint32_t	partition_id;
		char		name[_TRACE_MAX_STR_SIZE];
	} buf;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
									 _TRACE_SYSTEM_C,
	                                 _NTO_TRACE_SYS_APS_NAME
	                                );
	if (!name || !(*name) ) return;
	buf.partition_id = partition_id;
	STRLCPY(buf.name, name, sizeof(buf.name));
	if(_TRACE_CKH_EXE_EHB(
							trace_masks.class_system_ehd_p,
							trace_masks.system_ehd_p[_NTO_TRACE_SYS_APS_NAME],
							header,
							&buf,
							sizeof(buf.partition_id)+strlen(buf.name)+1
						   )) {
			add_trace_d1_string(header,
							    buf.partition_id,
							    "%s",
							    buf.name);
	}

	return;
}


// Emits APS scheduler partition budget change events
void aps_budgets_em(uint32_t partition_id, uint32_t percentage_budget, uint32_t critical_budget )
{
	struct buf {
		uint32_t	partition_id;
		uint32_t	percentage_budget;
		uint32_t	critical_budget;
	} buf;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
									 _TRACE_SYSTEM_C,
	                                 _NTO_TRACE_SYS_APS_BUDGETS
	                                );
	buf.partition_id = partition_id;
	buf.percentage_budget = percentage_budget;
	buf.critical_budget = critical_budget;
	if(_TRACE_CKH_EXE_EHB(
								trace_masks.class_system_ehd_p,
								trace_masks.system_ehd_p[_NTO_TRACE_SYS_APS_BUDGETS],
								header,
								(void *)&buf,
								sizeof(buf)
							   )) {
		add_trace_buffer(header, (void *)&buf, _TRACE_ROUND_UP(sizeof(buf)) );
	}

	return;
}


// Emits APS scheduler bankruptcy event
void aps_bankruptcy_em(uint32_t partition_id, pid_t pid, int tid)
{
	struct buf {
		pid_t		pid;
		int			tid;
		uint32_t	partition_id;
	} buf;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
									 _TRACE_SYSTEM_C,
	                                 _NTO_TRACE_SYS_APS_BNKR
	                                );
	buf.pid = pid;
	buf.tid = tid+1;
	buf.partition_id = partition_id;
	if(_TRACE_CKH_EXE_PT_EHB(
							trace_masks.class_system_ehd_p,
							trace_masks.system_ehd_p[_NTO_TRACE_SYS_APS_BNKR],
							header,
							buf.pid,
							buf.tid,
							(void *)&buf,
							sizeof(buf)
						   )) {
		add_trace_buffer(header, (void *)&buf, _TRACE_ROUND_UP(sizeof(buf)) );
	}
	return;
}


// Emits time control events
void time_em(uint32_t msb, uint32_t lsb)
{
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_CONTROL_C,
	                                 _TRACE_CONTROL_TIME
	                                );
	if(_TRACE_CKH_EXE_EH(
	                     trace_masks.class_control_ehd_p,
	                     trace_masks.control_time_ehd_p,
	                     header,
	                     msb,
	                     lsb
	                    )) {
		(void) add_trace_event(header, NULL, msb, lsb);
	}

	return;
}

static int force_emit[_TRACE_MAX_SMP_CPU_NUM];


// Generates time control events
static uint32_t get_time_off(int init)
{
	static union {
		uint64_t timestamp;
		uint32_t bits[2];
	} ts[_TRACE_MAX_SMP_CPU_NUM];
	uint32_t lsb;
	int      cpu=RUNCPU, force;

	force = init || force_emit[cpu];
	force_emit[cpu] = 0;

#if (_TRACE_CLOCK_SWAP != 0)
	if ( init )
		memset( ts, 0, sizeof(ts) );
#endif
	lsb = ts[cpu].bits[_TRACE_CLOCK_LSB];
	ts[cpu].timestamp = ClockCycles();
#if defined(VARIANT_smp) && defined(VARIANT_instr)
	ts[cpu].timestamp -= clockcycles_offset[cpu];
#endif
	if(ts[cpu].bits[_TRACE_CLOCK_LSB] == lsb) {
		// Make sure the timestamps are unique
		ts[cpu].bits[_TRACE_CLOCK_LSB]++;
	}
	if ( force || ts[cpu].bits[_TRACE_CLOCK_LSB] < lsb) {
#if (_TRACE_CLOCK_SWAP != 0)
		static uint32_t msb[_TRACE_MAX_SMP_CPU_NUM];
		if ( init ) {
			memset( msb, 0, sizeof(msb) );
		}
		if ( ts[cpu].bits[_TRACE_CLOCK_LSB] < lsb) {
			++msb[cpu];
		}
		ts[cpu].bits[_TRACE_CLOCK_MSB] = msb[cpu];
#endif
		_TRACE_EMIT_CONTROL_TIME(
		                         ts[cpu].bits[_TRACE_CLOCK_MSB],
		                         ts[cpu].bits[_TRACE_CLOCK_LSB]
		                        );
	}

	return (ts[cpu].bits[_TRACE_CLOCK_LSB]);
}

int trace_flushbuffer(void)
{
	tracebuf_t* tracebuf= privateptr->tracebuf;

	if(tracebuf) {
		if(trace_masks.ring_mode) {
			if(!trace_masks.main_flags) {
				(void) em_event(tracebuf);
			} else {
				return (ECANCELED);
			}
		} else {
			InterruptDisable();
			SPINLOCK(&trace_masks.state_spin);
			SPINLOCK(&tracebuf->h.spin);
			if(!(tracebuf->h.flags&_TRACE_FLAGS_FLUSH)) {
				tracebuf_t *b_p = tracebuf->h.next;

				tracebuf->h.flags |= _TRACE_FLAGS_FLUSH;

				privateptr->tracebuf = b_p;

				SPINUNLOCK(&tracebuf->h.spin);
				SPINUNLOCK(&trace_masks.state_spin);

				memset( &force_emit, 0xff, sizeof(force_emit));
				(void) em_event( tracebuf );
			} else {
				SPINUNLOCK(&tracebuf->h.spin);
				SPINUNLOCK(&trace_masks.state_spin);
			}
			InterruptEnable();
		}

		return (EOK);
	} else {
		return (EFAULT);
	}
}

// Intercepts thread states
void th_em_st(THREAD* thp, uint32_t s, int cpu)
{
	int bufsize;
	CRASHCHECK(TYPE_MASK(thp->type) != TYPE_THREAD);
	if(_TRACE_THPTID(thp,s)) {
		struct buf {
			uint32_t	pid;
			uint32_t	tid;
			uint32_t	priority;
			uint32_t	policy;
			//remaining fields present only if and external scheduler is loaded
			uint32_t	partition_id;
			uint32_t	sched_flags; //to indicate if threads runs or is billed critical. see kermacros.h:: AP_SCHED_* flags.
		} buf;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 cpu,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_PR_TH_C,
		                                 s
		                                );
		buf.pid   =(uint32_t) thp->process->pid;
		buf.tid   =(uint32_t) thp->tid+1;

		if(_TRACE_THREAD_ARG_WIDE&trace_masks.th_mask) {
			buf.priority = thp->priority;
			buf.policy = thp->policy;
			if (scheduler_type != SCHEDULER_TYPE_DEFAULT) {
				buf.partition_id = thp->dpp->id;
				buf.sched_flags = thp->sched_flags;
				bufsize = sizeof(buf);
			} else {
				bufsize = offsetof(struct buf, partition_id);
			}
			if(_TRACE_CKH_EXE_PT_EHB(
									trace_masks.class_thread_ehd_p,
									trace_masks.thread_ehd_p[s],
									header,
									buf.pid,
									buf.tid,
									(void *)&buf,
									bufsize
								   )) {
				add_trace_buffer(header, (void *)&buf, _TRACE_ROUND_UP(bufsize) );
			}
		} else {
			if(_TRACE_CKH_EXE_PT_EH(
									trace_masks.class_thread_ehd_p,
									trace_masks.thread_ehd_p[s],
									header,
									buf.pid,
									buf.tid,
									buf.pid,
									buf.tid
								   )) {
				(void) add_trace_event(header, NULL, buf.pid, buf.tid);
			}
		}
	}

	return;
}

// Intercepts thread creation/destruction
void th_em_cd(THREAD* thp, uint32_t s, uint32_t c)
{
	CRASHCHECK(TYPE_MASK(thp->type) != TYPE_THREAD);
	if(_TRACE_THPTID(thp,s)) {
		uint32_t pid   =(uint32_t) thp->process->pid;
		uint32_t tid   =(uint32_t) thp->tid+1;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 RUNCPU,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_PR_TH_C,
		                                 c
		                                );

		if(_TRACE_CKH_EXE_PT_EH(
								trace_masks.class_thread_ehd_p,
								trace_masks.thread_ehd_p[s],
								header,
								pid,
								tid,
								pid,
								tid
							   )) {
			(void) add_trace_event(header, NULL, pid, tid);
		}
	}

	return;
}

// Intercepts process creation
void pr_em_create(PROCESS* prp, pid_t pid)
{
	uint32_t ppid  =(uint32_t) prp->parent?prp->parent->pid:0;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_PR_TH_C,
	                                 _TRACE_PR_TH_CREATE_P
	                               );

	if(_TRACE_CKH_EXE_EH(
	                     trace_masks.class_process_ehd_p,
	                     trace_masks.pr_create_ehd_p,
	                     header,
	                     ppid,
	                     pid
	                    )) {
		(void) add_trace_event(header, NULL, ppid, (uint32_t) pid);
	}

	return;
}

// Intercepts process destruction
void pr_em_destroy(PROCESS* prp, pid_t pid)
{
	uint32_t ppid  =(uint32_t) prp->parent?prp->parent->pid:0;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_PR_TH_C,
	                                 _TRACE_PR_TH_DESTROY_P
	                               );

	if(_TRACE_CKH_EXE_EH(
	                     trace_masks.class_process_ehd_p,
	                     trace_masks.pr_destroy_ehd_p,
	                     header,
	                     ppid,
	                     pid
	                    )) {
		(void) add_trace_event(header, NULL, ppid, (uint32_t) pid);
	}

	return;
}

// Intercepts "process name" creation
void pr_em_create_name(PROCESS* prp)
{
	struct {
		uint32_t ppid;
		uint32_t pid;
		char	 name[_TRACE_MAX_STR_SIZE];
	} buf;
	char*  str_p   =prp->debug_name?prp->debug_name:"";
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 NULL,
	                                 _TRACE_PR_TH_C,
	                                 _TRACE_PR_TH_CREATE_P_NAME
	                               );

	buf.ppid  =(uint32_t) prp->parent?prp->parent->pid:0;
	buf.pid = prp->pid;
	STRLCPY(buf.name, str_p, sizeof(buf.name));
	if(_TRACE_CKH_EXE_EHB(
	                      trace_masks.class_process_ehd_p,
	                      trace_masks.pr_create_name_ehd_p,
	                      header,
	                      &buf,
	                      sizeof(buf.ppid)+sizeof(buf.pid)+strlen(buf.name)+1
	                     )) {
		add_trace_d2_string
		(
		 header,
		 prp->parent?(uint32_t)prp->parent->pid:NULL,
		 (uint32_t) prp->pid,
		 "%s",
		 str_p
		);
	}

	return;
}

//Assumes that if you call this with a NULL thread name, you still want an event (ie for name reset)
void th_em_name(THREAD *thp)
{
	struct {
		uint32_t	pid;
		uint32_t	tid;
		char	 name[_TRACE_MAX_STR_SIZE];
	} buf;
	uint32_t header;

	header=_TRACE_MAKE_CODE( RUNCPU, NULL, _TRACE_PR_TH_C, _TRACE_PR_TH_NAME_T);
	buf.pid = thp->process->pid;
	buf.tid = thp->tid+1;
	if(thp->name) {
		STRLCPY(buf.name, thp->name, sizeof(buf.name));
	} else {
		buf.name[0] = '\0';
	}

	if(_TRACE_CKH_EXE_EHB(
	                      trace_masks.class_process_ehd_p,
	                      trace_masks.th_name_ehd_p,
	                      header,
	                      &buf,
	                      sizeof(buf.pid)+sizeof(buf.tid)+strlen(buf.name)+1
	                     )) {
		add_trace_d2_string
		(
		 header,
		 buf.pid,
		 buf.tid,
		 "%s",
		 buf.name
		);
	}
}

// Intercepts "process name" destruction
void pr_em_destroy_name(PROCESS* prp)
{
	struct {
		uint32_t ppid;
		uint32_t pid;
		char	 name[_TRACE_MAX_STR_SIZE];
	} buf;
	char*  str_p   =prp->debug_name?prp->debug_name:"";
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 NULL,
	                                 _TRACE_PR_TH_C,
	                                 _TRACE_PR_TH_DESTROY_P_NAME
	                               );

	buf.ppid  =(uint32_t) prp->parent?prp->parent->pid:0;
	buf.pid = prp->pid;
	STRLCPY(buf.name, str_p, sizeof(buf.name));
	if(_TRACE_CKH_EXE_EHB(
	                      trace_masks.class_process_ehd_p,
	                      trace_masks.pr_destroy_name_ehd_p,
	                      header,
	                      &buf,
	                      sizeof(buf.ppid)+sizeof(buf.pid)+strlen(buf.name)+1
	                     )) {
		add_trace_d2_string
		(
		 header,
		 prp->parent?(uint32_t)prp->parent->pid:NULL,
		 (uint32_t) prp->pid,
		 "%s",
		 str_p
		);
	}

	return;
}

// Intercepts vthread creation/destruction
void vth_em_cd(THREAD* vthp, uint32_t s, uint32_t c)
{
	CRASHCHECK(TYPE_MASK(vthp->type) != TYPE_VTHREAD);
	if(_TRACE_VTHPTID(vthp,s)) {
		uint32_t pid   =(uint32_t) vthp->process->pid;
		uint32_t tid   =(uint32_t) vthp->tid+1;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 RUNCPU,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_PR_TH_C,
		                                 c
		                                );

		if(_TRACE_CKH_EXE_PT_EH(
		                        trace_masks.class_vthread_ehd_p,
		                        trace_masks.vthread_ehd_p[s],
		                        header,
		                        pid,
		                        tid,
		                        pid,
		                        tid
		                       )) {
			(void) add_trace_event(header, NULL, pid, tid);
		}
	}

	return;
}

// Intercepts vthread states
void vth_em_st(THREAD* vthp, uint32_t s)
{
	CRASHCHECK(TYPE_MASK(vthp->type) != TYPE_VTHREAD);
	if(_TRACE_VTHPTID(vthp,s)){
		uint32_t pid   =(uint32_t) vthp->process->pid;
		uint32_t tid   =(uint32_t) vthp->tid+1;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 RUNCPU,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_PR_TH_C,
		                                 _TRACE_MAX_TH_STATE_NUM+s
		                                );

		if(_TRACE_CKH_EXE_PT_EH(
		                        trace_masks.class_vthread_ehd_p,
		                        trace_masks.vthread_ehd_p[s],
		                        header,
		                        pid,
		                        tid,
		                        pid,
		                        tid
		                       )) {
			(void) add_trace_event(header, NULL, pid, tid);
		}
	}

	return;
}

// Intercepts communication info of "exe events"
void comm_exe_em(CONNECT* cop, uint32_t srcvid, uint32_t ev)
{
	uint32_t pid;
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_COMM_C,
	                                 ev
	                                );

	switch(ev) {
		case _NTO_TRACE_COMM_SPULSE_EXE:
		case _NTO_TRACE_COMM_SPULSE_DIS:
		case _NTO_TRACE_COMM_SPULSE_DEA:
		case _NTO_TRACE_COMM_SPULSE_UN:
		{
			if(cop->channel==net.chp) {
				pid = 0U;
			} else if((cop->channel != NULL) && (cop->channel->process != NULL)) {
				pid = cop->channel->process->pid;
			} else {
				pid = ~0;
			}
			break;
		}
		case _NTO_TRACE_COMM_SPULSE_QUN:
		{
			pid     = 0U;
			break;
		}
		default:
			return;
	}

	if((!trace_masks.comm_pid[ev] || trace_masks.comm_pid[ev] == pid) &&
			_TRACE_CKH_EXE_EH(
	                     trace_masks.class_comm_ehd_p,
	                     trace_masks.comm_ehd_p[ev],
	                     header,
	                     srcvid,
	                     pid
	                    )) {
		(void) add_trace_event(header, NULL, srcvid, pid);
	}

	return;
}

static void xfer_trace_fault_h(THREAD* act, CPU_REGISTERS* regs, unsigned flags) {
	if (_TRACE_GETSYSCALL(act->syscall)==__KER_MSG_RECEIVEPULSEV) {
		(void) add_trace_event
		(
		 _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, _TRACE_COMM_C, _NTO_TRACE_COMM_RPULSE),
		 NULL,
		 (~0),
		 (~0)
		);
	} else {
		(void) add_trace_event
		(
		 _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, _TRACE_COMM_C, _NTO_TRACE_COMM_RMSG),
		 NULL,
		 (~0),
		 (~0)
		);
	}
	act->syscall &= ~_TRACE_COMM_IPC_EXIT;
	act->syscall |= _TRACE_COMM_CALL_EXIT;

	return;
}

const static struct fault_handlers xfer_trace_fault_handler = {
	xfer_trace_fault_h, NULL
};

// Intercepts communication info of "ipc events"
void comm_em(THREAD* thp, CONNECT* cop, uint32_t srcvid, uint32_t ev)
{
	if(_TRACE_IPCPTID(thp, ev)) {
		uint32_t pid;
		uint32_t header;

		switch(ev) {
			case _NTO_TRACE_COMM_SMSG:
			case _NTO_TRACE_COMM_SPULSE:
			{
				if(cop->channel==net.chp) {
					pid = 0U;
				} else if((cop->channel != NULL) && (cop->channel->process != NULL)) {
					pid = cop->channel->process->pid;
				} else {
					pid = ~0;
				}
				break;
			}
			case _NTO_TRACE_COMM_REPLY:
			case _NTO_TRACE_COMM_ERROR:
			{
				pid = thp->process->pid;
				break;
			}
			case _NTO_TRACE_COMM_RMSG:
			{
				if(!(thp->flags&_NTO_TF_KERERR_SET)) {
					pid = (uint32_t) thp->process->pid;
					if((srcvid=KSTATUS(thp))==0U) {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) )
							return;
						SET_XFER_HANDLER(&xfer_trace_fault_handler);
						ev = _NTO_TRACE_COMM_RPULSE;
						if(((struct kerargs_msg_receivev*) _TRACE_ARGPTR(thp))->rparts < 0) {
							srcvid = ((struct _pulse*) ((struct kerargs_msg_receivev*)
							         _TRACE_ARGPTR(thp))->rmsg)->scoid;
						} else {
							srcvid = ((struct _pulse*) GETIOVBASE(((struct kerargs_msg_receivev*)
							         _TRACE_ARGPTR(thp))->rmsg))->scoid;
						}
						SET_XFER_HANDLER(NULL);
					}
					else {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG]&_TRACE_ENTER_COMM) )
							return;
					}
				} else {
					if (_TRACE_GETSYSCALL(thp->syscall)==__KER_MSG_RECEIVEV) {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG]&_TRACE_ENTER_COMM) )
							return;
						ev = _NTO_TRACE_COMM_RMSG;
					} else {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) )
							return;
						ev = _NTO_TRACE_COMM_RPULSE;
					}
					pid    = (~0);
					srcvid = (~0);
				}
				break;
			}
			case (unsigned)~_NTO_TRACE_COMM_RMSG:
			case (unsigned)~_NTO_TRACE_COMM_RPULSE:
			if(_TRACE_IPCPTID(thp, ~ev))
			{
				if(!(thp->flags&_NTO_TF_KERERR_SET)&&_TRACE_GETARGSFLAG(thp)) {
					pid = (uint32_t) thp->process->pid;
					if ((srcvid=KSTATUS(thp))==0U) {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RPULSE]&_TRACE_ENTER_COMM) )
							return;
						specret_kernel();
						unlock_kernel();
						SET_XFER_HANDLER(&xfer_trace_fault_handler);
						ev     = _NTO_TRACE_COMM_RPULSE;
						if(((struct kerargs_msg_receivev*) _TRACE_ARGPTR(thp))->rparts < 0) {
							srcvid = ((struct _pulse*) ((struct kerargs_msg_receivev*)
							         _TRACE_ARGPTR(thp))->rmsg)->scoid;
						} else {
							srcvid = ((struct _pulse*) GETIOVBASE(((struct kerargs_msg_receivev*)
							         _TRACE_ARGPTR(thp))->rmsg))->scoid;
						}
						SET_XFER_HANDLER(NULL);
						lock_kernel();
						unspecret_kernel();
					} else {
						if ( !(trace_masks.comm_mask[_NTO_TRACE_COMM_RMSG]&_TRACE_ENTER_COMM) )
							return;
						ev = ~ev;
					}
				} else {
					pid    = (~0);
					srcvid = (~0);
				}
				thp->syscall &= ~_TRACE_COMM_IPC_EXIT;
				thp->syscall |= _TRACE_COMM_CALL_EXIT;
				break;
			}
			/* fall through */
			default:
				return;
		}

		header = _TRACE_MAKE_CODE(
		                          RUNCPU,
		                          _TRACE_STRUCT_S,
		                          _TRACE_COMM_C,
		                          ev
		                         );
		if(_TRACE_CKH_EXE_PT_EH(
		                        trace_masks.class_comm_ehd_p,
		                        trace_masks.comm_ehd_p[ev],
		                        header,
		                        thp->process->pid,
		                        thp->tid+1,
		                        srcvid,
		                        pid
		                       )) {
			(void) add_trace_event(header, NULL, srcvid, pid);
		}
	}

	return;
}

// Intercepts "signals"
void comm_em_signal(THREAD* thp, siginfo_t* s_i)
{
	uint32_t header=_TRACE_MAKE_CODE(
	                                 RUNCPU,
	                                 _TRACE_STRUCT_S,
	                                 _TRACE_COMM_C,
	                                 _NTO_TRACE_COMM_SIGNAL
	                                );

	if(_TRACE_IPCPTID(thp, _NTO_TRACE_COMM_SIGNAL)) {
		if(_TRACE_COMM_ARG_WIDE&trace_masks.comm_mask[_NTO_TRACE_COMM_SIGNAL]) {
			uint32_t arg_arr[10];

			arg_arr[0] = s_i->si_signo;
			arg_arr[1] = s_i->si_code;
			arg_arr[2] = s_i->si_errno;
			arg_arr[3] = s_i->__data.__pad[0];
			arg_arr[4] = s_i->__data.__pad[1];
			arg_arr[5] = s_i->__data.__pad[2];
			arg_arr[6] = s_i->__data.__pad[3];
			arg_arr[7] = s_i->__data.__pad[4];
			arg_arr[8] = s_i->__data.__pad[5];
			arg_arr[9] = s_i->__data.__pad[6];

			if(_TRACE_CKH_EXE_PT_EHB(
			                         trace_masks.class_comm_ehd_p,
			                         trace_masks.comm_ehd_p[_NTO_TRACE_COMM_SIGNAL],
			                         header,
			                         thp->process->pid,
			                         thp->tid+1,
			                         (void*) arg_arr,
			                         sizeof(uint32_t)*NUM_ELTS(arg_arr)
			                        )) {
				add_trace_buffer(header, arg_arr, NUM_ELTS(arg_arr));
			}
		} else {
			if(_TRACE_CKH_EXE_PT_EH(
			                        trace_masks.class_comm_ehd_p,
			                        trace_masks.comm_ehd_p[_NTO_TRACE_COMM_SIGNAL],
			                        header,
			                        thp->process->pid,
			                        thp->tid+1,
			                        s_i->si_signo,
			                        s_i->si_code
			                       )) {
				(void) add_trace_event(header, NULL, s_i->si_signo, s_i->si_code);
			}
		}
	}

	return;
}

// Intercepts interrupt entries
void add_ktrace_int_handler_enter(INTRLEVEL* lp, INTERRUPT *isr)
{
  uint32_t l=_TRACE_INT_LEV(lp);

	if(_TRACE_ENTER_INT&trace_masks.int_handler_masks[l]) {
		struct _buf {
			uint32_t pid;
			uint32_t i_n;
			uint32_t ip;
			uint32_t area;
		} buf;
		int      cpu   =RUNCPU;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 cpu,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_INT_C,
		                                 _TRACE_INT_HANDLER_ENTRY
		                                );

		buf.pid  = (uint32_t)isr->thread->process->pid;
		buf.i_n   = _TRACE_INT_NUM(lp);
		buf.ip    = (uint32_t)isr->handler;
		buf.area  = (uint32_t)isr->area;

		if(_TRACE_CKH_EXE_EHBNL(
		                       trace_masks.class_int_handler_enter_ehd_p,
		                       trace_masks.int_handler_enter_ehd_p[l],
		                       header,
		                       &buf,
		                       sizeof(buf)
		                      )) {
			add_trace_buffer(header, (uint32_t *)&buf, sizeof(buf)/sizeof(uint32_t) );
		}
	}

	return;
}

// Intercepts interrupt exits
void add_ktrace_int_handler_exit(INTRLEVEL* lp, const struct sigevent *ev)
{
	uint32_t l=_TRACE_INT_LEV(lp);

	if(_TRACE_EXIT_INT&trace_masks.int_handler_masks[l]) {
		struct _buf {
			uint32_t i_n;
			struct sigevent ev;
		} buf;
		int      cpu   =RUNCPU;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 cpu,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_INT_C,
		                                 _TRACE_INT_HANDLER_EXIT
		                                );

		buf.i_n   =_TRACE_INT_NUM(lp);
		if ( ev != NULL )
			memcpy( &buf.ev, ev, sizeof(*ev));
		else
			memset( &buf.ev, 0, sizeof(buf.ev));
		if(_TRACE_CKH_EXE_EHBNL(
		                       trace_masks.class_int_handler_exit_ehd_p,
		                       trace_masks.int_handler_exit_ehd_p[l],
		                       header,
		                       &buf,
		                       sizeof(buf)
		                      )) {
			add_trace_buffer(header, (uint32_t *)&buf, sizeof(buf)/sizeof(uint32_t) );
		}
	}

	return;
}

// Intercepts interrupt entries
void add_ktrace_int_enter(INTRLEVEL* lp)
{
  uint32_t l=_TRACE_INT_LEV(lp);

	if(_TRACE_ENTER_INT&trace_masks.int_masks[l]) {
		uint32_t i_n   =_TRACE_INT_NUM(lp);
		int      cpu   =RUNCPU;
		uint32_t ip    =REGIP(&actives[cpu]->reg);
		uint32_t header=_TRACE_MAKE_CODE(
		                                 cpu,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_INT_C,
		                                 _TRACE_INT_ENTRY
		                                );

		if(_TRACE_CKH_EXE_EHNL(
		                       trace_masks.class_int_enter_ehd_p,
		                       trace_masks.int_enter_ehd_p[l],
		                       header,
		                       i_n,
		                       ip
		                      )) {
			(void) add_trace_event(header, NULL, i_n, ip);
		}
	}

	return;
}

// Intercepts interrupt exits
void add_ktrace_int_exit(INTRLEVEL* lp)
{
	uint32_t l=_TRACE_INT_LEV(lp);

	if(_TRACE_EXIT_INT&trace_masks.int_masks[l]) {
		uint32_t i_n   =_TRACE_INT_NUM(lp);
		uint32_t ink   =get_inkernel();
		int      cpu   =RUNCPU;
		uint32_t header=_TRACE_MAKE_CODE(
		                                 cpu,
		                                 _TRACE_STRUCT_S,
		                                 _TRACE_INT_C,
		                                 _TRACE_INT_EXIT
		                                );

		if(_TRACE_CKH_EXE_EHNL(
		                       trace_masks.class_int_exit_ehd_p,
		                       trace_masks.int_exit_ehd_p[l],
		                       header,
		                       i_n,
		                       ink
		                      )) {
			(void) add_trace_event(header, NULL, i_n, ink);
		}
	}

	return;
}

/* Yuck - we should really have a per-cpu buffer system, but hey... */
int add_trace_event(uint32_t header, uint32_t d_0, uint32_t d_1, uint32_t d_2)
{
	tracebuf_t		*t_b_p;
	traceevent_t	*e_p;
	int				skip, retry = 2, want_emit = 0, want_next = 0;

	/*
	 * This needs to be out here, before we lock anything down, since it
	 * may actually call add_trace_event itself!
	 */
	if(!_TRACE_GET_STRUCT(header)) {
		d_0 = get_time_off(0);
	}

   	//InterruptDisable();
begin:
   	InterruptDisable();
	/*
	 * Until we've decided which buffer we're going to use, we can't let anyone
	 * mess with the global buffer pointer
	 */
	SPINLOCK(&trace_masks.state_spin);
	skip = trace_masks.skip_buff;

	if(trace_masks.main_flags==0) {
		/* We're not actually tracing, bail out */
		SPINUNLOCK(&trace_masks.state_spin);
		InterruptEnable();
		return EOK;
	}
	t_b_p=privateptr->tracebuf;
	if(t_b_p==NULL) {
		/* No Trace Buffer allocated */
		SPINUNLOCK(&trace_masks.state_spin);
		InterruptEnable();
		return ECANCELED;
	}
	SPINLOCK(&t_b_p->h.spin);

	/* If the current trace buffer is already scheduled for flushing... */
	if(t_b_p->h.flags&_TRACE_FLAGS_FLUSH) {
		/* if we have retries available or we have specified via skip_buff, then ... */
		if( retry && ((t_b_p->h.flags&_TRACE_FLAGS_WRITING) || skip) ) {
			skip = 0;
			/* skip to next buffer */
			privateptr->tracebuf = t_b_p->h.next;
			SPINUNLOCK(&t_b_p->h.spin);
			SPINUNLOCK(&trace_masks.state_spin);
			InterruptEnable();
			retry--;
			goto begin;
		} else {
			if( t_b_p->h.flags&_TRACE_FLAGS_WRITING ) {
				SPINUNLOCK(&trace_masks.state_spin);
				SPINUNLOCK(&t_b_p->h.spin);
				dropped_events++;
				InterruptEnable();
				return EOK;
			}
			/* Drop that buffer! */
			_TRACE_CLR_BUFF(t_b_p);
			SPINUNLOCK(&trace_masks.state_spin);
		}
	}
	else {
		/* We've got our man! */
		SPINUNLOCK(&trace_masks.state_spin);
	}

	/* If this is the first event of the buffer, intialize with buffer event */
	if(t_b_p->h.num_events == 0) {
		t_b_p->h.num_events++;
		e_p = (traceevent_t *)t_b_p->h.tail_ptr;
		t_b_p->h.seq_buff_num = ++(trace_masks.send_buff_num);
		t_b_p->h.tail_ptr++;
		e_p->header  = _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, _TRACE_CONTROL_C, _TRACE_CONTROL_BUFFER);
		e_p->data[0] = d_0;
		e_p->data[1] = t_b_p->h.seq_buff_num;
		e_p->data[2] = 0;
	}
	if(t_b_p->h.num_events < _TRACELEMENTS) {
		/* ok, we have room in the buffer to add the event */
		t_b_p->h.num_events++;
		e_p = (traceevent_t *)t_b_p->h.tail_ptr;
		t_b_p->h.tail_ptr++;
		e_p->header  = header;
		e_p->data[0] = d_0;
		e_p->data[1] = d_1;
		e_p->data[2] = d_2;

		if(t_b_p->h.num_events >= trace_masks.max_events) {
			/* If we're getting low on space... */
			if(trace_masks.ring_mode) {
				want_next = 1;
				/* Nada? */
			} else {
				if ( (get_inkernel() & INKERNEL_INTRMASK) == 0 ) {
					want_next = 1;
					/* Schedule a flush */
					want_emit = 1;
				} else {
					trace_force_flush = 1;
					ker_exit_enable_mask = 0xffffffffUL;
				}
			}
		}
	} else {
		dropped_events++;
	}
	SPINUNLOCK(&t_b_p->h.spin);
	InterruptEnable();

	/*
	 * At this point we're done logging the event, but
	 * we still need to check to see if we need to emit this buffer,
	 * or simply move to the next buffer in ring mode.
	 */
	if ( want_next ) {
		InterruptDisable();
		SPINLOCK(&trace_masks.state_spin);
		if ( t_b_p == privateptr->tracebuf ) {
			tracebuf_t *b_p = t_b_p->h.next;
			SPINLOCK(&t_b_p->h.spin);
			if ( trace_masks.ring_mode ) {
				SPINLOCK(&b_p->h.spin);
				b_p->h.tail_ptr = b_p->h.begin_ptr;
				b_p->h.num_events = 0;
				SPINUNLOCK(&b_p->h.spin);
			}
			else if ( want_emit ) {
				t_b_p->h.flags |= _TRACE_FLAGS_FLUSH;
			}
			SPINUNLOCK(&t_b_p->h.spin);
			privateptr->tracebuf = b_p;
			SPINUNLOCK(&trace_masks.state_spin);
			memset( &force_emit, 0xff, sizeof(force_emit));
			if ( want_emit ) {
				(void) em_event( t_b_p );
			}
		} else {
			/* Someone else moved it? WTF? Assume that they flushed it too! */
			SPINUNLOCK(&trace_masks.state_spin);
		}
		InterruptEnable();
	}
	return (EOK);
}

// Adds buffer in the form of events.
void add_trace_buffer(uint32_t header, uint32_t* b_p, unsigned len)
{
	uint32_t  t_s = get_time_off(0);
	int       i;

	if(len > (_TRACEDATANUM*2)) {
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_CB);
		(void) add_trace_event(header, t_s, b_p[0], b_p[1]);
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_CC);
		for(i=_TRACEDATANUM, len-=_TRACEDATANUM; i<len; i+=_TRACEDATANUM) {
			(void) add_trace_event(header, t_s, b_p[i], b_p[i+1]);
		}
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_CE);
		if((len+_TRACEDATANUM) > (i+1)) {
			(void) add_trace_event(header, t_s, b_p[i], b_p[i+1]);
		} else {
			(void) add_trace_event(header, t_s, b_p[i], NULL);
		}
	} else if(len > _TRACEDATANUM) {
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_CB);
		(void) add_trace_event(header, t_s, b_p[0], b_p[1]);
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_CE);
		if(len == (_TRACEDATANUM+1)) {
			(void) add_trace_event(header, t_s, b_p[2], NULL);
		} else {
			(void) add_trace_event(header, t_s, b_p[2], b_p[3]);
		}
	} else if(len > 0) {
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_S);
		if(len == _TRACEDATANUM) {
			(void) add_trace_event(header, t_s, b_p[0], b_p[1]);
		} else {
			(void) add_trace_event(header, t_s, b_p[0], NULL);
		}
	} else {
		_TRACE_SET_STRUCT(header, _TRACE_STRUCT_S);
		(void) add_trace_event(header, t_s, NULL, NULL);
	}

	return;
}

// Initilizes the "trace state" structure ("stream")
void add_trace_b_init(trace_state_t* st_p)
{
	memset(st_p, 0, sizeof(trace_state_t));
	st_p->time_off = get_time_off(0);
}

// Ends the writting process ("stream")
void add_trace_b_end(trace_state_t* st_p, uint32_t header)
{
	//If no event has been emitted, and not data added, do nothing
	if(st_p->data_emitted == 0 && st_p->di == 0) {
		return;
	}

	//If no data has been emitted, but we have data, simple event
	//If we have emitted data then this is a terminating event
	_TRACE_SET_STRUCT(header, (st_p->data_emitted) ? _TRACE_STRUCT_CE : _TRACE_STRUCT_S);
	(void) add_trace_event(header, st_p->time_off, st_p->data.idata[0], st_p->data.idata[1]);
}

// Adds buffers as an event "stream"
void add_trace_b_add(trace_state_t* st_p, uint32_t header, char* b_p, unsigned len)
{
	unsigned bufamt, bi;
	unsigned dataamt, amt;

	bi = 0;

	while(bi < len) {
		//We want to put more data out, and we have a full buffer.
		//This means we are either the begin or continuation, but not simple
		if(st_p->di == sizeof(st_p->data)) {
			_TRACE_SET_STRUCT(header, (st_p->data_emitted) ? _TRACE_STRUCT_CC : _TRACE_STRUCT_CB);
			(void) add_trace_event(header, st_p->time_off, st_p->data.idata[0], st_p->data.idata[1]);

			//Set-up for the next event that is going out
			st_p->data_emitted = 1;
			st_p->di = 0;
        	memset(st_p->data.cdata, 0, sizeof(st_p->data));
		}

		//Determine how much data we can put in, and copy to a local buffer
		bufamt = len - bi;
		dataamt = sizeof(st_p->data) - st_p->di;
		amt = (bufamt <= dataamt) ? bufamt : dataamt;

		memcpy(st_p->data.cdata + st_p->di, b_p + bi, amt);
		bi += amt;
		st_p->di += amt;
    }
}

void add_trace_iovs(uint32_t header, IOV *iovs, unsigned iovlen)
{
	trace_state_t state;
	int 		  i;

	add_trace_b_init(&state);

	for(i = 0; i < iovlen; i++) {
		add_trace_b_add(&state, header, GETIOVBASE(iovs + i), GETIOVLEN(iovs + i));
	}

	add_trace_b_end(&state, header);
}

// Double (in/out) user interface
//
// Note: all parameters must have been validated by this point.  This routine
// is called locked from ker_trace_event and also called directly from
// interrupt service routines, so any segfault as a result of accessing a
// bad parameter will result in a kernel crash!
int trace_event(uint32_t* a_p)
{
	uint32_t code;
	switch(_TRACE_GET_FLAG(*a_p)) {
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSUSEREVENT):
		{
		 /* Note - simply masking off the user event code here may seem dubious, but I can't change it since
		  * I've actually seen users relying on this effect.  *SIGH*
		  */
		 code = *(a_p+1) & _NTO_TRACE_USERLAST;
		 if ( trace_masks.user_mask[code] ) {
			(void) add_trace_event
			(
			 _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, _TRACE_USER_C, code),
			 NULL,
			 *(a_p+2),
			 *(a_p+3)
			);
		 }

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCUSEREVENT):
		{
		 code = *(a_p+1) & _NTO_TRACE_USERLAST;
		 if ( trace_masks.user_mask[code]) {
			add_trace_buffer
			(
			 _TRACE_MAKE_CODE(RUNCPU, NULL, _TRACE_USER_C, code),
			 (uint32_t*) *(a_p+2),
			 (uint32_t ) *(a_p+3)
			);
		 }

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTSCLASSEVENT):
		{
		 code = *(a_p+2) & _NTO_TRACE_USERLAST;
		 if ( trace_masks.user_mask[code] ) {
			(void) add_trace_event
			(
			 _TRACE_MAKE_CODE(RUNCPU, _TRACE_STRUCT_S, (*(a_p+1))<<10, code),
			 NULL,
			 *(a_p+3),
			 *(a_p+4)
			);
		 }

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTCCLASSEVENT):
		{
		 code = *(a_p+2) & _NTO_TRACE_USERLAST;
		 if ( trace_masks.user_mask[code] ) {
			add_trace_buffer
			(
			 _TRACE_MAKE_CODE(RUNCPU, NULL, (*(a_p+1)<<10), code),
			 (uint32_t*) *(a_p+3),
			 (uint32_t ) *(a_p+4)
			);
		 }

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTUSRSTREVENT):
		{
		 code = *(a_p+1) & _NTO_TRACE_USERLAST;
		 if ( trace_masks.user_mask[code] ) {
			trace_state_t t_s;
			uint32_t      	h=_TRACE_MAKE_CODE(RUNCPU, NULL, _TRACE_USER_C, code);
			register char* 	s=(char*)*(a_p+2);
			uint32_t 		l;

			//Determine the length
			while(*s++) {
				// nothing to do
			}
			l = s - (char*)*(a_p+2);

			add_trace_b_init(&t_s);
			add_trace_b_add(&t_s, h, (char *)*(a_p+2), l);
			add_trace_b_end(&t_s, h);
		 }

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_INSERTEVENT):
		{
			(void) add_trace_event(*(a_p+1), *(a_p+2), *(a_p+3), *(a_p+4));

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_STOP):
		{
			if(trace_masks.main_flags && trace_masks.ring_mode) {
				tracebuf_t* t_b_p=privateptr->tracebuf;

				if(trace_masks.main_flags&&t_b_p!=NULL) {
					(void) em_event(t_b_p);
				} else {
					return (ECANCELED);
				}
			}
			trace_masks.main_flags = 0;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_STARTNOSTATE):
		{
			if(privateptr->tracebuf==NULL) return (ECANCELED);
			if(trace_masks.main_flags) return (EOK);
			trace_masks.main_flags = 1;

			return (EOK);
		}
		case _TRACE_GET_FLAG(_NTO_TRACE_START):
		{
			THREAD*  thp;
			PROCESS* prp;
			unsigned i_1;
			unsigned i_2;

			if(privateptr->tracebuf==NULL) return (ECANCELED);
			if(trace_masks.main_flags) return (EOK);
			trace_masks.main_flags = 1;
			(void) get_time_off(1);
			sched_trace_initial_parms();
			for (i_1=1;i_1<process_vector.nentries;++i_1) { // Dump sys. state including
				if(VECP(prp, &process_vector, i_1)) {         // process names and task IDs
					if(prp->debug_name) {
						_TRACE_PR_EMIT_CREATE_NAME(prp);
					} else {
						_TRACE_PR_EMIT_CREATE(prp);
					}
					for(i_2=0;i_2<prp->threads.nentries;++i_2) {
						if(VECP(thp, &prp->threads, i_2)) {

							_TRACE_TH_EMIT_CREATE(thp);
							_TRACE_TH_EMIT_STATE_ONCPU(
								thp,
								thp->state,
								thp->state == STATE_RUNNING ?
									thp->runcpu: RUNCPU
							);
							if(thp->name) {
								_TRACE_TH_EMIT_NAME(thp);
							}
						}
					}
				}
			}

			return (EOK);
		}
		default:
		{
			return (ENOTSUP);
		}
	}
}

// Inserts a string event into the trace buffer
void add_trace_string(uint32_t header, const char *fmt, ...)
{
	va_list  ap;
	uint32_t buff[_TRACE_ROUND_UP(_TRACE_MAX_STR_SIZE)];
	uint32_t l;

	va_start(ap, fmt);
	l = kvsnprintf((char*)buff, sizeof(buff)-1, fmt, ap);
	add_trace_buffer(header, buff, _TRACE_ROUND_UP(1+l));
	va_end(ap);

	return;
}

// Inserts "one data argument" and string event into the trace buffer
void add_trace_d1_string(uint32_t header, uint32_t d_1, const char *fmt, ...)
{
	va_list  ap;
	uint32_t buff[_TRACE_ROUND_UP(_TRACE_MAX_STR_SIZE)+1];
	uint32_t l;

	buff[0] = d_1;
	va_start(ap, fmt);
	l = kvsnprintf((char*)(buff+1), sizeof(buff)-(sizeof(buff[0])+1), fmt, ap);
	add_trace_buffer(header, buff, 1+_TRACE_ROUND_UP(1+l));
	va_end(ap);

	return;
}

// Inserts "two data arguments" and string event into the trace buffer
void add_trace_d2_string(uint32_t header, uint32_t d_1, uint32_t d_2, const char *fmt, ...)
{
	va_list  ap;
	uint32_t buff[_TRACE_ROUND_UP(_TRACE_MAX_STR_SIZE)+2];
	uint32_t l;

	buff[0] = d_1;
	buff[1] = d_2;
	va_start(ap, fmt);
	l = kvsnprintf((char*)(buff+2), sizeof(buff)-((2*sizeof(buff[0]))+1), fmt, ap);
	add_trace_buffer(header, buff, 2+_TRACE_ROUND_UP(1+l));
	va_end(ap);

	return;
}

void trace_emit_address( THREAD *thp, uint32_t vaddr )
{
	uint32_t	header;

	header=_TRACE_MAKE_CODE(
			RUNCPU, _TRACE_STRUCT_S, _TRACE_SYSTEM_C, _NTO_TRACE_SYS_ADDRESS);
	if(_TRACE_CKH_EXE_EH(
	                     trace_masks.class_system_ehd_p,
	                     trace_masks.system_ehd_p[_NTO_TRACE_SYS_ADDRESS],
	                     header,
	                     vaddr,
	                     0
	                    )) {
		(void) add_trace_event(header, NULL, vaddr, 0);
	}
}

#else

int trace_event(uint32_t* data) {
	INTERRUPT               *itp;
	struct sigevent *evp;
	struct sigevent *(*func)(void *);

	if((itp = interrupt_level[HOOK_TO_LEVEL(_NTO_HOOK_TRACE)].queue)) {
		THREAD  *thp;

		thp = itp->thread;
		if(thp->aspace_prp != NULL && thp->aspace_prp != aspaces_prp[RUNCPU]) {
			memmgr.aspace(thp->aspace_prp, &aspaces_prp[RUNCPU]);
		}

		func = (struct sigevent *(*)(void *))itp->handler;
		if((evp = func(data))) {
			intrevent_add(evp, thp, itp);
		}

		return(EOK);
	}

	return(ENOTSUP);
}

#endif

/* modules, like libmod_aps, composed of ker/aps/nano_aps.c, cannot have any variant ifdefs. But ifdefed macros are
 * how we normally invoke trace calls. So to invoke a trace in a module, we create a function here,
 * in the normal kernel, just to invoke the corresponding trace macro..
 */

void trace_emit_th_state(THREAD *thp, int state) {
	_TRACE_TH_EMIT_ANY_STATE(thp, state);
	return;
}

void trace_emit_sys_aps_name(uint32_t id, char *name) {
	_TRACE_SYS_APS_NAME(id,name);
	return;
}

void trace_emit_sys_aps_budgets(uint32_t id, uint32_t percent, uint32_t critical) {
	_TRACE_SYS_APS_BUDGETS(id, percent, critical);
	return;
}

void trace_emit_sys_aps_bankruptcy(uint32_t id, pid_t pid, int32_t tid) {
	_TRACE_SYS_APS_BANKRUPTCY(id, pid, tid);
	return;
}

__SRCVERSION("nano_trace.c $Rev: 206799 $");
