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


/*
 *  "mq"  -  message queues via kernel asynchronous messaging
 *
 *  John Garvey, QNX Software Systems Ltd
 */

#ifdef __USAGE
%C - POSIX message queue daemon

Options:
  -m    Set the default mq_maxmsg for a NULL mq_attr (64 msgs)
  -s    Set the default mq_msgsize for a NULL mq_attr (256 bytes)
  -N    Set the pathname of the directory for message queues ("/dev/mq")
  -d    Do not daemonize
#endif

struct _iofunc_ocb;
#define RESMGR_OCB_T struct _iofunc_ocb
#define IOFUNC_OCB_T struct _iofunc_ocb
struct mq;
#define RESMGR_HANDLE_T struct mq

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <mqueue.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/asyncmsg.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/resmgr.h>
#include <sys/resource.h>
#include <sys/sysmgr.h>

#define MQ_DFLT_PREFIX	"/dev/mq"
#define MQ_DFLT_NMSGS	64
#define MQ_DFLT_SZMSG	256

#define OCB2MQ(_ocb) (mq_t *)((_ocb)->attr)

typedef struct mq {
	iofunc_attr_t	attr;
	struct mq_attr	mqattr;
	char			*name;
	struct mq		*link;
} mq_t;

extern int	resmgr_mq_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
extern int	resmgr_mq_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra);
extern int	resmgr_mq_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
extern int	resmgr_mq_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);
extern int	resmgr_dir_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
extern int	resmgr_dir_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra);
extern int	resmgr_dir_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb);
extern int	resmgr_dir_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);
extern int	resmgr_dir_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb);

iofunc_mount_t			Mount;
mq_t					MessageQueues;
resmgr_connect_funcs_t	MqConnectFuncs, DirConnectFuncs;
resmgr_io_funcs_t		MqIoFuncs, DirIoFuncs;
resmgr_attr_t			ResmgrAttr;
int						DirPathname, MaxQueues;

/*
 *  Lookup the given queue in the list of created queues.
 */
static mq_t *lookup_mq(mq_t *head, const char *name)
{
mq_t	*list;

	for (list = head->link; list != NULL && strcmp(name, list->name); list = list->link)
		;
	return(list);
}

/*
 *  Create a new queue, and link into the list of created queues.
 */
static mq_t *create_mq(mq_t *head, const char *name, struct _client_info *cred, iofunc_attr_t *attr, struct mq_attr *mqattr)
{
mq_t	*mq;

	if (head->attr.nbytes >= MaxQueues) {
		errno = ENFILE;
	}
	else if ((mq = malloc(sizeof(mq_t))) != NULL) {
		if ((mq->name = strdup(name)) != NULL) {
			memcpy(&mq->attr, attr, sizeof(iofunc_attr_t));
			memcpy(&mq->mqattr, (mqattr != NULL) ? mqattr : &head->mqattr, sizeof(struct mq_attr));
			mq->mqattr.mq_sendwait = mq->mqattr.mq_recvwait = 0;
			if ((mq->attr.rdev = ChannelCreateExt(_NTO_CHF_GLOBAL | _NTO_CHF_FIXED_PRIORITY, attr->mode, mq->mqattr.mq_msgsize, mq->mqattr.mq_maxmsg, NULL, &cred->cred)) != -1) {
				mq->link = head->link, head->link = mq;
				++head->attr.nbytes;
				return(mq);
			}
			free(mq->name);
		}
		else {
			errno = ENOMEM;
		}
		free(mq);
	}
	else {
		errno = ENOMEM;
	}
	return(NULL);
}

/*
 *  Cleanup the given queue (as a result of unlink() or close() handling).
 */
static void destroy_mq(mq_t *head, mq_t *mq, int force)
{
mq_t	**mp;

	if (head != NULL) {
		for (mp = &head->link; *mp != mq; mp = &(*mp)->link)
			;
		*mp = mq->link;
		--head->attr.nbytes;
	}
	if ((!mq->attr.nlink && !mq->attr.count) || force) {
		ChannelDestroy(mq->attr.rdev);
		free(mq->name);
		free(mq);
	}
}

/*
 *  Construct a stat structure from the given queue (the rdev field
 *  is overloaded depending on whether we are providing the stat to
 *  client via mq_open() or the namespace).
 */
static void detail_mq(mq_t *mq, struct stat *st, int namespace)
{
union _channel_connect_attr	qattr;

	memset(st, 0, sizeof(struct stat));
	st->st_ino = (uintptr_t)mq;
	st->st_size = ChannelConnectAttr(mq->attr.rdev, &qattr, NULL, _NTO_CHANCON_ATTR_CURMSGS) ? 0 : qattr.num_curmsgs;
	st->st_dev = mq->attr.mount->dev;
	st->st_rdev = namespace ? S_INMQ : mq->attr.rdev;
	st->st_uid = mq->attr.uid, st->st_gid = mq->attr.gid;
	st->st_ctime = mq->attr.ctime, st->st_mtime = mq->attr.mtime, st->st_atime = mq->attr.atime;
	st->st_mode = mq->attr.mode, st->st_nlink = mq->attr.nlink;
	st->st_blocksize = st->st_blksize = mq->mqattr.mq_msgsize;
	st->st_nblocks = mq->mqattr.mq_maxmsg;
	st->st_blocks = (st->st_size * st->st_blksize) / 512;
}

/*
 *  Provide the nth item from the list of active message queues (readdir).
 */
static int get_nth_entry(mq_t *head, int index, mq_t **nth)
{
	while ((head = head->link) != NULL && --index >= 0)
		;
	return(*nth = head, index < 0);
}

/*
 *  Perform readdir() processing - list all the active message queues
 *  in a single ("/dev/mq") pseduo-directory.
 */
static int list_mq(mq_t *dir, off64_t *offset, struct dirent *dp, int dpsize, int xtra, int *nbytes)
{
mq_t						*mq;
struct dirent_extra_stat	*dextra;
int							nmlen;

	*nbytes = 0;
	for (;;) {
		if (!get_nth_entry(dir, *offset, &mq))
			return(EOK);
		nmlen = strlen(mq->name);
		if (dpsize < ((offsetof(struct dirent, d_name) + nmlen + 1 + 7) & ~7))
			break;
		memset(dp, 0, sizeof(struct dirent));
		dp->d_ino = (uintptr_t)mq;
		dp->d_offset = *offset;
		dp->d_namelen = nmlen;
		strcpy(dp->d_name, mq->name);
		dextra = _DEXTRA_FIRST(dp);
		dp->d_reclen = (char *)dextra - (char *)dp;
		if (xtra && dpsize >= ((dp->d_reclen + sizeof(struct dirent_extra_stat) + 7) & ~7)) {
			dextra->d_datalen = sizeof(struct stat);
			dextra->d_type = _DTYPE_LSTAT;
			detail_mq(mq, &dextra->d_stat, !0);
			dextra = _DEXTRA_NEXT(dextra);
			dp->d_reclen = (char *)dextra - (char *)dp;
		}
		*nbytes += dp->d_reclen, dpsize -= dp->d_reclen;
		dp = (struct dirent *)((char *)dp + dp->d_reclen);
		++*offset;
	}
	return((*nbytes != 0) ? EOK : EMSGSIZE);
}

/*
 *  Handle client/libc "mq_open()".  To prevent capturing an old mq_open()
 *  we insist on _IO_CONNECT_EXTRA_MQUEUE and use unique/partial length.
 */
int resmgr_mq_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
mq_t				*mq;
struct mq_attr		*mqattr;
struct _client_info	cred;
iofunc_attr_t		attr;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_MQUEUE || msg->connect.file_type != _FTYPE_MQUEUE)
		return(ENOSYS);
	if (ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE))
		return(ENOREMOTE);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if (*msg->connect.path == '\0') {
		return((msg->connect.ioflag & O_CREAT) ? EINVAL : ENOENT);
	}
	else if ((mq = lookup_mq(handle, msg->connect.path)) == NULL) {
		if (!(msg->connect.ioflag & O_CREAT))
			return(ENOENT);
		if (!msg->connect.extra_len)
			mqattr = NULL;
		else if (msg->connect.extra_len != offsetof(struct mq_attr, mq_sendwait))
			return(ENOSYS);
		else if ((mqattr = (struct mq_attr *)extra)->mq_maxmsg <= 0 || mqattr->mq_msgsize <= 0)
			return(EINVAL);
		msg->connect.mode = S_IFNAM | (msg->connect.mode & ~S_IFMT);
		if ((error = iofunc_open(ctp, msg, &attr, &handle->attr, &cred)) != EOK)
			return(error);
		iofunc_time_update(&attr);
		if ((mq = create_mq(handle, msg->connect.path, &cred, &attr, mqattr)) == NULL)
			return(errno);
	}
	else {
		if ((error = iofunc_open(ctp, msg, &mq->attr, NULL, &cred)) != EOK)
			return(error);
	}
	if ((error = iofunc_ocb_attach(ctp, msg, NULL, &mq->attr, NULL)) != EOK)
		return(error);
	return(resmgr_mq_stat(ctp, (io_stat_t *)msg, _resmgr_ocb(ctp, &ctp->info)));
}

/*
 *  Handle client/libc "mq_unlink()".
 */
int resmgr_mq_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
mq_t				*mq;
struct _client_info	cred;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE || msg->connect.file_type != _FTYPE_MQUEUE)
		return(ENOSYS);
	if (ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE))
		return(ENOREMOTE);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if (*msg->connect.path == '\0')
		return(EINVAL);
	else if ((mq = lookup_mq(handle, msg->connect.path)) == NULL)
		return(ENOENT);
	if ((error = iofunc_unlink(ctp, msg, &mq->attr, &handle->attr, &cred)) != EOK)
		return(error);
	destroy_mq(handle, mq, 0);
	return(EOK);
}

/*
 *  Handle client/libc "mq_close()".
 */
int resmgr_mq_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
mq_t	*mq;
int		error;

	mq = OCB2MQ(ocb);
	if ((error = iofunc_close_ocb(ctp, ocb, ocb->attr)) != EOK)
		return(error);
	destroy_mq(NULL, mq, 0);
	return(EOK);
}

/*
 *  Handle client/libc "mq_getattr()".  This overrides normal stat()
 *  handling to place the mq_attr fields in the struct stat.
 */
int resmgr_mq_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	detail_mq(OCB2MQ(ocb), &msg->o, 0);
	return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)));
}

/*
 *  Handle filesystem/pathname "open()".  The directory can be opened
 *  for readdir but the queues cannot be accessed except for stat.
 */
int resmgr_dir_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
mq_t				*mq;
struct _client_info	cred;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE || msg->connect.file_type != _FTYPE_ANY)
		return(ENOSYS);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if (*msg->connect.path == '\0')
		mq = handle;
	else if (msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR))
		return(EACCES);
	else if ((mq = lookup_mq(handle, msg->connect.path)) == NULL)
		return((msg->connect.ioflag & O_CREAT) ? EACCES : ENOENT);
	if ((error = iofunc_open(ctp, msg, &mq->attr, NULL, &cred)) != EOK)
		return(error);
	if ((error = iofunc_ocb_attach(ctp, msg, NULL, &mq->attr, NULL)) != EOK)
		return(error);
	return(EOK);
}

/*
 *  Handle filesystem/pathname "unlink()".
 */
int resmgr_dir_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
mq_t				*mq;
struct _client_info	cred;
int					error;

	if (msg->connect.extra_type != _IO_CONNECT_EXTRA_NONE || msg->connect.file_type != _FTYPE_ANY)
		return(ENOSYS);
	if ((error = iofunc_client_info(ctp, msg->connect.ioflag, &cred)) != EOK)
		return(error);
	if (*msg->connect.path == '\0')
		return(EBUSY);
	else if ((mq = lookup_mq(handle, msg->connect.path)) == NULL)
		return(ENOENT);
	if ((error = iofunc_unlink(ctp, msg, &mq->attr, &handle->attr, &cred)) != EOK)
		return(error);
	destroy_mq(handle, mq, 0);
	return(EOK);
}

/*
 *  Handle filesystem/pathname "read()".
 */
int resmgr_dir_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
struct _client_info	cred;
int					error, xtra, nbytes;

	if ((error = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK)
		return(error);
	if (!S_ISDIR(ocb->attr->mode))
		return(ENOTDIR);
	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE)
		return(ENOSYS);
	xtra = (msg->i.xtype & _IO_XFLAG_DIR_EXTRA_HINT) != 0 && iofunc_client_info(ctp, ocb->ioflag, &cred) == EOK && iofunc_check_access(NULL, ocb->attr, S_IXUSR, &cred) == EOK;
	if ((error = list_mq(OCB2MQ(ocb), &ocb->offset, (struct dirent *)msg, min(msg->i.nbytes, ctp->msg_max_size - ctp->offset), xtra, &nbytes)) != EOK)
		return(error);
	ocb->attr->flags |= IOFUNC_ATTR_ATIME;
	_IO_SET_READ_NBYTES(ctp, nbytes);
	return(_RESMGR_PTR(ctp, msg, nbytes));
}

/*
 *  Handle filesystem/pathname "close()".
 */
int resmgr_dir_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
	return(resmgr_mq_close(ctp, reserved, ocb));
}

/*
 *  Handle filesystem/pathname "fstat()".
 */
int resmgr_dir_stat(resmgr_context_t *ctp, io_stat_t *msg, RESMGR_OCB_T *ocb)
{
	iofunc_time_update(ocb->attr);
	if (S_ISDIR(ocb->attr->mode)) {
		iofunc_stat(ctp, ocb->attr, &msg->o);
	}
	else {
		detail_mq(OCB2MQ(ocb), &msg->o, !0);
	}
	return(_RESMGR_PTR(ctp, &msg->o, sizeof(msg->o)));
}

/*
 *  Parse a string (recognising common suffixes) into a numeric size;
 *  values <= 0 are invalid (as this configures various buffer sizes).
 */
static int parsesize(const char *str)
{
char	*cp;
long	number;
char	suffix;

	number = strtol(str, &cp, 0);
	if (number <= 0)
		return(0);
	suffix = toupper(*cp);
	if (suffix == 'B')
		number *= 1, ++cp;
	else if (suffix == 'K')
		number <<= 10, ++cp;
	else if (suffix == 'M')
		number <<= 20, ++cp;
	return((*cp == '\0') ? number : 0);
}

/*
 *  Display message regarding a fatal initialisation error, and exit.
 */
static void fatal(const char *errmsg, ...)
{
extern char	*__progname;
va_list 	args;

	fprintf(stderr, "%s: ", __progname);
	va_start(args, errmsg);
	vfprintf(stderr, errmsg, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

/*
 *  SIGTERM handler - the server has been terminated (destroy all the
 *  queues and release the global kernel channels).
 */
static void terminate(int signo)
{
mq_t	*mq;

	while ((mq = MessageQueues.link) != NULL) {
		destroy_mq(&MessageQueues, mq, !0);
	}
	exit(EXIT_SUCCESS);
}

/*
 *  POSIX message queues server.
 */
int main(int argc, char *argv[])
{
void				*dp;
resmgr_context_t	*ctp;
char				*mountpt;
struct rlimit		fdlimit;
int					sig, opt, mxmsg, szmsg;
int					daemonize = 1;

	mountpt = MQ_DFLT_PREFIX, mxmsg = MQ_DFLT_NMSGS, szmsg = MQ_DFLT_SZMSG;
	while ((opt = getopt(argc, argv, ":m:s:N:d")) != -1 || optind < argc) {
		switch (opt) {
		case 'm':
			if (!(mxmsg = parsesize(optarg)))
				fatal("invalid capacity for '-%c'", optopt);
			break;
		case 's':
			if (!(szmsg = parsesize(optarg)))
				fatal("invalid size for '-%c'", optopt);
			break;
		case 'N':
			if (*(mountpt = optarg) != '/')
				fatal("specify absolute pathname for '-%c'", optopt);
			break;
		case 'd':
			daemonize = 0;
			break;
		case ':':
			fatal("missing argument for '-%c'", optopt);
			break;
		case '?':
			fatal("unknown option '-%c'", optopt);
			break;
		case -1:
			fatal("unknown operand '%s'", argv[optind]);
			break;
		}
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &MqConnectFuncs, _RESMGR_IO_NFUNCS, &MqIoFuncs);
	MqConnectFuncs.open = resmgr_mq_open, MqConnectFuncs.unlink = resmgr_mq_unlink;
	MqIoFuncs.close_ocb = resmgr_mq_close, MqIoFuncs.stat = resmgr_mq_stat;
	MqIoFuncs.lock_ocb = NULL, MqIoFuncs.unlock_ocb = NULL;
	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &DirConnectFuncs, _RESMGR_IO_NFUNCS, &DirIoFuncs);
	DirConnectFuncs.open = resmgr_dir_open, DirConnectFuncs.unlink = resmgr_dir_unlink;
	DirIoFuncs.read = resmgr_dir_read, DirIoFuncs.close_ocb = resmgr_dir_close, DirIoFuncs.stat = resmgr_dir_stat;
	DirIoFuncs.lock_ocb = NULL, DirIoFuncs.unlock_ocb = NULL;

	if ((dp = dispatch_create()) == NULL)
		fatal("unable to allocate resmgr context - %s", strerror(errno));
	memset(&ResmgrAttr, 0, sizeof(ResmgrAttr));
	ResmgrAttr.nparts_max = 1, ResmgrAttr.msg_max_size = sizeof(struct _io_connect) + _POSIX_PATH_MAX;

	memset(&MessageQueues, 0, sizeof(MessageQueues));
	iofunc_attr_init(&MessageQueues.attr, S_IFDIR | S_ISVTX | S_IPERMS, NULL, NULL);
	MessageQueues.attr.mount = &Mount;
	MessageQueues.attr.flags |= IOFUNC_ATTR_SYNTHETIC;
	++MessageQueues.attr.nlink;
	MessageQueues.mqattr.mq_maxmsg = mxmsg, MessageQueues.mqattr.mq_msgsize = szmsg;
	MessageQueues.name = mountpt;
	memset(&Mount, 0, sizeof(Mount));
	Mount.conf = IOFUNC_PC_CHOWN_RESTRICTED | IOFUNC_PC_NO_TRUNC;
	if ((MessageQueues.attr.rdev = resmgr_attach(dp, &ResmgrAttr, NULL, _FTYPE_MQUEUE, _RESMGR_FLAG_DIR | _RESMGR_FLAG_FTYPEONLY, &MqConnectFuncs, &MqIoFuncs, &MessageQueues)) == -1)
		fatal("unable to register as mqueue server - %s", strerror(errno));
	if ((DirPathname = resmgr_attach(dp, &ResmgrAttr, MessageQueues.name, _FTYPE_ANY, _RESMGR_FLAG_DIR, &DirConnectFuncs, &DirIoFuncs, &MessageQueues)) == -1)
		fatal("unable to register as mqueue pathname server - %s", strerror(errno));
	resmgr_devino(MessageQueues.attr.rdev, &Mount.dev, &MessageQueues.attr.inode);

	if ((MaxQueues = sysconf(_SC_MQ_OPEN_MAX)) == -1) {
		sysmgr_sysconf_set(0, _SC_MESSAGE_PASSING, _POSIX_MESSAGE_PASSING);
		sysmgr_sysconf_set(0, _SC_MQ_OPEN_MAX, MaxQueues = ((getrlimit(RLIMIT_NOFILE, &fdlimit) != -1) ? fdlimit.rlim_cur - STDERR_FILENO - 2 : _POSIX_MQ_OPEN_MAX));
		sysmgr_sysconf_set(0, _SC_MQ_PRIO_MAX, MQ_PRIO_MAX);
	}

	if(daemonize) {
		procmgr_daemon(EXIT_SUCCESS, PROCMGR_DAEMON_NODEVNULL);
	}
	for (sig = _SIGMIN; sig <= _SIGMAX ; ++sig)
		signal(sig, (sig == SIGTERM) ? terminate : SIG_IGN);

	ctp = resmgr_context_alloc(dp);
	while ((ctp = resmgr_block(ctp)) != NULL)
		resmgr_handler(ctp);

	return(EXIT_FAILURE);
}

__SRCVERSION("mq.c $Rev: 169543 $");
