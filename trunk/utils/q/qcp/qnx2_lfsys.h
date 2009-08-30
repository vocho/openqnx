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





#ifndef _LFSYS_H_
#define _LFSYS_H_

#include <io.h>

/*
 * define file structure constants
 */

#define FBLK_SIZE		512
#define FBLK_BITS		9
#define FBLK_MASK		0x1FF
#define XTNT_HDR		16
#define BITMAP_BLK		2L
#define DISK_DESC_BLK	1L

/*
 * define bits in file status byte (fstat)
 */

#define _FILE_MODIFIED	0x10
#define _WINDOW_APP		0x20
#define _USED			0x40
#define _FILE_BUSY		0x80

/*
 * define operating system constants
 */

#define NCPFN 16

/*
 * Extent header
 */

struct xtnt_hdr {
	long prev_xtnt;
	long next_xtnt;
	long size_xtnt;
	long bound_xtnt;
	};

/*
 * Directory Entry
 */

struct dir_entry {
	char fstat;
	long ffirst_xtnt;
	long flast_xtnt;
	long fnum_blks;
	unsigned fnum_xtnt;
	unsigned char fowner;
	unsigned char fgroup;
	unsigned fnum_chars_free; /* No. unused chars in last block */
	long fseconds;
	unsigned char ftype;
	unsigned char fgperms;
	unsigned char fperms;
	unsigned char fattr;
	unsigned fdate[2];
	char fname[NCPFN + 1];
	};

/*
 * File format of directory
 */

struct dir_xtnt {		/* Top bit of dir_index is zero */
	unsigned parent_xtnt;
	unsigned dir_index;
	struct dir_entry directory[0];
	};

struct ext_dir_xtnt {	/* Top bit of dir_index is set */
	unsigned parent_xtnt;
	unsigned dir_index;
	unsigned ext_parent_xtnt;
	} ;

/*
 * Disk control block used by disk_open, disk_close, etc.
 */

struct dcb {
	unsigned dcb_admin;
	unsigned dcb_nid;
	unsigned dcb_tid;
	char dcb_drive;
	char dcb_spare[5];
	} ;

struct r_w_msg {
	char mtype;
	unsigned r_w_drive;
	long r_w_block;
	char r_w_filler[4];
	char r_w_data[0];
	};

/*
 * Disk entry used by drivers
 */

struct disk_entry {
	char disk_type;
	char disk_drv;
	unsigned blk_offset;
	unsigned num_sctrs;
	unsigned sctr_cyl;
	char sctr_trk;
	char disk_entry_reserved;
	unsigned ctl_addr;
	char ctl_int;
	unsigned num_tracks;
	char disk_filler[1];
	} ;

/*
 * Disk Entry used by disk_get_entry, disk_set_entry
 */

struct ldisk_entry {
	struct disk_entry _disk_entry;
	unsigned ext_blk_offset;
	unsigned ext_num_sctrs;
	unsigned write_precomp;
	unsigned disk_arg[3];
	unsigned disk_flags;
	unsigned disk_entry_id;		/* extension valid if 0x1234 */
	} ;
/*
 * Disk flags:
 */

#define DISK_LARGE_EXTENTS	0x0001
#define DISK_LARGE_BLOCKS	0x0002
#define DISK_LARGE_FSYS		0x0004

/*
 * File Error Messages ("ored" with ERROR=0x80 on return)
 */

#define UNABLE_TO_ACCESS	1	/* 0x81 */
#define FILE_BUSY			2	/* 0x82 */
#define DIRECTORY_FULL		3	/* 0x83 */
#define BAD_PATHNAME		4	/* 0x84 */
#define ATTRIBUTE_VIOLATION	5	/* 0x85 */
#define DISK_FULL			6	/* 0x86 */
#define NAME_TOO_LONG		7	/* 0x87 */
#define INVALID_DEVICE		8	/* 0x88 */
#define BAD_DRIVE_NUMBER	9	/* 0x89 */
#define PARTIAL_MATCH		10	/* 0x8A */
#define TOO_MANY_OPEN_FILES	11	/* 0x8B */
#define CORRUPT_DIRECTORY	12	/* 0x8C */
#define FSYS_NO_ACCESS		13	/* 0x8D */
#define TOO_MANY_EXTENTS	14	/* 0x8E */

#endif _LFSYS_H_
