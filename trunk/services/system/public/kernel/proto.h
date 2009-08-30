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

// Kernel prototypes

#include <kernel/hooks.h>

#if defined(__X86__)
#if defined(__WATCOMC__)
	//
	// The following overloading of fortran calling conventions allows us
	// a minor increase in performance by telling the compiler that it need
	// not save any registers for the kernel calls which are called from
	// the assembly stub kernel.asm. We also tell it to use stack calling
	// conventions for its arguments.
	//
	// The following overloading of pascal calling conventions allows us
	// a minor increase in performance by passing args through registers.
	//
	#pragma aux fortran "*" modify exact [eax ebx ecx edx esi edi gs] parm caller [];
	#define kdecl fortran
	
	#pragma aux pascal "*" parm [eax edx ebx ecx] routine;
	#define rdecl pascal

	#pragma aux specialret 		parm [eax] modify exact [eax ebx ecx edx esi edi gs];
	#pragma aux intrevent_add	parm [eax] [edx] [ecx] value [eax] modify exact [eax ecx edx edi gs];
	#pragma aux intrevent_drain	modify exact [eax ebx ecx edx esi edi gs];
	#define specialret_attr
	#define intrevent_add_attr
	#define intrevent_drain_attr rdecl
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
	/*-
	 * not much fancy for now; just a little gnu-insanity
	 */
	#define kdecl
	#define rdecl __attribute__((cdecl,regparm(3)))
	#define specialret_attr __attribute__((regparm(1)))
	#define intrevent_add_attr __attribute__((regparm(3)))
	#define intrevent_drain_attr
#else
#error Compiler not supported.
#endif

#elif defined(__PPC__) 
	/*-
	 * use the "kdecl" attribute to avoid having to save/restore extra
	 * registers for kcall fucntions
	 */
	//#define kdecl __attribute__((kdecl)) // Turn on when compilers support
	#define kdecl
	#define rdecl
	#define specialret_attr
	#define intrevent_add_attr
	#define intrevent_drain_attr
#elif defined(__MIPS__) \
   || defined(__SH__) \
   || defined(__ARM__)
	/*-
	 * nothing fancy for now; maybe later tap some gnu-insanity
	 */
	#define kdecl
	#define rdecl
	#define specialret_attr
	#define intrevent_add_attr
	#define intrevent_drain_attr
#else
    #error kernel entry conventions not configured for system
#endif

// Nanokernel calls
void          rdecl block(void);
void          rdecl unready(THREAD *thp, int newstate);
void          rdecl stop_threads(PROCESS *prp, unsigned flags, unsigned iflags);
void          rdecl cont_threads(PROCESS *prp, unsigned flags);
int           rdecl stop_one_thread(PROCESS *prp, int tid, unsigned flags, unsigned iflags);
int           rdecl cont_one_thread(PROCESS *prp, int tid, unsigned flags);
int           rdecl set_runmask(THREAD *thp, unsigned runmask);
void *        rdecl object_alloc(PROCESS *prp, SOUL *soulp);
void *        rdecl object_grow(SOUL *soulp);
void          rdecl object_free(PROCESS *prp, SOUL *soulp, void *ptr);
void          rdecl object_compact(SOUL *soulp);
unsigned      rdecl object_register_data(SOUL *soulp, size_t size);
#define             object_to_data(o, c) ((void *)((char *)(o) + (c)))
#define             object_from_data(o, c) ((void *)((char *)(o) - (c)))
void *        rdecl vector_lookup(VECTOR *vec, int id);
void *        rdecl vector_lookup2(VECTOR *vec, int id);
void *        rdecl vector_search(VECTOR *vec, unsigned id, unsigned *found);
int           rdecl vector_add(VECTOR *vec, void *object, unsigned index);
void *        rdecl vector_rem(VECTOR *vec, unsigned index);
int           rdecl vector_flag(VECTOR *vec, unsigned index, int flag);
void          rdecl vector_free(VECTOR *vec);
void *              nano_query(int type, unsigned index1, int subtype, unsigned index2, unsigned *next, void *objbuf, int objsize);
THREAD *            get_active(void);
int           rdecl force_ready(THREAD *thp, int err);
void          rdecl signal_clear_thread(THREAD *thp, siginfo_t *sip);
void          rdecl signal_ignore(PROCESS *prp, int signo);
void          rdecl signal_ignore_thread(THREAD *thp, int signo);
void          rdecl signal_block(THREAD *thp, SIGBITS *sig_blocked);
void          specialret_attr specialret(THREAD *thp);
void          rdecl deliver_fault(THREAD *thp, siginfo_t *info);
void          rdecl usr_fault(int code_signo, THREAD *act, uintptr_t addr);
void          rdecl timer_init(void);
TIMER *	      rdecl timer_alloc(PROCESS *);
void	      rdecl timer_free(PROCESS *, TIMER *);
void          rdecl timer_period(void);
void          rdecl timer_remaining(TIMER *tip, struct _itimer *left);
int			  rdecl timer_past(int clockid, int flags, uint64_t *firetime);
void          rdecl timer_activate(TIMER *tip);
int           rdecl timer_deactivate(TIMER *tip);
void		  rdecl timer_next(uint64_t *nexttime);
void		  rdecl timer_expiry(QTIME *qtp);
void		  rdecl timer_pending(TIMER *tip);
void          rdecl thread_init(void);
int           rdecl thread_create(THREAD *act, PROCESS *prp, const struct sigevent *evp, unsigned thread_create_flags);
void          rdecl thread_destroy(THREAD *thp);
void          rdecl thread_destroyall(THREAD *thp);
void          rdecl thread_cancel(THREAD *thp);
uint32_t      rdecl keygen(IOV *msg, int32_t parts, uint32_t key1);
void          rdecl get_rcvinfo(THREAD *thp, int tid, CONNECT *cop, struct _msg_info *rep);
void          rdecl connect_detach(CONNECT *cop, int prio);
void          rdecl connect_coid_disconnect(CONNECT *cop, CHANNEL *chp, int priority);
VTHREAD *     rdecl net_send1(int vtid, struct _vtid_info *vtp);
CONNECT *     rdecl net_send2(KERARGS *kap, int vtid, CONNECT *cop, THREAD *thp);
void          rdecl snap_time_wakeup(void);
void          rdecl snap_time(uint64_t *tsp, int incl_tod);
int           rdecl cred_set(CREDENTIAL **pcrp, struct _cred_info *cip);
void          rdecl cred_dirty(PROCESS *prp);
LIMITS *      rdecl limits_getset(uid_t uid, unsigned *newmaxes[]);

// memory object allocator calls
void *              malloc(unsigned size);
void *              calloc(unsigned size, unsigned num);
void *              realloc(void *data, unsigned size);
void                free(void *data);
void *              _smalloc(unsigned size);
void *              _scalloc(unsigned size);
void *              _srealloc(void *data, unsigned old_size, unsigned new_size);
void *              _sreallocfunc(void *data, unsigned old_size, unsigned new_size, unsigned (*alloc)(unsigned size, void **addr));
void                _sfree(void *data, unsigned size);
void                _kfree(void *data);          // does a __Ring0() first
void                _ksfree(void *data, unsigned size);          // does a __Ring0() first
void *              _ksmalloc(unsigned size);    // does a __Ring0() first
void *              sbrk(int increment);
int 				purger_register(int (*purge)(size_t), unsigned prio);
int 				purger_invoke(size_t amount);

void          		shutdown(unsigned sigcode, const CPU_REGISTERS *reg);
int                 kprintf(const char *fmt, ...);
int                 kdprintf(const char *fmt, ...);
int                 ksprintf(char *buffer, const char *fmt, ...);
int					ksnprintf(char *buffer, size_t len, const char *fmt, ...);
int					kvsnprintf(char *buf, size_t buflen, const char *fmt, __NTO_va_list ap);
void                dbug_display(const char *buffer, int len);
void                scrn_display(const char *buffer, int len);
int                 scrn_poll_key(void);
void                strscrn_display(const char *buffer);
void				module_init(unsigned pass);

PROCESS *     rdecl lookup_pid(pid_t pid);
CONNECT *     rdecl lookup_connect(int coid);
CONNECT *     rdecl lookup_rcvid(KERARGS *kap, int rcvid, THREAD **thpp);
LIMITS *      rdecl lookup_limits(uid_t uid);
SYNC *        rdecl sync_create(PROCESS *bill_prp, sync_t *sync, unsigned flags);
int			  rdecl sync_destroy(PROCESS *prp, sync_t *sync);
SYNC *        rdecl sync_lookup(sync_t *sync, unsigned create);
void          rdecl sync_wakeup(SYNC *syp, int all);

PULSE *       rdecl pulse_add(PROCESS *prp, PRIL_HEAD *queue, int priority, int code, int value, int id);
PULSE *       rdecl pulse_replace(PROCESS *prp, PRIL_HEAD *queue, int priority, int code, int value, int id);
int           rdecl pulse_deliver(CHANNEL *chp, int priority, int code, int value, int id, unsigned pulse_flags);
void		  rdecl pulse_remove(PROCESS *prp, PRIL_HEAD *queue, PULSE *pup);

void          rdecl remove_unblock(THREAD *thp, CONNECT *cop, int rcvid);

SIGHANDLER *   get_signal_handler(PROCESS *prp, int signo, int create);

int           rdecl signal_kill_thread(PROCESS *prp, THREAD *thp, int signo, int code, int value, pid_t pid, unsigned signal_flags);
int           rdecl signal_kill_process(PROCESS *prp, int signo, int code, int value, pid_t pid, unsigned signal_flags);
int           rdecl signal_kill_group(THREAD *killer, pid_t pgrp, int signo, int code, int value, pid_t pid, unsigned signal_flags);

/* Add support for sporatic cleanup */
void          rdecl sched_ss_cleanup(THREAD *thp);
int 		  rdecl sched_ss_adjust(void);
void 	      rdecl sched_ss_update(THREAD *thp, SSINFO *ssinfo, SSREPLENISH *ssrepl, int drop);
void 		  rdecl sched_ss_block(THREAD *thp, int preempted);
void 		  rdecl sched_ss_queue(THREAD *thp);

int           rdecl sched_thread(THREAD *thp, int policy, struct sched_param *param);

void          rdecl timeout_start(THREAD *thp);
void          rdecl timeout_stop(THREAD *thp);

int           rdecl sigevent_exe(const struct sigevent *evp, THREAD *thp, int verify);
int           rdecl sigevent_proc(const struct sigevent *evp);
void		  		intrevent_init(unsigned);
void				intrevent_preemption(unsigned);
int           intrevent_add_attr intrevent_add(const struct sigevent *evp, THREAD *thp, INTERRUPT *isr);
int			  event_add(struct sigevent *evp, THREAD *thp);
void          intrevent_drain_attr intrevent_drain(void);
void				intrevent_flush(void);

int                 viov_to_piov(PROCESS *prp, IOV *piop, int pparts, IOV *viop, int vparts, int offset);
void                set_intr_mask(int intnum, int set);

int           rdecl debug_process(PROCESS *prp, debug_process_t *dpp);
int           rdecl debug_thread(PROCESS *prp, THREAD *thp, debug_thread_t *dtp);
int           rdecl debug_stop(PROCESS *prp);
int           rdecl debug_run(PROCESS *prp, debug_run_t *drp);
int           rdecl debug_break(PROCESS *pid, debug_break_t *dbp);
int           rdecl debug_break_list(PROCESS *pid, debug_breaklist_t *dbp);

void				kdebug_init(int (*kdebug_path)(struct kdebug_entry *entry, char *buff, int buffsize));
int					kdebug_attach(struct kdebug_entry *entry, int resident);
int					kdebug_unlink(struct kdebug_entry *entry);
int					kdebug_detach(struct kdebug_entry *entry);
int					kdebug_watch_entry(struct kdebug_entry *entry, uintptr_t entry_addr);
unsigned			kdebug_enter(PROCESS *prp, unsigned sigcode, CPU_REGISTERS *reg);
void				kdebug_kdump_private(const struct kdump_private *);

void			mdriver_init(void);
void			mdriver_check(void);
void			mdriver_process_time(void);
int				mdriver_intr_attach(int);

void          rdecl send_ipi(int cpu, int cmd);
int           rdecl get_cpunum(void);
void          rdecl smp_flush_tlb(void);

void          rdecl crash(const char *file, int line);
struct asinfo_entry;
int 				walk_asinfo(const char *name, int (*func)(struct asinfo_entry *, char *, void *), void *data);
int			(xfer_memprobe)(void *ptr);

#define XSTR(x)	#x
#define STRINGIZE(x) XSTR(x)

#ifdef _lint
	#define		crash()	(exit(-1))
#else
	#define		crash() (crash)(STRINGIZE(_BASE_FILE_), __LINE__)
#endif

/*
 * STRLCPY(dst, src, len)
 * 
 * BSD 'strlcpy()' - google it
 * NOTE however that unlike the BSD version, this macro (currently) does
 * 	not return anything
 * 
 * In general ...
 * a more efficient and safer implementation of strncpy() (non debug)
 * - <dst> is guaranteed to be null terminated
 * - copying stops at first '\0' in <src>
 * - debug version will identify truncated strings
 *
*/
#ifndef NDEBUG
   #define STR_OVERFLOW(f, line, s, d, l) \
   					 kprintf("%s:%u - strncpy overflow, src truncated to %d bytes\n\tsrc - '%s'\n\tdst  - '%s'\n", \
   					 				(f), (line), (l)-1, (s), (d))
#else   
   #define STR_OVERFLOW(f, line, s, d, l)
#endif	/* NDEBUG */

#define STRLCPY(d, s, l) \
				do { \
					CRASHCHECK((size_t)(l) > 4096); /* what we believe to be a reasonable value */ \
					if(memccpy((d), (s), '\0', (l)) == NULL) { \
						(d)[((l) > 0) ? (l)-1 : (l)] = '\0'; \
						if ( ker_verbose >= 2 ) { \
							STR_OVERFLOW(STRINGIZE(_BASE_FILE_), __LINE__, (s), (d), (l)); \
						} \
					} \
				} while (0)

#undef MMF
#define MMF(r, f, p, e)		MMF_PROTO(r, f, p, e)
MM_FUNCS(smm)


//vendor hooks
struct _resmgr_context;
extern void	(rdecl *sync_mutex_lock_hook_for_block)(THREAD *thp); 
extern void	(rdecl *clock_handler_hook_for_ts_preemption)(THREAD *act, unsigned int reschedl); 
extern void	(rdecl *sync_destroy_hook)(PROCESS  *prp, sync_t *sync);
struct kerargs_sync_create; 
extern void	(rdecl *sync_create_hook)(PROCESS *prp); 
extern uint64_t	 (rdecl *clock_handler_hook_enter)(void); 
extern void	(rdecl *clock_handler_hook_exit)(uint64_t id_hook_context); /* parm is output of clock_handler_hook_enter */
extern uint64_t	 (rdecl *intrevent_drain_hook_enter)(void); 
extern void	(rdecl *intrevent_drain_hook_exit)(uint64_t id_hook_context); /* parm is output of interevent_drain_hook_enter */
extern void	(rdecl *timer_expiry_hook_max_timer_fires)( unsigned int nfires); 



/* __SRCVERSION("proto.h $Rev: 206838 $"); */
