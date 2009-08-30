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
 *  sendmx.h
 *

 */
#ifndef __SENDMX_H_INCLUDED

#ifndef _I86_H_INCLUDED
 #include <i86.h>
#endif

#ifndef __NEUTRINO_H_INCLUDED
 #include <sys/neutrino.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

#define _mxfer_entry				iovec
#define _setmx(_mx, _data, _len)   { (_mx)->addr = (void *)FP_OFF(_data); (_mx)->len = (unsigned)(_len); }
#define mxfer_len					len
#define mxfer_off					addr

/* maximum size of mx vector */
#define MAX_MX_TAB  255
#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __SENDMX_H_INCLUDED
#endif
