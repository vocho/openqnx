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
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <alloca.h>
#include <string.h>
#include <sys/pathmgr.h>
#include <sys/pathmsg.h>
#include <sys/procmgr.h>
#include <sys/netmgr.h>
#include "connect.h"

#include "stkchk.h"

#include <stdio.h>

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

/* If we think that we have space on the stack (guesstimate
   32 symlinks with 32 servers with 1K path == 32 * 2K == 64K)
   make it 80K for good measure. */
#define MIN_STACK_FOR_GROWTH 81920
#define CTRL_ALLOC(flag, len) (((flag) & FLAG_STACK_ALLOC) ? alloca((len)) : malloc((len)))
#define CTRL_FREE(flag, data) (((flag) & FLAG_STACK_ALLOC) ? ((void)0) : free((data)))

char _connect_malloc;

static const struct _io_connect_entry FIXCONST _connect_proc_entry = {
	ND_LOCAL_NODE,
	PATHMGR_PID,
	PATHMGR_CHID,
	PATHMGR_HANDLE,
};

static int _connect_io(struct _connect_ctrl const * ctrl, int fd, const char *prefix, unsigned prefix_len,
	const char *path, void *buffer, const struct _io_connect_entry *entry) {
	register struct _io_connect				*msg = ctrl->msg;
	register struct _io_connect_link_reply	*reply = &(ctrl->reply->link);
	iov_t									siov[5], riov[2];
	register unsigned						sparts;
	static const char FIXCONST				padding[_QNX_MSG_ALIGN - 1];
	int										ret, tryagain;
	struct _io_connect_entry				*ent = ctrl->entry;

	SETIOV(siov + 0, msg, offsetof(struct _io_connect, path));
	if(prefix_len) {
		SETIOV(siov + 1, prefix, prefix_len);
		sparts = 2;
	} else {
		sparts = 1;
	}
	SETIOV(siov + sparts, path, msg->path_len);
	sparts++;
	msg->path_len += prefix_len;
	if(msg->extra_len) {
		int	align;

		if((align = (_QNX_MSG_ALIGN - (offsetof(struct _io_connect, path) + msg->path_len)) & (_QNX_MSG_ALIGN - 1))) {
			SETIOV(siov + sparts, padding, align);
			sparts++;
		}
		SETIOV(siov + sparts, ctrl->extra, msg->extra_len);
		sparts++;
	}
	SETIOV(riov + 0, reply, sizeof *reply);
	SETIOV(riov + 1, buffer, msg->reply_max - sizeof *reply);

	/*
	 If our entries don't match, then try and resolve it to a link to find a match
	 by sending an COMBINE_CLOSE message to the handler to see if it will 
	 resolve into a bigger linke.  If we do find a match (later on as we recursed 
	 through the entries in the process) then actually sent the message that
	 we originally were supposed to send. See rename() to see how this is used.

	 This functionality is also used to resolve links "magically" by switching
	 the message type to be an open if the intial request fails.  This means
	 that we don't have to fill proc with all the resolution.
	*/

	//We don't want to test the entries, or we have a matching entry ... send original message
	if(!(ctrl->flags & FLAG_TEST_ENTRY)) {
		ret = 1;
	} else if (ent->pid == entry->pid && 
	           ent->chid == entry->chid && 
	           ND_NODE_CMP(ent->nd, entry->nd) == 0 && 
	           ((ctrl->flags & FLAG_TEST_NPC_ONLY) || ent->handle == entry->handle)) {
		ret = 1;
	} else {
		ret = 0;
	}

	if (ret && (!(ctrl->flags & FLAG_TEST_ND_ONLY) || ND_NODE_CMP(ctrl->nd, entry->nd) == 0)) {
		if ((ret = ctrl->send(fd, siov, sparts, riov, 2)) != -1 || (ctrl->flags & FLAG_TEST_ENTRY)) {
			return ret;
		}
	}

	//We only try to resolve a link reply if we didn't send in the first place or if
	//the request went out and returned with ENOSYS indicating no callout.
	if(ctrl->flags & FLAG_NO_RETRY) {
		tryagain = 0;
	} else {
		tryagain = ((ret != -1) || (errno == ENOSYS));
		switch (msg->subtype) {
		case _IO_CONNECT_OPEN:
		case _IO_CONNECT_COMBINE:
		case _IO_CONNECT_COMBINE_CLOSE:
			break;

		/* 
		 These have specific resmgr helpers, that return specific
		 errno's in the case of failure.
		*/
		case _IO_CONNECT_LINK:
			tryagain |= (errno == ENXIO) ? 1 : 0; 
			break;
		default:
			break;
		}
	}

	//If we failed (or we didn't have a matching entry) stuff a msg to resolve possible links
	if (tryagain) {
		uint32_t	saved_ioflag;
		uint16_t	saved_type;
		uint16_t	saved_elen;
		uint8_t		saved_eflag;
		uint8_t		saved_etype;
		int			saved_errno;

		saved_ioflag = msg->ioflag;
		saved_type = msg->subtype;
		saved_eflag = msg->eflag;
		saved_etype = msg->extra_type;
		saved_elen = msg->extra_len;

		msg->ioflag = 0;
		msg->extra_len = 0;
		msg->extra_type = _IO_CONNECT_EXTRA_NONE;
		msg->subtype = _IO_CONNECT_COMBINE_CLOSE;

		saved_errno = errno;							//Meaningless errno for matching entry
		ret = ctrl->send(fd, siov, sparts, riov, 2);
		if (ret == -1) {
			//Every other error is disregarded
			errno = (ctrl->flags & FLAG_TEST_ENTRY) ? ENXIO : saved_errno;

		//If we return anything other than a link here, it is an error 
		} else if ((ret & (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_TYPE_MASK)) !=
		                  (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_LINK)) {
			//If we want to error on a collision and we get a collision, return EEXIST
			//otherwise, just pretend that the error was what came from the first send
			if (ctrl->flags & FLAG_ERR_COLLISION) {
				errno = EEXIST;
			} else {
				errno = (ctrl->flags & FLAG_TEST_ENTRY) ? ENXIO : saved_errno;
			}
			ret = -1;
		}

		msg->subtype = saved_type;
		msg->eflag = saved_eflag;
		msg->extra_type = saved_etype;
		msg->extra_len = saved_elen;
		msg->ioflag = saved_ioflag;
	}
	else {
		errno = (errno == EOK) ? ENOENT : errno;
		ret = -1;
	}
	return ret;
}

static int _connect_request(struct _connect_ctrl *ctrl, const char *prefix, unsigned prefix_len, const char *path,
	unsigned path_skip, const struct _io_connect_entry *entry, void *buffer, int chrootlen) {
	register struct _io_connect				*msg = ctrl->msg;
	register struct _io_connect_link_reply	*reply = &(ctrl->reply->link);
	register int							fd;
	int										ftype;

	if((fd = ConnectAttach(entry->nd, entry->pid, entry->chid, ctrl->base, _NTO_COF_CLOEXEC)) != -1) {
		msg->handle = entry->handle;
		msg->eflag = reply->eflag;

		//Get the status of the node in question along with all of the node entries associated with it
		if((ctrl->status = _connect_io(ctrl, fd, prefix, prefix_len, path + path_skip, buffer, entry)) == -1) {
			ConnectDetach_r(fd);
			return(-1);
		}

		//Do we want any particular type of server connections, otherwise use what we got
		ftype = msg->file_type;
		if(ctrl->status & _IO_CONNECT_RET_FLAG) {
			if ((ftype == _FTYPE_ANY) && 
			    (ctrl->status & _IO_CONNECT_RET_TYPE_MASK) == _IO_CONNECT_RET_FTYPE) {
					ftype = msg->file_type = ctrl->reply->ftype.file_type;
					ctrl->nd = entry->nd;
					ctrl->flags |= FLAG_TEST_ND_ONLY;
			}
		}

		/*
		 If a link was returned it means that there are other managers out there
		 that might handle use.  We can cycle through each of the node entries
		 querying them to see if they can do what we need them to do
		*/
		if((ctrl->status & (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_TYPE_MASK)) ==
		                   (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_LINK)) {

			ConnectDetach_r(fd);

			//Nuke the chroot_len reply field if that flag not set
			if(!(ctrl->status & _IO_CONNECT_RET_CHROOT)) {
				reply->chroot_len = 0;
			}

			//Check the mode flag against our umask
			if(entry->pid == PATHMGR_PID && entry->chid == PATHMGR_CHID && !prefix) {
				if(ctrl->status & _IO_CONNECT_RET_UMASK) {
					msg->mode &= ~(reply->umask & ~S_IFMT);
				}
				if(ctrl->status & _IO_CONNECT_RET_NOCTTY) {
					ctrl->flags |= FLAG_NOCTTY;
				}
			}

			if (msg->entry_max == 0) {
				errno = ELOOP;
				return -1;
			}
			if (sizeof(*reply) + reply->nentries * sizeof(*entry) + reply->path_len >= msg->reply_max) {
				errno = ENAMETOOLONG;
				return -1;
			}

			//If we have multiple entries for this node, then look at each entry/object/connection
			//in turn to see if that connection can give us the services we desire
			if(reply->nentries) {
				char							*save;
				const struct _io_connect_entry				*save_entry;
				void							*buff;
				unsigned						path_len;
				uint8_t							cnt, num; /* 256 entries returned max */

				path_len = reply->path_len;
				cnt = num = reply->nentries;
				chrootlen += reply->chroot_len;

				ctrl->chroot_len = reply->chroot_len;

				buff = ((struct _io_connect_entry *)buffer + num);
				if (path_skip > 1 && path[path_skip-1] == '/') {
					path_skip--;
				}
				if(!(save = CTRL_ALLOC(ctrl->flags, path_len + path_skip))) {
					errno = ENOMEM;
					return -1;
				}
				memcpy(save, path, path_skip);
				memcpy(save + path_skip, buff, path_len); 

				if (ctrl->flags & FLAG_REUSE_BUFFER) {
					msg->reply_max -= num * sizeof *entry;
					entry = buffer;
					save_entry = NULL;
				}
				else {
					if (!(save_entry = entry = CTRL_ALLOC(ctrl->flags, num * sizeof(*entry)))) {
						CTRL_FREE(ctrl->flags, save); 
						errno = ENOMEM;
						return -1;
					}
					memcpy((void *)entry, buffer, num * sizeof(*entry));
					buff = buffer;
				}

				msg->entry_max--;
				for(; cnt > 0; entry++, cnt--) {
					/*
					 If this is a FTYPE_ALL/FTYPE_MATCHED, skip this entry, this is 
					 here since you might return an FTYPE_MATCHED at one of the 
					 earlier iterations.  A better solution would be to switch on 
					 msg->file_type below rather than ftype, then remove this test.
					*/
					if (msg->file_type == (unsigned)_FTYPE_ALL) {
						continue;
					}

					//Check to see if this entry/manager provides the type of service we want
					if ((msg->file_type == _FTYPE_ANY) || 
					    (entry->file_type == (unsigned)_FTYPE_ALL) || (msg->file_type == entry->file_type)) {
						int								pad_len;
						int								save_errno;
						int								marker;

						save_errno = errno;
						msg->path_len = path_len - entry->prefix_len;

						/*
						 We stuff the path piecewise whenever we have a skip component.
						 In order to do this we shift the path insert point forward 
						 before we recurse and then move it back and stuff the path
						 after we recurse.
						 We expect that the component that we are stuffing will always 
						 begin with a / and doesn't terminate with a slash. 
						*/
						pad_len = entry->prefix_len;
						pad_len -= (save[path_skip + pad_len -1] == '/') ? 1 : 0;
						if (ctrl->flags & FLAG_NO_PREFIX ||
							(ctrl->pathsize - ctrl->pathlen) < pad_len) {
							pad_len = 0;
						}
						ctrl->pathlen += pad_len;
						marker = ctrl->pathlen;		//We might be able to remove the marker

						fd = _connect_request(ctrl, 0, 0, save, entry->prefix_len + path_skip, entry, buff, chrootlen);

						if (pad_len && marker == ctrl->pathlen) {
							ctrl->pathlen -= pad_len;
							if (fd != -1 && ctrl->path && ctrl->pathsize) {
								memcpy(&ctrl->path[ctrl->pathlen], &save[path_skip], pad_len);
							}
						}

						//We want to continue cycling if we have an array, and we have space 
						//in the array to store fd's, otherwise we break out and return the fd
						if (fd != -1 && ctrl->fds) {
							continue;
						}

						//We had an error, try it again with another manager or exit if the 
						//manager says they handle us but want to return an error (ie ejected cdrom)
						if(fd == -1 && msg->file_type != (unsigned)_FTYPE_MATCHED) {
							switch(errno) {
							default:
								if(errno < ENETDOWN || errno > EHOSTUNREACH) {
									break;
								}
								/* FALL THROUGH: ipc/network software -- operational errors */
							case EROFS:
							case ENOSYS:
							case ENOENT:
							case ENXIO:
							case ESRCH:
							case EBADFSYS:
								if(save_errno != EOK && save_errno != ENOSYS) {
									errno = save_errno;
								}
								continue;
							}
						}
						
						break;
					}
				}
				msg->entry_max++;

				if (ctrl->flags & FLAG_REUSE_BUFFER) {
					msg->reply_max += num * sizeof *entry;
				}
				else {
					CTRL_FREE(ctrl->flags, (void *)save_entry);
				}
				CTRL_FREE(ctrl->flags, save);

			/*
			 Otherwise we are a link to somewhere, but there are no specific
			 entries (ie managers) for us to query, but we have a new path 
			 (ie symlink) for us to query and traverse, looking for handlers.

			 Proc always receives a request which is relative to the chroot
			 and returns a path which is absolute.  We do the translation
			 as required when a symlink is involved.
			*/
			} else {
				char							*savedpath, *buff = buffer;
				int								hold_pathlen;
				uint16_t						preoffset, postoffset;

				msg->path_len = reply->path_len;
				hold_pathlen = ctrl->pathlen;
				ctrl->pathlen = 0;

				/* Don't bother with the path work if we only want real path */
				if(ctrl->path && (ctrl->flags & FLAG_NO_SYM)) {
					savedpath = ctrl->path;
					ctrl->path = NULL;
				} else {
					savedpath = NULL;
				}

				if(*buff == '/') {					/* Don't mess with absolute links */
					preoffset = path_skip;
					postoffset = 0;
				} else if(chrootlen < path_skip) {	/* chroot fits under mount point */
					preoffset = chrootlen;
					postoffset = 0;
				} else {							/* chroot over mount point */
					preoffset = path_skip;
					postoffset = chrootlen - path_skip;
				}

				fd = _connect_request(ctrl, path + preoffset, path_skip - preoffset, 
										buff + postoffset, 0, &_connect_proc_entry, buff, 0);

				if(savedpath && fd != -1) {
					ctrl->path = savedpath;
					ctrl->pathlen = hold_pathlen;
					goto copy_path;
				}
			}

		/* File was matched but a request to change the file type was made */
		} else if((ctrl->status & (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_TYPE_MASK)) ==
		                          (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_FTYPE)) {
			ConnectDetach_r(fd);
			//Only if the file type is matched do we change it. Does it matter?
			if (ctrl->reply->ftype.file_type == (unsigned)_FTYPE_MATCHED) {
				msg->file_type = ctrl->reply->ftype.file_type;
			}
			errno = ctrl->reply->ftype.status;
			errno = (errno == EOK) ? ENOSYS : errno;
			fd = -1;

		//We have connected to the server, and the server returned some stuff for us to re-send
		} else if((ctrl->status & (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_MSG)) ==
		                          (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_MSG)) {

			msg->file_type = (unsigned)_FTYPE_MATCHED;
			ConnectDetach_r(fd);
			if(		(msg->subtype == _IO_CONNECT_OPEN &&
					msg->extra_len == 0) || 
					((msg->subtype == _IO_CONNECT_COMBINE ||
					msg->subtype == _IO_CONNECT_COMBINE_CLOSE) && 
					reply->path_len)) {
				register struct _server_info		*info = buffer;

				if((fd = ConnectAttach(info->nd, info->pid, info->chid, ctrl->base, _NTO_COF_CLOEXEC)) != -1) {
					if(reply->path_len) {
						register struct _io_combine		*next = (void *)(info + 1);
						iov_t							siov[2], riov[2];
						int								sparts = 1;

						SETIOV(siov + 0, next, reply->path_len);
						if(msg->extra_len) {
							next->combine_len |= _IO_COMBINE_FLAG;	
							SETIOV(siov + 1, ctrl->extra, msg->extra_len);
							sparts++;
						}
						SETIOV(riov + 0, reply, sizeof *reply);
						SETIOV(riov + 1, buffer, msg->reply_max - sizeof *reply);
						if((ctrl->status = ctrl->send(fd, siov, sparts, riov, 2)) == -1 ||
								msg->subtype == _IO_CONNECT_COMBINE_CLOSE) {
							io_close_t						msg2;
							int								save_errno;

							msg2.i.type = _IO_CLOSE;
							msg2.i.combine_len = sizeof msg2.i;
							SETIOV(siov + 0, &msg2.i, sizeof msg2.i);
							save_errno = errno;
							(void)ctrl->send(fd, siov, 1, 0, 0);
							errno = save_errno;
							if(ctrl->status == -1) {
								ConnectDetach_r(fd);
								fd = -1;
							}
						}
					}
				}
			} else {
				fd = -1;
				errno = ENOTSUP;
			}

		//We didn't get any return message from the server, end of recursion on this tree?
		} else {
			//Stick the discovered fd into an array of fds, resizing if required/allowed
			if (ctrl->fds) {
				if (ctrl->fds_index >= ctrl->fds_len-1) {
					int *tmp;
					//If we are not ok with resizing, or we run out of memory ... error out
					if (!(ctrl->flags & FLAG_MALLOC_FDS) ||
					    !(tmp = realloc(ctrl->fds, (ctrl->fds_len+FD_BUF_INCREMENT)*sizeof(*ctrl->fds)))) {
						errno = ENOMEM;
						return(-1);
					}
					ctrl->fds = tmp;
					ctrl->fds_len += FD_BUF_INCREMENT;
				}

				ctrl->fds[ctrl->fds_index++] = fd;
			}

			//This only gets done once, if the message has extra data ... copy it over
			//We use memmove since there may be overlapping areas involved here.
			if (ctrl->response_len) {
				int b;

				b = min(sizeof(*ctrl->reply), ctrl->response_len);  
				memmove(ctrl->response, ctrl->reply, b); 
				if (ctrl->response_len > b) {
					memmove(((char*)ctrl->response) + b, buffer, ctrl->response_len - b); 
				}

				ctrl->response_len = 0;
			}

			if(ctrl->flags & FLAG_SET_ENTRY) {
				*ctrl->entry = *entry;
				ctrl->entry->prefix_len = prefix_len + path_skip;
			}

copy_path:
			if(ctrl->path) {
				char *insert;
				int  cplen;

				cplen = strlen(path + path_skip) + 1;

				//path[path_skip] != '\0' && !(ctrl->flags & FLAG_NO_PREFIX)
				if (cplen != 1 && !(ctrl->flags & FLAG_NO_PREFIX)) {
					insert = &ctrl->path[ctrl->pathlen];
					cplen++;							//For the extra slash we insert 
				} else {
					insert = NULL;						//Marker for below
				}

				if((ctrl->pathsize - ctrl->pathlen) < cplen) {
					ctrl->path[0] = '\0';
					ctrl->pathsize = 0;				//Indicate that the path was too small
				} else {
					if (insert) {
						*insert++ = '/';
						cplen--;
					} else {
						insert = &ctrl->path[ctrl->pathlen];
					}

					memcpy(insert, path + path_skip, cplen);
				}
			}
		}
	}

	//We don't want to return -1 if we actually manage to cache other managers
	return ((fd == -1 && ctrl->fds && ctrl->fds_index) ? ctrl->fds[0] : fd);
}

int _connect_ctrl(struct _connect_ctrl *ctrl, const char *path, unsigned response_len, void *response) {
	struct _io_connect				*msg = ctrl->msg;
	int								fd;
	int								save_errno;
	int								save_ftype, save_ioflag;
#define MIN_FILLER_SIZE (sizeof(struct _io_connect_entry) * 3 * SYMLOOP_MAX + PATH_MAX + 1)
	struct {
		struct _io_connect_link_reply	reply;	
		char							filler[MIN_FILLER_SIZE];
	}								*buffer;
	int								oflag, freebuffer;
	extern int						__posixly_correct;

	ctrl->chroot_len = 0;

	if(*path == '\0') {
		errno = ENOENT;
		return -1;
	}

	/*
	 * Check if path exceeds PATH_MAX bytes (including terminating NULL)
	 */
	if (__posixly_correct && strlen(path) >= PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}

	/* If we have a valid entry, do we want to test ourselves against it? */
	if(msg->file_type == (unsigned)_FTYPE_MATCHED) {
		msg->file_type = _FTYPE_ANY;
		if(ctrl->entry) {
			ctrl->flags |= FLAG_TEST_ENTRY;
		}
	}
	save_ftype = msg->file_type;
	save_ioflag = msg->ioflag;

onemoretime:
	msg->file_type = save_ftype;
	msg->ioflag = save_ioflag;

	/* 
	 This is where the first response will go from the client. In the
	 case of multiple fd's only the first reply is permanently recorded, 
	 all others are ignored 
	*/
	response_len = (response) ? response_len : 0;
	ctrl->response_len = response_len;
	ctrl->response = response;

	/* TODO: Make the allocation, re-use and buffer sizes 
	         all user configurable somehow. */

	/* 
	 Decide which type of allocation policy we want to
	 use: malloc/free on the heap, alloc/xxx on the stack

	 If we think that we have space on the stack (guesstimate
	 32 symlinks with 32 servers with 1K path == 32 * 2K == 64K) 
	 then go with stack allocation?
	*/
	ctrl->flags |= FLAG_STACK_ALLOC;
	if(_connect_malloc && __stackavail() < MIN_STACK_FOR_GROWTH) {
		ctrl->flags &= ~FLAG_STACK_ALLOC;
	}

	/* 
	 Decide if we are going to re-use the allocated
	 buffer or if we are just going to allocate new
	 buffers as we go along.
	*/
	ctrl->flags |= FLAG_REUSE_BUFFER;
	
	/* 
	 We only need to allocate a copy if we are going to  be iterating
	 over multiple fds, otherwise just make response the same as the
	 reply buffer (unless the reply buffer won't hold our minimum size) 
	 This saves us from double allocating when we don't have to. 
	*/
	if (ctrl->fds || (ctrl->flags & FLAG_MALLOC_FDS) || response_len < sizeof(*buffer)) {
		freebuffer = 1;
		if (!(buffer = CTRL_ALLOC(ctrl->flags, max(response_len, sizeof(*buffer))))) {
			errno = ENOMEM;
			return(-1);
		}
	}
	else {
		freebuffer = 0;
		buffer = response;
	}
	ctrl->reply = (void*)buffer;
	msg->reply_max = max(response_len, sizeof(*buffer));

	ctrl->reply->link.eflag = 0;

	/*
	 Always send a connect message.  When we allow a user 
	 configurable buffer, set the entry_max accordingly.
	*/
	msg->type = _IO_CONNECT;
	msg->entry_max = SYMLOOP_MAX;
	msg->path_len = strlen(path) + 1;
	oflag = msg->ioflag;
	msg->ioflag = (oflag & ~(O_ACCMODE | O_CLOEXEC)) | ((oflag + 1) & msg->access & O_ACCMODE);

	save_errno = errno;
	errno = EOK;
	
	if((fd = _connect_request(ctrl, 0, 0, path, 0,
							  (ctrl->entry && (ctrl->flags & FLAG_NO_RETRY)) ? ctrl->entry : &_connect_proc_entry,
							  buffer->filler, 0)) != -1) {
		errno = save_errno;

		// If not close on exec, then turn off the close on exec bit
		if(!(ctrl->base & _NTO_SIDE_CHANNEL) && !(oflag & O_CLOEXEC)) {
			ConnectFlags_r(0, fd, FD_CLOEXEC, 0);
		}
		if((ctrl->flags & FLAG_NOCTTY) && !(oflag & O_NOCTTY) && isatty(fd)) {
			(void)procmgr_session(ND_LOCAL_NODE, getpid(), fd, PROCMGR_SESSION_TCSETSID);
		}

		//Unfortunately path = "" is both a valid return (internally) but an 
		//error condition (externally) so we resort to munging the path here
		if (ctrl->path && ctrl->pathsize > 1 && ctrl->path[0] == '\0') {
			ctrl->path[0] = '/';
			ctrl->path[1] = '\0';
		}
	} else if(save_ftype == _FTYPE_ANY && (errno == ENOSYS || errno == ENOENT) &&
	          msg->file_type != save_ftype && msg->file_type != (unsigned)_FTYPE_MATCHED) {
		/* If the filetype change, but we still failed, then try sending the
		   request again but with the new filetype.  This is for servers who
		   have mounted on top of filesystems using their services. */
		if (freebuffer) {
			CTRL_FREE(ctrl->flags, buffer);
		}
		save_ftype = msg->file_type;
		goto onemoretime;
	}

	if (freebuffer) {
		CTRL_FREE(ctrl->flags, buffer);
	}
	return fd;
}


__SRCVERSION("_connect_ctrl.c $Rev: 205764 $");
