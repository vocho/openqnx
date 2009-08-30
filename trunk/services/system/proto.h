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

struct mem_ocb;
struct _rsrc_alloc;
struct proc_mux_lock;

/* message.c */
void message_init(void);
int message_start(void);
int proc_thread_pool_reserve(void);
int proc_thread_pool_reserve_done(void);

/* support.c */
PROCESS * proc_lookup_pid(pid_t pid);
PROCESS * proc_lock_pid(pid_t pid);
PROCESS * proc_lock_parent(PROCESS *prp);
void proc_unlock(PROCESS *prp);
int proc_error(int ret, PROCESS *);
int proc_isaccess(PROCESS *prp, struct _msg_info *rcvinfo);
void *proc_object_alloc(SOUL *soulp);
void proc_object_free(SOUL *soulp, void *ptr);
int proc_status(resmgr_context_t *ctp, int status);
int rdecl proc_mux_lock(struct proc_mux_lock **mp);
int rdecl proc_mux_unlock(struct proc_mux_lock **mp);
int rdecl proc_mux_haslock(struct proc_mux_lock **mp, int owner);

/* imagefs.h */
int imagefs_mount_mounter (void);
void *imagefs_mount(paddr_t paddr, unsigned size, unsigned offset, unsigned flags, struct node_entry *root, const char *mountpoint);
int imagefs_check(const resmgr_io_funcs_t *funcs, mem_map_t *msg, void *h, OBJECT **pobp);
size_t imagefs_fname(OBJECT *, unsigned off, size_t max, char *name);

/* proc_loader.c */
void loader_exit(resmgr_context_t *hdr, union proc_msg_union *msg, PROCESS *prp);
void *loader_fork(void *lcp);
void *loader_load(void *lcp);

/* proc_termer.c */
void *terminator(void *lcp);

/* devnull.c */
void devnull_init(void);

/* devtty.c */
void devtty_init(void);

/* devstd.c */
void devstd_init(void);

/* devzero.c */
void devzero_init(void);
int devzero_check(const resmgr_io_funcs_t *funcs, mem_map_t *msg, void *h, OBJECT **pobp);

/* devmem.c */
int devmem_check(const resmgr_io_funcs_t *funcs, mem_map_t *msg, void *h, OBJECT **pobp);
int devmem_check_ocb_phys(resmgr_context_t *ctp, void *ocb);
int mem_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra);
int tymem_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra);
void devmem_init(void);

/* named semaphores */
void namedsem_init(void);

/* procfs.h */
int proc_debug_destroy(resmgr_context_t *ctp, PROCESS *prp);
void proc_debug_attach(PROCESS *prp);
void proc_debug_detach(PROCESS *prp);
void procfs_init(void);

/* devtext.c */
void devtext_init(void);

/* bootimage.c */
void bootimage_init(void);

/* $(CPU)/special.c */
void special_init(void);

/* sysmgr_init.c */
void sysmgr_init(void);

/* rsrcdbmgr_init.c */
void rsrcdbmgr_init(void);
void rsrcdbmgr_destroy_process(PROCESS *prp);
int rsrcdbmgr_proc_interface(void *list, int count, int command);
int rsrcdbmgr_proc_query(struct _rsrc_alloc *list, int count, int start, int flags);
int rsrcdbmgr_proc_devno(char *name, dev_t *devno, int minor_request, int flags);

/* rsrcdbmgr_cmd.c */
int rsrcdbmgr_proc_query_name(struct _rsrc_alloc *list, int count, int start, char *name);

/* proc_read.c */
ssize_t proc_read(int fd, void *buff, ssize_t nbytes, off64_t off);

/* loader_elf.c */
struct loader_startup;
struct stat;
int elf_load(int fd, const char *path, struct loader_startup *lsp, struct stat *stat, struct inheritance *parms);

/* __SRCVERSION("proto.h $Rev: 211988 $"); */
