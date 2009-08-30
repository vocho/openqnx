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


struct object_funcs;

/* pathmgr_init.c */
void pathmgr_init(void);
int pathmgr_link(const char *path, uint32_t nd, pid_t pid, int chid, unsigned handle, enum _file_type file_type, unsigned flags);

/* pathmgr_link.c */
extern const struct object_funcs		server_funcs;
struct symlink_object;
int reply_symlink(resmgr_context_t *ctp, unsigned eflag, struct _io_connect_link_reply *link, struct symlink_object *symlink, const char *path, const char *tail);
int pathmgr_dolink(resmgr_context_t *ctp, io_link_t *msg, void *handle, io_link_extra_t *extra);
int pathmgr_node2fullpath(NODE* node, char *path, int pathmax);

/* pathmgr_open.c */
int pathmgr_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra);
int pathmgr_dounlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *extra);
int pathmgr_domknod(resmgr_context_t *ctp, io_mknod_t *msg, void *handle, void *reserved);
int pathmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, void *handle, void *extra);

/* pathmgr_remote.c */
int pathmgr_remote(resmgr_context_t *ctp, struct _io_connect *connect);

/* pathmgr_resolve.c */
int pathmgr_resolve(resmgr_context_t *ctp, struct _io_connect *connect);
struct node_entry *pathmgr_resolve_path(struct node_entry *node, struct _io_connect *connect, char *path, struct node_entry *root);
int pathmgr_resolve_servers(resmgr_context_t *ctp, struct node_entry *node, struct _io_connect *connect, const char *path, struct node_entry *root);

NODE *pathmgr_node_alloc(const char *name, unsigned len);
NODE *pathmgr_node_lookup(NODE *nop, const char *path, unsigned flags, const char **result);
void pathmgr_node_detach(NODE *nop);
void pathmgr_node_detach_flags(NODE *nop, unsigned);
void pathmgr_node_unlink(NODE *nop);
void pathmgr_node_access(NODE *node);
void pathmgr_node_complete(NODE *node);
NODE *pathmgr_node_clone(NODE *node);
pid_t pathmgr_netmgr_pid(void);

OBJECT *pathmgr_object_attach(PROCESS *prp, NODE *nop, const char *path, int type, unsigned flags, void *data);
void pathmgr_object_detach(OBJECT *obp);
size_t pathmgr_object_pathname(OBJECT *obp, size_t max, char *dest);
OBJECT *pathmgr_object_clone(OBJECT *);
void pathmgr_object_unclone(OBJECT *);
void pathmgr_object_done(OBJECT *);

OBJECT *object_create(int type, void *extra, PROCESS *prp, memclass_id_t mcid);
int object_done(OBJECT *);
size_t object_name(OBJECT *, size_t, char *);
int object_devctl(resmgr_context_t *, io_devctl_t *, OBJECT *);

/* __SRCVERSION("pathmgr_proto.h $Rev: 198777 $"); */
