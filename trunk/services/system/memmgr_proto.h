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

/* memmgr_init.c */
void memmgr_init(void);
int	 memmgr_resize(OBJECT *obp, size_t size);
extern const struct object_funcs	mem_funcs;

/* memmgr_map.c */
int memmgr_find_object(resmgr_context_t *ctp, PROCESS *prp, int fd, mem_map_t *msg, OBJECT **obpp);
int memmgr_map(resmgr_context_t *ctp, PROCESS *prp, mem_map_t *msg);

/* memmgr_ctrl.c */
int memmgr_ctrl(PROCESS *prp, mem_ctrl_t *msg);

/* memmgr_info.c */
int memmgr_info(resmgr_context_t *ctp, PROCESS *prp, mem_info_t *msg);

/* memmgr_offset.c */
int memmgr_offset(resmgr_context_t *ctp, PROCESS *prp, mem_offset_t *msg);

/* memmgr_debug_info.c */
int memmgr_debug_info(resmgr_context_t *ctp, PROCESS *prp, mem_debug_info_t *msg);

/* memmgr_fd.c */
void memmgr_fd_init(void);
int memmgr_open_fd(resmgr_context_t *ctp, PROCESS *, mem_map_t *msg, OBJECT **obp);
int memmgr_fd_compact(void);
int memmgr_fd_setname(OBJECT *obp, char *name);

/* memmgr_swap.c */
struct _swap_page_list;
int memmgr_swap(resmgr_context_t *ctp, PROCESS *prp, mem_swap_t *msg);
int memmgr_swapout(struct _swap_page_list *list, int n, void *obj);
int memmgr_swapin(struct _swap_page_list *list, int n, void *obj);
int memmgr_swaprem(struct _swap_page_list *list, int n, void *obj);
int memmgr_swap_freemem(pid_t pid, unsigned size, unsigned flags);

/* memmgr_support.c */
int rdecl proc_rlock_adp(PROCESS *);
int rdecl proc_wlock_adp(PROCESS *);
int rdecl proc_unlock_adp(PROCESS *);
int rdecl proc_rlock_promote_adp(PROCESS *);
void rdecl proc_lock_owner_mark(PROCESS *);
int rdecl proc_lock_owner_check(PROCESS *, pid_t, unsigned);

int memmgr_tymem_open(const char *path, int oflags, OBJECT **obpp, PROCESS *prp);
void memmgr_tymem_close(OBJECT *obp);


/* __SRCVERSION("memmgr_proto.h $Rev: 160168 $"); */
