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

#include "externs.h"
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <gulliver.h>
#include <sys/image.h>
#include <sys/dcmd_all.h>
#include <sys/ftype.h>
#include <sys/elf.h>
#include <sys/elf_nto.h>
#include <pathmgr_object.h>

struct image_data {
	struct image_header			*addr;
	union image_dirent			*dir;
	dev_t						devno;
	size_t						size;
	size_t						pg_offset;
	OBJECT						*obp;
	int							in_user_space;
};

static struct image_ocb {
	struct image_ocb	*next;
	struct image_ocb	**prev;
	union image_dirent	*dir;
	struct image_data	*image;
	uint16_t			index;
	uint16_t			childcount;
	char				**children;
	int					pos;
	int					ioflag;
	unsigned			mode;
}							*ocb_list;

#define IMAGE_ADDRESSABLE_START(image)	\
			if((image)->in_user_space) ProcessBind(SYSMGR_PID);
#define IMAGE_ADDRESSABLE_DONE(image)	\
			if((image)->in_user_space) ProcessBind(0);
#define IMAGE_ADDRESSABLE_SAVE(image, old)	\
			if((image)->in_user_space) {	\
				(old) = ProcessBind(-1);	\
				ProcessBind(0);				\
				ProcessBind(SYSMGR_PID);	\
			} else {						\
				(old) = -1;					\
			}
#define IMAGE_ADDRESSABLE_RESTORE(image, old)	\
			if((old) >= 0) {					\
				if((old) != 0) ProcessBind(0);	\
				ProcessBind((old));				\
			}


static char *
image_get_path(union image_dirent *dir) {
	if (!dir || !dir->attr.ino) {
		return NULL;
	}

	switch (dir->attr.mode & S_IFMT) {
	case S_IFLNK:
        return dir->symlink.path;
	case S_IFREG:
        return dir->file.path;
	case S_IFDIR:
        return dir->dir.path;
	/*
	case S_IFBLK: case S_IFNAM: case S_IFCHR:
	case S_IFSOCK: case S_IFIFO:
	*/
	default:
        return dir->device.path;
	}
}

/*
 Get a list of all the unique entries under path and stick'em in an array
*/
static int 
image_getdir_children(union image_dirent *startdir, char *path, char ***children, uint16_t *childcount, uint16_t *index) {
	union image_dirent *dir;
	int					len, ncount;
	char				**names;

	ncount = 0;	
	names = NULL;
	len = strlen(path);

	for (dir = startdir; dir->attr.size; dir = (union image_dirent *)((char *)dir + dir->attr.size)) {
		char *entry_name;
		int	 i, entry_len;

		entry_name = image_get_path(dir);
		if (!entry_name || strncmp(path, entry_name, len) != 0) {
			continue;	
		}

		*index = ((uintptr_t)entry_name - (uintptr_t)startdir) + len;

		/* Advance us up to the relevant portion */
		entry_name += len;

		i = 0;	
		while (*entry_name && *entry_name == '/') { entry_name++; i++; }
		if (i <= 0  && len > 0) {
			continue;	//We need at least one slash character seperator, unless we are at root
		}
			
		entry_len = 0;
		while (entry_name[entry_len] && entry_name[entry_len] != '/') { entry_len++; }
		if (!entry_len) {
			continue;	//We need something after the slash
		}
	
		/* 
		 At this point entry_len either points at the end of the string
		 or it points at the / character.  If the null character, we
		 need go no further, just add it.  If a / then we need to check
		 to see if the entry has already been added.
		*/
		for (i=0; i<ncount && names[i]; i++) {
			if (strncmp(names[i], entry_name, entry_len) == 0 && 
			    (names[i][entry_len] == '/' || names[i][entry_len] == '\0')) {
				entry_name = NULL;
				break;
			}
		}

		if (entry_name == NULL) {
			continue;
		}

		if (i >= ncount) {
			char **tmp_names;
			tmp_names = _srealloc(names, ncount * sizeof(char **), (ncount+10) * sizeof(char **));
			if (!tmp_names) {
				break;
			}
			names = tmp_names;
			memset(&names[ncount], 0, 10*sizeof(char **));
			ncount += 10;
		}
		/*
		 Never allocate more path information, just point to the right spot
		 Let the read() call figure out that it needs to chop off the /
		*/
		names[i] = entry_name;
	}

	*children = names;
	*childcount = ncount;
	return ncount;
}

static int 
image_read(resmgr_context_t *ctp, io_read_t *msg, void *vocb) {
	struct image_ocb			*ocb = vocb;
	unsigned					nbytes;
	int							*pos, offset;
	union image_dirent			*dire;
	struct image_data			*image = ocb->image;


	if (!(ocb->ioflag & _IO_FLAG_RD)) {
		return EBADF;
	}

	switch (msg->i.xtype & _IO_XTYPE_MASK) {
	case _IO_XTYPE_NONE:
		pos = &ocb->pos;
		break;
	case _IO_XTYPE_OFFSET:
		*(pos = &offset) = ((struct _xtype_offset *)(msg + 1))->offset;
		break;
	default:
		return ENOSYS;
	}

	IMAGE_ADDRESSABLE_START(image);
	nbytes = 0;
	if ((dire = ocb->dir) && S_ISREG(ocb->mode)) {

		nbytes = msg->i.nbytes;
		if(dire->file.size - *pos < nbytes) {
			nbytes = dire->file.size - *pos;
		}
		SETIOV(ctp->iov + 0, (void *)((char *)image->addr + dire->file.offset + *pos), nbytes);
		if(image->in_user_space) {
			int		r;

			// If the IFS is in procnto's local user space, we have to
			// write the data to the client buffers here, while we still
			// have addressability to it.
			r = resmgr_msgwritev(ctp, ctp->iov, 1, 0);
			if(r == -1) {
				IMAGE_ADDRESSABLE_DONE(image);
				return errno;
			}
			// we don't need the MsgReply in the resmgr to transfer anything;
			SETIOV(ctp->iov + 0, NULL, 0);
		}
		*pos += nbytes;
	} else if (!dire || S_ISDIR(ocb->mode)) {
		struct dirent				*dirent;
		int							cplen, amount, reclen;

		dirent = (struct dirent *)&msg->i;
		amount = 0;											/* Number of bytes we have stuffed */
		nbytes = min(msg->i.nbytes, ctp->msg_max_size);		/* Number of bytes we can stuff */

		for (; ocb->children && *pos < ocb->childcount; ++*pos) {
			char *entry_name;

			if (!(entry_name = ocb->children[*pos])) {
				break;
			}
		
			/* Adjust the length to only include up until the slash */
			cplen = 0;
			while (entry_name[cplen] && entry_name[cplen] != '/') { cplen++; }
			if (cplen == 0) {	
				continue;
			}

			/* Dump the value into the dirent structure */
			reclen = sizeof(*dirent) + cplen + 1;
			reclen = (reclen & 7) ? (reclen | 7) + 1 : reclen;
			if (nbytes < reclen) {
				if(!amount) {
					IMAGE_ADDRESSABLE_DONE(image);
					return EMSGSIZE;
				}
				break;
			}

			memset(dirent, 0, sizeof(struct dirent));
			//RUSH3: readdir/d_ino should match stat/st_ino
			dirent->d_ino = *pos + 1;
			dirent->d_offset = *pos + 1;
			dirent->d_reclen = reclen;
			dirent->d_namelen = cplen + 1;
			memcpy(dirent->d_name, entry_name, cplen); dirent->d_name[cplen] = '\0';

            dirent = (struct dirent *) ((char *)dirent + reclen);
			/* TODO: Add stat stuffing here */
			amount += reclen;
            nbytes -= reclen;
		}
		nbytes = amount;		
		SETIOV(ctp->iov + 0, &msg->i, amount);
		resmgr_endian_context(ctp, _IO_READ, S_IFDIR, 0);
	}
	IMAGE_ADDRESSABLE_DONE(image);

	_IO_SET_READ_NBYTES(ctp, nbytes);
	return _RESMGR_NPARTS(1);
}

static int 
image_close_ocb(resmgr_context_t *ctp, void *reserved, void *vocb) {
	struct image_ocb			*ocb = vocb;

	pthread_sleepon_lock();
	if ((*ocb->prev = ocb->next)) {
		ocb->next->prev = ocb->prev;
	}
	pthread_sleepon_unlock();
	if (ocb->children) {
		_sfree(ocb->children, ocb->childcount * sizeof(char **));
	}
	_sfree(ocb, sizeof *ocb);
	return EOK;
}

static int 
image_stat(resmgr_context_t *ctp, io_stat_t *msg, void *vocb) {
	struct image_ocb			*ocb = vocb;
	struct image_data			*image = ocb->image;
	union image_dirent			*dire;

	memset(&msg->o, 0x00, sizeof msg->o);
	if ((dire = ocb->dir)) {
		IMAGE_ADDRESSABLE_START(image);
		msg->o.st_ino = ocb->index ? ocb->index : dire->attr.ino;
		if (S_ISREG(ocb->mode)) {
			msg->o.st_size = dire->file.size;
		} else {
			msg->o.st_size = ocb->childcount;
		}
		msg->o.st_ctime =
		msg->o.st_mtime =
		msg->o.st_atime = dire->attr.mtime;
		msg->o.st_gid = dire->attr.gid;
		msg->o.st_uid = dire->attr.uid;
		msg->o.st_mode = dire->attr.mode;
		if((image->addr->flags & IMAGE_FLAGS_INO_BITS) && (dire->attr.ino & IFS_INO_PROCESSED_ELF)) {
			// If it's a processed ELF file, the sticky bit doesn't mean
			// what other things think it means.
			msg->o.st_mode &= ~S_ISVTX;
		}
		if(S_ISDIR(dire->attr.mode)) {
			msg->o.st_nlink = 2;
		} else {
			//RUSH3: If there are execute perms turned on, mkifs fiddles the
			//RUSH3: 'sticky' bit even for [+raw] files :-(. We should make
			//RUSH3: it stop doing that and switch everything over to the
			//RUSH3: INO flag bits. We don't want to leave the 'sticky' bit
			//RUSH3: on because IFS mmap code isn't the greatest (we end
			//RUSH3: up with multiple in-memory copies, and it doesn't
			//RUSH3: make much sense to keep them since we already have
			//RUSH3: the thing in RAM anyway.
			if(dire->attr.mode & (S_IXUSR|S_IXGRP|S_IXOTH)) {
				msg->o.st_mode &= ~S_ISVTX;
			}
			msg->o.st_nlink = 1;
		}
		msg->o.st_nlink = S_ISDIR(dire->attr.mode) ? 2 : 1;
		msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | ocb->image->devno;
	
		if (S_ISLNK(msg->o.st_mode)) {
			char *p = &dire->symlink.path[dire->symlink.sym_offset];

			msg->o.st_size = strlen(p);
			msg->o.st_nlink = 1;
		}
		IMAGE_ADDRESSABLE_DONE(image);
	}

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static int 
image_pathconf(resmgr_context_t *ctp, io_pathconf_t *msg, void *vocb) {
	struct image_ocb			*ocb = vocb;
	struct image_data			*image = ocb->image;

	switch (msg->i.name) {
	case _PC_IMAGE_VADDR:
		if (ND_NODE_CMP(ND_LOCAL_NODE, ctp->info.nd) != 0) {
			return ENOREMOTE;
		}

		if (ocb->dir && S_ISREG(ocb->mode)) {
			IMAGE_ADDRESSABLE_START(image);
			_IO_SET_PATHCONF_VALUE(ctp, (intptr_t)((char *)image->addr + ocb->dir->file.offset));
			IMAGE_ADDRESSABLE_DONE(image);
			return EOK;
		}
		break;
	default:
		break;
	}
	return EINVAL;
}

static int 
image_devctl(resmgr_context_t *ctp, io_devctl_t *msg, void *vocb) {
	struct image_ocb			*ocb = vocb;
	union {
		int					oflag;
		int					mountflag;
	}					*data = (void *)(msg + 1);
	unsigned			nbytes = 0;

	switch ((unsigned)msg->i.dcmd) {
	case DCMD_ALL_GETFLAGS:
		data->oflag = (ocb->ioflag & ~O_ACCMODE) | ((ocb->ioflag - 1) & O_ACCMODE);
		nbytes = sizeof data->oflag;
		break;

	case DCMD_ALL_SETFLAGS:
		ocb->ioflag = (ocb->ioflag & ~O_SETFLAG) | (data->oflag & O_SETFLAG);
		break;

	case DCMD_ALL_GETMOUNTFLAGS: {
		data->mountflag = _MOUNT_READONLY;
		nbytes = sizeof data->mountflag;
		break;
	}

	default:
		return _RESMGR_DEFAULT;
	}

	if (nbytes) {
		msg->o.ret_val = 0;
		return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + nbytes);
	}
	return EOK;
}

static int 
image_lseek(resmgr_context_t *ctp, io_lseek_t *msg, void *vocb) {
	struct image_ocb			*ocb = vocb;
	union image_dirent			*dire = ocb->dir;
	int							size = 0;

	/* We should be able to do this for dirs as well */
	if (dire && S_ISREG(ocb->mode)) {
		IMAGE_ADDRESSABLE_START(ocb->image);
		size = dire->file.size;
		IMAGE_ADDRESSABLE_DONE(ocb->image);
	}

	switch (msg->i.whence) {
	case SEEK_SET:
		ocb->pos = msg->i.offset;
		break;
	case SEEK_CUR:
		ocb->pos += msg->i.offset;
		break;
	case SEEK_END:
		ocb->pos = size + msg->i.offset;
		break;
	default:
		return EINVAL;
	}
	if (ocb->pos < 0) {
		ocb->pos = 0;
	} else if (ocb->pos > size) {
		ocb->pos = size;
	}
	if (msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}
	msg->o = ocb->pos;
	SETIOV(ctp->iov + 0, &msg->o, sizeof msg->o);
	return _RESMGR_NPARTS(1);
}

static int 
image_mmap(resmgr_context_t *ctp, io_mmap_t *msg, void *vocb) {
	struct image_ocb			*ocb = (struct image_ocb *)vocb;
	struct image_ocb			*oldocb = ocb;
	unsigned					ioflag;

	ioflag = _IO_FLAG_RD;
	if (msg->i.prot & PROT_WRITE) {
		ioflag |= _IO_FLAG_WR;
	}

	if ((ocb->ioflag & ioflag) != ioflag) {
		return EACCES;
	}
			
	//RUSH3: we should be looking to see if we already have this
	//RUSH3: memory mapped file open and returning that coid.
	if (!(ocb = _smalloc(sizeof *ocb))) {
		return ENOMEM;
	}

	//Copy over some attributes to the new ocb
	ocb->next = NULL;
	ocb->prev = NULL;
	ocb->dir = oldocb->dir;
	ocb->image = oldocb->image;
	//No children for this ocb, ever
	ocb->index = 0;
	ocb->childcount = 0;
	ocb->children = NULL;
	ocb->pos = 0;
	ocb->ioflag = ioflag;
	ocb->mode = oldocb->mode;

	//ocb->flags = IOFUNC_OCB_PRIVILEGED | IOFUNC_OCB_MMAP;

	if (resmgr_open_bind(ctp, ocb, 0) == -1) {
		_sfree(ocb, sizeof *ocb);
	    return errno;
	}

	pthread_sleepon_lock();
	if ((ocb->next = ocb_list)) {
	    ocb->next->prev = &ocb->next;
	}
	ocb->prev = &ocb_list;
	ocb_list = ocb;
	pthread_sleepon_unlock();

	if ((ioflag & _IO_FLAG_WR) && !(ocb->ioflag & _IO_FLAG_WR)) {
		ocb->ioflag |= _IO_FLAG_WR;
	}

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.coid = ctp->info.coid;

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static resmgr_io_funcs_t image_io_funcs = {
	_RESMGR_IO_NFUNCS,
	image_read,			/* read */
	0,					/* write */
	image_close_ocb,	/* close_ocb */
	image_stat,			/* stat */
	0,					/* notify */
	image_devctl,		/* devctl */
	0,					/* unblock */
	image_pathconf,		/* pathconf */
	image_lseek,		/* lseek */
	0,					/* chmod */
	0,					/* chown */
	0,					/* utime */
	0,					/* openfd */
	0,					/* fdinfo */
	0,					/* lock */
	0,					/* space */
	0,					/* shutdown */
	image_mmap			/* mmap */
};

/*
 This will attempt to match the target path with an internal entry 
 based on the following preferences/criteria:
  -Symbolic links within the path are stored and the shortest link
   path wins above all else.
  -If the path contains no links, but a perfect match is found it 
   is returned (might be a symlink).
  -If the path contains no links, but a perfect match is not found,
   but the path is a sub-path of another entry then that other
   entry is returned.  This is the case with internal directory 
   references.

 This is kind of wastefull in terms of search time, but is cheap
 since it uses the existing in memory structure, even though we
 have to iterate over all the entries each open.
*/
#define LOOKUP_INT_DIR	0x1
#define LOOKUP_INT_LNK	0x2
static union image_dirent *
image_lookup(struct image_data *image, struct _io_connect *connect, int *state) {
	union image_dirent			*dir, *altdir, *lnkdir, *matchdir, *lastdir = 0;
	char						*entry_name, *name = connect->path;
	int							len, entry_len;

	altdir = lnkdir = matchdir = NULL;
	len = strlen(name);
	for (dir = image->dir; dir->attr.size; dir = (union image_dirent *)((unsigned)dir + dir->attr.size)) {
		if (!dir->attr.ino) {
			continue;
		}

		if (S_ISDIR(dir->attr.mode)) {
			lastdir = dir;
		}

		entry_name = image_get_path(dir);
		entry_len = strlen(entry_name);

		/* Check for symbolic links, less than equal to pathlength && a directory or endpoint */
		if (S_ISLNK(dir->attr.mode) && entry_len < len && 
			name[entry_len] == '/' && strncmp(entry_name, name, entry_len) == 0) {

			/* If the link is longer than the last one, forget it */
			if (lnkdir && entry_len >= strlen(lnkdir->symlink.path)) {
				continue;
			}
			lnkdir = dir;
		} else if (!lnkdir && strcmp(name, entry_name) == 0) {
			/* Check for exact match only if we don't have a symlink */
			matchdir = dir;
		} else if (!lnkdir && !altdir && (len == 0 || strncmp(name, entry_name, len) == 0)) {
			/* We only need one "internal" node to know that we have a directory. */
			if (len && entry_name[len] != '/') {
				continue;
			}
			altdir = lastdir;
		}
	}


	/* Return in order of preference: Internal Symlinks, Exact matches, Internal dirs. */
	if (lnkdir) {
		*state = LOOKUP_INT_LNK;
		return lnkdir;
	}
	if (matchdir) {
		*state = 0;
		return matchdir;
	}
	*state = LOOKUP_INT_DIR;
	errno = ENOENT;
	return altdir;
}

static int 
image_link_redirect(resmgr_context_t *ctp, io_open_t *msg, 
							   struct image_symlink *symlinkp, int redirect) {
	int len, eflag;
	char *p, *ap, *ip, *tp;		/* [p]ath, [a]fter, [i]nsert, [t]emp */

	p = &symlinkp->path[symlinkp->sym_offset];
	/* Save msg->connect.path since we wipe it out once we start writing ip */
	ap = msg->connect.path + strlen(symlinkp->path);
	len = strlen(ap) + 1;
	tp = (len > 1) ? alloca(len) : NULL;
	if (tp) {
		memcpy(tp, ap, len);
	}
	ap = tp;
	ip = (char *)(&msg->link_reply + 1);

	eflag = msg->connect.eflag;

	/* Perform a re-direction, unless we wanted the path information */
	if (redirect) {
		_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_LINK);

		/* Build the link pathname making it relative to our root entry but only 
		   if we are actually re-directing, otherwise show the symlink string */
		if (*p != '/') {
			char *nip;
			len = strlen(symlinkp->path);
			memcpy(ip, symlinkp->path, len + 1);
			//Move back through the list looking for the last / if it is there
			for (nip = ip + len; (nip > ip) && (*nip != '/'); nip--) { ; }
			ip = (*nip == '/') ? nip + 1 : nip;
		}
	}

	len = strlen(p) + 1;
	memcpy(ip, p, len);
	if (ap) {
		ip += len -1;
		memcpy(ip, ap, strlen(ap) + 1);
	}
	len = strlen((ip = (char *)(&msg->link_reply + 1))) + 1;

	msg->link_reply.eflag = eflag;
	msg->link_reply.nentries = 0;
	msg->link_reply.path_len = len;
	return _RESMGR_PTR(ctp, msg, sizeof(struct _io_connect_link_reply) + len);
}

static int 
image_open(resmgr_context_t *ctp, io_open_t *msg, void *handle, void *extra) {
	struct image_data			*image = handle;
	union image_dirent			*dire;
	int							notmatch = 0;

	if (handle == NULL) {
		return ENOSYS;
	}
	
	if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	IMAGE_ADDRESSABLE_START(image);
	if ((dire = image_lookup(image, &msg->connect, &notmatch))) {
		struct image_ocb			*ocb;

		//Deal with the readlink on a non-link right up front
		if (msg->connect.subtype == _IO_CONNECT_READLINK && !S_ISLNK(dire->attr.mode)) {
			IMAGE_ADDRESSABLE_DONE(image);
			return EINVAL;
		}

		/* We re-direct fifo's to the pipe manager */
		if (!notmatch && S_ISFIFO(dire->attr.mode)) {
			if(msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)) {
				_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_FTYPE);
				msg->ftype_reply.status = EOK;
				msg->ftype_reply.file_type = _FTYPE_PIPE;
				IMAGE_ADDRESSABLE_DONE(image);
				return _RESMGR_PTR(ctp, msg, sizeof(struct _io_connect_ftype_reply));
			}
			//What happens here?
		}

		/* We spit out the symbolic link information if required, else re-direct */
		if (notmatch != LOOKUP_INT_DIR && S_ISLNK(dire->attr.mode)) {
          if ((notmatch == LOOKUP_INT_LNK) || !S_ISLNK(msg->connect.mode) ||
              (msg->connect.subtype == _IO_CONNECT_READLINK) ||
              (msg->connect.eflag & (_IO_CONNECT_EFLAG_DOT | _IO_CONNECT_EFLAG_DIR))) {
				int redirect = 0;
				int	r;
				
				if (notmatch == LOOKUP_INT_LNK || msg->connect.subtype != _IO_CONNECT_READLINK) {
					redirect = 1;
				}

				r = image_link_redirect(ctp, msg, &dire->symlink, redirect);
				IMAGE_ADDRESSABLE_DONE(image);
				return r;
			}
		}

		/* Other file entries we simply bind in ... */
		if (!(ocb = _smalloc(sizeof *ocb))) {
			IMAGE_ADDRESSABLE_DONE(image);
			return ENOMEM;
		}
		
		/* If it is a directory, fetch the children (just the dirs?) */
		ocb->childcount = 0;
		ocb->children = NULL;
		ocb->index = 0;
		if (notmatch == LOOKUP_INT_DIR || S_ISDIR(dire->attr.mode)) {
			//kprintf("Get directory children [%s]\n", msg->connect.path);
			(void)image_getdir_children(image->dir, msg->connect.path, &ocb->children, &ocb->childcount, &ocb->index);
			if(notmatch != LOOKUP_INT_DIR) {
				ocb->index = 0;
			}
		}
		ocb->dir = dire;

		ocb->pos = 0;
		ocb->image = image;
		ocb->ioflag = msg->connect.ioflag;
		ocb->mode = dire->attr.mode;

		if (resmgr_open_bind(ctp, ocb, 0) == -1) {
			_sfree(ocb, sizeof *ocb);
			IMAGE_ADDRESSABLE_DONE(image);
			return errno;
		}
		pthread_sleepon_lock();
		if ((ocb->next = ocb_list)) {
			ocb->next->prev = &ocb->next;
		}
		ocb->prev = &ocb_list;
		ocb_list = ocb;
		pthread_sleepon_unlock();
		IMAGE_ADDRESSABLE_DONE(image);
		return EOK;
	}
	IMAGE_ADDRESSABLE_DONE(image);
	return errno;
}
		
static int 
image_readlink(resmgr_context_t *ctp, io_readlink_t *msg, void *handle, void *extra) {
	return image_open(ctp, (io_open_t *)msg, handle, extra);
}

static int 
image_unlink(resmgr_context_t *ctp, io_unlink_t *msg, void *handle, void *extra) {
	struct image_data			*image = handle;
	union image_dirent			*dire;
	int							notmatch = 0;
	int							r;


	if (handle == NULL) {
		return ENOSYS;
	}

    if (msg->connect.path_len + sizeof(*msg) >= ctp->msg_max_size) {
		return ENAMETOOLONG;
	}

	IMAGE_ADDRESSABLE_START(image);
	if ((dire = image_lookup(image, &msg->connect, &notmatch))) {
		/*Implicit directories have mode r-xr-xr-x */
		if (notmatch == LOOKUP_INT_DIR || S_ISFIFO(dire->attr.mode)) {
			IMAGE_ADDRESSABLE_DONE(image);
			return EROFS;
		}
		if (notmatch == LOOKUP_INT_LNK) {
			r = image_link_redirect(ctp, (io_open_t *)msg, &dire->symlink, 1);
			IMAGE_ADDRESSABLE_DONE(image);
			return r;
		}
		if (!(dire->attr.mode & S_ISVTX)) {
			if (!proc_isaccess(0, &ctp->info)) {
				IMAGE_ADDRESSABLE_DONE(image);
				return EPERM;
			}

			dire->attr.ino = 0;
		}
		r = dire->attr.ino ? EROFS : EOK;
		IMAGE_ADDRESSABLE_DONE(image);
		return r;
	}
	IMAGE_ADDRESSABLE_DONE(image);
	return errno;
}

static char *device_options[] = {
#define IMAGE_MOUNT_OFFSET 0
	"offset",
#define IMAGE_MOUNT_SIZE 1
	"size",
	NULL
};

// 
// Example usage: mount -tifs -ooffset=XXXX,size=YYYY /dev/mem /foobar
//
static int 
image_mount(resmgr_context_t *ctp, io_mount_t *msg, void *handle, io_mount_extra_t *extra) {
	char *		options;
	char *		p;
	void *		ifs_image;
	paddr_t		offset = 0;
	unsigned	size   = -1U;
	int			flags  = 0;

	/* We only care about the "ifs" mount type */
	if ((extra->extra.srv.type == NULL) || (strcmp(extra->extra.srv.type, "ifs"))) {
		return ENOSYS;
	}
	/* We only support regular mounting */
	if (extra->flags & (_MOUNT_ENUMERATE | _MOUNT_UNMOUNT | _MOUNT_REMOUNT)) {
		return ENOTSUP;
	}

	/* We only support mounting /dev/mem for now */
	if ((extra->extra.srv.ocb == NULL) || (devmem_check_ocb_phys(ctp, extra->extra.srv.ocb))) {
		return EINVAL;
	}

	/* Verify client credentials */
	if(!proc_isaccess(0, &ctp->info)) {
		return EPERM;
	}

	/* Parse the options */
	options = extra->extra.srv.data;
	while (options && *options) {
		switch (getsubopt(&options, device_options, &p)) {
		case IMAGE_MOUNT_OFFSET:
			offset = strtoul(p, NULL, 0);
			break;

		case IMAGE_MOUNT_SIZE:
			size = strtoul(p, NULL, 0);
			break;

		default:
			return EINVAL;
		}
	}

	/* Ensure an offset was given */
	if ((offset == (paddr_t)-1) || (size == (unsigned)-1)) {
		return EINVAL;
	}

	/* Convert the resmgr related mount flags */
	flags = (extra->flags & (_MOUNT_BEFORE | _MOUNT_AFTER | _MOUNT_OPAQUE)) >> 16;

	/* Perform the mount */
	ifs_image = imagefs_mount(offset, size, 0,
	                          flags, sysmgr_prp->root, msg->connect.path);
	if (ifs_image == (void *)-1) {
		return errno;
	}

	return EOK;
}

static const resmgr_connect_funcs_t image_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	image_open,
	image_unlink,
	0,	//rename
	0,	//mknod
	image_readlink,	//readlink
	0,	//link (must be null);
	0,	//unblock
	image_mount	//mount
};                                              

int 
imagefs_mount_mounter () {
	return resmgr_attach(dpp, NULL, NULL, _FTYPE_MOUNT, _RESMGR_FLAG_DIR | _RESMGR_FLAG_FTYPEONLY,
	                      &image_connect_funcs, &image_io_funcs, NULL);
}

static void 
kerext_validate_signature(void *p) {
	int					i;
	int					j;
	struct image_header	*ifs = p;

	/* We had to reverse the signature to avoid confusing dumpifs's image locaion routines */
	for(i = 0, j = sizeof(IMAGE_SIGNATURE_REV) - 2; j >= 0; i++, j--) {
		if(ifs->signature[i] != IMAGE_SIGNATURE_REV[j]) {
			KerextStatus(0, 1);
			return;
		}
	}
	KerextStatus(0, 0);
}

/* when memory management is added, this will probably pass in a memory object structure instead */
void *
imagefs_mount(paddr_t paddr, unsigned size, unsigned offset, unsigned flags, struct node_entry *root, 
		const char *mountpoint) {
	void						*addr;
	struct image_data			*image;
	OBJECT						*obp;
	int							r;
	size_t						pg_offset;
	mem_map_t					msg;
	int							in_user_space = 0;
	unsigned eflags=0;

	pg_offset = paddr & (__PAGESIZE - 1);
	msg.i.offset = paddr - pg_offset;
	msg.i.flags = MAP_PHYS;
	msg.i.len = size + pg_offset;

	obp = object_create(OBJECT_MEM_SHARED, &msg, NULL, sys_memclass_id);
	if(obp == NULL) {
		r = EAGAIN;
		goto fail1;
	}
	pathmgr_object_clone(obp);

	//FUTURE: We shouldn't mmap() all of the image file system - we really
	//FUTURE: only need the directory portion accessable. There's some
	//FUTURE: gear in imagefs_check() right now that looks at file data
	//FUTURE: and would need to be modified - possibly other places
	//FUTURE: as well.
	r = memmgr.mmap(NULL, 0, size, PROT_WRITE | PROT_READ, MAP_SHARED, obp, pg_offset,
					0, 0, NOFD, &addr, &size, obp->hdr.mpid);
	if((r == ENOTSUP) && (mountpoint != NULL)) {
		// memmgr can't handle the paddr - we're being asked to map something
		// into the sysaddr space on an architecture that only supports
		// sysaddr access to the 1-to-1 area. 
		// Instead, we'll do the mapping into procnto's local user address
		// space and bind each thread to that when they need to look at the IFS.
		// Only do this for things brought in via image_mount(), since the 
		// bootimage.c code assumes a pointer in sysaddr space is returned.
		in_user_space = 1;
		ProcessBind(SYSMGR_PID);
		r = memmgr.mmap(sysmgr_prp, 0, size, PROT_WRITE | PROT_READ, MAP_SHARED, obp,
						pg_offset, 0, 0, NOFD, &addr, &size, obp->hdr.mpid);
	}
	if(r != EOK) goto fail2;

	// Do signature validation inside the kernel so that if we get a 
	// fault because of a bad paddr, we don't kill proc.
	if(__Ring0(&kerext_validate_signature, addr) != 0) {
		r = EBADFSYS;
		goto fail3;
	}

#ifdef __LITTLEENDIAN__
#define NATIVE_ENDIAN   0
#else
#define NATIVE_ENDIAN   IMAGE_FLAGS_BIGENDIAN
#endif

	eflags = ((struct image_header *)addr)->flags & IMAGE_FLAGS_BIGENDIAN;

	if (NATIVE_ENDIAN ^ eflags) {
		// not matching endian
		r = EENDIAN;
		goto fail3;
	}

#ifdef PROC_BOOTIMAGE_DIR
	if (!mountpoint) {
		mountpoint = PROC_BOOTIMAGE_DIR;
	} else if (!*mountpoint) {
#else
	if (!mountpoint || !*mountpoint) {
#endif
		mountpoint = ((struct image_header *)addr)->mountpoint;
		while (*mountpoint == '/') {
			mountpoint++;
		}
		if (*mountpoint == '\0') {
			mountpoint = ".";
		}
	}
	if (!mountpoint || !*mountpoint || *mountpoint == '/') {
		r = EINVAL;
		goto fail3;
	}

	if (!(image = _smalloc(sizeof *image))) {
		r = ENOMEM;
		goto fail3;
	}

	if (rsrcdbmgr_proc_devno(_MAJOR_FSYS, &image->devno, -1, 0) == -1) {
		r = ENOMEM;
		goto fail4;
	}

	image->addr = (struct image_header *)((char *)addr + offset);
	image->dir = (union image_dirent *)((char *)addr + ((struct image_header *)addr)->dir_offset);
	image->size = size;
	image->pg_offset = pg_offset;
	image->obp = obp;
	image->in_user_space = in_user_space;
	pathmgr_object_clone(obp);

	// Hide the image pointer in the object - imagefs_fname needs it.
	obp->hdr.next = (void *)image;

	if (resmgr_attach(dpp, NULL, mountpoint, _FTYPE_ANY, flags | _RESMGR_FLAG_DIR,
	                  &image_connect_funcs, &image_io_funcs, image) == -1) {
		r = errno;
		goto fail5;

	}

	if(in_user_space) ProcessBind(0);
	return (void *)((char *)addr - offset);

fail5:
	pathmgr_object_done(obp);
	//Undo rsrcdbmgr_proc_devno() above
	rsrcdbmgr_proc_devno(NULL, &image->devno, -1, 0);

fail4:	
	_sfree(image, sizeof *image);

fail3:	
	(void)memmgr.munmap(in_user_space ? sysmgr_prp : NULL, 
						(uintptr_t)addr, size, 0, obp->hdr.mpid);

fail2:
	if(in_user_space) ProcessBind(0);
	pathmgr_object_done(obp);

fail1:
	errno = r;
	return (void *)-1;
}

int 
imagefs_check(const resmgr_io_funcs_t *funcs, mem_map_t *msg, void *handle, OBJECT **pobp) {
	struct image_ocb			*ocb = handle;
	struct image_file			*file;
	union image_dirent			*dire;
	struct image_data			*image;
	unsigned					offset;
	int							old_aspace;

	if(funcs != &image_io_funcs) {
		return -1;
	}

	if (!(dire = ocb->dir) || !S_ISREG(ocb->mode)) {
		return -1;		//Can't check a non-file
	}

	image = ocb->image;
	*pobp = image->obp;

	if(msg != NULL) {
		IMAGE_ADDRESSABLE_SAVE(image, old_aspace);
		file = &dire->file;
		offset = msg->i.offset <= file->size ? msg->i.offset : 0;

		//RUSH3: Check for the IMAGE_FLAGS_INO_BITS flag and
		//RUSH3: make use of the information if present
		if(msg->i.flags & MAP_ELF) {
			//Attempting to map in an executable/shared object. See
			//if the segment was marked as uip or copy.
			Elf32_Ehdr	*ehdr;
			Elf32_Phdr	*phdrs;
			Elf32_Phdr	*phdr = NULL;
			unsigned	i;

			ehdr = (void *)((uint8_t *)image->addr + file->offset);

			if(memcmp(ehdr->e_ident, ELFMAG, SELFMAG) != 0
				|| ehdr->e_ident[EI_DATA] != ELFDATANATIVE
				|| UNALIGNED_RET16(&ehdr->e_phentsize) != sizeof *phdr) {
				IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
				return -1;
			}
			phdrs = (void *)((uint8_t *)ehdr + UNALIGNED_RET32(&ehdr->e_phoff));
			for(i = 0; i < UNALIGNED_RET16(&ehdr->e_phnum); ++i) {
				unsigned	p_off;

				phdr = &phdrs[i];
				p_off = UNALIGNED_RET32(&phdr->p_offset);
				if((offset >= p_off) && (offset < (p_off+UNALIGNED_RET32(&phdr->p_memsz)))) {
					unsigned	p_paddr;

					//This is the segment we're mapping in. If the p_paddr
					//field is zero, the user marked it as a "copy" segment, so
					//we want the memmgr to act the same as if the file was
					//coming off a hard disk.
					p_paddr = UNALIGNED_RET32(&phdr->p_paddr);
					if(p_paddr != 0) {
						//RUSH3: Make use of the IFS_INO_* bits, if possible
						// Kludge here. We can't tell the difference between
						// a [+raw] executable and one that's been processed
						// by mkifs. The linker puts p_paddr==p_vaddr,
						// so if we see that, assume it's [+raw]. In the
						// future, mkifs should use the top bit in the inode
						// field to mark a processed entry.
						if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_MMU) {
							unsigned	p_vaddr;

							p_vaddr = UNALIGNED_RET32(&phdr->p_vaddr);
							if(p_paddr != p_vaddr) break;
						}
					}
					//RUSH3: We can be smarter about this. Right
					//RUSH3: now, this is going to cause us to allocate
					//RUSH3: backing store for the mmap'd file, which we
					//RUSH3: then COW when the page gets modified. Might as well
					//RUSH3: use the in-memory data here as the backing store
					//RUSH3: by doing (assuming alignment is good):
					//RUSH3:   msg->i.offset = file->offset + offset + image->pg_offset;
					//RUSH3:   *pobp = ocb->image->obp;
					//RUSH3:   return EOK;
					//RUSH3: Think about IFS in slow memory though...
					IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
					return -2;
				}
			}
			if(phdr && (UNALIGNED_RET32(&phdr->p_flags) & PF_W)
			 &&(UNALIGNED_RET16(&ehdr->e_type) == ET_EXEC)) {
				// For executable use-in-place R/W segments, change the
				// MAP_PRIVATE to a MAP_SHARED. Also mark the file as deleted,
				// since we're going to damage the R/W data when we run.
				msg->i.flags = (msg->i.flags & ~MAP_TYPE) | MAP_SHARED;
				dire->attr.ino = 0;
			}
		} else {
			// Is it aligned by page?
			if((image->pg_offset + file->offset) & (__PAGESIZE - 1)) {
				IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
				return -2;
			}
		}

		if((msg->i.flags & MAP_TYPE) != MAP_SHARED) {
			IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
			return -2;
		}

		if(!(ocb->ioflag & _IO_FLAG_WR)) {
			// Indicate that mprotect() can't grant PROT_WRITE permissions later
			msg->i.flags |= IMAP_OCB_RDONLY;
		}
		msg->i.offset = file->offset + offset + image->pg_offset;
		IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
	}
	return EOK;
}

size_t
imagefs_fname(OBJECT *obp, unsigned off, size_t max, char *dest) {
	struct image_data			*image;
	union image_dirent			*dir;
	char						*name;
	int							old_aspace;

	// Extract the image pointer from the place that imagefs_mount() hid it.
	image = (void *)obp->hdr.next;

	IMAGE_ADDRESSABLE_SAVE(image, old_aspace);
	name = "";
	for(dir = image->dir; dir->attr.size; dir = (union image_dirent *)((uintptr_t)dir + dir->attr.size)) {
		// Don't skip ino == 0 entries, they're [data=uip]
		if(S_ISREG(dir->attr.mode)) {
			if((off >= dir->file.offset) && (off < (dir->file.offset + dir->file.size))) {
				//RUSH3: prefix with mount path?
				name = dir->file.path;
				break;
			}
		}
	}
	STRLCPY(dest, name, max);
	IMAGE_ADDRESSABLE_RESTORE(image, old_aspace);
	return strlen(dest);
}

__SRCVERSION("imagefs.c $Rev: 198523 $");
