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

#if !defined(KARGSLOT)
	#define KARGSLOT( arg )		arg
#endif

union kerargs {
	struct kerargs_null {
		KARGSLOT(int32_t	dummy);
	} null;

	struct kerargs_channel_create {
		KARGSLOT(int32_t	flags);
		KARGSLOT(mode_t		mode);
		KARGSLOT(size_t		bufsize);
		KARGSLOT(unsigned 	maxbuf);
		KARGSLOT(struct sigevent	*event);
		KARGSLOT(struct _cred_info	*cred);
	} channel_create;

	struct kerargs_channel_destroy {
		KARGSLOT(int32_t	chid);
	} channel_destroy;

	struct kerargs_connect_attach {
		KARGSLOT(uint32_t	nd);
		KARGSLOT(pid_t		pid);
		KARGSLOT(int32_t	chid);
		KARGSLOT(int32_t	index);
		KARGSLOT(int32_t	flags);
		KARGSLOT(void		*cd);
	} connect_attach;

	struct kerargs_connect_detach {
		KARGSLOT(int32_t	coid);
	} connect_detach;

	struct kerargs_connect_client_info {
		KARGSLOT(int32_t				scoid);
		KARGSLOT(struct _client_info	*info);
		KARGSLOT(int32_t				ngroups);
	} connect_client_info;

	struct kerargs_connect_server_info {
		KARGSLOT(pid_t					pid);
		KARGSLOT(int32_t				coid);
		KARGSLOT(struct _server_info	*info);
	} connect_server_info;

	struct kerargs_connect_flags {
		KARGSLOT(int32_t	pid);
		KARGSLOT(int32_t	coid);
		KARGSLOT(int32_t	mask);
		KARGSLOT(int32_t	bits);
	} connect_flags;

	struct kerargs_channel_connect_attr {
		KARGSLOT(int32_t				id);
		KARGSLOT(union _channel_connect_attr *old_attrs);
		KARGSLOT(union _channel_connect_attr *new_attrs);
		KARGSLOT(int32_t				flags);
	} channel_connect_attr;

	struct kerargs_msg_sendv {
		KARGSLOT(int32_t	coid);
		KARGSLOT(IOV		*smsg);
		KARGSLOT(int32_t	sparts);
		KARGSLOT(IOV		*rmsg);
		KARGSLOT(int32_t	rparts);
	} msg_sendv;

	struct kerargs_msg_receivev {
		KARGSLOT(int32_t			chid);
		KARGSLOT(IOV				*rmsg);
		KARGSLOT(int32_t			rparts);
		KARGSLOT(struct _msg_info	*info);
		KARGSLOT(int32_t			coid);
	} msg_receivev;

	struct kerargs_msg_replyv {
		KARGSLOT(int32_t	rcvid);
		KARGSLOT(int32_t	status);
		KARGSLOT(IOV		*smsg);
		KARGSLOT(int32_t	sparts);
	} msg_replyv;

	struct kerargs_msg_readv {
		KARGSLOT(int32_t	rcvid);
		KARGSLOT(IOV		*rmsg);
		KARGSLOT(int32_t	rparts);
		KARGSLOT(int32_t	offset);
	} msg_readv;

	struct kerargs_msg_writev {
		KARGSLOT(int32_t	rcvid);
		KARGSLOT(IOV		*smsg);
		KARGSLOT(int32_t	sparts);
		KARGSLOT(int32_t	offset);
	} msg_writev;

	struct kerargs_msg_readwritev {
		KARGSLOT(int32_t	src_rcvid);
		KARGSLOT(IOV		*src_msg);
		KARGSLOT(int32_t	src_parts);
		KARGSLOT(int32_t	src_offset);
		KARGSLOT(int32_t	dst_rcvid);
		KARGSLOT(IOV		*dst_msg);
		KARGSLOT(int32_t	dst_parts);
		KARGSLOT(int32_t	dst_offset);
	} msg_readwritev;

	struct kerargs_msg_readiov {
		KARGSLOT(int32_t	rcvid);
		KARGSLOT(IOV		*iov);
		KARGSLOT(int32_t	parts);
		KARGSLOT(int32_t	offset);
		KARGSLOT(int32_t	flags);
	} msg_readiov;

	struct kerargs_msg_info {
		KARGSLOT(int32_t				rcvid);
		KARGSLOT(struct _msg_info		*info);
	} msg_info;

	struct kerargs_msg_sendpulse {
		KARGSLOT(int32_t	coid);
		KARGSLOT(int32_t	priority);
		KARGSLOT(int32_t	code);
		KARGSLOT(int32_t	value);
	} msg_sendpulse;

	struct kerargs_msg_deliver_event {
		KARGSLOT(int32_t			rcvid);
		KARGSLOT(struct sigevent	*event);
	} msg_deliver_event;

	struct kerargs_msg_verify_event {
		KARGSLOT(int32_t			rcvid);
		KARGSLOT(struct sigevent	*event);
	} msg_verify_event;

	struct kerargs_msg_keydata {
		KARGSLOT(int32_t	 rcvid);
		KARGSLOT(int32_t	 op);
		KARGSLOT(uint32_t	 key);
		KARGSLOT(uint32_t	*newkey);
		KARGSLOT(IOV		*msg);
		KARGSLOT(int32_t	 parts);
	} msg_keydata;

	struct kerargs_msg_error {
		KARGSLOT(int32_t	rcvid);
		KARGSLOT(int32_t	err);
	} msg_error;

	struct kerargs_msg_current {
		KARGSLOT(int32_t	rcvid);
	} msg_current;

	struct kerargs_signal_kill {
		KARGSLOT(uint32_t	nd);
		KARGSLOT(pid_t		pid);
		KARGSLOT(int32_t	tid);
		KARGSLOT(int32_t	signo);
		KARGSLOT(int32_t	code);
		KARGSLOT(int32_t	value);
	} signal_kill;

	struct kerargs_signal_return {
		KARGSLOT(SIGSTACK 	*s);
	} signal_return;

	struct kerargs_signal_action {
		KARGSLOT(pid_t				pid);
		KARGSLOT(void				(*sigstub)());
		KARGSLOT(int32_t			signo);
		KARGSLOT(struct sigaction	*act);
		KARGSLOT(struct sigaction	*oact);
	} signal_action;

	struct kerargs_signal_procmask {
		KARGSLOT(pid_t		pid);
		KARGSLOT(int32_t	tid);
		KARGSLOT(int32_t	how);
		KARGSLOT(sigset_t	*sig_blocked);
		KARGSLOT(sigset_t	*old_sig_blocked);
	} signal_procmask;

	struct kerargs_signal_suspend {
		KARGSLOT(sigset_t	*sig_blocked);
	} signal_suspend;

	struct kerargs_signal_wait {
		KARGSLOT(sigset_t	*sig_wait);
		KARGSLOT(siginfo_t	*sig_info);
	} signal_wait;

	struct kerargs_signal_fault {
		KARGSLOT(unsigned	sigcode);
		KARGSLOT(void		*regs);
		KARGSLOT(uintptr_t	addr);
	} signal_fault;

	struct kerargs_thread_create {
		KARGSLOT(pid_t					 pid);
		KARGSLOT(void					(*func)(union sigval));
		KARGSLOT(void					*arg);
		KARGSLOT(struct _thread_attr	*attr);
	} thread_create;

	struct kerargs_thread_destroy {
		KARGSLOT(int32_t	tid);
		KARGSLOT(int32_t	priority);
		KARGSLOT(void	*status);
	} thread_destroy;

	struct kerargs_thread_detach {
		KARGSLOT(int32_t	tid);
	} thread_detach;

	struct kerargs_thread_cancel {
		KARGSLOT(int32_t	tid);
		KARGSLOT(void	(*canstub)());
	} thread_cancel;

	struct kerargs_thread_join {
		KARGSLOT(int32_t	tid);
		KARGSLOT(void	**status);
	} thread_join;

	struct kerargs_thread_ctl {
		KARGSLOT(int32_t	 cmd);
		KARGSLOT(void	*data);
	} thread_priv;

	struct kerargs_sched_get {
		KARGSLOT(pid_t					pid);
		KARGSLOT(int32_t				tid);
		KARGSLOT(struct sched_param		*param);
	} sched_get;

	struct kerargs_sched_set {
		KARGSLOT(pid_t					pid);
		KARGSLOT(int32_t				tid);
		KARGSLOT(int32_t				policy);
		KARGSLOT(struct sched_param		*param);
	} sched_set;

	struct kerargs_sched_info {
		KARGSLOT(pid_t					pid);
		KARGSLOT(int					policy);
		KARGSLOT(struct _sched_info		*info);
	} sched_info;

	struct kerargs_sched_ctl {
		KARGSLOT(int32_t	 			cmd);
		KARGSLOT(void					*data);
		KARGSLOT(size_t					length);
	} sched_ctl;

	struct kerargs_interrupt_attach {
		KARGSLOT(int32_t				intr);
		KARGSLOT(const struct sigevent	*(*handler)(void *area, int id));
		KARGSLOT(void					*area);
		KARGSLOT(int32_t				areasize);
		KARGSLOT(int32_t				flags);
	} interrupt_attach;

	struct kerargs_interrupt_detach_func {
		KARGSLOT(int32_t				intr);
		KARGSLOT(const struct sigevent	*(*handler)(void *area, int id));
	} interrupt_detach;

	struct kerargs_interrupt_detach {
		KARGSLOT(int32_t				id);
	} interrupt_detach_id;

	struct kerargs_interrupt_wait {
		KARGSLOT(int32_t			 flags);
		KARGSLOT(uint64_t			*timeout);
	} interrupt_wait;

	struct kerargs_interrupt_mask {
		KARGSLOT(int32_t			 intr);
		KARGSLOT(int32_t			 id);
	} interrupt_mask;

	struct kerargs_interrupt_unmask {
		KARGSLOT(int32_t			 intr);
		KARGSLOT(int32_t			 id);
	} interrupt_unmask;

	struct kerargs_sync_create {
		KARGSLOT(unsigned		type);
		KARGSLOT(sync_t			*sync);
		KARGSLOT(sync_attr_t	*attr);
	} sync_create;

	struct kerargs_sync_destroy {
		KARGSLOT(sync_t	*sync);
	} sync_destroy;

	struct kerargs_sync_mutex_lock {
		KARGSLOT(sync_t	*sync);
	} sync_mutex_lock;

	struct kerargs_sync_mutex_unlock {
		KARGSLOT(sync_t	*sync);
	} sync_mutex_unlock;

	struct kerargs_sync_mutex_revive {
		KARGSLOT(sync_t	*sync);
	} sync_mutex_revive;

	struct kerargs_sync_ctl {
		KARGSLOT(int cmd);
		KARGSLOT(sync_t	*sync);
		KARGSLOT(void *data);
	} sync_ctl;

	struct kerargs_sync_condvar_wait {
		KARGSLOT(sync_t	*sync);
		KARGSLOT(sync_t	*mutex);
	} sync_condvar_wait;

	struct kerargs_sync_condvar_signal {
		KARGSLOT(sync_t	*sync);
		KARGSLOT(int32_t	 all);
	} sync_condvar_signal;

	struct kerargs_sync_sem_wait {
		KARGSLOT(sync_t	*sync);
		KARGSLOT(int32_t	 try);
	} sync_sem_wait;

	struct kerargs_sync_sem_post {
		KARGSLOT(sync_t	*sync);
	} sync_sem_post;

	struct kerargs_clock_time {
		KARGSLOT(clockid_t		 	id);
		KARGSLOT(uint64_t			*new);
		KARGSLOT(uint64_t			*old);
	} clock_time;

	struct kerargs_clock_adjust {
		KARGSLOT(clockid_t			  	id);
		KARGSLOT(struct _clockadjust	*new);
		KARGSLOT(struct _clockadjust	*old);
	} clock_adjust;

	struct kerargs_clock_period {
		KARGSLOT(clockid_t				 id);
		KARGSLOT(struct _clockperiod	*new);
		KARGSLOT(struct _clockperiod	*old);
		KARGSLOT(int32_t				 external);
	} clock_period;

	struct kerargs_clock_id {
		KARGSLOT(int32_t				 pid);
		KARGSLOT(int32_t				 tid);
	} clock_id;

	struct kerargs_timer_create {
		KARGSLOT(clockid_t			 id);
		KARGSLOT(struct sigevent	 *event);
	} timer_create;

	struct kerargs_timer_destroy {
		KARGSLOT(int32_t	id);
	} timer_destroy;

	struct kerargs_timer_settime {
		KARGSLOT(timer_t				 id);
		KARGSLOT(int32_t				 flags);
		KARGSLOT(struct _itimer			*itime);
		KARGSLOT(struct _itimer			*oitime);
	} timer_settime;

	struct kerargs_timer_info {
		KARGSLOT(pid_t					 pid);
		KARGSLOT(timer_t				 id);
		KARGSLOT(int32_t				 flags);
		KARGSLOT(struct _timer_info	*info);
	} timer_info;

	struct kerargs_timer_alarm {
		KARGSLOT(clockid_t			 id);
		KARGSLOT(struct _itimer		*itime);
		KARGSLOT(struct _itimer		*otime);
	} timer_alarm;

	struct kerargs_timer_timeout {
		KARGSLOT(clockid_t			 id);
		KARGSLOT(uint32_t			 timeout_flags);
		KARGSLOT(struct sigevent	*event);
		KARGSLOT(uint64_t			*ntime);
		KARGSLOT(uint64_t			*otime);
	} timer_timeout;

	struct kerargs_ring0 {
		KARGSLOT(void		(*func)(void *));
		KARGSLOT(void		*arg);
	} ring0;

	struct kerargs_net_cred {
		KARGSLOT(int32_t				coid);
		KARGSLOT(struct _client_info	*info);
	} net_cred;

	struct kerargs_net_vtid {
		KARGSLOT(int32_t			vtid);
		KARGSLOT(struct _vtid_info	*info);
	} net_vtid;

	struct kerargs_net_unblock {
		KARGSLOT(int32_t	vtid);
	} net_unblock;

	struct kerargs_net_infoscoid {
		KARGSLOT(int32_t	scoid);
		KARGSLOT(int32_t	infoscoid);
	} net_infoscoid;

	struct kerargs_trace_event {
		KARGSLOT(uint32_t	*data);
	} trace_event;

	struct kerargs_sys_cpupage_get {
		KARGSLOT(int32_t	index);
	} sys_cpupage_get;

	struct kerargs_sys_cpupage_set {
		KARGSLOT(int32_t	index);
		KARGSLOT(intptr_t	value);
	} sys_cpupage_set;

	struct kerargs_net_signal_kill {
		KARGSLOT(void*		sigdata);
		KARGSLOT(struct _cred_info	*cred);
	} net_signal_kill;

	struct kerargs_mt_ctl {
		KARGSLOT(int            type);
		KARGSLOT(void*          data);
	} mt_init;
};

/* __SRCVERSION("kerargs.h $Rev: 153052 $"); */
