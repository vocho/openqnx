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

/*==============================================================================
 * 
 * apsmgr_init
 * 
 * Provide resource manager initialization for the scheduler partitioning module
 * 
*/

#include "apsmgr.h"
#include <sys/sched_aps.h>

static apsmgr_attr_t  _root_spart;

static apsmgr_attr_t  _sys_spart;
static apsmgr_attr_t  *sys_spart = &_sys_spart;

static rsrcmgr_schedpart_fnctbl_t  rsrcmgr_schedpart_fnctbl =
{
	STRUCT_FLD(validate_association) validate_sp_association,
//	STRUCT_FLD(spart_event_handler) apsmgr_event_trigger,
};

static const resmgr_connect_funcs_t apsmgr_connect_funcs =
{
	STRUCT_FLD(nfuncs) _RESMGR_CONNECT_NFUNCS,
	STRUCT_FLD(open) apsmgr_open,
	STRUCT_FLD(unlink) apsmgr_unlink,
	STRUCT_FLD(rename) apsmgr_rename,				// this line needed for watcom compiler
	STRUCT_FLD(mknod) apsmgr_mknod,
	STRUCT_FLD(readlink) apsmgr_readlink,
	STRUCT_FLD(link) apsmgr_link,
};

/* needs to be externally visible to apsmgr_open() */
const resmgr_io_funcs_t apsmgr_io_funcs =
{
	STRUCT_FLD(nfuncs) _RESMGR_IO_NFUNCS,
	STRUCT_FLD(read) apsmgr_read,
	STRUCT_FLD(write) NULL,					// this line needed for watcom compiler
	STRUCT_FLD(close_ocb) apsmgr_close,
	STRUCT_FLD(stat) apsmgr_stat,
	STRUCT_FLD(notify) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(devctl) apsmgr_devctl,
	STRUCT_FLD(unblock) apmgr_unblock,
	STRUCT_FLD(pathconf) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(lseek) apxmgr_lseek,
	STRUCT_FLD(chmod) apxmgr_chmod,
	STRUCT_FLD(chown) apxmgr_chown,
	STRUCT_FLD(utime) apmgr_utime,
	STRUCT_FLD(openfd) apmgr_openfd,
	STRUCT_FLD(fdinfo) apmgr_fdinfo,
};

/*
 * schedpart_rsrcmgr_fnctbl
 * 
 * Pointer to the table of scheduler partition interface routines required by the
 * scheduler partitioning resource manager.
*/
schedpart_rsrcmgr_fnctbl_t  *schedpart_rsrcmgr_fnctbl = NULL;
apsmgr_attr_t  *root_spart = &_root_spart;
dev_t  apsmgr_devno;

#define strcat(d, s)	STRLCPY(&d[strlen(d)], s, strlen(s) + 1)

/*******************************************************************************
 * _mpmgr_st_size
 * 
 * FIX ME
 * This is a support routine to get the size of free scheduler from a partitioning
 * perspective (ie. a processes "world view" of free scheduler is the free scheduler
 * in the hierarchy of partitions for its associated sysram scheduler class.
*/
extern int (*spmgr_st_size)(void *attr, memsize_t *size);
static int _spmgr_st_size(void *attr, memsize_t *size)
{
	extern int spmgr_get_st_size(apsmgr_attr_t *attr, struct stat *st);
	struct stat  st;
	int r = spmgr_get_st_size(attr, &st);
	if (r == EOK)
		*size = st.st_size;
	return r;
}

void apsmgr_init(char *basepath)
{
	char name[PATH_MAX + 1];
	resmgr_attr_t  attr;

	/*
	 * if the scheduler partitioning module is not installed, we do not enable
	 * the scheduler partitioning resource manager code
	*/ 
	if ((!SCHEDPART_INSTALLED()) || (schedpart_fnctbl->rsrcmgr_attach == NULL) ||
		((schedpart_rsrcmgr_fnctbl = schedpart_fnctbl->rsrcmgr_attach(&rsrcmgr_schedpart_fnctbl)) == NULL))
	{
#ifndef NDEBUG
		if (ker_verbose) kprintf("Scheduler Partitioning module not installed\n");
#endif	/* NDEBUG */
		return;
	}
	/* fill in the (*spmgr_st_size) function pointer */
	spmgr_st_size = _spmgr_st_size;

	STRLCPY(name, basepath, sizeof(name));
	if ((strlen(name) + strlen("/sched")) >= sizeof(name))
		return;

	strcat(name, "/sched");

    /* initialize resource manager attributes */
    memset(&attr, 0, sizeof(attr));
    attr.nparts_max = 1;
    attr.msg_max_size = 2048;
	attr.flags |= _RESMGR_FLAG_DIR;

#ifdef USE_RESMGR_FUNC_DEFAULTS
	/* initialize functions for handling messages */
	apsmgr_io_funcs.nfuncs = _RESMGR_IO_NFUNCS;
	apsmgr_connect_funcs.nfuncs = _RESMGR_CONNECT_NFUNCS;
	iofunc_func_init(apsmgr_connect_funcs.nfuncs, &apsmgr_connect_funcs,
					 apsmgr_io_funcs.nfuncs, &apsmgr_io_funcs);

	/* connect func overrides */
	apsmgr_connect_funcs.open = apsmgr_open;
	apsmgr_connect_funcs.unlink = apsmgr_unlink;
//	apsmgr_connect_funcs.readlink = apsmgr_read;
	apsmgr_connect_funcs.link = apsmgr_link;
	apsmgr_connect_funcs.readlink = apsmgr_readlink;
	apsmgr_connect_funcs.mknod = apsmgr_mknod;

	/* io_func overrides */
	apsmgr_io_funcs.read = apsmgr_read;
	apsmgr_io_funcs.close_ocb = apsmgr_close;
	apsmgr_io_funcs.stat = apsmgr_stat;
	apsmgr_io_funcs.devctl = apsmgr_devctl;
#endif	/* USE_IO_FUNC_DEFAULTS */

	// allocate schedpart directory entry
	resmgr_attach(dpp, &attr, name, _FTYPE_ANY, _RESMGR_FLAG_DIR | _RESMGR_FLAG_BEFORE, &apsmgr_connect_funcs, &apsmgr_io_funcs, 0);
//	rsrcdbmgr_proc_devno("part", &schedpart_devno, -1, 0);

	/* initialize the root entry */
	iofunc_attr_init(&root_spart->attr, 0555 | S_IFDIR, NULL, NULL);
	if (S_ISDIR(root_spart->attr.mode)) ++root_spart->attr.nlink;	// iofunc_unlink() requires nlink >=2
	root_spart->type = part_type_ROOT;
	root_spart->name = "";
	root_spart->data.spid = part_id_t_INVALID;
	root_spart->parent = NULL;
	root_spart->hdr.next = NULL;
	root_spart->hdr.prev = NULL;
	LIST_INIT(root_spart->children);
	LIST_ADD(root_spart->children, sys_spart);
	if (S_ISDIR(sys_spart->attr.mode)) ++root_spart->attr.nlink;
//kprintf("root_spart = 0x%x, root_spart->spart = 0x%x\n", root_spart, root_spart->spid);

	/* initialize the initial system partition entry */
	iofunc_attr_init(&sys_spart->attr, 0555 | S_IFDIR, &root_spart->attr, NULL);
	if (S_ISDIR(sys_spart->attr.mode)) ++root_spart->attr.nlink;	// iofunc_unlink() requires nlink >=2
	sys_spart->type = part_type_SCHEDPART_REAL;
	sys_spart->name = APS_SYSTEM_PARTITION_NAME;	// will never be freed
	sys_spart->data.spid = SCHEDPART_T_TO_ID(sys_schedpart);
	sys_schedpart->rmgr_attr_p = sys_spart;
	sys_spart->hdr.next = NULL;
	sys_spart->hdr.prev = NULL;
	LIST_INIT(sys_spart->children);
	sys_spart->parent = root_spart;
//kprintf("sys_spart = 0x%x, sys_spart->spart = 0x%x\n", sys_spart, sys_spart->spid);

	rsrcdbmgr_proc_devno(_MAJOR_FSYS, &apsmgr_devno, -1, 0);
}



__SRCVERSION("$IQ: apsmgr_init.c,v 1.23 $");

