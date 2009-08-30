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



#if __WATCOMC__ < 950
/*
 * prototype stdio-type system...
 */
#ifndef _fstio_h_included
#define _fstio_h_included


#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#define fgetc(p)		(((p)->_flag & _UNGET) ? \
						((p)->_flag &= ~_UNGET, (p)->_ungotten) : \
						(--(p)->_cnt < 0 ? _fillbuf(p) : (int) *(p)->_ptr++))
						
#if 0
#define Putc(x,p)	(--(p)->_cnt < 0 ? _flushbuf((unsigned char)(x),(p)) : \
					(int) (*(p)->_ptr++ = (unsigned char)(x)))

#define Getchar()	Getc(stdin)
#define Putchar(c)  Putc(c,stdout)
#endif

_fillbuf(FILE *fp);
_flushbuf(int x, FILE *fp);

#define fgets Fgets
char *Fgets(char *bufp, int lim, FILE *f);

#endif
#ifdef __cplusplus
};
#endif
#else
#include <stdio.h>
#endif
