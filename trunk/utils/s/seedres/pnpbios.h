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





#include <_pack1.h>
struct pnpbios_header {
	unsigned char				signature[4];		// "$PnP"
	unsigned char				version;			// In bcd
	unsigned char				length;
	unsigned short				control;
	unsigned char				check_sum;
	unsigned long				event_flag;
	unsigned short				entry_real_off;
	unsigned short				entry_real_seg;
	unsigned short				entry_prot_off;
	unsigned long				entry_prot_seg;
	unsigned long				oem_id;
	unsigned short				data_real_seg;
	unsigned long				data_prot_seg;
};

typedef struct dev_node {
	unsigned short				size;
	unsigned char				handle;
	unsigned long				id;
	unsigned char				type[3];
	unsigned short				attrib;
	/* variable data */
}							pnpbios_node_t;

#include <_packpop.h>

typedef struct pnpbios_handle {
	unsigned					node_size;
	struct pnpbios_header		hdr;
}							pnpbios_t;

extern pnpbios_t *pnpbios_attach(void);
extern void pnpbios_detach(pnpbios_t *handle);
extern int pnpbios_get_num_nodes(pnpbios_t *handle, int *pnum_nodes, int *pnode_size);
extern int pnpbios_get_device_node(pnpbios_t *handle, int *pnode, pnpbios_node_t *devnode, int control);
extern int pnpbios_get_boot_version(pnpbios_t *handle, int *version);

#define PNPBIOS_TYPE_BRIDGE		0x06
