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
 *  dirent.h    Portable directory entry definitions
 *

 */

#ifndef _DIRENT_H_INCLUDED
#define _DIRENT_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__EXT_QNX)
#ifndef __STAT_H_INCLUDED
#include <sys/stat.h>
#endif
#endif

#if defined(__OFF_T)
typedef __OFF_T		off_t;
#undef __OFF_T
#endif

#if defined(__OFF64_T)
typedef __OFF64_T	off64_t;
#undef __OFF64_T
#endif

#if defined(__INO_T)
typedef __INO_T		ino_t;
#undef __INO_T
#endif

#if defined(__INO64_T)
typedef __INO64_T	ino64_t;
#undef __INO64_T
#endif

#include <_pack64.h>

struct dirent {
#if _FILE_OFFSET_BITS - 0 == 64
	ino_t			d_ino;			/* File serial number.					*/
	off_t			d_offset;
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
#if defined(__LITTLEENDIAN__)
	ino_t			d_ino;			/* File serial number.					*/
	ino_t			d_ino_hi;
	off_t			d_offset;
	off_t			d_offset_hi;
#elif defined(__BIGENDIAN__)
	ino_t			d_ino_hi;
	ino_t			d_ino;			/* File serial number.					*/
	off_t			d_offset_hi;
	off_t			d_offset;
#else
 #error endian not configured for system
#endif
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
	_Int16t				d_reclen;
	_Int16t				d_namelen;
	char				d_name[1];
};

#if _LARGEFILE64_SOURCE - 0 > 0
struct dirent64 {
	ino64_t				d_ino;
	off64_t				d_offset;
	_Int16t				d_reclen;
	_Int16t				d_namelen;
	char				d_name[1];
};
#endif

#if defined(__EXT_QNX)
enum dirent_extra_type {
	_DTYPE_NONE,
	_DTYPE_STAT,	
	_DTYPE_LSTAT
};

struct dirent_extra {
	_Uint16t			d_datalen;		/* data size (without header) */
	_Uint16t			d_type;
	_Uint32t			d_reserved;
	/* unsigned char	d_data[d_datalen] */
};

struct dirent_extra_stat {
	_Uint16t			d_datalen;		/* sizeof(struct stat) */
	_Uint16t			d_type;
	_Uint32t			d_reserved;
	struct stat			d_stat;
};


/*
 * The following macros can be used to extract the data.
 * e.g. for(extra = _DEXTRA_FIRST(dirent); _DEXTRA_VALID(extra, dirent); extra = _DEXTRA_NEXT(extra)) {
 *          switch(extra->d_type) {
 */
#define _DEXTRA_FIRST(dirent)           ((struct dirent_extra*)((_Uintptrt)(dirent) + ((((_Uintptrt)(dirent)->d_name - \
                                        (_Uintptrt)(dirent)) + (dirent)->d_namelen + 1 + 7) & ~7)))
#define _DEXTRA_VALID(extra, dirent)    (  (_Intptrt)(((_Uintptrt)(extra) - (_Uintptrt)(dirent)) + \
                                            sizeof(struct dirent_extra)) <= (dirent)->d_reclen && \
                                           (_Intptrt)(((_Uintptrt)(extra) - (_Uintptrt)(dirent)) + sizeof(struct dirent_extra) + \
                                            ((struct dirent_extra *)(extra))->d_datalen) <= (dirent)->d_reclen)
#define _DEXTRA_NEXT(last)              ((struct dirent_extra*)(((_Uintptrt)(last) + sizeof(struct dirent_extra) + \
                                            (((struct dirent_extra *)(last))->d_datalen) + 7) & ~7))

#endif

typedef struct _dir		DIR;

#include <_packpop.h>


__BEGIN_DECLS
/*
 *  POSIX 1003.1 Prototypes.
 */

extern int		closedir(DIR *__dirp);
extern DIR		*opendir(__const char *__dirname);
extern struct dirent	*readdir(DIR *__dirp) __ALIAS64("readdir64");
extern void		rewinddir(DIR *__dirp);
extern void		seekdir(DIR *__dirp, long int __pos);
extern long int		telldir(DIR *__dirp);

#if defined(__EXT_POSIX1_199506)
extern int		readdir_r(DIR *__dirp, struct dirent *__dirent, struct dirent **__result);
#endif

#if _LARGEFILE64_SOURCE - 0 > 0
extern struct dirent64 *readdir64(DIR *__dirp);
#endif

#if defined(__EXT_UNIX_MISC)
extern int		alphasort(const void *__a, const void *__b);
extern int		scandir(char *__dirname, struct dirent ***__namelist,
				int (*__select)(const struct dirent *),
				int (*__compar)(const void *,const void *));
#endif

#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__GNUC__)
/* use __ALIAS64 define */
#elif defined(__WATCOMC__)
#pragma aux readdir "readdir64";
#elif defined(_PRAGMA_REDEFINE_EXTNAME)
#pragma redefine_extname readdir readdir64
#else
#define readdir readdir64
#endif
#endif

#if defined(__EXT_QNX)
/*
 The following function can be used to control the behaviour
 of the directory operations which occur on a DIR * structure.
*/
#define D_GETFLAG		1			/* Get the DIR * status flag */
#define D_SETFLAG		2			/* Set the DIR * status flag */

/* DIR * status flag values */
#define		D_FLAG_FILTER   0x00000001	/* Filter out duplicate entries in the readdir */
#define		D_FLAG_STAT     0x00000002	/* Attempt to return extra stat information */
#define		D_FLAG_RESERVED 0x80000000	

int dircntl(DIR *__dir, int __cmd, ...);

#endif

__END_DECLS

#endif

/* __SRCVERSION("dirent.h $Rev: 163784 $"); */
