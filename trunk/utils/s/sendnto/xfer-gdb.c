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





#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include "sendnto.h"

#include _NTO_HDR_(sys/elf.h)
	

static int
safe_getdevice(int devicefd, int timeout) {
	int	c;

	c = output.get(devicefd, timeout);
	if(c == -1) {
		fprintf(stderr, "No response from target\n");
		exit(EXIT_FAILURE);
	}
	return(c);
}

static void
send_packet(int devicefd, char *buff) {
	unsigned	cksum;
	char		*p;
	int			c;

	cksum = 0;
	for(p = &buff[1]; *p != '\0'; ++p) cksum += *p;
	p += sprintf(p, "#%2.2x", cksum & 0xff);

	//Think about compression in the future....

	do {
		output.write(devicefd, buff, p - buff);
		c = safe_getdevice(devicefd, 5);
	} while(c != '+');
}

static void
get_packet(int devicefd, char *buff) {
	int			c;
	char		*p;

	p = buff;
	do {
		c = safe_getdevice(devicefd, 5);
	} while(c != '$');
	for( ;; ) {
		*p = safe_getdevice(devicefd, 1);
		if(*p == '#') break;
		++p;
	}
	*p = '\0';
	//eat cksum
	safe_getdevice(devicefd, 1);
	safe_getdevice(devicefd, 1);

	if(buff[0] == 'E') {
		fprintf(stderr, "Error response from target: '%s'\n", buff);
		exit(EXIT_FAILURE);
	}
}

struct reginfo {
	unsigned short	cpu;
	unsigned short	regnum;
	unsigned short	regsize;
};

const struct reginfo rinfo[] = {
	{ EM_386,	8, 4 },
	{ EM_486,	8, 4 },
	{ EM_PPC,	64, 4 },
	{ EM_MIPS,	37,	8 },
	{ EM_ARM,	15, 4 },
	{ EM_SH,	16, 4 },
};

int
xfer_start_gdb(int devicefd) {
	unsigned	i;

	if(gdb_pc_regnum == -1) {
		i = 0;
		for( ;; ) {
			if(i >= NUM_ELTS(rinfo)) {
				fprintf(stderr, "Unknown machine type %d - no PC regnum known\n", gdb_cputype);
				exit(EXIT_FAILURE);
			}
			if(rinfo[i].cpu == gdb_cputype) break;
			++i;
		}
		gdb_pc_regnum = rinfo[i].regnum;
		gdb_pc_regsize = rinfo[i].regsize;
	}
	return(0);
}

int
xfer_data_gdb(int devicefd, int seq, unsigned long addr, const void *data, int nbytes) {
	char			*p;
	static char		*buff = NULL;
	static unsigned	buffsize = 0;
	unsigned		newsize;

	if(nbytes > 150) {
		//
		// Don't let a packet go over 400 bytes in size
		//
		unsigned 	half = nbytes >> 1;

		xfer_data_gdb(devicefd, 0, addr, data, half);
		xfer_data_gdb(devicefd, 0, addr + half, (uint8_t *)data + half, nbytes - half);
		return(0);
	}

	newsize = (nbytes * 2) + 32;
	if(newsize > buffsize) {
		buffsize = newsize;
		buff = realloc(buff, newsize);
	}
		 
	p = buff;
	p += sprintf(buff, "$M%lx,%x:", addr, nbytes);
	while(nbytes != 0) {
		p += sprintf(p, "%2.2x", *(uint8_t *)data);
		data = (uint8_t *)data + 1;
		--nbytes;
	}
	send_packet(devicefd, buff);

	get_packet(devicefd, buff);

	return(0);
}

int
xfer_done_gdb(int devicefd, int seq, unsigned long start_addr) {
	char		buff[64];
	char		*p;
	unsigned	i;

	p = buff;
	p += sprintf(buff, "$P%x=", gdb_pc_regnum);
	if(target_endian) {
		//big endian
		sprintf(p, "%*.*lx", gdb_pc_regsize*2, gdb_pc_regsize*2, start_addr);
	} else {
		// little endian
		for(i = 0; i < gdb_pc_regsize; ++i) {
			p += sprintf(p, "%2.2lx", start_addr & 0xff);
			start_addr >>= 8;
		}
	}
	send_packet(devicefd, buff);
	get_packet(devicefd, buff);
	if(buff[0] == '\0') {
		fprintf(stderr, "Target needs to support 'P' request\n");
		exit(EXIT_FAILURE);
	}

	sprintf(buff, "$c");
	send_packet(devicefd, buff);

	return(0);
}
