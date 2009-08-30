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





int io_open(resmgr_context_t *ctp, io_open_t *msg, MQDEV *attr, void *extra);
int io_unlink(resmgr_context_t *ctp, io_unlink_t *msg, MQDEV *attr, void *reserved);
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, struct ocb *ocb);
int io_closedup(resmgr_context_t *ctp, io_close_t *msg, struct ocb *ocb);
int io_closeocb(resmgr_context_t *ctp, void *remqved, struct ocb *ocb);
int io_read(resmgr_context_t *ctp, io_read_t *msg, struct ocb *ocb);
int io_stat(resmgr_context_t *ctp, io_stat_t *msg, struct ocb *ocb);
int io_unblock(resmgr_context_t *ctp, io_pulse_t *msg, struct ocb *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, struct ocb *ocb);
int io_notify(resmgr_context_t *ctp, io_notify_t *msg, struct ocb *ocb);

extern resmgr_io_funcs_t mq_io_funcs;
extern resmgr_io_funcs_t mq_io_dir_funcs;
extern resmgr_connect_funcs_t mq_connect_funcs;

void func_init(void);
void options(int argc, char *argv[]);
void unblock_all(resmgr_context_t *ctp, struct ocb *ocb);
void delete_msgs(MQDEV *dev);

extern void LINK_PRI_CLIENT(MQWAIT **head, MQWAIT *client);
extern void LINK_PRI_MSG(MQMSG *head[2], MQMSG *msg);

struct ocb *ocb_calloc(resmgr_context_t *ctp, MQDEV *attr);
void ocb_free(struct ocb *ocb);

extern int		MemchunkInit(struct MemchunkCtrl **memctrl, int n, const size_t cfg[]);
extern void		*MemchunkMalloc(struct MemchunkCtrl *memctrl, size_t nbytes);
extern void		*MemchunkCalloc(struct MemchunkCtrl *memctrl, int n, size_t nbytes);
extern void		MemchunkFree(struct MemchunkCtrl *memctrl, const void *ptr);
