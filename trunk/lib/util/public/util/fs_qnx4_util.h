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
 *  sys/fs_qnx4_util.h
 *
 *  Based on Neutrino's sys/fs_qnx4.h; utilities require this in both QNX4
 *  and nto platforms, and require endian conversion routines. Hence this
 *  header which is common to all utilities on any platform.
 *

 */

/* uses __FS_QNX4_H_INCLUDED as this is a superset of sys/fs_qnx4.h */

#ifndef __FS_QNX4_H_INCLUDED
#define __FS_QNX4_H_INCLUDED

#include <sys/stat.h>
#include <time.h>

#ifdef __BIGENDIAN__
	#define FLIP_ENDIAN_XTNT(xtptr) flip_endian_xtnt(xtptr)
	#define FLIP_ENDIAN_XBLK(xbptr) flip_endian_xblk(xbptr)
    /* endian conversions of dir entry differ depending on the actual
       type of data present in the union (inode or link) */
	#define FLIP_ENDIAN_INO(iptr) flip_endian_ino(iptr)
	#define FLIP_ENDIAN_LINK(lptr) flip_endian_link(lptr)
    /* this will check (dptr->d_inode.status&QNX4FS_FILE_LINK) and will call
       flip_endian_ino or flip_endian_link as appropriate */
    #define FLIP_ENDIAN_DIR(dptr) flip_endian_dir(dptr)

    /* the following will endian-flip values for their respective types;
       used when you want to use the values but not change the originals */
    #define ENDIAN_NXTNT_T(val) endian_nxtnt_t(val)
    #define ENDIAN_MODE_T(val) endian_mode_t(val)
    #define ENDIAN_NLINK_T(val) endian_nlink_t(val)
    #define ENDIAN_UID_T(val) endian_uid_t(val)
    #define ENDIAN_GID_T(val) endian_gid_t(val)
    #define ENDIAN_DADDR_T(val) endian_daddr_t(val)
    #define ENDIAN_SIZE_T(val) endian_size_t(val)
    #define ENDIAN_TIME_T(val) endian_time_t(val)

#else
	#define FLIP_ENDIAN_XTNT(xtptr)
	#define FLIP_ENDIAN_XBLK(xbptr)
	#define FLIP_ENDIAN_INO(iptr)
	#define FLIP_ENDIAN_LINK(lptr)
    #define FLIP_ENDIAN_DIR(dptr)

    #define ENDIAN_NXTNT_T(val) val
    #define ENDIAN_MODE_T(val) val
    #define ENDIAN_NLINK_T(val) val
    #define ENDIAN_UID_T(val) val
    #define ENDIAN_GID_T(val) val
    #define ENDIAN_DADDR_T(val) val
    #define ENDIAN_SIZE_T(val) val
    #define ENDIAN_TIME_T(val) val
#endif

#define QNX4FS_BLOCK_SIZE			512
#define QNX4FS_BOOT_BLOCK			1L
#define QNX4FS_ROOT_BLOCK			2L
#define QNX4FS_BITMAP_BLOCK			3L
#define QNX4FS_MAX_XTNTS_PER_XBLK	60
#define QNX4FS_LONG_NAME_MAX		48
#define QNX4FS_SHORT_NAME_MAX		16

#define QNX4FS_FILE_USED			0x01
#define QNX4FS_FILE_MODIFIED		0x02
#define QNX4FS_FILE_BUSY			0x04
#define QNX4FS_FILE_LINK			0x08
#define QNX4FS_FILE_INODE			0x10
#define QNX4FS_FSYS_CLEAN			0x20
#define QNX4FS_FILE_GROWN			0x40

typedef unsigned short		qnx4fs_nxtnt_t;
typedef unsigned char		qnx4fs_ftype_t;
typedef unsigned short		qnx4fs_mode_t;
typedef unsigned short		qnx4fs_nlink_t;
typedef unsigned short		qnx4fs_uid_t;
typedef unsigned short		qnx4fs_gid_t;
typedef unsigned long		qnx4fs_daddr_t;
typedef unsigned long		qnx4fs_size_t;

typedef struct qnx4fs_xtnt {
	qnx4fs_daddr_t		xtnt_blk;
	qnx4fs_size_t		xtnt_size;
} qnx4fs_xtnt_t;

typedef struct qnx4fs_xblk {
	qnx4fs_daddr_t		xblk_next_xblk;
	qnx4fs_daddr_t		xblk_prev_xblk;
	unsigned char		xblk_num_xtnts;
	char				xblk_spare[3];
	long				xblk_num_blocks;
	qnx4fs_xtnt_t		xblk_xtnts[QNX4FS_MAX_XTNTS_PER_XBLK];
	char				xblk_signature[8];
	qnx4fs_xtnt_t		xblk_first_xtnt;
} qnx4fs_xblk_t;

typedef union qnx4fs_dir_entry  {
	struct qnx4fs_inode_info {                                /* nbytes */
		char				i_fname[QNX4FS_SHORT_NAME_MAX];   /* 16 */
		qnx4fs_size_t		i_size;                           /*  4 */
		qnx4fs_xtnt_t		i_first_xtnt;                     /*  8 (struct) */
		qnx4fs_daddr_t		i_xblk;                           /*  4 */
		time_t				i_ftime;                          /*  4 */
		time_t				i_mtime;                          /*  4 */
		time_t				i_atime;                          /*  4 */
		time_t				i_ctime;                          /*  4 */
		qnx4fs_nxtnt_t		i_num_xtnts;                      /*  2 */
		qnx4fs_mode_t		i_mode;                           /*  2 */
		qnx4fs_uid_t		i_uid;                            /*  2 */
		qnx4fs_gid_t		i_gid;                            /*  2 */
		qnx4fs_nlink_t		i_nlink;                          /*  2 */
		char				i_zero[4];                        /*  4 */
		qnx4fs_ftype_t		i_type;                           /*  1 */
		unsigned char		i_status;                         /*  1 */
	}		d_inode;
	struct qnx4fs_link_info {
		char				l_fname[QNX4FS_LONG_NAME_MAX];    /* 48 */
		qnx4fs_daddr_t		l_inode_blk;                      /*  4 */
		unsigned char		l_inode_ndx;                      /*  1 */
		char				l_spare[10];                      /* 10 */
		unsigned char		l_status;                         /*  1 */
	}		d_link;
} qnx4fs_dir_entry_t;

/*
 *  Extended stat structure.
 *  Contains a lot of QNX specific data.
 *  (Mainly filesystem data corresponding to data on disk.)
 */
#if defined(__CYGWIN32__) || defined(__MINGW32__) || defined(__SOLARIS__) || defined(__LINUX__)
#undef st_mtime
#undef st_atime
#undef st_ctime
#endif

struct _qnx4fs_fsys_stat {
    ino_t           st_ino;         /*  File serial number.                 */
    dev_t           st_dev;         /*  ID of device containing file.       */
    off_t           st_size;
    dev_t           st_rdev;        /*  Device ID, for inode that is device */
    qnx4fs_xtnt_t   st_first_xtnt;
    time_t          st_ftime,       /*  Time created                        */
                    st_mtime,       /*  Time of last data modification      */
                    st_atime,       /*  Time last accessed                  */
                    st_ctime;       /*  Time of last status change          */
    qnx4fs_nxtnt_t  st_num_xtnts;
    mode_t          st_mode;        /*  see below                           */
    qnx4fs_gid_t    st_uid;
    qnx4fs_uid_t    st_gid;
    qnx4fs_nlink_t  st_nlink;
    unsigned char   st_status;
    qnx4fs_daddr_t  st_xblk;
};

#ifdef __BIGENDIAN__
void flip_endian_xtnt(qnx4fs_xtnt_t *);
void flip_endian_xblk(qnx4fs_xblk_t *);
void flip_endian_ino(qnx4fs_dir_entry_t *);
void flip_endian_link(qnx4fs_dir_entry_t *);
void flip_endian_dir(qnx4fs_dir_entry_t *);

qnx4fs_nxtnt_t endian_nxtnt_t(qnx4fs_nxtnt_t);
qnx4fs_mode_t  endian_mode_t(qnx4fs_mode_t);
qnx4fs_nlink_t endian_nlink_t(qnx4fs_nlink_t);
qnx4fs_uid_t   endian_uid_t(qnx4fs_uid_t);
qnx4fs_gid_t   endian_gid_t(qnx4fs_gid_t);
qnx4fs_daddr_t endian_daddr_t(qnx4fs_daddr_t);
qnx4fs_size_t  endian_size_t(qnx4fs_size_t);
time_t         endian_time_t(time_t);
#endif

#endif
