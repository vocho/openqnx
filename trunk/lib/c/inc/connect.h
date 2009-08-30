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




/* Flags used in the connect_ctrl structure */
#define FLAG_NOCTTY			0x0001
#define FLAG_SET_ENTRY		0x0002
#define FLAG_TEST_ENTRY		0x0004		/* Test the entry, nid,pid,handle,chid */
#define FLAG_NO_PREFIX		0x0008
#define FLAG_NO_SYM			0x0010
#define FLAG_MALLOC_FDS		0x0020
#define FLAG_TEST_NPC_ONLY	0x0040		/* Test only the nid,pid,chid when FLAG_TEST_ENTRY is set*/

#define FLAG_REUSE_BUFFER	0x0080		/* Re-use the initial buffer instead of allocting more*/
#define FLAG_STACK_ALLOC	0x0100		/* Allocate from stack (alloca) not heap (malloc) */
#define FLAG_TEST_ND_ONLY	0x0200		/* Only send full requests to the node matching the 'nd' value */
#define FLAG_NO_RETRY		0x0400		/* If at first you don't succeed, then fail (no link resolution) */
#define FLAG_ERR_COLLISION	0x0800		/* Return an error if an earlier remgr has a matching file */

/*
	Typecast through a void * to keep the Metaware compiler quiet about
	dangerous type casting.
*/
#define PCAST( typ, val )	((typ *)(void *)(val))
#define FD_BUF_INCREMENT    10

struct _connect_ctrl {
	int									base;
	struct _io_connect					*msg;

	union {
		struct _io_connect_link_reply		link;
		struct _io_connect_ftype_reply		ftype;
	}									*reply;

	struct _io_connect_entry			*entry;
	const void							*extra;
	int									status;
	void								*response;
	int (*send)(int coid, const iov_t *smsg, int sparts, const iov_t *rmsg, int rparts);
	int									flags;
	char								*path;
	int									pathsize;
	int									pathlen;
	int									prefix_len;

	uint16_t							fds_len;
	uint16_t							fds_index;
	int									*fds;
	int									response_len;
	uint32_t							nd;
	size_t								chroot_len;
};


int _connect_ctrl(struct _connect_ctrl *ctrl, const char *path, unsigned response_len, void *response);

/* __SRCVERSION("connect.h $Rev: 205764 $"); */
