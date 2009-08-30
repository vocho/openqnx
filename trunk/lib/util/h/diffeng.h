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




#ifndef _DIFFENG_H_INCLUDED
#define _DIFFENG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
/*
 * genlcs takes two lists of integers, and generates a mapping table
 * of the longest common subsequence between the tables.
 * The length of the generated table is alen+1.
 */

int *genlcs(int *A, int *B, int alen, int blen);

#ifdef __cplusplus
};
#endif

#endif
