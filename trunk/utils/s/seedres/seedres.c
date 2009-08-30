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





#include	<stdio.h>
#include	<inttypes.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<errno.h>
#include	<signal.h>
#include	<sys/rsrcdbmgr.h>
#include	<sys/rsrcdbmsg.h>
#include	<sys/syspage.h>
#include	<sys/procmgr.h>
#include	<hw/sysinfo.h>
#include	<sys/types.h>
#include	"pnpbios.h"
#include	<stddef.h>
#include	<sys/mman.h>
#include	<x86/v86.h>

#ifndef _V86_OPTION_CALLDIRECT
#define _V86_OPTION_CALLDIRECT		(1 << 8)
#endif

#define USERDATA_SEG			0
#define USERDATA_OFF			offsetof(struct _v86_memory, userdata)

#define GET_NUM_NODES			0x00
#define GET_DEVICE_NODE			0x01
#define SET_DEVICE_NODE			0x02
#define GET_EVENT				0x03
#define SEND_MSG				0x04
#define GET_DOCK_INFO			0x05
#define SET_STATIC_RESOURCES	0x09
#define GET_STATIC_RESOURCES	0x0a
#define GET_APM_TABLE			0x0b
#define GET_ISA_CONFIG_STRUC	0x40
#define GET_ESCD_INFO			0x41
#define READ_ESCD				0x42
#define WRITE_ESCD				0x43
#define GET_VER_INSTALL_CHECK	0x60
#define GET_DEVICE_COUNT		0x61
#define GET_PRI_AND_TABLE		0x62
#define SET_PRI					0x63
#define GET_LAST_IPL_DEVICE		0x64
#define GET_FIRST_BOOT			0x65
#define SET_FIRST_BOOT			0x66

#include <_pack1.h>

struct stub {
	uint8_t				push6;			// 0x68
	uint16_t			arg6;
	uint8_t				push5;			// 0x68
	uint16_t			arg5;
	uint8_t				push4;			// 0x68
	uint16_t			arg4;
	uint8_t				push3;			// 0x68
	uint16_t			arg3;
	uint8_t				push2;			// 0x68
	uint16_t			arg2;
	uint8_t				push1;			// 0x68
	uint16_t			arg1;
	uint8_t				pushfunc;		// 0x68
	uint16_t			func;
	uint8_t				pushseg1;		// 0x68
	uint16_t			iretseg;
	uint8_t				pushoff1;		// 0x68
	uint16_t			iretoff;		// addsp + userdata
	uint8_t				pushseg2;		// 0x68
	uint16_t			seg;
	uint8_t				pushoff2;		// 0x68
	uint16_t			off;
	uint8_t				retf;			// 0xcb
	uint8_t				addsp[3];		// 0x83 0xc4 0x12
	uint8_t				iret;			// 0xcf
	uint8_t				padding[2];
};
#include <_packpop.h>

#define		LOW_MEM		0xa0000
#define		HIGH_MEM	0xfffff
#define		ALLOC_LOW	0xd4000
#define		ALLOC_HIGH	0xd5fff
#define		LOW_PORT	0x100
#define		HIGH_PORT	0x1ff

static	int32_t		mem_idx, verbose = 0;
static	uint64_t	free_mem_start, free_mem_end;
//static	uint64_t	mem_low, mem_high;
static	struct {
		uint64_t	start, end;
		} upper_mem [20];

/**************************************************************************/
/*                                                                        */
/**************************************************************************/
#if defined(__X86__)
pnpbios_t *pnpbios_attach (void)

{
uint8_t			*bios, *p;
pnpbios_t		*handle;

	// Get a pointer to the BIOS ROM
	if((bios = mmap(0, 0x10000, PROT_READ, MAP_SHARED | MAP_PHYS, NOFD, 0xf0000)) == MAP_FAILED) {
		return 0;
		}

	// Check every 16 bytes for a valid header
	handle = 0;
	for(p = bios; p < bios + 0x10000; p += 16) {
		struct pnpbios_header		*hdr = (struct pnpbios_header *)p;
		
		if(!memcmp(hdr->signature, "$PnP", 4)) {
			int32_t				i;                 
			uint8_t				cksum;

			cksum = 0;
			for(i = 0; i < hdr->length; i++) {
				cksum += p[i];
				}
			if(cksum == 0) {
				if(handle = malloc(sizeof *handle)) {
					handle->hdr = *hdr;
					handle->node_size = 0;
					}
				break;
				}
			}
   		 }

	munmap(bios, 0x10000);

	return (handle);
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

static int32_t call (pnpbios_t *handle, uint32_t func, struct stub *p, uint32_t stubsize, int32_t nargs, uint16_t *args)

{
int32_t					i;
struct _v86reg			regs;
const uint32_t			offsets[] = {
	offsetof(struct stub, arg1),
	offsetof(struct stub, arg2),
	offsetof(struct stub, arg3),
	offsetof(struct stub, arg4),
	offsetof(struct stub, arg5),
	offsetof(struct stub, arg6)
	};

	/* There is room for 5 args, the function plus the bios selector */
	if(nargs > 5) {
		return (-1);
		}

	p->push6 = p->push5 = p->push4 = p->push3 = p->push2 = p->push1 =
		p->pushfunc = p->pushoff1 = p->pushseg1 = p->pushoff2 = p->pushseg2 = 0x68;
	for(i = 0; i < nargs; i++) {
		*(uint16_t *)((char *)p + offsets[i]) = args[i];
		}
	*(uint16_t *)((char *)p + offsets[i]) = handle->hdr.data_real_seg;
	p->func = func;
	p->iretseg = USERDATA_SEG;
	p->iretoff = offsetof(struct stub, addsp) + USERDATA_OFF;
	p->off = handle->hdr.entry_real_off;
	p->seg = handle->hdr.entry_real_seg;
	p->addsp[0] = 0x83;
	p->addsp[1] = 0xc4;
	p->addsp[2] = 0x0e;
	p->retf = 0xcb;
	p->iret = 0xcf;

	memset(&regs, 0, sizeof regs);
	if(stubsize < sizeof *p) {
		stubsize = sizeof *p;
		}
	if((i = _intr_v86(_V86_OPTION_CALLDIRECT | 0xff, &regs, p, stubsize)) == -1) {
		return (-1);
		}
	return (regs.eax);
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

void pnpbios_detach(pnpbios_t *handle)

{
	free(handle);
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

int32_t pnpbios_get_num_nodes(pnpbios_t *handle, int32_t *pnum_nodes, int32_t *pnode_size)

{
int32_t					ret;
struct buffer {
	struct stub			stub;
	uint16_t			node_size;
	uint8_t				num_nodes;
	uint8_t				old_bios_overflow;	/* Old spec states num_nodes is a word, make room in this structure */
	} buffer;
uint16_t			args[] = {
	offsetof(struct buffer, num_nodes) + USERDATA_OFF,
	USERDATA_SEG,
	offsetof(struct buffer, node_size) + USERDATA_OFF,
	USERDATA_SEG
	};

	ret = call(handle, GET_NUM_NODES, &buffer.stub, sizeof buffer, sizeof args / sizeof *args, args);

	handle->node_size = buffer.node_size;

	if(pnum_nodes) {
		*pnum_nodes = buffer.num_nodes;	
		}

	if(pnode_size) {
		*pnode_size = buffer.node_size;	
		}

	return (ret);
}

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

int32_t	pnpbios_get_device_node (pnpbios_t *handle, int32_t *pnode, pnpbios_node_t *devnode, int32_t control)

{
int32_t			ret;
int32_t			size;
struct buffer {
	struct stub				stub;
	uint8_t					node;
	uint8_t					padding[3];
	pnpbios_node_t			buff;		/* Must be last */
	} *buffer;
uint16_t args[] = {
	offsetof(struct buffer, node) + USERDATA_OFF,
	USERDATA_SEG,
	offsetof(struct buffer, buff) + USERDATA_OFF,
	USERDATA_SEG,
	control
	};

	if(handle->node_size == 0) {
		pnpbios_get_num_nodes(handle, 0, 0);
		}

	size = offsetof(struct buffer, buff) + handle->node_size;

	if(!(buffer = alloca(size))) {
		errno = ENOMEM;
		return (-1);
		}
	buffer->node = pnode ? *pnode : 0;

	ret = call(handle, GET_DEVICE_NODE, &buffer->stub, size, sizeof args / sizeof *args, args);

	if(pnode) {
		*pnode = buffer->node;	
		}

	if(devnode) {
		memcpy(devnode, &buffer->buff, min(buffer->buff.size, handle->node_size));
		}

	return (ret);
}

#endif

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

int32_t		find_ram (void)

{
struct		asinfo_entry  *asinfo;
int32_t		i, num, cnt;
uint64_t	highest_ram = 0;
struct	{
	uint64_t	start, end;
	} arr [50];

	num = _syspage_ptr->asinfo.entry_size / sizeof(*asinfo);
	asinfo = SYSPAGE_ENTRY (asinfo);

	for (i = cnt = 0; i < num; ++i) {
		char *name;
		if (asinfo->attr & AS_ATTR_KIDS || !asinfo->attr) {
			asinfo++;
			continue;
			}
		name = __hwi_find_string (asinfo->name);
		if (strcmp (name, "sysram") && strcmp (name, "rom")) {
			asinfo++;
			continue;
			}
		arr [cnt].start = asinfo->start;
		arr [cnt++].end = asinfo->end;
		if (! strcmp (name, "sysram")) {
			if (asinfo->end > highest_ram) {
				highest_ram = asinfo->end;
				}
			}
		asinfo++;
		}
	cnt--;
	while (1) {
		int32_t		swap;
		uint64_t	tstart, tend;
		for (i = swap = 0; i < cnt; i++) {
			if (arr [i + 1].start < arr [i].start) {
				tstart = arr [i].start;
				tend = arr [i].end;
				arr [i].start = arr [i + 1].start;
				arr [i].end = arr [i + 1].end;
				arr [i + 1].start = tstart;
				arr [i + 1].end = tend;
				swap = 1;
				}
			}
		if (! swap)
			break;
		}
	for (i = 0; i <= cnt; i++) {
		if (arr [i].end < highest_ram) {
			continue;
			}
		if (i < cnt && (arr [i].end + 1 < arr [i + 1].start)) {
			free_mem_start = arr [i].end + 1;
			if (highest_ram = (free_mem_start % 0x100000)) {	//Some bioses report incorrect
				free_mem_start += (0x100000 - highest_ram);		//memory, so make mod. 1Meg
				}
			free_mem_end = arr [i + 1].start;
			}
		}
	return (0);
}

#if	0
/**************************************************************************/
/*                                                                        */
/**************************************************************************/

static	void	find_ranges (void)

{
rsrc_request_t	rreq;

	memset ((char *) &rreq, 0, sizeof (rreq));
	rreq.length = 1;
	rreq.start = LOW_MEM;
	rreq.end = HIGH_MEM;
	rreq.flags = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE;
	rsrcdbmgr_attach (&rreq, 1);
	if (verbose)
		fprintf (stderr, "Low memory %llx\n", rreq.start);
	mem_low = rreq.start;
	rsrcdbmgr_detach (&rreq, 1);

	memset ((char *) &rreq, 0, sizeof (rreq));
	rreq.length = 1;
	rreq.start = LOW_MEM;
	rreq.end = HIGH_MEM;
	rreq.flags = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_TOPDOWN | RSRCDBMGR_FLAG_RANGE;
	rsrcdbmgr_attach (&rreq, 1);
	if (verbose)
		fprintf (stderr, "High memory %llx\n", rreq.start);
	mem_high = rreq.start + 1;
	rsrcdbmgr_detach (&rreq, 1);
}
#endif

/*************************************************************************/
/*                                                                       */
/*************************************************************************/

int32_t		seed_resource (uint64_t *start, uint64_t *end, int32_t type)

{
rsrc_alloc_t	ralloc;

	if (*start == 0 && *end == 0)
		return (EOK);
	memset ((char *) &ralloc, 0, sizeof (ralloc));
	ralloc.start = *start;
	ralloc.end = *end;
	ralloc.flags = type | RSRCDBMGR_FLAG_NOREMOVE;
	if (verbose)
		fprintf (stderr, "Seed Start %llx - End %llx - Flags %x\n", ralloc.start, ralloc.end, ralloc.flags);
	
	if (rsrcdbmgr_create (&ralloc, 1) == -1) {
		if (verbose) {
			perror ("Unable to seed resource");
			}
		return (ENOMEM);
		}
	return (EOK);
}

/*************************************************************************/
/*                                                                       */
/*************************************************************************/

int32_t		alloc_resource (uint64_t *start, uint64_t *end, int32_t type)

{
rsrc_request_t	rreq;

	if (*start == 0 && *end == 0)
		return (EOK);
	memset ((int8_t *) &rreq, 0, sizeof (rsrc_request_t));
	rreq.flags = type | RSRCDBMGR_FLAG_NOREMOVE | RSRCDBMGR_FLAG_RANGE;
	rreq.start = *start;
	rreq.end = *end;
	rreq.length = *end - *start + 1;
	if (verbose)
		fprintf (stderr, "Attach Start %llx - End %llx - Flags %x\n", rreq.start, rreq.end, rreq.flags);
	if (rsrcdbmgr_attach (&rreq, 1) == -1) {
		if (verbose) {
			perror ("Unable to allocate resource");
			}
		return (ENOMEM);
		}
	return (EOK);
}

/*************************************************************************/
/*  Routine to parse the /etc/config/pcmcia.(cfg/node) file.             */
/*  The routine will parse "string" for a start and an end value and     */
/*  save the result in *start and *end.                                  */
/*  The format of "string" can be:-                                      */
/*        start-end, start-end... or                                     */
/*        start+length, start+length... or                               */
/*        val, val...                                                    */
/*  Where only one value is found, only *start will be updated.          */
/*  The routine saves the current pointer to "string", so can be called  */
/*  recursively, with "string" set to NULL.                              */
/*  Return value: The number of arguments found or                       */
/*    0 if no arguments found.                                           */
/*************************************************************************/

static	int32_t		get_values (int8_t *string, uint64_t *start, uint64_t *end)

{
static	int8_t	*val_ptr;
int32_t			i;
int8_t			*ptr, x;

	ptr = string == NULL ? val_ptr : string;	//for recursive call
	if (ptr == NULL)
		return (0);		//return if end of string
	i = strlen (ptr);
	*start = *end = 0;
	while (i) {
		*start = strtol (ptr, NULL, 0);
		if (verbose)
			fprintf (stderr, "Start %llx ", *start);
		while (*ptr != '-' && *ptr != '+' && --i) {
			if (*ptr == ',') {
				val_ptr = ++ptr;
				if (verbose)
					fprintf (stderr, "\n");
				return (1);
				}
			ptr++;
			}
		if (! i) {
			if (*start) {
				if (verbose)
					fprintf (stderr, "\n");
				val_ptr = NULL;
				return (1);		//only one parameter found
				}
			return (0);
			}
		x = *ptr++;
		*end = strtol (ptr, NULL, 0);
		if (x == '+') {
			*end += *start;
			(*end)--;
			}
		if (verbose)
			fprintf (stderr, "- end %llx\n", *end);
		while (*ptr++ != ',' && --i);
		val_ptr = i ? ptr : NULL;
		return (2);		//both parameters found
		}
	return (0);
}

#if	0
/*************************************************************************/
/*                                                                       */
/*************************************************************************/

static	void	setup_memory (void)

{
rsrc_request_t	rreq;
uint64_t	start, tmp = 0, end;
int32_t		i, swap;

	upper_mem [mem_idx].start = ALLOC_LOW;
	upper_mem [mem_idx++].end = ALLOC_HIGH;
	if (mem_idx > 1) {
		while (1) {
			for (i = swap = 0; i < mem_idx - 1; i++) {
				if (upper_mem [i].start > upper_mem [i + 1].start) {
					start = upper_mem [i].start;
					end = upper_mem [i].end;
					upper_mem [i].start = upper_mem [i + 1].start;
					upper_mem [i].end = upper_mem [i + 1].end;
					upper_mem [i + 1].start = start;
					upper_mem [i + 1].end = end;
					swap = 1;
					}
				}
			if (! swap)
				break;
			}
		}

	if (upper_mem [0].start > (uint64_t) LOW_MEM) {
		start = LOW_MEM;
		end = upper_mem [0].start - 1;
		alloc_resource (&start, &end, RSRCDBMGR_MEMORY);
		}
	tmp = upper_mem [0].end + 1;
	for (i = 1; i < mem_idx - 1; i++) {
		if (upper_mem [i].start > tmp) {
			start = tmp;
			end = upper_mem [i].start - 1;
			alloc_resource (&start, &end, RSRCDBMGR_MEMORY);
			}
		tmp = upper_mem [i].end + 1;
		}

	memset ((char *) &rreq, 0, sizeof (rreq));
	start = ALLOC_LOW;
	end = ALLOC_HIGH;
	if (verbose)
		fprintf (stderr, "Memory Start %x - End %x\n", ALLOC_LOW, ALLOC_HIGH);
	seed_resource (&start, &end, RSRCDBMGR_MEMORY);
}
#endif

/**************************************************************************/
/*                                                                        */
/**************************************************************************/

int32_t		pnpbios (void)

{
#if defined(__X86__)
pnpbios_t		*h;
int32_t			num, size, node;
uint64_t		start, end;
pnpbios_node_t	*dev;

	if(!(h = pnpbios_attach())) {
		fprintf (stderr, "Can't locate PNP bios (%s)\n", strerror(errno));
		return (0);
		}

	if(pnpbios_get_num_nodes(h, &num, &size) != 0) {
		fprintf (stderr, "Can't locate PNP nodes\n");
		return (0);
		}
		
	if(!(dev = malloc(size + 0x100))) {
		fprintf (stderr, "No memory\n");
		return (0);
		}

	node = 0;
	while(node != 0xff) {
		uint8_t				*p;
		uint32_t			type;
		int32_t				len;
		int32_t				pos;

		if(pnpbios_get_device_node(h, &node, dev, 0x02) != 0) {
			break;
			}

		p = (int8_t *)(dev + 1);
		len = 0;
		for(type = 0, pos = 1; type != 0x0f && pos < size; p += len, pos += len) {
			int32_t		n;

			if(p[0] & 0x80) {
				len = p[1] + p[2] * 256;
				type = p[0];
				p += 3;
				pos += 3;
				}
			else {
				len = p[0] & 0x07;
				type = ((p[0] >> 3) & 0x0f);
				p++;
				pos++;
				}

			if (len > 0x100) {
				node = 0xff;
				break;
				}

			if(len == 0 || type == 0x0f) {
				p += len;
				break;
				}

			switch(type) {
				case 0x01:		// Plug an Play version number
					break;

				case 0x03:		// Compatible device ID
					break;

				case 0x04:		// IRQ format
					for(n = 0; n < 16; n++) {
						if(*(uint16_t *)&p[0] & (1 << n)) {
							if (n == 7)		//We don't use IRQ 7
								continue;
							start = end = n;
							alloc_resource (&start, &end, RSRCDBMGR_IRQ);
							}
						}
					break;
					
				case 0x05:		// DMA format
					break;

				case 0x08:		// I/O port descriptor
					start = end = 0;
					start = *(uint16_t *) &p [1];
					end = start + (p [6] ? p [6] - 1 : 0);
					alloc_resource (&start, &end, RSRCDBMGR_IO_PORT);
					break;

				case 0x09:		// Fixed location I/O port descriptor
					start = end = 0;
					start = *(uint16_t *)p & 0x3fff;
					end = start + (p [2] ? p [2] - 1 : 0);
					alloc_resource (&start, &end, RSRCDBMGR_IO_PORT);
					break;
	
				case 0x85:		// 32-bit memory
					start = end = 0;
					start = *(uint32_t *) &p [1];
					end = start;
//					fprintf (stderr, "Memory %llx\n", start);
					break;
	
				case 0x86:		// 32-bit Fixed location memory
					start = end = 0;
					start = *(uint32_t *) &p [1];
					end = start + (*(uint32_t *) &p [5] ? *(uint32_t *) &p [5] - 1 : 0);
					alloc_resource (&start, &end, RSRCDBMGR_MEMORY);
					if (start >= LOW_MEM && end <= HIGH_MEM) {
						upper_mem [mem_idx].start = start;
						upper_mem [mem_idx++].end = end;
						}
					break;
				}
			}

		/* Skip over possible configurations */
		for(type = 0; type != 0x0f; p += len) {
			if(p[0] & 0x80) {
				len = p[1] + p[2] * 256;
				type = p[0];
				p += 3;
				}
			else {
				len = p[0] & 0x07;
				type = ((p[0] >> 3) & 0x0f);
				p++;
				}

			if (len > 0x100) {
				node = 0xff;
				break;
				}

			if(len == 0 || type == 0x0f) {
				p += len;
				break;
				}
			}

		/* Compatible device ids */
		for(type = 0; type != 0x0f; p += len) {
			if(p[0] & 0x80) {
				len = p[1] + p[2] * 256;
				type = p[0];
				p += 3;
				}
			else {
				len = p[0] & 0x07;
				type = ((p[0] >> 3) & 0x0f);
				p++;
				}

			if (len > 0x100) {
				node = 0xff;
				break;
				}

			if(len == 0 || type == 0x0f) {
				p += len;
				break;
				}
			}
		}
	pnpbios_detach(h);

#endif
	return (0);
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/

void		parse_values (int8_t *val, int8_t type)

{
uint64_t	start, end;
int8_t		*ptr;
uint32_t	flags;

	ptr = val;
	ptr++;
	while (get_values (ptr, &start, &end)) {
		ptr = NULL;
		if (! end)
			end = start;
		switch (*val) {
			case	'i':
				flags = RSRCDBMGR_IRQ;
				if (verbose)
					fprintf (stderr, "Irq %llx\n", start);
				break;
			case	'm':
				flags = RSRCDBMGR_MEMORY | RSRCDBMGR_FLAG_RANGE;
				if (verbose)
					fprintf (stderr, "Memory Start %llx - End %llx\n", start, end);
				break;
			case	'p':
				flags = RSRCDBMGR_IO_PORT | RSRCDBMGR_FLAG_RANGE;
				if (verbose)
					fprintf (stderr, "Port Start %llx - End %llx\n", start, end);
				break;
			case	'u':
				flags = RSRCDBMGR_PCI_MEMORY | RSRCDBMGR_FLAG_RANGE;
				if (verbose)
					fprintf (stderr, "PCI Memory Start %llx - End %llx\n", start, end);
				break;
			default:
				return;
			}
		switch (type) {
			case	'a':
				alloc_resource (&start, &end, flags);
				break;
			case	'r':
				flags |= RSRCDBMGR_FLAG_RSVP;
			case	's':
				seed_resource (&start, &end, flags);
				break;
			}
		}
}

/***************************************************************************/
/*                                                                         */
/***************************************************************************/

int32_t		main (int argc, char **argv)

{
int32_t		c;
//uint64_t	start, end;

	mem_idx = 0;
	memset ((int8_t *) upper_mem, 0, sizeof (upper_mem));

	while (optind < argc) {
		if ((c = getopt (argc, argv, "a:r:s:v")) == -1) {
			optind++;
			continue;
			}

		switch (c) {
			case	'a':
			case	'r':
			case	's':
				if (*optarg && *optarg != ' ') {
					parse_values (optarg, c);
					}
				break;
			case	'v':
				verbose++;
				break;

			default:
				fprintf (stderr, "seedres - unknown option\n");
				break;
			}
		}

	pnpbios ();

//	find_ram ();
//	if (seed_resource (&free_mem_start, &free_mem_end, RSRCDBMGR_PCI_MEMORY))
//		return (1);

//	find_ranges ();

//	setup_memory ();

//	start = LOW_PORT;
//	end = HIGH_PORT;
//	alloc_resource (&start, &end, RSRCDBMGR_IO_PORT);
//	start = end = 14;	
//	alloc_resource (&start, &end, RSRCDBMGR_IRQ);
//	start = end = 15;	
//	alloc_resource (&start, &end, RSRCDBMGR_IRQ);

	return (0);
}
