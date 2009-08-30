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
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <sys/procmgr.h>
#include <sys/resmgr.h>
#include <sys/stat.h>

#include "dmgr_priv.h"

resmgr_connect_funcs_t di_connect_funcs;
resmgr_io_funcs_t      di_io_funcs;
resmgr_attr_t          res_attr;

int                    obufsize = 8192;
pthread_rwlock_t       rcb_list_rwlock = PTHREAD_RWLOCK_INITIALIZER;
struct relay_cb        **rcb_list = 0;
int                    rcb_total = 0;
int                    ncoid = -1;
#define RCB_GROW       4

/* global option */
int                    cloner_key = 1;

static int path_init(char *devname, dispatch_t *dpp, int cloner_key)
{
	struct relay_cb   *rcbp;
	char              ldevname[PATH_MAX + 1];
	int               fd, i;
	struct stat       sb;
	
	if ((fd = open(devname, O_RDWR|O_NOCTTY)) == -1 || fstat(fd, &sb) != 0)
	  return errno;
	
	/* make sure this is a CHR device */
	if (!S_ISCHR(sb.st_mode))
	  return EINVAL;
	
	if ((rcbp = malloc(sizeof(*rcbp))) == 0)
	  return errno;
	
	memset(rcbp, 0, sizeof(*rcbp));
	iofunc_attr_init(&rcbp->attr, 0666 | S_IFCHR, 0, 0);
	pthread_mutex_init(&rcbp->mutex, 0);
	pthread_rwlock_init(&rcbp->ocache.ca_rwlock, 0);
	pthread_rwlock_init(&rcbp->ccache.ca_rwlock, 0);
	rcbp->dpp = dpp;
	rcbp->rcoid = fd;
	rcbp->cloner_key = cloner_key;
	
	if ((rcbp->ocache.ca_buf = malloc(obufsize)) == 0 || 
		(rcbp->ccache.ca_buf = malloc(obufsize)) == 0)
	{
		free(rcbp);
		return errno;
	}
	rcbp->bpbuf_size     = obufsize;
	
	rcbp->ocache.ca_size = obufsize;
	rcbp->ocache.ca_end  = rcbp->ocache.ca_buf + rcbp->ocache.ca_size;
	rcbp->ocache.ca_rp   = rcbp->ocache.ca_wp = (struct wrec *)rcbp->ocache.ca_buf;
	rcbp->ocache.ca_rp->next = 0;

	rcbp->ccache.ca_size = obufsize;
	rcbp->ccache.ca_end  = rcbp->ccache.ca_buf + rcbp->ccache.ca_size;
	rcbp->ccache.ca_rp   = rcbp->ccache.ca_wp = (struct wrec *)rcbp->ccache.ca_buf;
	rcbp->ccache.ca_rp->next = 0;

	if ((rcbp->resid =
		 resmgr_attach(dpp, &res_attr, devname, _FTYPE_ANY,
					   _RESMGR_FLAG_BEFORE, &di_connect_funcs,
					   &di_io_funcs, rcbp)) == -1)
	{
		int n = errno;
		free(rcbp->ocache.ca_buf);
		free(rcbp->ccache.ca_buf);
		free(rcbp);
		return n;
	}

	ldevname[PATH_MAX] = 0;
	snprintf(ldevname, PATH_MAX, "%s.ditto", devname);
	if ((rcbp->resdid = 
		 resmgr_attach(dpp, &res_attr, ldevname, _FTYPE_ANY,
					   0, &di_connect_funcs, &di_io_funcs, rcbp)) == -1)
	{
		int n = errno;
		resmgr_detach(dpp, rcbp->resid,  _RESMGR_DETACH_ALL);
		free(rcbp->ocache.ca_buf);
		free(rcbp->ccache.ca_buf);
		free(rcbp);
		return n;
	}
	rcbp->devname = strdup(devname);

	pthread_rwlock_wrlock(&rcb_list_rwlock);
	for (i = 0; i < rcb_total; i++)
	  if (rcb_list[i] == NULL)
		break;
	
	if (i == rcb_total) {
		rcb_list = realloc(rcb_list, sizeof(rcb_list) * (rcb_total + RCB_GROW));
		if (!rcb_list) {
			int n = errno;
			resmgr_detach(dpp, rcbp->resid, _RESMGR_DETACH_PATHNAME);
			resmgr_detach(dpp, rcbp->resdid, _RESMGR_DETACH_PATHNAME);
			free(rcbp->ocache.ca_buf);
			free(rcbp->ccache.ca_buf);
			free(rcbp);
			return n;
		}
		memset(&rcb_list[rcb_total], 0, sizeof(rcb_list) * RCB_GROW);
		rcb_total += RCB_GROW;
	}
	rcbp->index = i;
	rcb_list[i] = rcbp;
	pthread_rwlock_unlock(&rcb_list_rwlock);
	
	return EOK;
}

static int mt_mount(resmgr_context_t *ctp, io_mount_t *msg, void *handle, io_mount_extra_t *extra)
{
	struct relay_cb  *rcb = handle;
	struct _client_info cinfo;
	
	if((extra->flags & _MFLAG_OCB) && 
	   strcmp(extra->extra.srv.type, "clone") &&  strcmp(extra->extra.srv.type, "ditto"))
	  return ENOSYS;

	/* only root allow to mount/umount */
	if (ConnectClientInfo(ctp->info.scoid, &cinfo, 0) == -1)
	  return errno;
	
	if (cinfo.cred.euid != 0)
	  return EPERM;
	
	if(extra->flags & _MFLAG_OCB)
	{
		int optkey = cloner_key;
		char *opt;

		/* is there an option */
		if ((opt = extra->extra.srv.data)) {
			if (strnicmp(opt, "noinput", 7) == 0)
			  optkey = 0;
			else if (strnicmp(opt, "input", 5) == 0)
			  optkey = 1;
		}
		
		if (path_init(extra->extra.srv.special, ctp->dpp, optkey) == -1)
		  return errno;
		return EOK;
	}
	
	if(extra->flags & _MOUNT_UNMOUNT)
	{
		/* detach the name */
		resmgr_detach(rcb->dpp, rcb->resid, _RESMGR_DETACH_PATHNAME);
		resmgr_detach(rcb->dpp, rcb->resdid, _RESMGR_DETACH_PATHNAME);
		free(rcb->devname);
		rcb->devname = 0;

		pthread_rwlock_wrlock(&rcb_list_rwlock);
		if (rcb->index < rcb_total && rcb_list[rcb->index] == rcb)
		  rcb_list[rcb->index] = NULL;
		pthread_rwlock_unlock(&rcb_list_rwlock);
		
		if (!rcb->owner && !rcb->cloner) {
			rcb_list[rcb->index] = 0;
			free(rcb->ocache.ca_buf);
			free(rcb->ccache.ca_buf);
			free(rcb);
			return EOK;
		}
		
	}
	return EOK;
}

/* SIGUSR1 handler */
static void sigio_handle(int signo)
{
	return;
}

/* notify pulse from device */
int di_notify_pulse(message_context_t *ctp, int code, unsigned flag, void *hdl)
{
	struct relay_cb  *rcb;
	struct relay_ocb *o;
	unsigned value = ctp->msg->pulse.value.sival_int;
	int index = (value & 0xffff);
	int trig = 0;

	pthread_rwlock_rdlock(&rcb_list_rwlock);
	if ((index >= rcb_total) || (rcb = rcb_list[index]) == NULL) {
		pthread_rwlock_unlock(&rcb_list_rwlock);
		return EINVAL;
	}
	iofunc_attr_lock(&rcb->attr);
	pthread_rwlock_unlock(&rcb_list_rwlock);
	
	if (value & _NOTIFY_COND_INPUT)
	  trig |= IOFUNC_NOTIFY_INPUT;
	if (value & _NOTIFY_COND_OUTPUT)
	  trig |= IOFUNC_NOTIFY_OUTPUT;
	if (value & _NOTIFY_COND_OBAND)
	  trig |= IOFUNC_NOTIFY_OBAND;

	pthread_mutex_lock(&rcb->mutex);
	for (o = rcb->owner; o; o = o->next) {
		iofunc_notify_trigger(o->notify, 1, trig);
	}
	pthread_mutex_unlock(&rcb->mutex);
	iofunc_attr_unlock(&rcb->attr);
	return EOK;
}

extern int di_mgr_init(resmgr_connect_funcs_t *cf, resmgr_io_funcs_t *iof);
int main(int argc, char **argv)
{
	int                    chid, ch;
	extern dispatch_t      *_dispatch_create(int chid, int flag);
	dispatch_t             *dpp;
	thread_pool_attr_t     pool_attr;
	thread_pool_t          *tpp;
	resmgr_connect_funcs_t mt_connect_funcs;
	
	while ((ch = getopt(argc, argv, "ko:")) != -1) {
		switch (ch) {
		  case 'o':
			obufsize = atoi(optarg);
			break;
		  case 'k':
			cloner_key = 0;
			break;
		  default:
			break;
		}
	}
	
	argc -= optind;
	argv += optind;

	memset(&res_attr, 0, sizeof(res_attr));
	res_attr.nparts_max = 10;
	
	di_mgr_init(&di_connect_funcs, &di_io_funcs);
	di_connect_funcs.mount  = mt_mount;
	
	if ((chid = ChannelCreate(_NTO_CHF_UNBLOCK | _NTO_CHF_DISCONNECT | _NTO_CHF_REPLY_LEN | _NTO_CHF_SENDER_LEN)) == -1)
	{
		perror("ChannelCreate");
		return EXIT_FAILURE;
	}
	
	memset(&pool_attr, 0x00, sizeof pool_attr);
	if(!(pool_attr.handle = dpp = _dispatch_create(chid, 0))) {
		perror("dispatch_create");
		return EXIT_FAILURE;
	}
	pool_attr.context_alloc = (void *)dispatch_context_alloc;
	pool_attr.block_func    = (void *)dispatch_block;
	pool_attr.handler_func  = (void *)dispatch_handler;
	pool_attr.context_free  = (void *)dispatch_context_free;
	pool_attr.lo_water      = 2;
	pool_attr.hi_water      = 5;
	pool_attr.increment     = 2;
	pool_attr.maximum       = 10;
	
	if(!(tpp = thread_pool_create(&pool_attr, POOL_FLAG_EXIT_SELF))) {
		perror("thread_pool_create");
		return EXIT_FAILURE;
	}

	if ((ncoid = ConnectAttach(0, 0, chid, 0, 0)) == -1) {
		perror("ConnectAttach");
		return EXIT_FAILURE;
	}
	
	memset(&mt_connect_funcs, 0, sizeof(mt_connect_funcs));
	mt_connect_funcs.nfuncs = _RESMGR_CONNECT_NFUNCS;
	mt_connect_funcs.mount  = mt_mount;
	if(resmgr_attach(dpp, &res_attr, 0, _FTYPE_MOUNT,
					 _RESMGR_FLAG_FTYPEONLY | _RESMGR_FLAG_DIR, 
					 &mt_connect_funcs, 0, 0) == -1) 
	{
		perror("resmgr_attach(0)");
		return EXIT_FAILURE;
	}
	
	while (argc--) {
		if (argv[0][0] != '/') {
			fprintf(stderr, "Warning: Devicename '%s' must start with /\n", argv[0]);
			continue;
		}
		if ((ch = path_init(argv[0], dpp, cloner_key)) != 0) {
			fprintf(stderr, "Warning: Can't initialize '%s': %s\n", argv[0], strerror(ch));
		}
		argv++;
	}

	if (pulse_attach(dpp, 0, SI_NOTIFY, di_notify_pulse, 0) == -1)
	{
		perror("pulse_attach");
		return errno;
	}
	signal(SIGUSR1, sigio_handle);
	
#ifdef NDEBUG
	if (procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NOCLOSE | PROCMGR_DAEMON_NODEVNULL) == -1)
	{
		perror("procmgr_daemon");
		return -1;
	}
#endif	

	return thread_pool_start(tpp);
}
