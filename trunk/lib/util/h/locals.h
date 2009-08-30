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
 *	locals.h	prototypes and miscellaneous stuff for the local lib routines
 *

 */
#ifndef _LOCALS_H_INCLUDED
#define _LOCALS_H_INCLUDED

/*	equal macros */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef strequal
#define strequal(a,b)		(!strcmp((a),(b)))
#define strnequal(a,b,n)	(!strncmp((a),(b),(n)))
#endif

char	*get_disk_name( char *, char * );
char	*get_rawdisk( char *, char * );
char	*get_partn( char *, char * );
int		 set_escape_string(char *match, char *xlat);
int		 stresc(char *s, char **update);
int		 strnbcmp( char *l, char *m );
char	*(strconcat)( char *dest, ... );
int		 (streqany)( char *dest, ... );
int		 (strneqany)( char *base, char *src, ... );
int		 (streqlist)( char *base, char **list );
int		 (strneqlist)( char *base, char **list, int n );
int		 (strequal)( char *base, char *cmp );
int		 (strnequal)( char *base, char *cmp, int n );
char	*(strpasswhite)( char *src );
char	*(strpretrim)( char *src );
char	*(strsqz)( char *str, char c );
char	*(strtrim)( char *src );

#define iswhite(a) ((unsigned)a <= ' ')

/*
 *	Local prototypes for functions in "fs_mount.c"
 *	Required by the MOUNT utility
 */
int mount 				 (char *, char *, int);
int fsys_mount_partition (char *, unsigned, long, long);
int fsys_mount_ext_part  (char *, unsigned, unsigned, long, long);

#ifdef __cplusplus
};
#endif
#endif	/*  _LOCALS_H_INCLUDED */
