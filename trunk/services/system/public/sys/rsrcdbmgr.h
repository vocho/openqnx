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
*  sys/rsrcdbmgr.h
*

*/
#ifndef __RSRCDBMGR_H_INCLUDED
#define __RSRCDBMGR_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#include _NTO_HDR_(_pack64.h)

__BEGIN_DECLS

/*
 Basic Resource Types (first byte of flag)
*/
#define RSRCDBMGR_TYPE_MASK		0xff
enum _rsrc_types {
	RSRCDBMGR_MEMORY			= 0,    /* Maps to "memory" */
	RSRCDBMGR_IRQ,                      /* Maps to "irq" */ 
	RSRCDBMGR_IO_PORT,                  /* Maps to "io" */
	RSRCDBMGR_DMA_CHANNEL,              /* Maps to "dma" */
	RSRCDBMGR_PCI_MEMORY,               /* Maps to "pcimemory" */
	RSRCDBMGR_RESERVED_5,
	RSRCDBMGR_RESERVED_6,
	RSRCDBMGR_RESERVED_7,
	RSRCDBMGR_TYPE_COUNT		= 8
};

/*
 Resource Request Flags
*/
#define RSRCDBMGR_FLAG_MASK		0xffffff00	
#define RSRCDBMGR_FLAG_USED		0x00000100			/* Used resource (for query) */

#define RSRCDBMGR_FLAG_ALIGN	0x00000200			/* Alignment field is valid */
#define RSRCDBMGR_FLAG_RANGE	0x00000400			/* Start and End fields are valid */
#define RSRCDBMGR_FLAG_SHARE	0x00000800			/* Share this resource w/ others */
#define RSRCDBMGR_FLAG_TOPDOWN	0x00001000			/* Search from the end->start */
#define RSRCDBMGR_FLAG_NAME		0x00002000			/* Name field is valid, and is a system name */
#define RSRCDBMGR_FLAG_LIST		0x00004000			/* This is one item is a list to search */

/*
 Resource Allocation Flags
*/
#define RSRCDBMGR_FLAG_RSVP		0x00004000			/* Avoid this block unless only choice (for create) */
#define RSRCDBMGR_FLAG_NOREMOVE	0x00008000			/* Don't remove this block when the process dies */

/*
 Basic resource request items
*/
typedef struct _rsrc_alloc {
	_Uint64t	start;		/* Start of resource range */
	_Uint64t	end;		/* End of resource range */
	_Uint32t	flags;		/* Resource type | Resource flags */
	const char	*name;		/* Future use: the name of the resource */
} rsrc_alloc_t;

typedef struct _rsrc_request {
	_Uint64t	length;		/* Length of resource desired */
	_Uint64t	align;		/* Alignment of resource desired */
	_Uint64t	start;		/* Start of resource range */
	_Uint64t	end;		/* End of resource range */
	_Uint32t	flags;		/* Resource type | Resource flags */
	_Uint32t	zero[2];	/* Reserved */
	const char *name;		/* Name of resource class (NULL) */
} rsrc_request_t;

typedef struct {
	_Int32t		major;		/* Major number */
	_Int32t		minor;		/* Minor number (can only be 8 bits, -1 for choose) */
	_Int32t		flags;
	const char *name;		/* Minor name */
} rsrc_minor_request_t;

/** 
 RSRCMGR API
**/

/*
 Add or remove system resources globally for the system
 pass in an array of resource types. Set the resource 
 type in the flags field.
 Returns: EOK on success
*/
int rsrcdbmgr_create(rsrc_alloc_t *__item, int __count);
int rsrcdbmgr_destroy(rsrc_alloc_t *__item, int __count);

/*
 Earmark resources as being allocated/release to/from a process.

 -If the ALIGN flag is specified then the alignment is important
 -If the RANGE flag is specified then the range is important

 ie) 4 byte aligned, length 50, any range
	 list.align = 4 list.length = 50;
	 list.flags = RSRCMGR_FLAG_ALIGN | RSRMGR_IO_PORT
	 rsrcmgr_attach(&list, 1);
 
 ie) 1 IO port (io port 0) and 1 IRQ (IRQ 10,11, or 12)
   list[0].start = list[0].end = 0; list[0].length = 1; 
   list[0].flags = RSRCMGR_FLAG_RANGE | RSRCMGR_IO_PORT;
   list[1].start = 10; list[1].end = 12; list[1].length = 1; 
   list[1].flags =  RSRCMGR_FLAG_RANGE | RSRCMGR_IRQ;
   rsrcmgr_attach(list, 2);

 This call will fail if one resource from the array is not available.  
 The range requested is returned as the start, end values in the list
 and should be passed to the detach function.
 Returns: EOK on success
*/
int rsrcdbmgr_attach(rsrc_request_t *__list, int __count);
int rsrcdbmgr_detach(rsrc_request_t *__list, int __count);


/*
 Returns a list of resources from the database. 
 Supersceded by rsrcdbmgr_query_name().
*/
int rsrcdbmgr_query(rsrc_alloc_t *list, int listcnt, int start, _Uint32t type);

/*
 Returns a list of resources from the database.
 pid == -1
    -> Query all blocks
 pid == 0
    -> Query all free blocks
 pid > 0
    -> Query blocks owned by pid

 If list == NULL || listcnt == 0
    -> Return the number of blocks which match the criteria

 Otherwise return a maximum of 'listcnt' blocks of data from the offset
 'start' (0 based) which match the criteria 'name'.
*/
int rsrcdbmgr_query_name(rsrc_alloc_t *__list, int __listcnt, int __start, 
                         pid_t __pid, char *__name, unsigned __flags);

/*
 Attach/Detach retreives a free minor number for a given major 
 number.  A request of -1 indicates any minor number will do

 Returns: -1 on error
*/
int rsrcdbmgr_minor_attach(int __major, int __minor_request);
int rsrcdbmgr_minor_detach(int __major, int __minor);

dev_t rsrcdbmgr_devno_attach(const char *__name, int __minor_request, int __flags);
int rsrcdbmgr_devno_detach(dev_t __devno, int __flags);

__END_DECLS

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("rsrcdbmgr.h $Rev: 153052 $"); */
