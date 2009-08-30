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

#ifndef _include_APMMGR_H_
#define _include_APMMGR_H_

#include "externs.h"
#include "apmgr.h"
#include "apm.h"
#include "../memmgr/apm/apm_internal.h"
#include <kernel/event.h>
#include <fcntl.h>
#include <sys/dcmd_all.h>
#include <dirent.h>
#include <sys/iofunc.h>



extern mempart_rsrcmgr_fnctbl_t  *mempart_rsrcmgr_fnctbl;

/*==============================================================================
 * 
 * 				interfaces to the memory partition module
 * 
*/

/*******************************************************************************
 * MEMPART_CREATE
 * 
*/
#define MEMPART_CREATE(p, c, mc) \
		((mempart_rsrcmgr_fnctbl == NULL) ? part_id_t_INVALID : mempart_rsrcmgr_fnctbl->create((p), (c), (mc)))

/*******************************************************************************
 * MEMPART_DESTROY
 * 
*/
#define MEMPART_DESTROY(p) \
		((mempart_rsrcmgr_fnctbl == NULL) ? ENOSYS : mempart_rsrcmgr_fnctbl->destroy((p)))

/*******************************************************************************
 * MEMPART_CHANGE
 * 
*/
#define MEMPART_CHANGE(p, c, k) \
		((mempart_rsrcmgr_fnctbl == NULL) ? ENOSYS : mempart_rsrcmgr_fnctbl->config((p), (c), (k)))

/*******************************************************************************
 * MEMPART_GETINFO
 * 
*/
#define MEMPART_GETINFO(p, i) \
		((mempart_rsrcmgr_fnctbl == NULL) ? NULL : mempart_rsrcmgr_fnctbl->getinfo((p), (i)))

/*******************************************************************************
 * MEMPART_FINDPID
 * 
*/
#define MEMPART_FINDPID(p, mp) \
		((mempart_rsrcmgr_fnctbl == NULL) ? NULL : mempart_rsrcmgr_fnctbl->find_pid((p), (mp)))

/*******************************************************************************
 * VALIDATE_CFG_CREATION
 * VALIDATE_CFG_MODIFICATION
 * 
 * These 2 routines are used to validate configurations for partition creation
 * or modification based on the policies in use. They provide common routines
 * for validation so that these types of checks are not littered throughout the
 * code.
 *
 * VALIDATE_CFG_CREATION() requires a part_id_t for the parent partition
 * under which the new partition, described by <cfg> is proposed to be created.
 * The parent partition <pmpid> may be part_id_t_INVALID if this is a root
 * partition.
 * The configuration <cfg> can also be NULL however nothing useful will be done
 * unless a parent partition identifier is provided
 * 
 * VALIDATE_CFG_MODIFICATION requires a part_id_t for the partition to which
 * the modifications specified by <cfg> apply. The partition <mpid> must be valid
 * 
 * Both routines will return EOK if <cfg> is valid for the requested operation
 * and policies in place, otherwise and errno is returned. 
*/
#define VALIDATE_MP_CFG_CREATION(pmpid, cfg, mcid) \
		((mempart_rsrcmgr_fnctbl == NULL) ? ENOSYS : mempart_rsrcmgr_fnctbl->validate_cfg_new((pmpid), (cfg), (mcid)))

#define VALIDATE_MP_CFG_MODIFICATION(mpid, cfg) \
		((mempart_rsrcmgr_fnctbl == NULL) ? ENOSYS : mempart_rsrcmgr_fnctbl->validate_cfg_change((mpid), (cfg)))



/*==============================================================================
 * 
 * 				internal memory partition manager module types
 * 
*/

typedef struct
{
	PART_EVT_T_common;
	union {
		memsize_t		size;			// used internally for delta from value
	} evt_data;						// used internally for event specific storage
} mempart_evt_t;


typedef struct apmmgr_attr_s
{
	APMXMGR_ATTR_T_common(struct apmmgr_attr_s);

	union {
		part_id_t			mpid;
		char *				symlink;
		memclass_entry_t *	mclass_entry;
	} data;
	
	part_evtlist_t			event_list[max(NUM_MEMPART_EVTTYPES, NUM_MEMCLASS_EVTTYPES)];

} apmmgr_attr_t;

typedef struct
{
	iofunc_ocb_t	ocb;
	unsigned		create_key;
} apmmgr_ocb_t;

extern apmmgr_attr_t  *root_mpart;
extern const resmgr_io_funcs_t apmmgr_io_funcs;
extern dev_t  apmmgr_devno;

/* memory partitioning resource manager connect and I/O functions */
extern int apmmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);
extern int apmmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
extern int apmmgr_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apmmgr_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *_ocb);
extern int apmmgr_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *_ocb);
extern int apmmgr_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apmmgr_rename(resmgr_context_t *ctp, io_rename_t *msg, RESMGR_HANDLE_T *handle, io_rename_extra_t *extra);
extern int apmmgr_link(resmgr_context_t *ctp, io_link_t *msg, RESMGR_HANDLE_T *handle, io_link_extra_t *extra);
extern int apmmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apmmgr_mknod(resmgr_context_t *ctp, io_mknod_t *msg, RESMGR_HANDLE_T *handle, void *reserved);


/* memory partitioning resource manager externally accessible functions */
extern int validate_mp_association(apmmgr_attr_t *attr, struct _cred_info *cred);
extern void apmmgr_event_trigger(apmmgr_attr_t *attr, mempart_evttype_t evtype, ...);
void apmmgr_event_trigger2(apmmgr_attr_t *attr, memclass_evttype_t evtype,
								memclass_sizeinfo_t *size_info,	memclass_sizeinfo_t *prev_size_info);

/* memory partitioning resource manager internal support routines */
extern apmmgr_attr_t *mpath_find(apmmgr_attr_t *root, char *path, apmmgr_attr_t **parent);
extern apmmgr_attr_t *find_pid_in_mpart_hierarchy(pid_t pid, apmmgr_attr_t *mp);
extern int register_mclass_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb);
extern int register_mpart_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb);

// FIX ME - function tables ?
extern void apmmgr_init(char *);
extern int mpmgr_read(PROCESS *prp, apmmgr_attr_t *attr, off_t *offset, void *buf, size_t *size);
extern apmmgr_attr_t *mpmgr_getattr(char *path);
extern int mpmgr_get_st_size(apmmgr_attr_t *attr, struct stat *st);

extern int apm_deliver_event(evtdest_t *evtdest, struct sigevent *se);


#endif	/* _include_APMMGR_H_ */

__SRCVERSION("$IQ: apmmgr.h,v 1.23 $");

