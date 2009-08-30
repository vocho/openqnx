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




#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

LIBC_WEAK(memcpyv, __memcpyv);

/*
 * memcpyv
 *
 * Copy data from one iov to another.
 */
size_t memcpyv(const struct iovec *dst, int dparts, int doff, const struct iovec *src, int sparts, int soff) {
	unsigned char	*saddr, *daddr;
	int				slen, dlen;
	size_t			nbytes;

	/* Check for a dst offset and skip over it. */
	while(doff >= (dlen = GETIOVLEN(dst))) {
		doff -= dlen;
		if(--dparts == 0) { 	/* No more parts. */
			return 0;
		}
		dst++;
	}
	dlen -= doff;
	daddr = (unsigned char *)GETIOVBASE(dst) + doff;

	/* Check for a src offset and skip over it. */
	while(soff >= (slen = GETIOVLEN(src))) {
		soff -= slen;
		if(--sparts == 0) { 	/* No more parts. */
			return 0;
		}
		src++;
	}
	slen -= soff;
	saddr = (unsigned char *)GETIOVBASE(src) + soff;

	/* Now we move the data. */
	nbytes = 0;
	for(;;) {
		int						len;

		/* Check how many bytes can be moved. */
		if((len = min(slen, dlen))) {
			nbytes += len;
			memcpy(daddr, saddr, len);
		}

		/* Adjust source. */
		saddr += len;
		if((slen -= len) == 0) {
			if(--sparts == 0) {
				break;
			}
			src++;
			saddr = (unsigned char *)GETIOVBASE(src);
			slen  = GETIOVLEN(src);
		}

		/* Adjust dest. */
		daddr += len;
		if((dlen -= len) == 0) {
			if(--dparts == 0) {
				break;
			}
			dst++;
			daddr = (unsigned char *)GETIOVBASE(dst);
			dlen  = GETIOVLEN(dst);
		}
	}

	return nbytes;
}

__SRCVERSION("memcpyv.c $Rev: 167420 $");
