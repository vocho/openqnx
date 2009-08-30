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





#include <stdio.h>

/* get rid of register declarations */
#define register

#ifdef SUN4
#include <sys/stdtypes.h>
#else
# ifndef getc
#   define getc(p)         (--(p)->_cnt < 0 ? _filbuf(p) : (int) *(p)->_ptr++)
# endif
# ifndef putc
#   define putc(x, p)      (--(p)->_cnt < 0 ? _flsbuf((unsigned char) (x), (p)) : (int) (*(p)->_ptr++ = (unsigned char) (x)))
# endif
#ifndef BSD4_2
#ifndef __QNXNTO__
  typedef unsigned short u_short;
#endif
#endif
#endif

/* end (for GCC) */

#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utime.h>

#ifdef DEBUG
#include <assert.h>
#endif

#ifdef __QNX__
#include <unistd.h>
#include <string.h>
#include <limits.h>

#define SIGRET void
#endif

#ifdef __TURBOC__
#define MSDOS
#include <io.h>
#endif

#ifdef MSDOS
#include <stdlib.h>
#include <fcntl.h>
#endif

typedef unsigned char uchar;

#if defined(BITS) && BITS > 14
typedef unsigned long hash_t;
#else
typedef u_short hash_t;
#endif

#ifdef  lint
#define N               256
#else
#define N               8192    /* buffer size */
#endif

#define F               256     /* pre-sence buffer size */
#define THRESHOLD	2

#define _NCHAR         (256 - THRESHOLD + _F + 1) /* {code : 0 .. N_CHAR-1} */

#define _T              (_NCHAR * 2 - 1)        /* size of table */
#define _R              (_T - 1)                 /* root position */
#define _NN             8092
#define _F              256

#ifndef COMPAT
#define N_CHAR          _NCHAR
#define T               _T
#define R               _R
#else
#define _FO             60
#define _NO             4096
#define _NCHARO        (256 - THRESHOLD + _FO + 1)
#define _TO             (_NCHARO * 2 - 1)
#define _RO             (_TO - 1)
#define N_CHAR          (new_flg ? _NCHAR : _NCHARO)
#define T               (new_flg ? _T : _TO)
#define R               (new_flg ? _R : _RO)
#endif

#define ENDOF           256

extern uchar   Table[];

extern long in_count, bytes_out;

extern uchar    text_buf[];
extern u_short   match_position, match_length;

extern short quiet;

/* Note indicator_threshold is triangle number of Kbytes */

extern unsigned long indicator_threshold, indicator_count;

extern short do_melt, topipe;

extern uchar magic_header[];

extern int exit_stat;

#include <proto.h>

#ifdef DEBUG
extern short debug;
extern short verbose;
extern char * pr_char();
extern long symbols_out, refers_out;
#endif /* DEBUG */

#ifdef GATHER_STAT
extern long node_steps, node_matches;
#endif

extern short DecodeChar(), DecodePosition(), GetByte(), GetNBits(u_short);

#ifdef COMPAT
extern short DecodeCOld(), DecodePOld(), new_flg;
#endif

#if defined(BSD42) && !defined(BSD4_2)
#define BSD4_2
#endif
