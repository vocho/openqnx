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




#include <fcntl.h>
#include <sys/iofunc.h>

/*
 *  64-bit fields (offset, nbytes) only visible if client has opened
 *  O_LARGEFILE and the filesystem module is not IOFUNC_MOUNT_32BIT.
 */
#define IS32BIT(_attr, _ioflag) ((!((_ioflag) & O_LARGEFILE)) || ((_attr)->mount != NULL && (_attr)->mount->flags & IOFUNC_MOUNT_32BIT))

/*
 *  Use the bottom bit of a pointer (malloc will be word-aligned) as
 *  the lock/in-use/exclusive status (for attr lock_list).
 */
#define PTR_ISLOCKED(_ptr)		((uintptr_t)(_ptr) & 0x01)
#define PTR_LOCK(_ptr)			((_ptr) = (void *)((uintptr_t)(_ptr) |  0x01))
#define PTR_UNLOCK(_ptr)		((_ptr) = (void *)((uintptr_t)(_ptr) & ~0x01))
#define PTR_VALUE(_ptr, _type)	(_type *)((uintptr_t)(_ptr) & ~0x01)
#define PTR_WCHAN(_ptr)			(&(_ptr))

struct _iofunc_mmap_list {
	struct _iofunc_mmap_list        *next;
	int                             scoid;
	int                             coid;
	iofunc_ocb_t					*ocb;
};

struct _iofunc_lock_blocked {
	struct _iofunc_lock_blocked		*next;
	int								rcvid;
	off64_t							start;
	off64_t							end;
	flock_t							*pflock;
};

struct _iofunc_lock_list {
	struct _iofunc_lock_list		*next;
	//struct _iofunc_lock_list		**prev;		//UNUSED
	int								zero2;
	short							type;		// F_RDLCK or F_WRLCK
	unsigned short					zero[3];
	//int							rcvid;		//UNUSED
	struct _iofunc_lock_blocked		*blocked;	// blocked list
	int								scoid;
	off64_t							start;		// start of locked area
	off64_t							end;		// end of locked area
};

int _iofunc_lock(resmgr_context_t *ctp, iofunc_lock_list_t **list, int type, off64_t start, off64_t end);
int _iofunc_unlock_scoid(iofunc_lock_list_t **list, int scoid, off64_t start, off64_t end);
void _iofunc_lock_free(iofunc_lock_list_t *lock);
iofunc_lock_list_t *_iofunc_lock_find(iofunc_lock_list_t *list, int scoid, int type, off64_t start, off64_t end);
int _iofunc_isnonblock(int ioflag, int xtype);
int _iofunc_create(resmgr_context_t *ctp, iofunc_attr_t *attr, mode_t *mode, iofunc_mount_t *mount, struct _client_info *info);
int _iofunc_open(resmgr_context_t *ctp, io_open_t *msg, iofunc_attr_t *attr, iofunc_attr_t *dattr, struct _client_info *info);

int _iofunc_llist_lock(iofunc_attr_t *attr);
int _iofunc_llist_unlock(iofunc_attr_t *attr);

extern struct _sleepon_handle _iofunc_sleepon_default;

/* __SRCVERSION("iofunc.h $Rev: 153052 $"); */
