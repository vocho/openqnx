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
 *  tar.h    Symbolic constants for Extended tar format
 *

 */

#ifndef _TAR_H_INCLUDED
#define _TAR_H_INCLUDED

/*
 * This header is taken from IEEE Std. 1003.1-1988 10.1.1
 */

#define TMAGIC      "ustar"     /* ustar and a null */
#define TMAGLEN     6
#define TVERSION    "00"        /* 00 and no null */
#define TVERSLEN    2

/* Values used in typeflag field. */
#define REGTYPE     '0'     /* Regular File */
#define AREGTYPE    '\0'        /* Regular File */
#define LNKTYPE     '1'     /* Hard Link */
#define SYMTYPE     '2'     /* Symbolic Link */
#define CHRTYPE     '3'     /* Character Special File */
#define BLKTYPE     '4'     /* Block Special File */
#define DIRTYPE     '5'     /* Directory */
#define FIFOTYPE    '6'     /* FIFO */
#define CONTTYPE    '7'     /* Reserved */

/* Bits used in the mode field. */
#define TSUID   04000   /* Set UID on execution */
#define TSGID   02000   /* Set GID on execution */
#define TSVTX   01000   /* Sticky bit */

/* Mode field continued; File permissions. */
#define TUREAD  00400   /* read by owner */
#define TUWRITE 00200   /* write by owner */
#define TUEXEC  00100   /* execute/search by owner */
#define TGREAD  00040   /* read by group */
#define TGWRITE 00020   /* write by group */
#define TGEXEC  00010   /* execute/search by group */
#define TOREAD  00004   /* read by other */
#define TOWRITE 00002   /* write by other */
#define TOEXEC  00001   /* execute/search by other */

#endif  

/* __SRCVERSION("tar.h $Rev: 153052 $"); */
