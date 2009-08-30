/*
 * $QNXLicenseC:
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software
 * Systems (QSS) and its licensors.  Any use, reproduction, modification,
 * disclosure, distribution or transfer of this software, or any software
 * that includes or is based upon any of this code, is prohibited unless
 * expressly authorized by QSS by written agreement.  For more information
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
 */
#ifndef DMGR_PRIV_INCLUDED
#define DMGR_PRIV_INCLUDED

#include <pthread.h>
#include <sys/iofunc.h>

struct wrec {
	struct wrec *next;
	uint32_t    reclen;        /* length of this record */
};
#define WRECSIZE sizeof(struct wrec)

struct relay_cb;
struct relay_ocb {
	struct relay_cb  *rcb;
	struct relay_ocb *next;
	int              realfd;
	uint32_t         flag;
	struct wrec      *rp;
	uint32_t         rec_off;
	int              rcvid;
	iofunc_notify_t  notify[3];
	pthread_t        block_tid;
	char             *bpbuf;       /* the buffer for by pass request */
};
#define OCB_FLAG_UNBLOCK 0x00000001
#define OCB_FLAG_NOTIFY  0x00000002

struct relay_cache {
	pthread_rwlock_t ca_rwlock;
	uint8_t          *ca_buf;
	uint8_t          *ca_end;
	int              ca_size;
	struct wrec      *ca_wp;
	struct wrec      *ca_rp;
};

struct relay_cb {
	iofunc_attr_t    attr;
	struct relay_cb  *next;

	char             *devname;
	int              rcoid;       /* real coid to orignal manager */
	int              resid;
	int              resdid;
	dispatch_t       *dpp;
	int              bpbuf_size;  /* bypass buffer (per ocb) size */
	int              index;       /* index in the rcb_list */
	int              cloner_key;  /* is cloner allowed to write ? */
	
	pthread_mutex_t  mutex;       /* mutex to protect owner/cloner list */
	struct relay_ocb *owner;      /* owner of real device */
	struct relay_ocb *cloner;     /* ocbs for .ditto device */

	/* the buffer cache _IO_WRITE for owner/cloner */
	struct relay_cache ocache;
	struct relay_cache ccache;
};
#endif
