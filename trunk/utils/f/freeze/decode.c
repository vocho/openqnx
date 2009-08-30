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

/*
 * Melt stdin to stdout.
 */

void melt ()
{
	register short    i, j, k, n, r, c;
#ifdef COMPAT
	if(new_flg)
#endif
	if(read_header() == EOF)
		return;

	StartHuff();
	n = N;
	for (i = 0; i < n - F; i++)
		text_buf[i] = ' ';
	r = n - F;
	n --;   /* array size --> mask */
	for (in_count = 0;; ) {
		c =
#ifdef COMPAT
		new_flg ? DecodeChar() : DecodeCOld();
#else
		DecodeChar();
#endif
		if (c == ENDOF)
			break;
		if (c < 256) {
			putchar (c);
			text_buf[r++] = c;
			r &= n;
			in_count++;
		} else {
			i = (r -
#ifdef COMPAT
				(new_flg ? DecodePosition() : DecodePOld())
#else
				DecodePosition()
#endif
				- 1) & n;
			j = c - 256 + THRESHOLD;
			for (k = 0; k < j; k++) {
				c = text_buf[(i + k) & n];
				putchar (c);
				text_buf[r++] = c;
				r &= n;
				in_count++;
			}
		}

		if (!quiet && (in_count > indicator_count)) {
			if (ferror(stdout))
				writeerr();
			fprintf(stderr, "%5ldK\b\b\b\b\b\b", in_count / 1024);
			fflush (stderr);
			indicator_count += indicator_threshold;
			indicator_threshold += 1024;
		}
	}
}
