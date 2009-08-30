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

#ifndef _include_APMGR_H_
#define _include_APMGR_H_

#include "externs.h"
#include "ap.h"
#include <kernel/event.h>
#include <fcntl.h>
#include <sys/dcmd_all.h>
#include <dirent.h>
#include <sys/iofunc.h>



/*==============================================================================
 * 
 * 				internal partition manager module types
 * 
*/

/*
 * These structure are no loner required as this resource manager
 * has been modified to eliminate the need for an iofunc_attr_t
 * structure on the /proc/<pid>. The only saved information was the
 * 'pid_t' (from /proc/<pid>) and that is no stored in the ocb.
 * I leave the definitions here for reference ... just in case
typedef struct
{
	iofunc_attr_t	attr;
	pid_t			pid;
} pid_attr_t;

typedef struct pid_attr_entry_s
{
	part_qnode_t		hdr;
	pid_attr_t			pid_attr;
} pid_attr_entry_t;
*/

/*
 * part_type_t
*/
typedef enum
{
	part_type_ROOT,
	part_type_MEMPART_REAL,		// a real memory partition
	part_type_MEMPART_PSEUDO,	// a pseudo memory partition (symlink to a real partition)
	part_type_SCHEDPART_REAL,	// a real scheduler partition
	part_type_SCHEDPART_PSEUDO,	// a pseudo scheduler partition (symlink to a real partition)
	part_type_GROUP,			// a directory containing pseudo partitions only
	part_type_MEMCLASS,
} part_type_t;

#define APMXMGR_ATTR_T_common(_base_type_t_) \
		part_qnode_t			hdr;	/* sibling list */ \
		iofunc_attr_t			attr; \
		const char * 			name; \
		part_type_t				type; \
		_base_type_t_ *			parent; \
		part_qnodehdr_t			children

typedef struct apxmgr_attr_s
{
	APMXMGR_ATTR_T_common(struct apxmgr_attr_s);
} apxmgr_attr_t;


typedef enum
{
apmgr_type_first = 10,

	apmgr_type_NONE,		// /proc/<pid>/
	apmgr_type_PART,		// /partition
	apmgr_type_PROC_PART,	// /proc/<pid>/partition/
	apmgr_type_MEM,			// /proc/<pid>/partition/mem/..
	apmgr_type_SCHED,		// /proc/<pid>/partition/sched/..
	apmgr_type_NAME,		// /partition/..

apmgr_type_space,
apmgr_type_last = apmgr_type_space-1,
} apmgr_type_t;

typedef struct apmgr_attr_s
{
	APMXMGR_ATTR_T_common(struct apmgr_attr_s);
	char *symlink;	// used for pseudo partition names
} apmgr_attr_t;

typedef struct
{
	iofunc_ocb_t	ocb;
	apmgr_type_t	apmgr_type;
	pid_t			pid;
	apmgr_attr_t *	attr;
} apmgr_ocb_t;

/*
 * part_evt_t
*/
#define PART_EVT_T_common \
		part_qnode_t		hdr; \
		part_qnodehdr2_t	*onlist; \
		struct evt_info_s	evt_reg;	/* the event that was registered for */ \
		evtdest_t			evt_dest; \
		void *				ocb;		/* the ocb that that attached the event */ \
		unsigned			undeliverable_count; \
		volatile unsigned	inuse

typedef struct
{
	PART_EVT_T_common;
} part_evt_t;

/*
 * GET_PART_ATTR
 * 
 * macro to extract a pointer to the relevant 'apxmgr_attr_t' from an ocb
*/
#define GET_PART_ATTR(_ocb_) \
		((apxmgr_attr_t *)((unsigned int)(((iofunc_ocb_t *)(_ocb_))->attr) - offsetof(apxmgr_attr_t, attr)))

/*
 * Attribute locking/unlocking macros
*/
#define PART_ATTR_LOCK(a)		(((a) == NULL) ? EOK : iofunc_attr_lock(&((a)->attr)))
#define PART_ATTR_UNLOCK(a)		if ((a) != NULL) (void)iofunc_attr_unlock(&((a)->attr))

extern int apmgr_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved);
extern int apmgr_proc_open(resmgr_context_t *ctp, io_open_t *msg, void *extra, void *reserved);
extern int apmgr_unlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *reserved);
extern int apmgr_proc_unlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *reserved);
extern int apmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apmgr_rename(resmgr_context_t *ctp, io_rename_t *msg, RESMGR_HANDLE_T *handle, io_rename_extra_t *extra);
extern int apmgr_mknod(resmgr_context_t *ctp, io_mknod_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apmgr_link(resmgr_context_t *ctp, io_link_t *msg, void *attr, io_link_extra_t *extra);

extern int apmgr_close(resmgr_context_t *ctp, void *reserved, void *_ocb);
extern int apmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb);
extern int apmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *ocb);
extern int apmgr_read(resmgr_context_t *ctp, io_read_t *msg, void *_ocb);
extern int apmgr_chown(resmgr_context_t *ctp, io_chown_t *msg, void *_ocb);
extern int apmgr_chmod(resmgr_context_t *ctp, io_chmod_t *msg, void *_ocb);
extern int apmgr_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *_ocb);
extern int apxmgr_chown(resmgr_context_t *ctp, io_chown_t *msg, RESMGR_OCB_T *_ocb);
extern int apxmgr_chmod(resmgr_context_t *ctp, io_chmod_t *msg, RESMGR_OCB_T *_ocb);
extern int apxmgr_lseek(resmgr_context_t *ctp, io_lseek_t *msg, RESMGR_OCB_T *_ocb);

/* common partitioning resource manager connect and I/O functions */
extern int apmgr_utime(resmgr_context_t *ctp, io_utime_t *msg, RESMGR_OCB_T *ocb);
extern int apmgr_unblock(resmgr_context_t *ctp, io_pulse_t *msg, RESMGR_OCB_T *_ocb);
extern int apmgr_openfd(resmgr_context_t *ctp, io_openfd_t *msg, RESMGR_OCB_T *_ocb);
extern int apmgr_fdinfo(resmgr_context_t *ctp, io_fdinfo_t *msg, RESMGR_OCB_T *_ocb);

extern bool isProcessPath(char *path, pid_t *pid_p);
extern apmgr_attr_t *npath_find(apmgr_attr_t *root, char *path, apmgr_attr_t **parent);
//extern pid_attr_entry_t *find_pidentry(pid_t pid);
extern bool nametoolong(const char *name, unsigned chktype, void *apmgrtype);
extern void clean_inactive_list(part_qnodehdr2_t *evlist);
extern int unregister_events(apxmgr_attr_t *p, iofunc_ocb_t *ocb, int num_evts);
void deactivate_event(part_evtlist_t *evlist, part_evt_t  *event);


extern dev_t  apmgr_devno;
extern const resmgr_io_funcs_t apmgr_io_funcs;
extern part_qnodehdr_t		pid_entry_list;
extern iofunc_attr_t 	dummy_apmgr_attr;
extern dev_t  apmmgr_devno;
extern dev_t  apsmgr_devno;
extern apmgr_attr_t  *root_npart;

/*==============================================================================
 * 
 * 			interfaces to the memory partition resource manager module
 * 
*/

/*******************************************************************************
 * APMMGR_INIT
 * 
*/
#define APMMGR_INIT(path) \
		apmmgr_init((path))

/*******************************************************************************
 * APMMGR_GETATTR
 * 
 * Given <path> (which must be below the "/partition/mem" namespace) return
 * the corresponding 'mempart_attr_t' structure for the last entry or NULL
 * if the path does not exist
*/
#define APMMGR_GETATTR(path)	mpmgr_getattr((path))
		
/*******************************************************************************
 * APMMGR_READ
 * 
 * Call the memory partitioning module to perform a read operation at offset <o>
 * for the entry identified by attribute structure <a>. The read should be
 * filtered based on PROCESS <p> and the contents read placed into buffer <d>.
 * The size of buffer <d> is pointed to by <n>.
 * 
 * Returns: EOK or an errno. If EOK is returned, <n> will point to the number of
 * 			bytes placed into <d> (which will never be greater than the original
 * 			value pointed to by <n>)
*/
#define APMMGR_READ(p, a, o, d, n)	mpmgr_read((p), (a), (o), (d), (n))
		

/*==============================================================================
 * 
 * 			interfaces to the scheduling partition resource manager module
 * 
*/
// FIX ME - function tables ?
extern void apsmgr_init(char *);

/*******************************************************************************
 * APSMGR_INIT
 * 
*/
#define APSMGR_INIT(path) \
		apsmgr_init((path))


/*******************************************************************************
 * APSMGR_GETATTR
 * 
 * Given <path> (which must be below the "/partition/sched" namespace) return
 * the corresponding 'schedpart_attr_t' structure for the last entry
*/
#define APSMGR_GETATTR(path)	spmgr_getattr((path))
		
/*******************************************************************************
 * APSMGR_READ
 * 
 * Call the scheduling partitioning module to perform a read operation at offset
 * <o> for the entry identified by attribute structure <a>. The read should be
 * filtered based on PROCESS <p> and the contents read placed into buffer <d>.
 * The size of buffer <d> is pointed to by <n>.
 * 
 * Returns: EOK or an errno. If EOK is returned, <n> will point to the number of
 * 			bytes placed into <d> (which will never be greater than the original
 * 			value pointed to by <n>)
*/
#define APSMGR_READ(p, a, o, d, n)	spmgr_read((p), (a), (o), (d), (n))


/* partitioning resource manager support routines */
extern apxmgr_attr_t *path_find(apmgr_type_t type, apxmgr_attr_t *root, char *path, apxmgr_attr_t **parent);
extern int check_access_perms(resmgr_context_t *ctp, apxmgr_attr_t *attr, mode_t check,
								struct _client_info *info, bool recurse);
extern bool isStringOfDigits(char *part_name);


#endif	/* _include_APMGR_H_ */

__SRCVERSION("$IQ: apmgr.h,v 1.23 $");

