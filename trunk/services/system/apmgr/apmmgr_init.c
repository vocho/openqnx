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
 * apmmgr_init
 * 
 * Provide resource manager initialization for the memory partitioning module
 * 
*/

#include "apmmgr.h"


static apmmgr_attr_t  _root_mpart;

static apmmgr_attr_t  _sys_mclass;
static apmmgr_attr_t  *sys_mclass = &_sys_mclass;

static apmmgr_attr_t  _sys_mpart;
static apmmgr_attr_t  *sys_mpart = &_sys_mpart;

static rsrcmgr_mempart_fnctbl_t  rsrcmgr_mempart_fnctbl =
{
	STRUCT_FLD(validate_association) validate_mp_association,
	STRUCT_FLD(mpart_event_handler) apmmgr_event_trigger,
	STRUCT_FLD(mclass_event_handler) apmmgr_event_trigger2,
};

static const resmgr_connect_funcs_t apmmgr_connect_funcs =
{
	STRUCT_FLD(nfuncs) _RESMGR_CONNECT_NFUNCS,
	STRUCT_FLD(open) apmmgr_open,
	STRUCT_FLD(unlink) apmmgr_unlink,
	STRUCT_FLD(rename) apmmgr_rename,				// this line needed for watcom compiler
	STRUCT_FLD(mknod) apmmgr_mknod,
	STRUCT_FLD(readlink) apmmgr_readlink,
	STRUCT_FLD(link) apmmgr_link,
};

/* needs to be externally visible to apmmgr_open() */
const resmgr_io_funcs_t apmmgr_io_funcs =
{
	STRUCT_FLD(nfuncs) _RESMGR_IO_NFUNCS,
	STRUCT_FLD(read) apmmgr_read,
	STRUCT_FLD(write) NULL,					// this line needed for watcom compiler
	STRUCT_FLD(close_ocb) apmmgr_close,
	STRUCT_FLD(stat) apmmgr_stat,
	STRUCT_FLD(notify) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(devctl) apmmgr_devctl,
	STRUCT_FLD(unblock) apmgr_unblock,
	STRUCT_FLD(pathconf) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(lseek) apxmgr_lseek,
	STRUCT_FLD(chmod) apxmgr_chmod,
	STRUCT_FLD(chown) apxmgr_chown,
	STRUCT_FLD(utime) apmgr_utime,			// touch wants this function implemented
	STRUCT_FLD(openfd) apmgr_openfd,
	STRUCT_FLD(fdinfo) apmgr_fdinfo,
};

/*
 * mempart_rsrcmgr_fnctbl
 * 
 * Pointer to the table of memory partition interface routines required by the
 * memory partitioning resource manager.
*/
mempart_rsrcmgr_fnctbl_t  *mempart_rsrcmgr_fnctbl = NULL;
apmmgr_attr_t  *root_mpart = &_root_mpart;
dev_t  apmmgr_devno;

#define strcat(d, s)	STRLCPY(&d[strlen(d)], s, strlen(s) + 1)

/*******************************************************************************
 * _mpmgr_st_size
 * 
 * FIX ME
 * This is a support routine to get the size of free memory from a partitioning
 * perspective (ie. a processes "world view" of free memory is the free memory
 * in the hierarchy of partitions for its associated sysram memory class.
*/
extern int (*mpmgr_st_size)(void *attr, memsize_t *size);
static int _mpmgr_st_size(void *attr, memsize_t *size)
{
	extern int mpmgr_get_st_size(apmmgr_attr_t *attr, struct stat *st);
	struct stat  st;
	int r = mpmgr_get_st_size(attr, &st);
	if (r == EOK) {
		*size = (memsize_t)st.st_size;
#if (!defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32) && !defined(_MEMSIZE_and_memsize_are_different_)
		*size |= ((memsize_t)st.st_size_hi << 32);
#endif
	}
	return r;
}

void apmmgr_init(char *basepath)
{
	char name[PATH_MAX + 1];
	resmgr_attr_t  attr;
	memclass_entry_t *mclass_entry = memclass_find(NULL, sys_memclass_id);
	
	CRASHCHECK(mclass_entry == NULL);

	/*
	 * if the memory partitioning module is not installed, we do not enable
	 * the memory partitioning resource manager code
	*/ 
	if ((!MEMPART_INSTALLED()) || (mempart_fnctbl->rsrcmgr_attach == NULL) ||
		((mempart_rsrcmgr_fnctbl = mempart_fnctbl->rsrcmgr_attach(&rsrcmgr_mempart_fnctbl)) == NULL))
	{
#ifndef NDEBUG
		if (ker_verbose) kprintf("Memory Partitioning module not installed\n");
#endif	/* NDEBUG */
		return;
	}
	/* fill in the (*mmgr_st_size) function pointer for free_mem() */
	mpmgr_st_size = _mpmgr_st_size;

	STRLCPY(name, basepath, sizeof(name));
	if ((strlen(name) + strlen("/mem")) >= sizeof(name))
		return;

	strcat(name, "/mem");

    /* initialize resource manager attributes */
    memset(&attr, 0, sizeof(attr));
    attr.nparts_max = 1;
    attr.msg_max_size = 2048;
	attr.flags |= _RESMGR_FLAG_DIR;

#ifdef USE_RESMGR_FUNC_DEFAULTS
	/* initialize functions for handling messages */
	apmmgr_io_funcs.nfuncs = _RESMGR_IO_NFUNCS;
	apmmgr_connect_funcs.nfuncs = _RESMGR_CONNECT_NFUNCS;
	iofunc_func_init(apmmgr_connect_funcs.nfuncs, &apmmgr_connect_funcs,
					 apmmgr_io_funcs.nfuncs, &apmmgr_io_funcs);

	/* connect func overrides */
	apmmgr_connect_funcs.open = apmmgr_open;
	apmmgr_connect_funcs.unlink = apmmgr_unlink;
//	apmmgr_connect_funcs.readlink = apmmgr_read;
	apmmgr_connect_funcs.link = apmmgr_link;
	apmmgr_connect_funcs.readlink = apmmgr_readlink;
	apmmgr_connect_funcs.mknod = apmmgr_mknod;

	/* io_func overrides */
	apmmgr_io_funcs.read = apmmgr_read;
	apmmgr_io_funcs.close_ocb = apmmgr_close;
	apmmgr_io_funcs.stat = apmmgr_stat;
	apmmgr_io_funcs.devctl = apmmgr_devctl;
#endif	/* USE_IO_FUNC_DEFAULTS */

	// allocate mempart directory entry
	resmgr_attach(dpp, &attr, name, _FTYPE_ANY, _RESMGR_FLAG_DIR | _RESMGR_FLAG_BEFORE, &apmmgr_connect_funcs, &apmmgr_io_funcs, 0);
//	rsrcdbmgr_proc_devno("part", &mempart_devno, -1, 0);

	/* initialize the root entry */
	iofunc_attr_init(&root_mpart->attr, 0555 | S_IFDIR, NULL, NULL);
	if (S_ISDIR(root_mpart->attr.mode)) ++root_mpart->attr.nlink;	// iofunc_unlink() requires nlink >=2
	root_mpart->type = part_type_ROOT;
	root_mpart->name = "";
	root_mpart->data.mpid = part_id_t_INVALID;
	root_mpart->parent = NULL;
	root_mpart->hdr.next = NULL;
	root_mpart->hdr.prev = NULL;
	LIST_INIT(root_mpart->children);
	LIST_ADD(root_mpart->children, sys_mclass);
	if (S_ISDIR(sys_mclass->attr.mode)) ++root_mpart->attr.nlink;
//kprintf("root_mpart = 0x%x, root_mpart->mpart = 0x%x\n", root_mpart, root_mpart->mpid);

	/* initialize the initial system memory class entry */
	iofunc_attr_init(&sys_mclass->attr, 0666 | S_IFDIR, &root_mpart->attr, NULL);
	if (S_ISDIR(sys_mclass->attr.mode)) ++sys_mclass->attr.nlink;	// iofunc_unlink() requires nlink >=2
	sys_mclass->type = part_type_MEMCLASS;
	sys_mclass->data.mclass_entry = mclass_entry;
	/* sys memclass can never be removed, so this pointer is OK */
	sys_mclass->name = sys_mclass->data.mclass_entry->name;
	sys_mclass->data.mclass_entry->data.rmgr_attr_p = sys_mclass;
	sys_mclass->hdr.next = NULL;
	sys_mclass->hdr.prev = NULL;
	LIST_INIT(sys_mclass->children);
	LIST_ADD(sys_mclass->children, sys_mpart); 
	if (S_ISDIR(sys_mpart->attr.mode)) ++sys_mclass->attr.nlink;
	sys_mclass->parent = root_mpart;
//kprintf("sys_mclass = 0x%x\n", sys_mclass);

	/* initialize the initial system partition entry */
	iofunc_attr_init(&sys_mpart->attr, 0555 | S_IFDIR, &sys_mclass->attr, NULL);
	if (S_ISDIR(sys_mpart->attr.mode)) ++sys_mpart->attr.nlink;	// iofunc_unlink() requires nlink >=2
	sys_mpart->type = part_type_MEMPART_REAL;
	sys_mpart->name = "sys";	// will never be freed
	sys_mpart->data.mpid = MEMPART_T_TO_ID(sys_mempart);
	sys_mempart->rmgr_attr_p = sys_mpart;
	sys_mpart->hdr.next = NULL;
	sys_mpart->hdr.prev = NULL;
	LIST_INIT(sys_mpart->children);
	sys_mpart->parent = sys_mclass;
//kprintf("sys_mpart = 0x%x, sys_mpart->mpart = 0x%x\n", sys_mpart, sys_mpart->mpid);

	rsrcdbmgr_proc_devno(_MAJOR_FSYS, &apmmgr_devno, -1, 0);
}



__SRCVERSION("$IQ: apmmgr_init.c,v 1.23 $");

