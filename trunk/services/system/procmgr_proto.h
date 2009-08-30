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

/* sysmgr_cmd.c */
int sysmgr_cmd(resmgr_context_t *ctp, sys_cmd_t *msg);

/* sysmgr_conf.c */
int sysmgr_conf(resmgr_context_t *ctp, sys_conf_t *msg);
int sysmgr_conf_set(pid_t pid, int cmd, int name, long value, const char *str);
void sysmgr_conf_destroy(PROCESS *prp);

/* procmgr_getsetid.c */
int procmgr_getsetid(resmgr_context_t *ctp, proc_getsetid_t *msg);

/* procmgr_init.c */
char *procmgr_init_objects(void);
void procmgr_init(void);

/* procmgr_misc.c */
struct loader_context *procmgr_context_alloc(unsigned msgsize, int state);
void procmgr_context_free(struct loader_context *lcp);
void procmgr_context_wait(PROCESS *prp);
void procmgr_thread_attr(struct _thread_attr *attr, struct loader_context *lcp, resmgr_context_t*ctp);

/* procmgr_setpgid.c */
int procmgr_setpgid(resmgr_context_t *ctp, proc_setpgid_t *msg);

/* procmgr_fork.c */
int procmgr_fork(resmgr_context_t *ctp, proc_fork_t *msg);

/* procmgr_spawn.c */
int procmgr_spawn(resmgr_context_t *ctp, void *vmsg, proc_create_attr_t *partlist);
struct loader_context *procmgr_exec(resmgr_context_t *ctp, void *msg, PROCESS *prp);

/* procmgr_pspawn.c (POSIX) */
int procmgr_pspawn(resmgr_context_t *ctp, void *vmsg);
int inherit_mempart_list(PROCESS *prp, part_list_t **part_list);
int inherit_schedpart_list(PROCESS *prp, part_list_t **part_list);

/* procmgr_termer.c */
int procmgr_termer(message_context_t *ctp, int code, unsigned flags, void *handle);

/* procmgr_coredump.c */
int procmgr_coredump(message_context_t *ctp, int code, unsigned flags, void *handle);

/* procmgr_stack.c */
int procmgr_stack(message_context_t *ctp, int code, unsigned flags, void *handle);
void procmgr_stack_executable(int enable);

/* procmgr_guardian.c */
int procmgr_msg_guardian(resmgr_context_t *ctp, proc_guardian_t *msg);

/* procmgr_resource.c */
int procmgr_msg_resource(resmgr_context_t *ctp, void *msg);

/* procmgr_session.c */
int procmgr_sleader_detach(PROCESS *prp);
int procmgr_msg_session(resmgr_context_t *ctp, proc_session_t *msg);

/* procmgr_daemon.c */
int procmgr_msg_daemon(resmgr_context_t *ctp, proc_daemon_t *msg);

/* procmgr_event.c */
void procmgr_event_destroy(PROCESS *prp);
int procmgr_event(resmgr_context_t *ctp, proc_event_t *msg);
void procmgr_trigger(unsigned flags);

/* procmgr_umask.c */
int procmgr_umask(resmgr_context_t *ctp, proc_umask_t *msg);

/* procmgr_wait.c */
struct wait_entry;
int procmgr_wait_check(PROCESS *prp, PROCESS *parent, struct wait_entry *wap, int event);
int procmgr_wait(resmgr_context_t *ctp, proc_wait_t *msg);
int procmgr_stop_or_cont(message_context_t *ctp, int code, unsigned flags, void *handle);
void procmgr_nozombie(PROCESS *prp, int status);

/* __SRCVERSION("procmgr_proto.h $Rev: 174816 $"); */
