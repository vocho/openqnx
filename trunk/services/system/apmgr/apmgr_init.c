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
 * apmgr_init
 * 
 * NOTE:
 * The apmgr_xxx() code provides management of the "/proc/<pid>/partition/"
 * namespace. This could be implemented as /pathmgr/ *.c code (similar to
 * devtext.c, etc) however it is currently lumped in the /apmgr/ directory
 * so that it can be loaded as a separate module (ie. /proc/<pid>/partition/
 * won't show up in the name space unless the 'apmgr' module is included). An
 * alternate packaging stategy would be to always include the 'apmgr_xxx()'
 * code so that the /proc/<pid>/partition/ name is always present, but empty and
 * only if the 'apmgr' module is included (which has the apmmgr_xxx() and
 * apsmgr_xxx() code) will the namespace under /proc/<pid>/partition/
 * be populated with information. The second approach requires the apmmgr
 * and apsmgr to expose their interface via a function table so load time
 * linkage can be established. The first approach allows 'apmgr' to call
 * directly into 'apmmgr' and 'apsmgr' routines (since both will be
 * loaded in the same module). The first approach will be coded with macros
 * for the functions called so that the second approach is easily implemented. 
 * 
 * Provide resource manager initialization for the partitioning module
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

static void apmgr_init(const char * const path);


/*******************************************************************************
 * initialize_apmgr
 * 
 * Called from kmain.c during module initialization. This function will
 * initialize the partitioning resource manager
*/
void initialize_apmgr(unsigned version, unsigned pass)
{
	if(version != LIBMOD_VERSION_CHECK) {
		kprintf("Version mismatch between procnto (%d) and libmod_apmgr (%d)\n",
					version, LIBMOD_VERSION_CHECK);
		crash();
	}
	/*
	 * FIX ME
	 * if this code becomes separate from apmmgr/apsmgr and that code
	 * is not installed (module not loaded), don't install anything
	*/
	switch(pass)
	{
		case 10:
		{
			apmgr_init("/partition");
			APMMGR_INIT("/partition");
			APSMGR_INIT("/partition");
			apmgr_init("/proc");

			kprintf("partition resource manager module loaded\n");
			
			break;
		}
		default:
			break;
	}
}


/*
 * apmgr_init
*/
const resmgr_io_funcs_t apmgr_io_funcs =
{
	_RESMGR_IO_NFUNCS,
	STRUCT_FLD(read) apmgr_read,
	STRUCT_FLD(write) NULL,					// this line needed for watcom compiler
	STRUCT_FLD(close_ocb) apmgr_close,		// included as per comment in apmgr_close.c
	STRUCT_FLD(stat) apmgr_stat,
	STRUCT_FLD(notify) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(devctl) apmgr_devctl,
	STRUCT_FLD(unblock) apmgr_unblock,
	STRUCT_FLD(pathconf) NULL,				// this line needed for watcom compiler
	STRUCT_FLD(lseek) apmgr_lseek,
	STRUCT_FLD(chmod) apmgr_chmod,
	STRUCT_FLD(chown) apmgr_chown,
	STRUCT_FLD(utime) apmgr_utime,			// touch wants this function implemented
	STRUCT_FLD(openfd) apmgr_openfd,
	STRUCT_FLD(fdinfo) apmgr_fdinfo,
};

/*
 * This resource manager registers in "/proc" and "/partition" namespaces so
 * we have a different set of connect functions depending on which
*/
static const resmgr_connect_funcs_t apmgr_connect_funcs =
{
	_RESMGR_CONNECT_NFUNCS,
	STRUCT_FLD(open) apmgr_open,
	STRUCT_FLD(unlink) apmgr_unlink,
	STRUCT_FLD(rename) apmgr_rename,
	STRUCT_FLD(mknod) apmgr_mknod,
	STRUCT_FLD(readlink) apmgr_readlink,
	STRUCT_FLD(link) apmgr_link,
};

static const resmgr_connect_funcs_t apmgr_proc_connect_funcs =
{
	_RESMGR_CONNECT_NFUNCS,
	STRUCT_FLD(open) apmgr_proc_open,
	STRUCT_FLD(unlink) apmgr_proc_unlink,
};

static apmgr_attr_t  _root_npart;
apmgr_attr_t  *root_npart = &_root_npart;
dev_t  apmgr_devno;
static void apmgr_init(const char * const path)
{
	static resmgr_attr_t  partition_attr;
	static resmgr_attr_t  proc_attr;
	static resmgr_attr_t  *attr;
	const resmgr_connect_funcs_t *connect_funcs;

	if (memcmp(path, "/proc", sizeof("/proc")-1) == 0) {
		attr = &proc_attr;
		connect_funcs = &apmgr_proc_connect_funcs;
	} else {
		attr = &partition_attr;
		connect_funcs = &apmgr_connect_funcs;
	}
	memset(attr, 0, sizeof(*attr));
	attr->nparts_max = 1;
	attr->msg_max_size = 2048;
	attr->flags |= _RESMGR_FLAG_DIR;
//	kprintf("%s: attaching %s\n", __func__, path);

	resmgr_attach(dpp, attr, path, _FTYPE_ANY, _RESMGR_FLAG_DIR | _RESMGR_FLAG_AFTER, connect_funcs, &apmgr_io_funcs, 0);

	/* initialize the root entry for "/partition" mount */
	if (connect_funcs == &apmgr_connect_funcs)
	{
		iofunc_attr_init(&root_npart->attr, 0777 | S_IFDIR, NULL, NULL);
		if (S_ISDIR(root_npart->attr.mode)) ++root_npart->attr.nlink;	// iofunc_unlink() requires nlink >=2
		root_npart->type = part_type_ROOT;
		root_npart->name = "";
		root_npart->parent = NULL;
		root_npart->hdr.next = NULL;
		root_npart->hdr.prev = NULL;
		LIST_INIT(root_npart->children);
	}

	rsrcdbmgr_proc_devno(_MAJOR_FSYS, &apmgr_devno, -1, 0);
}


__SRCVERSION("$IQ: apmgr_init.c,v 1.23 $");

