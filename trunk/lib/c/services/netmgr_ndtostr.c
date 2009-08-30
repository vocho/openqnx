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




#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/neutrino.h>
#include "netmgr_send.h"

int netmgr_ndtostr(unsigned flags, int nd, char *buf, size_t maxbuf) {
	netmgr_ndtostr_t		msg;
	int						status;
	int						len;

	msg.i.hdr.type = _IO_MSG;
	msg.i.hdr.combine_len = sizeof msg.i;
	msg.i.hdr.mgrid = _IOMGR_NETMGR;
	msg.i.hdr.subtype = _NETMGR_NDTOSTR;
	msg.i.len = maxbuf;
	msg.i.flags = flags;
	msg.i.nd = nd;
	if((status = __netmgr_send(&msg.i, sizeof msg.i, 0, 0, buf, maxbuf)) != -1 ||
			errno != ENOTSUP ||
			ND_NODE_CMP(nd, ND_LOCAL_NODE) != 0) {
		return status;
	}

	len = 1; // for the '\0'
	if(maxbuf) {
		*buf = '\0';
	}

	if((flags & (ND2S_LOCAL_STR | ND2S_DIR_SHOW | ND2S_NAME_SHOW | ND2S_DOMAIN_SHOW | ND2S_QOS_SHOW))
			== (ND2S_LOCAL_STR | ND2S_DIR_SHOW)) {
		return len + straddstr("/", 1, &buf, &maxbuf);
	}

	// Default to show at least hostname
	if((flags & (ND2S_DIR_SHOW | ND2S_NAME_HIDE | ND2S_DOMAIN_SHOW | ND2S_QOS_SHOW)) == 0) {
		flags |= ND2S_NAME_SHOW;
	}

	// If only path is requested, don't add domain or hostname
	if((flags & (ND2S_DIR_SHOW | ND2S_NAME_SHOW | ND2S_DOMAIN_SHOW | ND2S_QOS_SHOW)) != ND2S_DIR_SHOW) {

		// If nothing hidden, always show the hostname
		if((flags & (ND2S_DIR_HIDE | ND2S_NAME_HIDE | ND2S_DOMAIN_HIDE | ND2S_QOS_HIDE)) == 0) {
			flags |= ND2S_NAME_SHOW;
		}

		// If no domain view specified, pick a default
		if(!(flags & (ND2S_LOCAL_STR | ND2S_NAME_HIDE | ND2S_DOMAIN_HIDE | ND2S_DOMAIN_SHOW))) {
			flags |= ND2S_DOMAIN_SHOW;
		}
	}

	// If dir and domain requested, add hostname
	if(		(flags & (ND2S_DIR_SHOW | ND2S_NAME_HIDE | ND2S_DOMAIN_SHOW)) ==
			(ND2S_DIR_SHOW | ND2S_DOMAIN_SHOW)) {
		flags |= ND2S_NAME_SHOW;
	}

	// The local directory is always a slash for the root
	if(flags & ND2S_DIR_SHOW) {
		len += straddstr("/", 1, &buf, &maxbuf);
	}

	// Always add name unless specificly told not to
	if(flags & ND2S_NAME_SHOW) {
		int					nbytes;

		if((nbytes = confstr(_CS_HOSTNAME, buf, maxbuf)) > 0) {
			len += nbytes - 1;
			if(maxbuf) {
				int size = strlen(buf);
   	
				buf += size;
				maxbuf -= size;
			}
		}
	}

	if(flags & ND2S_DOMAIN_SHOW) {
		if(confstr(_CS_DOMAIN, 0, 0) > 0) {
			// If anything was before, add a dot
			if(flags & (ND2S_DIR_SHOW | ND2S_NAME_SHOW | ND2S_SEP_FORCE)) {
				len += straddstr(".", 1, &buf, &maxbuf);
			}

			len += confstr(_CS_DOMAIN, buf, maxbuf) - 1;
			if(maxbuf) {
				int size = strlen(buf);
   	
				buf += size;
				maxbuf -= size;
			}
		} else {
			flags &= ~ND2S_DOMAIN_SHOW;
		}
	}

	// Add a trailing slash if dir requested and something followed it
	// Made the directory so it nulls it self out if included as a path
	if((flags & ND2S_DIR_SHOW) && ((flags & (ND2S_DOMAIN_SHOW | ND2S_NAME_SHOW)) != 0)) {
		len += straddstr("/../", 4, &buf, &maxbuf);
	}

	return len;
}

__SRCVERSION("netmgr_ndtostr.c $Rev: 153052 $");
