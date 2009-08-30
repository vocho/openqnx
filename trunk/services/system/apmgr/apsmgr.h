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

#ifndef _include_APSMGR_H_
#define _include_APSMGR_H_

#include "externs.h"
#include "apmgr.h"
#include "aps.h"
#include <kernel/event.h>
#include <fcntl.h>
#include <sys/dcmd_all.h>
#include <dirent.h>
#include <sys/iofunc.h>



extern schedpart_rsrcmgr_fnctbl_t  *schedpart_rsrcmgr_fnctbl;

/*==============================================================================
 * 
 * 				interfaces to the scheduler partition module
 * 
*/

/*******************************************************************************
 * SCHEDPART_CREATE
 * 
*/
#define SCHEDPART_CREATE(p, n, c) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? part_id_t_INVALID : schedpart_rsrcmgr_fnctbl->create((p), (n), (c)))

/*******************************************************************************
 * SCHEDPART_DESTROY
 * 
*/
#define SCHEDPART_DESTROY(p) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? ENOSYS : schedpart_rsrcmgr_fnctbl->destroy((p)))

/*******************************************************************************
 * SCHEDPART_CHANGE
 * 
*/
#define SCHEDPART_CHANGE(p, c, k) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? ENOSYS : schedpart_rsrcmgr_fnctbl->config((p), (c), (k)))

/*******************************************************************************
 * SCHEDPART_GETINFO
 * 
*/
#define SCHEDPART_GETINFO(p, i) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? NULL : schedpart_rsrcmgr_fnctbl->getinfo((p), (i)))

/*******************************************************************************
 * SCHEDPART_FINDPID
 * 
*/
#define SCHEDPART_FINDPID(p, sp) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? NULL : schedpart_rsrcmgr_fnctbl->find_pid((p), (sp)))

/*******************************************************************************
 * VALIDATE_SP_CFG_CREATION
 * VALIDATE_SP_CFG_MODIFICATION
 * 
 * These 2 routines are used to validate configurations for partition creation
 * or modification based on the policies in use. They provide common routines
 * for validation so that these types of checks are not littered throughout the
 * code.
 *
 * VALIDATE_CFG_CREATION() requires a part_id_t for the parent partition
 * under which the new partition, described by <cfg> is proposed to be created.
 * The parent partition <pspid> may be part_id_t_INVALID if this is a root
 * partition.
 * The configuration <cfg> can also be NULL however nothing useful will be done
 * unless a parent partition identifier is provided
 * 
 * VALIDATE_CFG_MODIFICATION requires a part_id_t for the partition to which
 * the modifications specified by <cfg> apply. The partition <spid> must be valid
 * 
 * Both routines will return EOK if <cfg> is valid for the requested operation
 * and policies in place, otherwise and errno is returned. 
*/
#define VALIDATE_SP_CFG_CREATION(pspid, cfg) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? ENOSYS : schedpart_rsrcmgr_fnctbl->validate_cfg_new((pspid), (cfg)))

#define VALIDATE_SP_CFG_MODIFICATION(spid, cfg) \
		((schedpart_rsrcmgr_fnctbl == NULL) ? ENOSYS : schedpart_rsrcmgr_fnctbl->validate_cfg_change((spid), (cfg)))



/*==============================================================================
 * 
 * 				internal scheduling partition manager module types
 * 
*/

typedef struct
{
	PART_EVT_T_common;
} schedpart_evt_t;


typedef struct apsmgr_attr_s
{
	APMXMGR_ATTR_T_common(struct apsmgr_attr_s);

	union {
		part_id_t			spid;
		char *				symlink;
	} data;
	
	part_evtlist_t			event_list[NUM_SCHEDPART_EVTTYPES];	

} apsmgr_attr_t;

typedef struct
{
	iofunc_ocb_t	ocb;
	unsigned		create_key;
} apsmgr_ocb_t;

extern apsmgr_attr_t  *root_spart;
extern const resmgr_io_funcs_t apsmgr_io_funcs;
extern dev_t  apsmgr_devno;

/* scheduler partitioning resource manager connect and I/O functions */
extern int apsmgr_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);
extern int apsmgr_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
extern int apsmgr_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apsmgr_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *_ocb);
extern int apsmgr_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *_ocb);
extern int apsmgr_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apsmgr_rename(resmgr_context_t *ctp, io_rename_t *msg, RESMGR_HANDLE_T *handle, io_rename_extra_t *extra);
extern int apsmgr_link(resmgr_context_t *ctp, io_link_t *msg, RESMGR_HANDLE_T *handle, io_link_extra_t *extra);
extern int apsmgr_readlink(resmgr_context_t *ctp, io_readlink_t *msg, RESMGR_HANDLE_T *handle, void *reserved);
extern int apsmgr_mknod(resmgr_context_t *ctp, io_mknod_t *msg, RESMGR_HANDLE_T *handle, void *reserved);

/* scheduler partitioning resource manager externally accessible functions */
extern int validate_sp_association(apsmgr_attr_t *attr, struct _cred_info *cred);
extern void apsmgr_event_trigger(apsmgr_attr_t *attr, schedpart_evttype_t evtype, ...);
void apsmgr_event_trigger2(apsmgr_attr_t *attr, memclass_evttype_t evtype,
								memclass_sizeinfo_t *size_info,	memclass_sizeinfo_t *prev_size_info);

/* scheduler partitioning resource manager internal support routines */
extern apsmgr_attr_t *spath_find(apsmgr_attr_t *root, char *path, apsmgr_attr_t **parent);
extern apsmgr_attr_t *find_pid_in_spart_hierarchy(pid_t pid, apsmgr_attr_t *mp);
extern int register_spart_event(struct evt_info_s *evt, evtdest_t *evtdest, void *ocb);

// FIX ME - function tables ?
extern int spmgr_read(PROCESS *prp, apsmgr_attr_t *attr, off_t *offset, void *buf, size_t *size);
extern apsmgr_attr_t *spmgr_getattr(char *path);
extern int spmgr_get_st_size(apsmgr_attr_t *attr, struct stat *st);


#endif	/* _include_APSMGR_H_ */

__SRCVERSION("$IQ: apsmgr.h,v 1.23 $");

