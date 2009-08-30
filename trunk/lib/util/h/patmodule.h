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



#ifndef _PATMODULE_H_INCLUDED
#define _PATMODULE_H_INCLUDED
#ifndef _CDEFS_H_INCLUDED
#include <util/defns.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#define P_AFTERDATE		1
#define P_BEFOREDATE	2
#define P_GROUP			4
#define P_OWNER			8
#define P_MODE		 	16
#define P_PATTERN		32

int		patmodule_init  ( int max_patterns, int max_groups, int max_owners,
							  int max_modes );
int		patmodule_enter	( int type , char *opt_arg );
void	patmodule_commit( void );
bool	patmodule_check ( char *fname, struct stat *statbufp);
void	patmodule_free  ( void );
#ifdef __cplusplus
};
#endif
#endif
