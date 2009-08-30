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





#define _FILE_OFFSET_BITS	64
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <share.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include <sys/pathmsg.h>
#include <sys/netmgr.h>
#include <sys/iomsg.h>
#include <sys/statvfs.h>

#include <sys/mount.h>
#include "connect.h"

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

/*
 This function does nothing but package the data into a message to send off to a 
 device that answers the call presented in this message.
 
 The ... arguments are defined as
 [datalen],[special]

 [datalen] is the size in bytes of data being passed to the client,
		   only valid if the _MFLAG_STRUCT flag is set.  Otherwise
		   it is assumed that data is a null terminated character string.
 [special] is a null terminated character string indicating the 
		   special device and is only valid if the _MFLAG_SPEC flag 
		   is set.
*/
int _mount(int mflags, const char *type, const char *dir, int flags, const void *data, ...) {
	va_list					argument;
	io_mount_extra_t		*mount_xtra;
	int						fd, ret, datalen, extra_len;
	char					*dst, *spec;

    struct _connect_ctrl            ctrl;
	struct _io_connect              msg;
    struct _io_connect_entry        entry;

	// Check for invalid operations or overlapping flags, our mount 
	if (!dir) {
		errno = EINVAL;
		return(-1);
	}
	else if (flags & (_MOUNT_REMOUNT | _MOUNT_UNMOUNT)) {
		type = "";
	}
	else if (!type) {
		errno = EINVAL;
		return(-1);
	}

	fd = -1;
	spec = NULL;
	va_start(argument, data);

	//Determine how long the extra data passed in is
	if (mflags & _MFLAG_STRUCT) {
		datalen = va_arg(argument, int);
	}
	else {
		datalen = (data) ? strlen(data) + 1 : 0;
	}

	//Check to see if we have a special device name
	if (mflags & _MFLAG_SPEC || mflags & _MFLAG_OCB) {
		spec = va_arg(argument, char *);
	}

	//We expect that data has been properly formatted for the rcvr
	extra_len = sizeof(*mount_xtra) ;
	extra_len += strlen(type) + 1 + datalen;
	extra_len += (spec && (mflags & _MFLAG_SPEC)) ? strlen(spec) + 1 : 0;

	//Allocate a temporary block of data (until we can use an IOV or something)
	if (!(mount_xtra = (io_mount_extra_t *)alloca(extra_len))) {
		errno = ENOMEM;
		return(-1);
	}
	memset(mount_xtra, 0, extra_len);
	mount_xtra->flags = flags;	
	mount_xtra->nbytes = extra_len;
	mount_xtra->datalen = datalen;

	//Do we have a spec entry, and do we need to fill and ocb for it?
	if ((mflags & _MFLAG_OCB) && !(flags & (_MOUNT_REMOUNT | _MOUNT_UNMOUNT))) {
		for (;;) {
		    memset(&ctrl, 0x00, sizeof(ctrl));
			memset(&msg, 0x00, sizeof(msg));
			ctrl.send = MsgSendvnc;
			ctrl.msg = &msg;
		    ctrl.entry = &entry;
		    ctrl.flags = FLAG_SET_ENTRY;

			//Resolve through all links, in the mode the user asked for
			msg.subtype = _IO_CONNECT_OPEN;
			msg.ioflag = ((flags & _MOUNT_READONLY) ? O_RDONLY : O_RDWR) | O_LARGEFILE | O_NOCTTY;
			msg.sflag = SH_DENYNO;
			msg.access = _IO_FLAG_RD | _IO_FLAG_WR;

			if ((fd = _connect_ctrl(&ctrl, spec, 0, 0)) == -1) {
				// try an implicit read-only mount on non-writable media
				if (errno != EROFS || flags & _MOUNT_READONLY)
					return(-1);
				mount_xtra->flags = flags |= _MOUNT_READONLY;
			}
			else {
				//We could probably copy this information from the entry structure
				if (ConnectServerInfo(0, fd, &mount_xtra->extra.cl.info) != fd)
					return(-1);
				//This may be re-dundant after the above call.
				mount_xtra->extra.cl.info.nd = netmgr_remote_nd(mount_xtra->extra.cl.info.nd, ND_LOCAL_NODE);
				mount_xtra->extra.cl.info.pid = getpid();
				//mount_xtra->extra.cl.info.chid = info.chid;
				//mount_xtra->extra.cl.info.scoid = info.scoid;
				mount_xtra->extra.cl.info.coid = fd;
				break;
			}
		}
	}
	else {
	    memset(&ctrl, 0x00, sizeof(ctrl));
		memset(&msg, 0x00, sizeof(msg));
		ctrl.send = MsgSendvnc;
		ctrl.msg = &msg;
	}

	//Copy the use data over, would be better as an iov 
	//Actually send off the request to a matching manager (we hope)
	dst = (char *)mount_xtra + sizeof(*mount_xtra);

	memcpy(dst, data, datalen); 
	dst += datalen;

	memcpy(dst, type, strlen(type) + 1);
	dst += strlen(type) + 1;

	if (spec && (mflags & _MFLAG_SPEC)) {
		memcpy(dst, spec, strlen(spec) + 1);
		dst += strlen(spec) + 1;
	}

    msg.subtype = _IO_CONNECT_MOUNT;
	msg.mode = 0;
	msg.ioflag = ((flags & _MOUNT_READONLY) ? O_RDONLY : O_RDWR) | O_LARGEFILE | O_NOCTTY;
	msg.sflag = SH_DENYNO;
	msg.access = _IO_FLAG_RD | _IO_FLAG_WR;
	if (flags & (_MOUNT_REMOUNT | _MOUNT_UNMOUNT))
		msg.file_type = _FTYPE_ANY;
	else
		msg.file_type = _FTYPE_MOUNT;
    msg.extra_type = (fd == -1) ? _IO_CONNECT_EXTRA_MOUNT
							    : _IO_CONNECT_EXTRA_MOUNT_OCB;
    msg.extra_len = extra_len;
    ctrl.extra = mount_xtra;
    ctrl.flags = (fd == -1) ? 0 : FLAG_TEST_ENTRY | FLAG_TEST_NPC_ONLY;

    ret = _connect_ctrl(&ctrl, dir, 0, 0);

	if (ret != -1)
		ConnectDetach(ret);
	if (fd != -1)
		close(fd);

	va_end(argument);

	return((ret == -1) ? -1 : 0);
}

/*
 If the datalen field is < 0 then it is assumed that the data structure
 being passed in is a NULL terminated string.  Otherwise it is assumed
 that the data pointed to by data is of length datalen.

 If spec is not preceeded with a leading slash then _mount will
 be called with the _MFLAG_SPEC flag and asked to pass that mount information
 along as a string.

 OCB handling is finicky.  If the _MFLAG_OCB is turned on on the
 call to mount(), this is an over-ride that indicates that we should
 NOT try and get a server connection for the special device.  The
 flag gets turned off for the call to _mount().  Otherwise we will
 stat() the special device, and if successful then turn on the 
 _MFLAG_OCB for _mount().  This is better than relying on a leading
 slash since it allows us to use relative paths to devices.

 We let the top bits of the _MOUNT and _MFLAG flags overlap so that we
 can let the user override the desicions that are made for them here.
*/
#define MFLAG_MOUNT_MASK (_MFLAG_OCB | _MFLAG_SPEC | _MFLAG_STRUCT)
int mount(const char *spec, const char *dir, int flags, const char *type, const void *data, int datalen) {
	struct stat st;
	int			ret, mflags;
	char		*fullspec;

	//At some point we could let the user specify the _MFLAG_XXX values ... not now
	mflags = 0;
	fullspec = NULL;

	if (spec) {					//Always include the special device if we have one
		mflags |= _MFLAG_SPEC;
		//We want to stat the special device to see if it exists 
		if (!(flags & _MFLAG_OCB) && stat(spec, &st) != -1) {
			//TODO: determine a better way to get the size required
			if (!(fullspec = malloc(PATH_MAX)) ||
			    !_fullpath(fullspec, spec, PATH_MAX)) {
				if (fullspec) { 
					free(fullspec);
				}
				return -1;
			}
			mflags |= _MFLAG_OCB;	
		}
		else {
			mflags &= ~_MFLAG_OCB;
		}
	}

	if (datalen >= 0) {
		ret = _mount(mflags | _MFLAG_STRUCT, type, dir, flags, 
						data, datalen, (fullspec) ? fullspec : spec);
	}
	else {
		ret = _mount(mflags, type, dir, flags, 
						data, (fullspec) ? fullspec : spec);
	}

	if (fullspec) {
		free(fullspec);
	}

	return(ret);
}

int umount(const char *dir, int flags) {
	return(_mount(0, NULL, dir, flags | _MOUNT_UNMOUNT, NULL));
}

char * mount_parse_generic_args(char *options, int *flags) {
	static char * const FIXCONST mount_opts[] = { "ro", "rw",
								  "noexec", "exec",
								  "nosuid", "suid",
								  "nocreat", "creat",
								  "noatime", "atime",
								  "remount", "update", 
								  "before", 
								  "after", 
								  "opaque", 
								  "nostat",
								  "enum", "enumerate",
								  "implied",
								   NULL };
	char *value, *saved, *ret;

	if (!options || !flags) {
		return(NULL);
	}

	ret = options;
	saved = alloca(strlen(options) + 1);
	if (!saved) {
		return NULL;
	}
	memset(saved, 0, strlen(options) + 1);

	while (*options != '\0') {
		switch(getsubopt(&options, mount_opts, &value)) {
		case 0:                         //ro
        *flags |= _MOUNT_READONLY; break;
        case 1:                         //rw
        *flags &= ~_MOUNT_READONLY; break;

        case 2:                         //noexec
        *flags |= _MOUNT_NOEXEC; break;
        case 3:                         //exec
        *flags &= ~_MOUNT_NOEXEC; break;

        case 4:                         //nosuid
        *flags |= _MOUNT_NOSUID; break;
        case 5:                         //suid
        *flags &= ~_MOUNT_NOSUID; break;

        case 6:                         //nocreat
        *flags |= _MOUNT_NOCREAT; break;
        case 7:                         //creat
        *flags &= ~_MOUNT_NOCREAT; break;

        case 8:                         //noatime
        *flags |= _MOUNT_NOATIME; break;
        case 9:                         //atime
        *flags &= ~_MOUNT_NOATIME; break;

        case 10:                         //remount, update
		case 11:
        *flags |= _MOUNT_REMOUNT; break;
        case 12:                         //before
        *flags |= _MOUNT_BEFORE; break;
        case 13:                         //after
        *flags |= _MOUNT_AFTER; break;
        case 14:                         //opaque
        *flags |= _MOUNT_OPAQUE; break;
        case 15:                         //nostat
        *flags |= _MFLAG_OCB; break;
        case 16:                         //enum, enumerate
        case 17:                         
        *flags |= _MOUNT_ENUMERATE; break;
        case 18:                         //implied
        *flags |= _MOUNT_IMPLIED; break;

        default:
			if (saved && saved[0]) 
				strcat(saved, ",");
			strcat(saved, value);
        }
	}
	if (strlen(saved)) 
		strcpy(ret, saved);
	else
		ret = NULL;
	return(ret);
}


__SRCVERSION("mount.c $Rev: 153052 $");
