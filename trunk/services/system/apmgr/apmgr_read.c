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

/*==============================================================================
 * 
 * apmgr_read
 * 
 * Provide resource manager read() processing for the partitioning module
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"

static int _apmgr_readdir(apmgr_attr_t *attr, off_t *offset, void *buf, size_t size);

/*
 * apmgr_read
 * 
 * Only ever called for a /proc/<pid> and only ever returns a single directory
 * entry for "partition"
*/
int apmgr_read(resmgr_context_t *ctp, io_read_t *msg, void *_ocb)
{
	apmgr_ocb_t  *ocb = (apmgr_ocb_t *)_ocb;
	int  status;
	struct dirent *dir = NULL;
	unsigned int  nbytes = 0;
	PROCESS *prp = NULL;
	int  ret = EOK;

	if (ocb == NULL)
		return ENOENT;

	if (msg->i.type != _IO_READ)
		return EBADF;
	else if ((status = iofunc_read_verify(ctp, msg, &ocb->ocb, NULL)) != EOK)
		return status;
	else if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return ENOSYS;
	else if ((ocb->pid > 0) && ((prp = proc_lock_pid(ocb->pid)) == NULL))
		return EBADF;	// FIX ME - if the process /proc/<pid> is gone
						//		is this ret code adequate to cause the
						//		opener to close thus releasing the ocb ?

	switch (ocb->apmgr_type)
	{
		/*
		 * The ocb is for an open on /proc/<pid>/. The only information to be
		 * read is a single ./partition/ directory entry
		*/
		case apmgr_type_NONE:
		{
			unsigned size = msg->i.nbytes;
			static const char part_str[] = "partition";
			
			if (ocb->ocb.offset > 0)
				break;

			dir = (struct dirent *)(void *)msg;
			memset(dir, 0, sizeof(*dir));
		
			dir->d_reclen = (sizeof(*dir) + sizeof(part_str) + 3) & ~3;
			nbytes += dir->d_reclen;
			if(nbytes > size) {
				ret = EMSGSIZE;
				break;
			}

			dir->d_ino = 0;	// FIX ME - need unique ino for all of my resmgrs;
			dir->d_offset = ocb->ocb.offset++;
			memcpy(dir->d_name, part_str, sizeof(part_str));
			dir->d_namelen = sizeof(part_str) - 1;
			break;
		}

		/*
		 * The ocb is for an open on /proc/<pid>/partition/. There is (currently)
		 * only 2 directory entries to return, ./mem/ and ./sched/ depending on
		 * whether they are installed or not
		*/
		case apmgr_type_PART:
		case apmgr_type_PROC_PART:
		{
			unsigned size = msg->i.nbytes;
			unsigned installed_entries = 0;

			dir = (struct dirent *)(void *)msg;
			memset(dir, 0, min(sizeof(*dir), size - nbytes));
			installed_entries += MEMPART_INSTALLED() ? 1 : 0;
			installed_entries += SCHEDPART_INSTALLED() ? 1 : 0;

			if ((ocb->ocb.offset >= 0) && (ocb->ocb.offset < installed_entries))
			{
				if (MEMPART_INSTALLED())
				{
					static const char mem_str[] = "mem";
				
					dir->d_reclen = (sizeof(*dir) + sizeof(mem_str) + 3) & ~3;
					nbytes += dir->d_reclen;
					if(nbytes > size) {
						ret = EMSGSIZE;
						break;
					}

					dir->d_ino = 0;	// FIX ME - need unique ino for all of my resmgrs;
					dir->d_offset = ocb->ocb.offset++;
					memcpy(dir->d_name, mem_str, sizeof(mem_str));
					dir->d_namelen = sizeof(mem_str) - 1;
					dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
					memset(dir, 0, min(sizeof(*dir), size - nbytes));
				}
				if (SCHEDPART_INSTALLED())
				{
					static const char sched_str[] = "sched";
				
					dir->d_reclen = (sizeof(*dir) + sizeof(sched_str) + 3) & ~3;
					nbytes += dir->d_reclen;
					if(nbytes > size) {
						ret = EMSGSIZE;
						break;
					}
			
					dir->d_ino = 0;	// FIX ME - need unique ino for all of my resmgrs;
					dir->d_offset = ocb->ocb.offset++;
					memcpy(dir->d_name, sched_str, sizeof(sched_str));
					dir->d_namelen = sizeof(sched_str) - 1;
					dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
					memset(dir, 0, min(sizeof(*dir), size - nbytes));
				}
			}
			CRASHCHECK(ocb->ocb.offset < installed_entries);

			/* fill in any group or pseudo names under "/partition" */
			if (ocb->apmgr_type == apmgr_type_PART)
			{
				if ((ret = PART_ATTR_LOCK(ocb->attr)) == EOK)
				{
					if (S_ISDIR(ocb->attr->attr.mode))
					{
						size_t  space_left = size - nbytes;
						off_t  pretend_offset = (ocb->ocb.offset >= installed_entries) ? (ocb->ocb.offset - installed_entries) : 0;
						/*
						 * use _apmgr_readdir() function to fill in the children of
						 * ocb->attr.
						 * Note that the "mem" and "sched" entries above need to be
						 * hidden from _apmgr_readdir() (hence the pretend_offset = 0)
						 * as they do not appear as children to the ocb->attr and
						 * _apmgr_readdir() would otherwise see an ocb->ocb.offset >
						 * the number of children
						*/
						int  n = _apmgr_readdir((apmgr_attr_t *)ocb->attr, (off_t *)&pretend_offset, dir, space_left);

						ret = (n < 0) ? -n : EOK;
						if (n > 0)
						{
							ocb->ocb.offset += pretend_offset;
							nbytes += n;
							if(nbytes > size) {
								ret = EMSGSIZE;
							}
						}
					}
					else
					{
						ret = EBADF;
					}
					PART_ATTR_UNLOCK(ocb->attr);
				}
			}

			dir = (struct dirent *)(void *)msg;	// point back to beginning of buffer
			break;
		}

		case apmgr_type_NAME:
		{
			int  r;
			void *reply_msg;

			// FIX ME - why can't we use 'msg' as 'reply_msg' ?
//			if ((reply_msg = calloc(1, msg->i.nbytes)) == NULL)
//				return ENOMEM;
reply_msg = msg;
		
			if ((r = PART_ATTR_LOCK(ocb->attr)) != EOK)
			{
//				free(reply_msg);	// FIX ME - won't be required as per above
				return r;
			}
			if (S_ISDIR(ocb->attr->attr.mode))
			{
				size_t size = msg->i.nbytes;
				if ((r = _apmgr_readdir((apmgr_attr_t *)ocb->attr, (off_t *)&ocb->ocb.offset, reply_msg, size)) >= 0)
				{
					MsgReply(ctp->rcvid, r, reply_msg, r);
					r = EOK;
				}
			}
			else
				r = EBADF;

			PART_ATTR_UNLOCK(ocb->attr);
//			free(reply_msg);
			return (r == EOK) ? _RESMGR_NOREPLY : -r;
		}

		/*
		 * The ocb is for an open on '/proc/<pid>/partition/mem/', The specific
		 * partitions that the process is associated with will be obtained from
		 * the memory partitioning resource manager 
		*/
		case apmgr_type_MEM:
		{
			off_t  offset = ocb->ocb.offset;
			nbytes = msg->i.nbytes;
			dir = (struct dirent *)(void *)msg;
			memset(dir, 0, nbytes);
			ret = APMMGR_READ(prp, (apmmgr_attr_t *)ocb->attr, &offset, dir, &nbytes);
			ocb->ocb.offset = offset;
#if _IOFUNC_OFFSET_BITS - 0 == 32
			ocb->ocb.offset_hi = 0;
#endif
			break;
		} 
		/*
		 * The ocb is for an open on '/proc/<pid>/partition/sched/', The specific
		 * partitions that the process is associated with will be obtained from
		 * the scheduling partitioning resource manager 
		*/
		case apmgr_type_SCHED:
		{
			nbytes = msg->i.nbytes;
			dir = (struct dirent *)(void *)msg;
			memset(dir, 0, nbytes);
			ret = APSMGR_READ(prp, (apsmgr_attr_t *)ocb->attr, (off_t *)&ocb->ocb.offset, dir, &nbytes);
			break;
		}

#ifndef NDEBUG		
		default: crash();
#else	/* NDEBUG */
		default:
			ret = EBADF;
			break;
#endif	/* NDEBUG */
	}
	if (prp != NULL) proc_unlock(prp);

	if ((ret == EOK) && (nbytes > 0))
	{
		resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
		_IO_SET_READ_NBYTES(ctp, nbytes);
		return _RESMGR_PTR(ctp, dir, nbytes);
	}
	else
		return ret;
}

/*
 * fill in <buf> with all of the directory entries for <attr> (ie. its children)
 * The read is performed starting at the offset pointed to by <offset> the
 * contents are placed into buffer <buf>. The size of <buf> is pointed to by
 * <size>.
 * <offset> is adjusted acordingly.
 * 
 * Returns: the number of bytes placed into <buf> (never more than <size>)
*/
static int _apmgr_readdir(apmgr_attr_t *attr, off_t *offset, void *buf, size_t size)
{
	size_t space_left;
	struct dirent *dir;

	CRASHCHECK(attr == NULL);
	CRASHCHECK(offset == NULL);
	CRASHCHECK(buf == NULL);
	CRASHCHECK(!S_ISDIR(attr->attr.mode));

	if (*offset >= LIST_COUNT(attr->children)) {
		return 0;
	}

	dir = (struct dirent *)buf;
	space_left = size;
	memset(dir, 0, min(sizeof(*dir), space_left));

	if (*offset < LIST_COUNT(attr->children))
	{
		apmgr_attr_t *sibling = (apmgr_attr_t *)LIST_FIRST(attr->children);
		off_t  dir_offset = 0;

		/* add all of the child partitions */
		while (sibling != NULL)
		{
			if (dir_offset >= *offset)
			{
				if (space_left >= (sizeof(struct dirent) + strlen(sibling->name) + 1))
				{
//					dir->d_ino = (int)sibling->data.mpid;
					dir->d_offset = (*offset)++;
					STRLCPY(dir->d_name, sibling->name, NAME_MAX + 1);
					dir->d_namelen = strlen(dir->d_name);
					dir->d_reclen = (sizeof(*dir) + dir->d_namelen + 1 + 3) & ~3;

					space_left -= dir->d_reclen;
					dir = (struct dirent *)((unsigned int)dir + (unsigned int)dir->d_reclen);
					memset(dir, 0, min(sizeof(*dir), space_left));
				}
				else
				{
					break;
				}
			}
			++dir_offset;
			sibling = (apmgr_attr_t *)LIST_NEXT(sibling);
		}
	}
	return size - space_left;
}


__SRCVERSION("$IQ: apmgr_read.c,v 1.23 $");

