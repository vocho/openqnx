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

#ifndef __KEREXTERNS_H
#define __KEREXTERNS_H


#define __KERNEL_H_INCLUDED

#include <inttypes.h>
#include <setjmp.h>

/* include this early so that atomic ops can be overridden */
#include <atomic.h>

#include "kercpu.h"
#include <sys/syspage.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <intr.h>
#include <malloc.h>
#include <time.h>
#include <ucontext.h>

#include <sched.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/trace.h>
#include <sys/startup.h>
#include <sys/netmgr.h>

#include <kernel/types.h>
#include <kernel/macros.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include <sys/kdebug.h>
#include <sys/fault.h>
#include <sys/debug.h>
#include <kernel/debug.h>
#include <kernel/objects.h>
#include <kernel/kerext.h>
#include <kernel/proto.h>
#include <kernel/query.h>
#include <sys/fault.h>
#include <sys/kercalls.h>
#include <sys/asyncmsg.h>
#include "smpswitch.h"
#include "kermacros.h"
#include "kerargs.h"
#include "kertrace.h"
#include "kerproto.h"

/*
 * Note that the order in which the different globals are declared does
 * make a difference, especially on lower-end machines that have small
 * caches. Grouping together commonly-used globals together can reduce
 * the number of cache misses and improve performance.
 *
 * Note too that this file is included from kexterns.c which includes
 * the scheduler lookup table. Thus, the way things will be laid in memory
 * will be the globals in this file, followed by the sched table. To
 * optimize cache efficieny, we therefore put any scheduler-related globals at
 * the end of this file.
 */

#ifdef KERDEFN
	#define COND_EXT(x) x
	#define EXT
	#define INIT1(a)				= { a }
	#define INIT2(a,b)				= { a,b }
	#define INIT3(a,b,c)			= { a,b,c }
	#define INIT7(a,b,c,d,e,f,g)	= { a,b,c,d,e,f,g }
#else
	#define COND_EXT(x) 0
	#define EXT extern
	#define INIT1(a)
	#define INIT2(a,b)
	#define INIT3(a,b,c)
	#define INIT7(a,b,c,d,e,f,g)
	#undef  INITSOUL
	#define INITSOUL(a,b,c,d,e)
#endif

#if !defined(NDEBUG)
	// Some debug globals. The code never touches them but you can assign
	// them values via wd to simplify structure viewing of void * ptrs.
	EXT PROCESS		*__prp;
	EXT THREAD		*__thp;
	EXT CHANNEL		*__chp;
	EXT CONNECT		*__cop;
	EXT TIMER		*__tip;
	EXT unsigned	stats[4];
#endif

EXT int		procmgr_scoid;
EXT mode_t	procfs_umask INIT1(0022);

// Additional variables used for debug etc.
EXT struct cpu_extra_state	*kdextra;	// for debugging

// Low bandwidth globals
EXT int					pid_unique;
EXT int					align_fault;
EXT int					noncoherent_caches;
EXT int					fpuemul;
EXT int					nohalt;
EXT unsigned			user_boundry_addr	INIT1(VM_USER_SPACE_BOUNDRY);
EXT int					ker_verbose;
EXT int					limits_max[LIMITS_NUM];
EXT unsigned			max_fds			INIT1(1000);
EXT unsigned			fd_close_timeout	INIT1(30);
EXT unsigned			priv_prio		INIT1(64);
EXT unsigned			clk_tck			INIT1(100);
EXT unsigned long		tick_minimum INIT1(_NTO_TICKSIZE_MIN);
EXT pthread_attr_t		*main_attrp;
EXT volatile int		soul_compact;				// This is not currently used
EXT int					_argc;
EXT char				**_argv;
EXT char				**environ;
EXT VECTOR				interrupt_vector;
EXT VECTOR				process_vector;
EXT VECTOR				query_vector;
EXT VECTOR				soul_vector;
EXT VECTOR				vthread_vector;
EXT VECTOR				mempart_vector;
EXT VECTOR				schedpart_vector;
EXT VECTOR				chgbl_vector;
EXT CREDENTIAL			*credential_list;
EXT LIMITS				*limits_list;
EXT PROCESS				*procnto_prp;
EXT uintptr_t			run_ker_stack_bot;
EXT uintptr_t			run_ker_stack_top;
EXT struct cpupage_entry	*_cpupage_ptr;
EXT struct system_private_entry	*privateptr;
EXT unsigned			intrinfo_num;
EXT unsigned			__cpu_flags;
EXT NET					net;
EXT PROCMGR				procmgr;
EXT int					num_processors;
EXT int					scheduler_type; //one of SCHEDULER_TYPE_* values in kermacros.h

EXT int 				(rdecl *debug_process_stopped)(PROCESS *prp, int signo, int sigcode, int sigval, int sender);
EXT int					(rdecl *debug_thread_fault)(THREAD *thp, siginfo_t *info);
EXT	int					(rdecl *debug_thread_signal)(THREAD *thp, int signo, int sigcode, int sigval, int sender);
EXT	int					(rdecl *debug_process_exit)(PROCESS *prp, int priority);
EXT void				(rdecl *debug_attach_brkpts)(DEBUG *dep);
EXT void				(rdecl *debug_detach_brkpts)(DEBUG *dep);
EXT void				(rdecl *debug_moduleinfo)(PROCESS *prp, THREAD *thp, void *dbg);
EXT void				( *sched_trace_initial_parms)();
EXT DISPATCH			*(*init_scheduler)(void) INIT1(init_scheduler_default);

EXT uint64_t 			startup_stack[STARTUP_STACK_NBYTES / sizeof(uint64_t)] INIT1(__STACK_SIG);
EXT int					__ealready_value;
EXT unsigned        rr_interval_mul INIT1(_NTO_RR_INTERVAL_MUL_DEFAULT);


// Main kernel variables used in all paths - group those that are modified
// together and those that are static afterwards.
#if !COND_EXT(HAVE_INKERNEL_STORAGE)
	EXT volatile unsigned	inkernel;
#endif
#if !COND_EXT(HAVE_ACTIVES_STORAGE)
	EXT THREAD				*actives[PROCESSORS_MAX];
#endif
EXT int						inspecret;
EXT PROCESS					*actives_prp[PROCESSORS_MAX];
EXT PROCESS					*aspaces_prp[PROCESSORS_MAX];
EXT const struct fault_handlers		*volatile xfer_handlers[PROCESSORS_MAX];
EXT struct cpupage_entry	*cpupageptr[PROCESSORS_MAX];
#if !COND_EXT(HAVE_KERSTACK_STORAGE)
	EXT uintptr_t			ker_stack[PROCESSORS_MAX];
#endif
EXT THREAD					*actives_fpu[PROCESSORS_MAX];
EXT int						nopreempt;
EXT MEMMGR					memmgr;
EXT memclass_id_t			sys_memclass_id;	// generic system ram memory class
EXT void					(rdecl *mark_running)(THREAD *act);
EXT THREAD					*actives_pcr[PROCESSORS_MAX];
EXT HASH	sync_hash        INIT1(0x3ff);	// Must be a mask


// Interrupt path globals
EXT INTREVENT				*intrevent_pending;
EXT volatile unsigned 		queued_event_priority;
EXT struct intrinfo_entry	*intrinfoptr;
EXT INTRLEVEL				*interrupt_level;
EXT int						intrs_aps_critical INIT1(1);
EXT struct _thread_local_storage	intr_tls INIT3(0, 0, &intr_tls.__errval);
EXT int						intrespsave;
EXT struct sigevent			intr_fault_event INIT3(SIGEV_SIGNAL_THREAD, {SIGSEGV}, {SI_IRQ});


// Clock timer
EXT char					overrun;
EXT INTERRUPT				*clock_isr;
EXT volatile uint8_t		ticker_preamble;
#if defined(COUNT_CYCLES)
	EXT uint64_t			cycles;
#endif
EXT struct qtime_entry		*qtimeptr;

EXT struct callout_entry	*calloutptr;
EXT struct syspage_entry	*_syspage_ptr;
EXT SSREPLENISH 			*ss_replenish_list;
EXT int						(rdecl *scheduler_tick_hook)(); /* return true if the current thread's timeslice should be ended */
EXT int						(rdecl *kerop_thread_create_hook)(THREAD *act, PROCESS *prp,
									const struct sigevent *evp, unsigned thread_create_flags, THREAD *thp);
EXT void					(rdecl *kerop_thread_destroy_hook)(THREAD *thp);
EXT void					(rdecl *kerop_clock_handler_hook)(THREAD *act);
EXT void                    (rdecl *kerop_microaccount_hook)(THREAD *old, THREAD *new);
EXT unsigned				(*callout_timer_value)(struct syspage_entry *, struct qtime_entry *);

EXT int           			(rdecl *kerop_thread_ctl_hook)(THREAD *act, THREAD *op, struct kerargs_thread_ctl *kap);
EXT int 					(*kdinvoke_hook)(union kd_request *r);

// vendor hooks
EXT void           			(rdecl *interrupt_hook)(INTRLEVEL *ilp);
EXT void				(rdecl *sync_mutex_lock_hook_for_block)(THREAD *thp);
EXT void				(rdecl *clock_handler_hook_for_ts_preemption)(THREAD *act, unsigned int reschedl);
EXT void 				(rdecl *sync_create_hook)(PROCESS *prp);
EXT void				(rdecl *sync_destroy_hook)(PROCESS  *prp, sync_t *sync);
EXT uint64_t				(rdecl *intrevent_drain_hook_enter)(void);
EXT void				(rdecl *intrevent_drain_hook_exit)(uint64_t id_hook_context); //parm is output of interevent_drain_hook_enter
EXT void				(rdecl *timer_expiry_hook_max_timer_fires)( unsigned int nfires);


// MT tracer extends
EXT THREAD				*mt_controller_thread;
EXT int					mt_flush_evt_channel;
EXT uintptr_t			mt_tracebuf_addr;


// Object allocator - put most-used object pools at beginning.
EXT int		alloc_critical;
EXT SOUL	pulse_souls      INITSOUL(LIMITS_PULSE,     sizeof(PULSE),     64,	SOUL_CRITICAL, 0);
EXT SOUL	sync_souls       INITSOUL(LIMITS_SYNC,      sizeof(SYNC),      64,	0, 0);
EXT SOUL	timer_souls      INITSOUL(LIMITS_TIMER,     sizeof(TIMER),     12, 	0, 0);
EXT SOUL	thread_souls     INITSOUL(LIMITS_THREAD,    sizeof(THREAD),    24,	0, 0);
EXT SOUL	connect_souls    INITSOUL(LIMITS_CONNECT,   sizeof(CONNECT),   64,	0, 0);
EXT SOUL	channel_souls    INITSOUL(LIMITS_CHANNEL,   sizeof(CHANNEL),    8,	0, 0);
EXT SOUL	process_souls    INITSOUL(LIMITS_PROCESS,   sizeof(PROCESS),   12,	0, 0);
EXT SOUL	chasync_souls	 INITSOUL(LIMITS_CHANNEL,   sizeof(CHANNELASYNC),    8,	0, 0);
EXT SOUL	chgbl_souls		 INITSOUL(LIMITS_CHANNEL,   sizeof(CHANNELGBL),    8,	0, 0);
EXT SOUL	interrupt_souls  INITSOUL(LIMITS_INTERRUPT, sizeof(INTERRUPT),  8,	0, 0);
EXT SOUL	syncevent_souls  INITSOUL(LIMITS_SYNCEVENT, sizeof(SYNCEVENT),  4,	0, 0);
EXT SOUL	fpu_souls        INITSOUL(0,                sizeof(FPU_REGISTERS),4,	0, FPUDATA_ALIGN);
EXT SOUL	sigtable_souls   INITSOUL(0,                sizeof(SIGTABLE),   8,	0, 0);
EXT SOUL	client_souls     INITSOUL(0,                sizeof(CLIENT),     4,	0, 0);
EXT SOUL	credential_souls INITSOUL(0,                sizeof(CREDENTIAL), 4,	0, 0);
EXT SOUL	limits_souls     INITSOUL(0,                sizeof(LIMITS),     4,	0, 0);
EXT SOUL	vthread_souls    INITSOUL(0,                SIZEOF_VTHREAD,    4,	0, 0);
EXT SOUL	debug_souls      INITSOUL(0,                sizeof(DEBUG),      1,	0, 0);
EXT SOUL	breakpt_souls    INITSOUL(0,                sizeof(BREAKPT),    1,	SOUL_CRITICAL, 0);
EXT SOUL	ssinfo_souls     INITSOUL(0,                sizeof(SSINFO),    1,	0, 0);
EXT SOUL	threadname_souls INITSOUL(0,                THREAD_NAME_FIXED_SIZE,    1,	0, 0);


// High-bandwidth scheduler variables/func pointers
EXT volatile uint64_t		ss_replenish_time;
EXT int						(rdecl *may_thread_run)(THREAD *thp);
EXT void					(rdecl *block_and_ready)(THREAD *thp);
EXT void					(rdecl *ready)(THREAD *thp);
EXT THREAD *				(rdecl *select_thread)(THREAD *act, int cpu, int prio);
EXT void					(rdecl *adjust_priority)(THREAD *thp, int prio, DISPATCH *dpp, int priority_inherit);
EXT void					(rdecl *resched)(void);
EXT void					(rdecl *yield)(void);


// SMP variables
EXT THREAD         		*need_to_run INIT1(NULL);
EXT int            		need_to_run_cpu INIT1(-1);
EXT	void				*kercallptr;
EXT char				alives[PROCESSORS_MAX];
EXT uint64_t			clockcycles_offset[PROCESSORS_MAX];
#if defined(VARIANT_smp)

	EXT volatile uint32_t ipicmds[PROCESSORS_MAX];
	EXT intrspin_t		intr_slock;
	EXT intrspin_t		clock_slock;
	EXT intrspin_t		ker_slock;
	EXT intrspin_t		debug_slock;
	extern uint8_t		cpunum;		// Defined in assembly

#else
	EXT uint8_t			cpunum;		// Defined for kernel modules like APS
#endif


// Prototypes for external declarations
extern const struct fault_handlers			xfer_src_handlers;
extern const struct fault_handlers			xfer_dst_handlers;
extern const struct fault_handlers			xfer_fault_handlers;
extern const struct fault_handlers			xfer_async_handlers;
extern jmp_buf								*xfer_env;
extern const long	kernel_conf_table[];

extern const uint8_t	byte_log2[1 << 8]; // defined in externs.c

extern int kdecl (* ker_call_table[])();
extern const int ker_call_entry_num;

#if defined(VARIANT_instr)

EXT struct trace_masks {
	// Mask declarations/definitions
	uint32_t ring_mode;
	uint32_t main_flags;
	uint32_t control_mask;
	uint32_t pr_mask;
	uint32_t th_mask;
	uint32_t vth_mask;
	uint32_t system_mask    [_TRACE_MAX_SYSTEM_NUM  ];
	uint32_t comm_mask      [_TRACE_MAX_COMM_NUM    ];
	uint32_t int_masks      [_TRACE_MAX_INT_NUM     ];
	uint32_t int_handler_masks      [_TRACE_MAX_INT_NUM     ];
	uint32_t ker_call_masks [_TRACE_MAX_KER_CALL_NUM];
	uint32_t user_mask 		[_TRACE_MAX_USER_NUM];

	// emitting handle function process
	// pointers and their argument areas
	ehandler_data_t* control_time_ehd_p;
	ehandler_data_t* pr_create_ehd_p;
	ehandler_data_t* pr_destroy_ehd_p;
	ehandler_data_t* pr_create_name_ehd_p;
	ehandler_data_t* pr_destroy_name_ehd_p;
	ehandler_data_t* th_name_ehd_p;			//Part of the process class

	ehandler_data_t* thread_ehd_p         [_TRACE_MAX_TH_STATE_NUM];
	ehandler_data_t* vthread_ehd_p        [_TRACE_MAX_TH_STATE_NUM];
	ehandler_data_t* int_enter_ehd_p      [_TRACE_MAX_INT_NUM     ];
	ehandler_data_t* int_exit_ehd_p       [_TRACE_MAX_INT_NUM     ];
	ehandler_data_t* int_handler_enter_ehd_p      [_TRACE_MAX_INT_NUM     ];
	ehandler_data_t* int_handler_exit_ehd_p       [_TRACE_MAX_INT_NUM     ];
	ehandler_data_t* ker_call_enter_ehd_p [_TRACE_MAX_KER_CALL_NUM];
	ehandler_data_t* ker_call_exit_ehd_p  [_TRACE_MAX_KER_CALL_NUM];
	ehandler_data_t* system_ehd_p         [_TRACE_MAX_SYSTEM_NUM  ];
	ehandler_data_t* comm_ehd_p           [_TRACE_MAX_COMM_NUM    ];

	uint32_t ker_call_pid [_TRACE_MAX_KER_CALL_NUM];
	uint32_t ker_call_tid [_TRACE_MAX_KER_CALL_NUM];
	uint32_t thread_pid   [_TRACE_MAX_TH_STATE_NUM];
	uint32_t thread_tid   [_TRACE_MAX_TH_STATE_NUM];
	uint32_t vthread_pid  [_TRACE_MAX_TH_STATE_NUM];
	uint32_t vthread_tid  [_TRACE_MAX_TH_STATE_NUM];
	uint32_t system_pid   [_TRACE_MAX_SYSTEM_NUM  ];
	uint32_t system_tid   [_TRACE_MAX_SYSTEM_NUM  ];
	uint32_t comm_pid     [_TRACE_MAX_COMM_NUM    ];
	uint32_t comm_tid     [_TRACE_MAX_COMM_NUM    ];

	// class emitting (global) handle function process
	// pointers and their argument areas
	ehandler_data_t* class_control_ehd_p;
	ehandler_data_t* class_ker_call_enter_ehd_p;
	ehandler_data_t* class_ker_call_exit_ehd_p;
	ehandler_data_t* class_int_enter_ehd_p;
	ehandler_data_t* class_int_exit_ehd_p;
	ehandler_data_t* class_int_handler_enter_ehd_p;
	ehandler_data_t* class_int_handler_exit_ehd_p;
	ehandler_data_t* class_process_ehd_p;
	ehandler_data_t* class_thread_ehd_p;
	ehandler_data_t* class_vthread_ehd_p;
	ehandler_data_t* class_system_ehd_p;
	ehandler_data_t* class_comm_ehd_p;

	// Event handlers storage and synchronization
	ehandler_data_t eh_storage [_TRACE_MAX_EV_HANDLER_NUM];
	uint32_t eh_num;
	struct   intrspin eh_spin;

	// State management
	struct   intrspin state_spin;
	void*             buff_0_ptr;
	uint32_t          buff_num;
	uint32_t          send_buff_num;
	uint32_t          skip_buff;
	uint32_t          max_events;
} trace_masks;
EXT uint32_t trace_force_flush;

// global masks
EXT uint32_t ker_exit_enable_mask;
EXT uint32_t int_enter_enable_mask;
EXT uint32_t int_exit_enable_mask;

// global declarations
extern int kdecl (* _trace_ker_call_table[])();
extern int kdecl (* _trace_call_table    [])();

#endif

EXT int signal_kill_updates_terming_priority INIT1(1);

#if defined(__X86__)

	EXT short unsigned	ker_cs		INIT1(0x80);
	EXT short unsigned	ker_ds		INIT1(0x27);
	EXT short unsigned	ker_ss		INIT1(0x88);
	EXT short unsigned	ker2_cs		INIT1(0x90);
	EXT short unsigned	sys_cs		INIT1(0x1d);
	EXT short unsigned	sys_ds		INIT1(0x27);
	EXT short unsigned	sys_ss		INIT1(0x99);
	EXT short unsigned	usr_cs		INIT1(0x1f);
	EXT short unsigned	usr_ds		INIT1(0x27);
	EXT short unsigned	usr_ss		INIT1(0x27);
	EXT X86_TSS						*tss[PROCESSORS_MAX];
	EXT short unsigned				cpupage_segs[PROCESSORS_MAX];
	EXT unsigned					realmode_addr;
	EXT	volatile unsigned *__inkp	INIT1(&inkernel); // For debugging
	EXT unsigned					fault_code;
	EXT unsigned					startup_cpunum;

	EXT X86_PERFREGS				disabled_perfregs;

#elif defined(__PPC__)

	EXT unsigned			ppc_ienable_bits INIT1(PPC_MSR_EE);
	EXT union ppc_alt_regs	*actives_alt[PROCESSORS_MAX];
	EXT void				(*tlb_flush_all)(void);
	// The size field is filled in at runtime.
	EXT SOUL				alt_souls INITSOUL(0, 0, 2, 0, 16);

	#if defined(VARIANT_booke)
		EXT unsigned		ppcbke_tlb_select;
	#endif

	EXT PPC_PERFREGS				disabled_perfregs;

#elif defined(__MIPS__)

	EXT	void					*l1pagetable;
	EXT volatile unsigned long	*__shadow_imask;
	EXT	void					*sys_kercallptr;

	#if defined(VARIANT_smp)
		//INIT1() values used while kernel is initializing
		EXT uintptr_t	ker_stack_bot	INIT1(0x80000000);
		EXT uintptr_t	ker_stack_top	INIT1(0xffffffff);
	#endif

	EXT MIPS_PERFREGS				disabled_perfregs;

#elif defined(__C6X__)

#elif defined(__ARM__)

	EXT unsigned		mmu_domain[PROCESSORS_MAX];
	EXT void			(*mmu_abort)(CPU_REGISTERS *);
	EXT uint64_t		last_cycles;
	#if defined(VARIANT_v6) && !defined(VARIANT_smp)
		/*
		 * We need to clear the ARMv6 exclusive monitor on context switches:
		 * - on SMP (ARMv6K architectures) we can use clrex.
		 * - on non-ARMv6K, we need to perform a dummy strex, so we provide
		 *   dummy_strex as a safe location to perform the store.
		 */
		EXT unsigned	dummy_strex;
	#endif

#elif defined(__SH__)

	EXT volatile unsigned long	*__shadow_imask;
	EXT volatile unsigned long	*__cpu_imask[PROCESSORS_MAX];

	#if defined(VARIANT_smp)
		/*
		 * These are required to ensure that am_inkernel will return true
		 * during early initialisation.
		 * They will be assigned to their real values by ker_start().
		 */
		EXT uintptr_t	ker_stack_bot	INIT1(0x80000000);
		EXT uintptr_t	ker_stack_top	INIT1(0xffffffff);
	#endif

#else

    #error not configured for system

#endif

#endif

/* __SRCVERSION("externs.h $Rev: 211164 $"); */
