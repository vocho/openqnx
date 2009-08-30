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

/*
 * Prototypes that are private to the kernel itself.
 */

int           kdecl ker_bad(THREAD *act, struct kerargs_null *kap);
int           kdecl ker_nop(THREAD *act, struct kerargs_null *kap);
int           kdecl ker_sys_cpupage_get(THREAD *act, struct kerargs_sys_cpupage_get *kap);
int           kdecl ker_sys_cpupage_set(THREAD *act, struct kerargs_sys_cpupage_set *kap);
int           kdecl ker_channel_create(THREAD *act, struct kerargs_channel_create *kap);
int           kdecl ker_channel_destroy(THREAD *act, struct kerargs_channel_destroy *kap);
int           kdecl ker_channel_connect_attrs(THREAD *act, struct kerargs_channel_connect_attr *kap);

int           kdecl ker_connect_attach(THREAD *act, struct kerargs_connect_attach *kap);
int           kdecl ker_connect_detach(THREAD *act, struct kerargs_connect_detach *kap);
int           kdecl ker_connect_client_info(THREAD *act, struct kerargs_connect_client_info *kap);
int           kdecl ker_connect_server_info(THREAD *act, struct kerargs_connect_server_info *kap);
int           kdecl ker_connect_flags(THREAD *act, struct kerargs_connect_flags *kap);

int           kdecl ker_msg_sendv(THREAD *act, struct kerargs_msg_sendv *kap);
int           kdecl ker_msg_receivev(THREAD *act, struct kerargs_msg_receivev *kap);
int           kdecl ker_msg_replyv(THREAD *act, struct kerargs_msg_replyv *kap);
int           kdecl ker_msg_sendpulse(THREAD *act, struct kerargs_msg_sendpulse *kap);
int           kdecl ker_msg_error(THREAD *act, struct kerargs_msg_error *kap);
int           kdecl ker_msg_readv(THREAD *act, struct kerargs_msg_readv *kap);
int           kdecl ker_msg_readwritev(THREAD *act, struct kerargs_msg_readwritev *kap);
int           kdecl ker_msg_readiov(THREAD *act, struct kerargs_msg_readiov *kap);
int           kdecl ker_msg_writev(THREAD *act, struct kerargs_msg_writev *kap);
int           kdecl ker_msg_info(THREAD *act, struct kerargs_msg_info *kap);
int           kdecl ker_msg_deliver_event(THREAD *act, struct kerargs_msg_deliver_event *kap);
int           kdecl ker_msg_verify_event(THREAD *act, struct kerargs_msg_verify_event *kap);
int           kdecl ker_msg_keydata(THREAD *act, struct kerargs_msg_keydata *kap);
int           kdecl ker_msg_current(THREAD *act, struct kerargs_msg_current *kap);

int                 do_ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap);
int           kdecl ker_signal_kill(THREAD *act, struct kerargs_signal_kill *kap);
int           kdecl ker_signal_return(THREAD *act, struct kerargs_signal_return *kap);
int           kdecl ker_signal_action(THREAD *act, struct kerargs_signal_action *kap);
int           kdecl ker_signal_procmask(THREAD *act, struct kerargs_signal_procmask *kap);
int           kdecl ker_signal_suspend(THREAD *act, struct kerargs_signal_suspend *kap);
int           kdecl ker_signal_waitinfo(THREAD *act, struct kerargs_signal_wait *kap);
int           kdecl ker_signal_fault(THREAD *act, struct kerargs_signal_fault *kap);

int           kdecl ker_thread_create(THREAD *act, struct kerargs_thread_create *kap);
int           kdecl ker_thread_destroy(THREAD *act, struct kerargs_thread_destroy *kap);
int           kdecl ker_thread_destroyall(THREAD *act, struct kerargs_null *kap);
int           kdecl ker_thread_detach(THREAD *act, struct kerargs_thread_detach *kap);
int           kdecl ker_thread_join(THREAD *act, struct kerargs_thread_join *kap);
int           kdecl ker_thread_cancel(THREAD *act, struct kerargs_thread_cancel *kap);
int           kdecl ker_thread_ctl(THREAD *act, struct kerargs_thread_ctl *kap);
int                 kerop_thread_ctl(THREAD *act, THREAD *op, struct kerargs_thread_ctl *kap);

int           kdecl ker_interrupt_attach(THREAD *act, struct kerargs_interrupt_attach *kap);
int           kdecl ker_interrupt_detach_func(THREAD *act, struct kerargs_interrupt_detach_func *kap);
int           kdecl ker_interrupt_detach(THREAD *act, struct kerargs_interrupt_detach *kap);
int           kdecl ker_interrupt_wait(THREAD *act, struct kerargs_interrupt_wait *kap);
int           kdecl ker_interrupt_mask(THREAD *act, struct kerargs_interrupt_mask *kap);
int           kdecl ker_interrupt_unmask(THREAD *act, struct kerargs_interrupt_unmask *kap);

int           kdecl ker_sched_get(THREAD *act, struct kerargs_sched_get *kap);
int           kdecl ker_sched_set(THREAD *act, struct kerargs_sched_set *kap);
int           kdecl ker_sched_yield(THREAD *act, struct kerargs_null *kap);
int           kdecl ker_sched_info(THREAD *act, struct kerargs_sched_info *kap);

int           kdecl ker_clock_time(THREAD *act, struct kerargs_clock_time *kap);
int           kdecl ker_clock_adjust(THREAD *act, struct kerargs_clock_adjust *kap);
int           kdecl ker_clock_period(THREAD *act, struct kerargs_clock_period *kap);
int           kdecl ker_clock_id(THREAD *act, struct kerargs_clock_id *kap);

int           kdecl ker_timer_create(THREAD *act, struct kerargs_timer_create *kap);
int           kdecl ker_timer_destroy(THREAD *act, struct kerargs_timer_destroy *kap);
int           kdecl ker_timer_settime(THREAD *act, struct kerargs_timer_settime *kap);
int           kdecl ker_timer_info(THREAD *act, struct kerargs_timer_info *kap);
int           kdecl ker_timer_alarm(THREAD *act, struct kerargs_timer_alarm *kap);
int           kdecl ker_timer_timeout(THREAD *act, struct kerargs_timer_timeout *kap);

int           kdecl ker_sync_create(THREAD *act, struct kerargs_sync_create *kap);
int           kdecl ker_sync_destroy(THREAD *act, struct kerargs_sync_destroy *kap);
int           kdecl ker_sync_mutex_lock(THREAD *act, struct kerargs_sync_mutex_lock *kap);
int           kdecl ker_sync_mutex_unlock(THREAD *act, struct kerargs_sync_mutex_unlock *kap);
int			  kdecl ker_sync_mutex_revive(THREAD*act, struct kerargs_sync_mutex_revive *kap);
int           kdecl ker_sync_ctl(THREAD *act, struct kerargs_sync_ctl *kap);
int           kdecl ker_sync_condvar_wait(THREAD *act, struct kerargs_sync_condvar_wait *kap);
int           kdecl ker_sync_condvar_signal(THREAD *act, struct kerargs_sync_condvar_signal *kap);
int           kdecl ker_sync_sem_wait(THREAD *act, struct kerargs_sync_sem_wait *kap);
int           kdecl ker_sync_sem_post(THREAD *act, struct kerargs_sync_sem_post *kap);

int           kdecl ker_net_cred(THREAD *act, struct kerargs_net_cred *kap);
int           kdecl ker_net_vtid(THREAD *act, struct kerargs_net_vtid *kap);
int           kdecl ker_net_unblock(THREAD *act, struct kerargs_net_unblock *kap);
int           kdecl ker_net_infoscoid(THREAD *act, struct kerargs_net_infoscoid *kap);
int           kdecl ker_net_signal_kill(THREAD *act, struct kerargs_net_signal_kill *kap);

int           kdecl ker_mt_ctl(THREAD *act, struct kerargs_mt_ctl *kap);

int           kdecl ker_trace_event(THREAD *act, struct kerargs_trace_event *kap);

int           kdecl ker_ring0(THREAD *act, struct kerargs_ring0 *kap);

void          rdecl kererr(THREAD *thp, int err);
void          rdecl kerunerr(THREAD *thp);
int           rdecl kerisroot(THREAD *thp);
int           rdecl kerisusr(THREAD *thp, PROCESS *dst);
int           rdecl keriskill(THREAD *thp, PROCESS *dst, int signo);
int           rdecl kerschedok(THREAD *thp, int policy, const struct sched_param *param);
int                 kercall_attach(unsigned callnum, int kdecl (*func)());
void                ker_start(void);
void                sync_mutex_lock(THREAD *act, sync_t *sync, int flags);
void                sync_mutex_unlock(THREAD *act, sync_t *sync, unsigned incr);
void				mutex_holdlist_add(THREAD *, SYNC *);
void				mutex_holdlist_rem(SYNC *);
int					mutex_set_prioceiling(SYNC *syp, unsigned ceiling, unsigned *ceiling_old);
void          rdecl thread_specret(THREAD *thp);
void          rdecl signal_specret(THREAD *thp);

void				heap_init(size_t);
int					crit_sfree(void *p, unsigned size);

const struct sigevent *   clock_handler(void *, int id);
void                clock_ticker(void);
void                fpu_init(void);
struct sigevent *   fpu_exception();
struct sigevent *   fpu_switch();
int 		  rdecl begin_fp_emulation(THREAD *act);
uintptr_t 	  rdecl fpu_emulation_prep(CPU_REGISTERS *, THREAD *act, int size);

void                init_cpu(void);
void                init_traps(void);
void                init_objects(void);
void                init_memmgr(void);
void				init_smp(void);
DISPATCH *			init_scheduler_default();

void          rdecl clock_resolution(unsigned long nsec);
void				clock_start(unsigned long nsec);

void				idle(void);

void                _smpstart(void);

void          rdecl interrupt_init(void);
int           rdecl interrupt_attach( int intr, const struct sigevent *(*handler)(void *area, int id), void *area, int flags );
void          rdecl interrupt_detach( int intr, const struct sigevent *(*handler)(void *area, int id) );
void          rdecl interrupt_detach_entry(PROCESS *prp, int index);
int           rdecl interrupt_mask(int intr, INTERRUPT *itp);
int           rdecl interrupt_unmask(int intr, INTERRUPT *itp);
int           rdecl interrupt_mask_vector(unsigned vector, int id);
int           rdecl interrupt_unmask_vector(unsigned vector, int id);
int           rdecl get_interrupt_level(THREAD *act, unsigned vector);

unsigned            (xferiov_pos)(CPU_REGISTERS *regs);
int                 (xferiov)(THREAD *sthp, IOV *dst, IOV *src, int dparts, int sparts, int doff, int soff);

int                 (xfermsg)(THREAD *dthp, THREAD *sthp, int doff, int soff);
int                 (xferpulse)(THREAD *dthp, IOV *dst, int parts, uint32_t code, uint32_t value, int32_t scoid);
int                 (xferlen)(THREAD *thp, IOV *iov, int parts);
int 				(xfer_memcpy)(void *dst, const void *src, size_t len);
int 				(xfer_cpy_diov)(THREAD* thpd, IOV *dst, uint8_t *saddr, int dparts, unsigned slen);
int					(xfer_memchk)(uintptr_t bound, const IOV *iov, size_t iov_len);
void 				xfer_restart(THREAD *thp, CPU_REGISTERS *regs);
void 				xfer_async_restart(THREAD *thp, CPU_REGISTERS *regs);

void                __ker_exit(void);
int                 __pagefault_handler(int faultcode, void *faultaddr, THREAD *thp);
struct sigevent *   reboot_handler();

void                kernel_main(int argc, char *argv[], char *environ[]);

int           rdecl cpu_debug_sstep(DEBUG *dep, THREAD *thp);
int           rdecl cpu_debug_brkpt(DEBUG *dep, BREAKPT *bpp);
void          rdecl cpu_debug_attach_brkpts(DEBUG *dep);
void          rdecl cpu_debug_detach_brkpts(DEBUG *dep);
int           rdecl cpu_debug_fault(DEBUG *dep, THREAD *thp, siginfo_t *info, unsigned *pflags);
int           rdecl cpu_debug_get_altregs(THREAD *thp, debug_altreg_t *regs);
int           rdecl cpu_debug_set_altregs(THREAD *thp, debug_altreg_t *regs);
int           rdecl cpu_debug_set_perfregs(THREAD *thp, debug_perfreg_t *regs);
int           rdecl cpu_debug_get_perfregs(THREAD *thp, debug_perfreg_t *regs);

void                halt(void);

int			  rdecl within_syspage(uintptr_t vaddr, unsigned size);

int           rdecl synchash_add(OBJECT *obp, unsigned addr, SYNC *syp);
SYNC *        rdecl synchash_lookup(OBJECT *obp, unsigned addr);
void          rdecl synchash_rem(unsigned addr, OBJECT *obp, unsigned addr1, unsigned addr2, PROCESS *prp, void *vaddr);

uint32_t            exe_pt_event_h(ehandler_data_t*, uint32_t, pid_t, int, uint32_t, uint32_t);
uint32_t            exe_event_h(ehandler_data_t*, uint32_t, uint32_t, uint32_t);
uint32_t            exe_pt_event_h_buff(ehandler_data_t*, uint32_t, pid_t, int, void*, uint32_t);
uint32_t            exe_event_h_buff(int, ehandler_data_t*, uint32_t, void*, uint32_t);
int                 trace_event(uint32_t*);
void                add_ktrace_int_handler_enter(INTRLEVEL*, INTERRUPT *);
void                add_ktrace_int_handler_exit(INTRLEVEL*, const struct sigevent *);
void                add_ktrace_int_enter(INTRLEVEL*);
void                add_ktrace_int_exit(INTRLEVEL*);
void                time_em(uint32_t, uint32_t);
int                 trace_flushbuffer(void);
void                destroy_eh(PROCESS*);
void                add_trace_string(uint32_t header, const char *fmt, ...);
void				add_trace_d1_string(uint32_t header, uint32_t d_1, const char *fmt, ...);
void                add_trace_d2_string(uint32_t header, uint32_t d_1, uint32_t d_2, const char *fmt, ...);
int                 add_trace_event(unsigned code, unsigned data_0, unsigned data_1, unsigned data_2);
void                add_trace_buffer(uint32_t header, uint32_t* b_p, unsigned len);
void                add_trace_iovs(uint32_t header, IOV *iovs, unsigned iovlen);
void                add_ktrace_event(unsigned event, THREAD *thp);
void                th_em_st(THREAD* thp, uint32_t s, int cpu);
void                th_em_cd(THREAD* thp, uint32_t s, uint32_t c);
void 				th_em_name(THREAD *thp);
void                vth_em_cd(THREAD* vthp, uint32_t s, uint32_t c);
void                vth_em_st(THREAD* vthp, uint32_t s);
void                pr_em_create(PROCESS* prp, pid_t pid);
void                pr_em_destroy(PROCESS* prp, pid_t pid);
void                pr_em_create_name(PROCESS* prp);
void                pr_em_destroy_name(PROCESS* prp);
void                comm_em(THREAD* thp, CONNECT* cop, uint32_t id, uint32_t ev);
void                comm_exe_em(CONNECT* cop, uint32_t id, uint32_t ev);
void                comm_em_signal(THREAD* thp, siginfo_t* s_i);
void                trace_emit_th_state(THREAD *thp, int state);
void				trace_emit_sys_aps_name(uint32_t id, char *name);
void				trace_emit_sys_aps_budgets(uint32_t id, uint32_t percent, uint32_t critical);
void				trace_emit_sys_aps_bankruptcy(uint32_t id, pid_t pid, int32_t tid);
void				trace_emit_address(THREAD *thp, unsigned vaddr);

void                waitpage_status_get(THREAD *thp, debug_thread_t *dtp);


uint64_t	get_nsec_since_block(THREAD *thp);	//in nano_debug.c

//CPU specific things
void        cpu_syspage_init(void);
void        cpu_interrupt_init(unsigned);
void        cpu_thread_init(THREAD *act, THREAD *thp, int align);
void        cpu_thread_waaa(THREAD *thp);
void        cpu_thread_priv(THREAD *thp);
void        cpu_thread_align_fault(THREAD *thp);
void        cpu_thread_destroy(THREAD *thp);
void        cpu_signal_save(SIGSTACK *ssp, THREAD *thp);
void        cpu_signal_restore(THREAD *thp, SIGSTACK *ssp);
void        cpu_intr_attach(INTERRUPT *itp, THREAD *thp);
void        cpu_greg_load(THREAD *thp, CPU_REGISTERS *regs);
void        cpu_process_init(THREAD *thp, uintptr_t pc, uintptr_t sp);
void        cpu_reboot(void);
void        cpu_start_ap(uintptr_t start);
void        cpu_force_thread_destroyall(THREAD *thp);
void		cpu_force_fpu_save(THREAD *thp);
void 		cpu_invoke_func(THREAD *, uintptr_t);
void		cpu_mutex_adjust(THREAD *);
void		cpu_process_startup(THREAD *thp, int forking);

//Routines that get called from outside the kernel
int         outside_intr_mask(unsigned vector, int id);
int         outside_intr_unmask(unsigned vector, int id);
void        outside_intr_lock(struct intrspin *);
void        outside_intr_unlock(struct intrspin *);
int         outside_trace_event(int*);
int			outside_kd_request(union kd_request *);
unsigned	outside_vaddr_to_paddr(struct kdebug_entry *entry, uintptr_t vaddr, paddr64_t *paddr, paddr64_t *size);
int         outside_kdebug_path(struct kdebug_entry *, char *, int);


//Hook routines that the kernel calls
void		hook_idle(uint64_t *, struct qtime_entry *, INTERRUPT *);
struct sigevent *hook_trace(void *, INTERRUPT *);

int rdecl 	net_sendmsg(THREAD *act, CONNECT *cop, int prio);
int rdecl	net_send_pulse(THREAD *act, CONNECT* cop, int coid, int prio, int code, int value);
int rdecl	net_deliver_signal(THREAD *act, struct kerargs_signal_kill *kap);

unsigned	cpu_perfreg_id(void);
void rdecl	cpu_save_perfregs(void *);
void rdecl	cpu_restore_perfregs(void *);
void rdecl	cpu_free_perfregs(THREAD *thp);
void rdecl 	cpu_debug_init_perfregs(void);
//int  rdecl	cpu_alloc_perfregs(THREAD *thp);

int rdecl msgreceive_gbl(THREAD *act, CHANNELGBL *chp, void *msg, size_t size, struct _msg_info *info, CONNECT *cop, int coid);
int rdecl msgsend_gbl(THREAD *act, CONNECT *cop, void *msg, size_t size, unsigned priority, int coid);
int rdecl msgsend_async(THREAD *act, CONNECT *cop);
int rdecl msgreceive_async(THREAD *act, CHANNELASYNC *chp, iov_t *iov, unsigned parts);

int (rcvmsg)(THREAD *dthp, PROCESS *sprp, void *destp, int destparts, void *srcp, int srcparts);

#undef MMF
#define MMF(r, f, p, e)		MMF_PROTO(r, f, p, e)
MM_FUNCS(emm)

#ifdef DEBUG_PRIL
	void * rdecl pril_first(PRIL_HEAD *);
	void * rdecl pril_next(void *);
#else
	#define pril_first(ph)	((void *)(ph)->data)
	#define pril_next(p)	((void *)(p)->next.pril)
#endif
void * rdecl pril_next_prio(void *);
void * rdecl pril_find_insertion(PRIL_HEAD *, unsigned);
void rdecl pril_add(PRIL_HEAD *, void *);
void rdecl pril_rem(PRIL_HEAD *, void *);
void rdecl pril_update_register(PRIL_HEAD *, struct pril_update *);
void rdecl pril_update_unregister(PRIL_HEAD *, struct pril_update *);

/* __SRCVERSION("kerproto.h $Rev: 198590 $"); */
