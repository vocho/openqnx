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





#include "freeze.h"
#include "huf.h"

/* get one byte */
/* returning in Bit7...0 */

short GetByte ()
{
	register u_short dx = getbuf;
	register u_short c;

	if (getlen <= 8) {
		c = getchar ();
		if ((short)c < 0) {

/* Frozen file is too short. This is fatal error.
 * Really the second absent byte indicates a error.
 * ("Packed file is corrupt." :-) )
 */
		    if (corrupt_flag)
			corrupt_message();
		    corrupt_flag = 1;
		    c = 0;
		}
		dx |= c << (8 - getlen);
		getlen += 8;
	}
	getbuf = dx << 8;
	getlen -= 8;
	return (dx >> 8) & 0xff;
}

/* get N bit */
/* returning in Bit(N-1)...Bit 0 */

short GetNBits (n)
	register u_short n;
{
	register u_short dx = getbuf;
	register u_short c;

	static u_short mask[17] = {
		0x0000,
		0x0001, 0x0003, 0x0007, 0x000f,
		0x001f, 0x003f, 0x007f, 0x00ff,
		0x01ff, 0x03ff, 0x07ff, 0x0fff,
		0x1fff, 0x3fff, 0x7fff, 0xffff };

	static u_short shift[17] = {
		16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
	};

	if (getlen <= 8)
		{
			c = getchar ();
			if ((short)c < 0) {
			    if (corrupt_flag)
				corrupt_message();
			    corrupt_flag = 1;
			    c = 0;
			}
			dx |= c << (8 - getlen);
			getlen += 8;
		}
	getbuf = dx << n;
	getlen -= n;
	return (dx >> shift[n]) & mask[n];
}

/* output C bits */
void Putcode (l, c)
	register u_short l;
	u_short c;
{
	register u_short len = putlen;
	register u_short b = putbuf;
	b |= c >> len;
	if ((len += l) >= 8) {
		putchar ((int)(b >> 8));
		if ((len -= 8) >= 8) {
			putchar ((int)b);
			bytes_out += 2;
			len -= 8;
			b = c << (l - len);
		} else {
			b <<= 8;
			bytes_out++;
		}
	}
	if (ferror(stdout))
		writeerr();
	putbuf = b;
	putlen = len;
}
