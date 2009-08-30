/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "kdebug.h"
#include <stdarg.h>
#include <string.h>

//#define SYSPAGE_BUSTED
//#define SYSPAGE_REALLY_BUSTED

#ifdef SYSPAGE_BUSTED

#define MONBUF		0xb0000
#define COLBUF		0xb8000

void
hw_vgaout(struct syspage_entry *syspage, char c) {
	unsigned char *dp;
	unsigned row, col, ncols, nrows;
#ifdef SYSPAGE_REALLY_BUSTED
	unsigned base = 0xfec00000; 
#else
	unsigned base = (unsigned)_syspage_ptr->un.x86.real_addr;
#endif

	col = *(unsigned char *) (base + 0x450);
	row = *(unsigned char *) (base + 0x451);

	ncols = *(unsigned short *) (base + 0x44a);
	nrows = 25;

	if (c == '\n'  ||  c == 0x0a) {
		++row;
	} else if (c == '\r') {
		col = 0;
	} else if (c == 0x0c) {
		row = col = 0;
	} else {
		dp = (unsigned char *) (base + MONBUF + (row * ncols + col) * 2);
		*dp = c;
		*(dp+1) = 0x0f;
		dp = (unsigned char *) (base + COLBUF + (row * ncols + col) * 2);
		*dp = c;
		*(dp+1) = 0x0f;
		++col;
	}

	if (col >= ncols) {
		++row;
		col = 0;
	}

	if(row >= nrows) {
		--row;
		dp = (unsigned char *) (base + MONBUF);
		memcpy(dp, dp + 2*ncols, 2*24*ncols);
		memset(dp + 24*2*ncols, 0, 2*ncols);
		dp = (unsigned char *) (base + COLBUF);
		memcpy(dp, dp + 2*ncols, 2*24*ncols);
		memset(dp + 24*2*ncols, 0, 2*ncols);
	}

	*(unsigned char *) (base + 0x450) = col;
	*(unsigned char *) (base + 0x451) = row;
}		

#endif


void
kprintf_init(void) {
	void (*outchar)(struct syspage_entry *, char);

#if defined(SYSPAGE_BUSTED)
	outchar = hw_vgaout;
#else
	outchar = channel0->display_char;
#endif
	kprintf_setup(outchar, sizeof(paddr_t));
}
