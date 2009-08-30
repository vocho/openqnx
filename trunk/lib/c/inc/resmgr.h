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
 * The following are some assumptions about default values.  There
 * will always be edge cases where they are not accurate thereror:
 *
 * - _resmgr_io_table.min which is based off
 *   _RESMGR_CLIENT_MIN * _RESMGR_CLIENT_FD_MIN can
 *   be overidden with resmgr_handle_grow()
 * - _resmgr_io_table.nlists_max which is based off
 *    _RESMGR_CLIENT_FD_MAX can be overidden with
 *    resmgr_handle_max()
 */

         /* value */                  /* Assumption */
#define _RESMGR_CLIENT_FD_MIN (16)    /* Most clients will have < this */
#define _RESMGR_CLIENT_FD_MAX (1024)  /* _SC_OPEN_MAX == 1000 by default */
#define _RESMGR_CLIENT_MIN (8)        /* Most resmgrs will probably be serving < this number of simultaneous clients */

#define _RESMGR_OBJ_LOWAT (8)         /* Always try to keep this many handles and buckets around
                                         to avoid allocator thrashing when the above are conservative */

#define			_RESMGR_LINK_OTHERFUNC	0x00000001
#define			_RESMGR_LINK_DETACHWAIT 0x00000002
#define			_RESMGR_LINK_HALFOPEN   0x00000004      /* This link is not fully ready for use yet */

struct link {
	struct link							*next;             
	int									id;             /* numeric id for this item in the linked list */
	int									link_id;        /* fd connection for the pathname to proc */
	const resmgr_connect_funcs_t		*connect_funcs;
	const resmgr_io_funcs_t				*io_funcs;
	void								*handle;        /* user data handle to provide for connect functions */
	unsigned							flags;          /* _RESMGR_LINK_XXX flags */
	unsigned							count;          /* reference count on this structure */
};

struct binding {
	void								*ocb;           /* user allocated data handle (per open) */
	const resmgr_io_funcs_t				*funcs;         /* functions from the link structure */
	int									id;             /* numeric id of the link structure */
	unsigned							count;          /* reference count on this structure */
};

struct pulse_func {
	struct pulse_func					*next;
	int									code;
	void								(*func)(resmgr_context_t *ctp, int code, union sigval value, void *handle);
	void								*handle;
};

extern struct link					*_resmgr_link_list;
extern struct pulse_func 			*_resmgr_pulse_list;
extern struct _resmgr_handle_table	_resmgr_io_table;
extern pthread_key_t				_resmgr_thread_key;

extern int _resmgr_connect_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg);
extern int _resmgr_disconnect_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, int scoid);
extern int _resmgr_dup_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg);
extern void _resmgr_handler(resmgr_context_t *ctp);
extern _resmgr_func_t _resmgr_io_func(const resmgr_io_funcs_t *funcs, unsigned type);
extern int _resmgr_io_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct binding *binding);
extern struct link *_resmgr_link_alloc(void);
extern int _resmgr_link_free(int id, unsigned flags);

/* Every _resmgr_link_query() call should be followed up with a _resmgr_link_return() call */
extern struct link *_resmgr_link_query(int id, int countadj);
extern void _resmgr_link_return(struct link *link, int countadj);

extern int _resmgr_mmap_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg);
extern int _resmgr_openfd_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg);
extern void *_resmgr_thread(void *ctrl);
extern int _resmgr_unblock_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, int rcvid);
extern int _resmgr_notify_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg);
extern int _resmgr_access(resmgr_context_t *ctp, struct _msg_info *msginfo, int ioflag, unsigned key1, void *msg, int size);
extern int _resmgr_link_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct link *link, struct _msg_info *info,
			int	(*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *ocb));
extern void _resmgr_close_handler(resmgr_context_t *ctp, struct binding *binding);

int _resmgr_mount_handler(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, struct link *link, io_mount_extra_t *extra,
            int (*func)(resmgr_context_t *ctp, resmgr_iomsgs_t *msg, void *handle, void *extra));


/* __SRCVERSION("resmgr.h $Rev: 153052 $"); */
