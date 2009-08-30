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




#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <time.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include _NTO_HDR_(gulliver.h)
#include _NTO_HDR_(sys/elf.h)
#include <util/fs_qnx4_util.h>

#ifdef __BIGENDIAN__
void flip_endian_xtnt(qnx4fs_xtnt_t *xtptr)
{
	ENDIAN_SWAP32(&xtptr->xtnt_blk);
	ENDIAN_SWAP32(&xtptr->xtnt_size);
}

void flip_endian_xblk(qnx4fs_xblk_t *xbptr)
{
	int i;

	ENDIAN_SWAP32(&xbptr->xblk_next_xblk);
	ENDIAN_SWAP32(&xbptr->xblk_prev_xblk);
	/* xbptr->xblk_num_xtnts is unsigned char */
    /* xbptr->xblk_spare[3] doesn't need swapping (and isn't dword aligned) */    
    ENDIAN_SWAP32(&xbptr->xblk_num_blocks);
	for (i=0;i<QNX4FS_MAX_XTNTS_PER_XBLK;i++) 
		flip_endian_xtnt(&xbptr->xblk_xtnts[i]);
    /* xbptr->xblk_signature[8] is char and doesn't need swapping */
    flip_endian_xtnt(&xbptr->xblk_first_xtnt);
}

void flip_endian_link(qnx4fs_dir_entry_t *deptr)
{
    /* d_link.l_fname, l_inode_blk, l_spare, l_status are all char fields */
	ENDIAN_SWAP32(&deptr->d_link.l_inode_blk);
}

void flip_endian_ino(qnx4fs_dir_entry_t *deptr)
{
	/* d_inode.name is char */
	ENDIAN_SWAP32(&deptr->d_inode.i_size);
	flip_endian_xtnt(&deptr->d_inode.i_first_xtnt);
	ENDIAN_SWAP32(&deptr->d_inode.i_xblk);
	ENDIAN_SWAP32(&deptr->d_inode.i_ftime);
	ENDIAN_SWAP32(&deptr->d_inode.i_mtime);
	ENDIAN_SWAP32(&deptr->d_inode.i_atime);
	ENDIAN_SWAP32(&deptr->d_inode.i_ctime);
    ENDIAN_SWAP16(&deptr->d_inode.i_num_xtnts);
    ENDIAN_SWAP16(&deptr->d_inode.i_mode);
    ENDIAN_SWAP16(&deptr->d_inode.i_uid);
    ENDIAN_SWAP16(&deptr->d_inode.i_gid);
    ENDIAN_SWAP16(&deptr->d_inode.i_nlink);
    /* d_inode.i_zero is char */
    /* d_inode.i_type is ftype_t which is char */
    /* d_inode.i_status is char */
}

void flip_endian_dir(qnx4fs_dir_entry_t *deptr)
{
	if (deptr->d_inode.i_status&QNX4FS_FILE_LINK)
		flip_endian_link(deptr);
    else
		flip_endian_ino(deptr);
}

/* the following routines are for situations where the structures
   must be byte-wise preserved but where the values contained in them
   need to be used. They can also be used when a big-endian value 
   needs to be assigned to a little-endian struct/union member */

qnx4fs_nxtnt_t endian_nxtnt_t(qnx4fs_nxtnt_t val)
{
	return ENDIAN_RET16(val);
}

qnx4fs_mode_t endian_mode_t(qnx4fs_mode_t val)
{
	return ENDIAN_RET16(val);

}

qnx4fs_nlink_t endian_nlink_t(qnx4fs_nlink_t val)
{
	return ENDIAN_RET16(val);
}

qnx4fs_uid_t endian_uid_t(qnx4fs_uid_t val)
{
	return ENDIAN_RET16(val);
}

qnx4fs_gid_t endian_gid_t(qnx4fs_gid_t val)
{
	return ENDIAN_RET16(val);
}

qnx4fs_daddr_t endian_daddr_t(qnx4fs_daddr_t val)
{
	return ENDIAN_RET32(val);
}

qnx4fs_size_t endian_size_t(qnx4fs_size_t val)
{
	return ENDIAN_RET32(val);
}

time_t endian_time_t(time_t val)
{
	return ENDIAN_RET32(val);
}

#endif
