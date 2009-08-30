/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to
 * QNX Software Systems before you may reproduce, modify or distribute this
 * software, or any work that includes all or part of this software.   Free
 * development licenses are available for evaluation and non-commercial
 * purposes.  For more information visit http://licensing.qnx.com or email
 * licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information. $
 */

/*
 *  "qkcp"  --  QuickCopy (fast but robust directory copy)
 *
 *  Uses FSYS_DCMD_DIRECT_IO to move data with DMA to/from user
 *  buffers, avoiding data copy and buffer cache residency during
 *  message passing.  Same ramifications of doing this: mainly that
 *  since the disk has been changed behind-the-back of the filesystem,
 *  the destination must be re-mounted afterwards (and thus must
 *  coordinate write-behind cache with this invalidation).  Another
 *  aspect of this utility is to perform everything synchronously
 *  (without necessarily having the filesystem mounted "commit=high",
 *  which would cause multiple disk overwrites when copying metadata),
 *  so a power-fail leaves the destination in a known state (allow
 *  checkpointing at the conclusion of each operation for a restart).
 *
 *  John Garvey, QNX Software Systems Ltd, Nov 2003.
 * 
 *  Sven Behnsen, Harman Becker, Dec 2003.
 *  added:
 *  - Copy single files and directory trees
 *  - Statistics
 *  - Checkpointing
 */

 
#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <pthread.h>
#include <share.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dcmd_all.h>
#include <sys/dcmd_blk.h>
#include <sys/fs_qnx6.h>
#include <sys/ftype.h>
#include <sys/iomsg.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <sys/trace.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <atomic.h>
#include <libgen.h>

#include "qkcp_if.h"

#define STDOUT_BUFLEN		512  	// 1/2k output buffer
#define STDERR_BUFLEN		256  	// 1/4k stderr buffer

#define BUFFER_SIZE  		65536	// default DMA buffer size
#define MIN_BUFFER_SIZE		4096 	// minimum DMA buffer size

#define T1_SIZE        		2222
#define T1_FILE        		4
#define PER            		30

/* Return codes
 *   0 - success
 *   1 - failure (default)
 *   2 - failure (graceful stop)
 *   3 - failure (checkpoint corrupt)
 *   4 - failure (read error)
 *   5 - failure (write error)
 *   6 - failure (corrupt file system)
 *   7 - failure (no space left on device)
 */

/* Failure code */
#define QK_FAIL_DEFAULT    	1
#define QK_FAIL_GRACESTOP	2
#define QK_FAIL_BADCKFILE	3
#define QK_FAIL_READING    	4
#define QK_FAIL_WRITING    	5
#define QK_FAIL_FSYSCORRUPT	6
#define QK_FAIL_NOSPACE    	7

// DirectIO DMA buffer structure
typedef struct {
	pthread_mutex_t msync;
	pthread_cond_t  cond;
	int             idx;
	size_t          size;
	int             sync;
	void            *vaddr;
	off64_t         paddr;
	int             condition;
	off_t           posn;
	int             sblks;
} buffer_t;

// Format of checkpoint file
struct qkcp_checkpoint 
{
	char offset[24];
	char srcfile[PATH_MAX];
	char n1;
	char dstfile[PATH_MAX];
	char n2;
	char cksum[10];
};

// Exchange data between 'READ' thread and 'WRITE' thread
struct thread_data
{	
	int src;
	blksize_t ssz;
	int dst;
	off_t nbytes;
	int error;
	off_t offset;
};

// Customisation/Optimisation for destination fsys
struct fsys_data {
	int			destination_fd;
	unsigned	sync_after_create;
	unsigned	sync_after_grow;
	unsigned	sync_after_metadata;
	unsigned	sync_after_copy;
	unsigned	suspend_sync;
};

int          	optv = 0;
int          	optV = 0;
int          	optW = 0;
int          	optr = 0;
int          	optS = 0;
int          	optc = 128;          // checkpoint size
char         	*optf = NULL;        // checkpoint file

int             ckfile_fd = -1;      // checkpoint file fd

char         	Source[PATH_MAX], Destination[PATH_MAX];
int          	Prefix;

uint64_t     	recoveryFileOffset;
uint64_t     	copyFileCount;
uint32_t     	copyFileCountPrinted;
uint32_t     	copyCheckpointCount;
uint32_t     	copyCheckpointCountF;
uint64_t     	copyFileTimeStart;
uint64_t     	copyFileTimeNow;
char         	recoveryFile[PATH_MAX];
char         	copyFile[PATH_MAX];
char         	copyDstFile[PATH_MAX];
double       	cpu_freq;

// Progress information 
uint64_t     	job_size;            // job size in 1k
uint64_t     	job_pos;             // job position in 1k
uint32_t     	job_files;           // job files
uint32_t     	job_pos_files;       // job position in files
uint64_t     	t_remaining;
int          	job_phase;
int          	optr2 = 0;           // duplicate optr for job size calculation

static int      progress_info = 0;

// Two DMA buffers to support DirectIO read/write simultaneously
static buffer_t dio_buffer, dio_buffer2;

static volatile unsigned dio_read_stop     = 0;
static volatile unsigned dio_read_finished = 0;

// Destination fsys configuration
static struct fsys_data fsys_target = { -1, O_SYNC, O_SYNC, O_SYNC, O_SYNC, 0 };

// Shared memory used to record progress information
enum shmem_info_phase {P_GET_JOB_SIZE = 1, P_COPY_DATA, P_COPY_DONE};
static struct qkcp_shmem *shm_info = NULL;
static char *shname = NULL;
static char sfile[128];

// User provided shared memory object and offset. QKCP may allocate
// DMA buffer from it for DirectIO operation.
static uint32_t shm_buff_off = 0;
static char    *shm_buff_name = NULL;

// DIRECTIO enable
static int direct_io_wr = !0;
static int direct_io_rd = !0;

// External 'encryption' DLL callouts
#define XFUNC(_f, _a...) ((x_dll != NULL) ? (*_f)(_a) : EOK)
static char *x_dll_name = NULL, *x_dll_args = "";
static void *x_dll = NULL;
static int (*x_dll_init)(const char *, const char *, char *), (*x_dll_fname)(const struct stat *, char *, int), (*x_dll_data)(off64_t, void *, int);

// error code during copy
static int qkcp_fail_code = QK_FAIL_DEFAULT;

// Prototypes
static int makeCheckpoint (char *checkpointfile, char *file, off_t offset, char *df);
static int readCheckpoint (char *checkpointfile);
static int form_path(char *fullpath, mode_t dir_mode);
static void rmtrailslash(char *filename);



static void VERBOSE(int level, char *msg, ...)
{
	va_list	args;

	if (optv >= level) {
		va_start(args, msg);
		vfprintf(stdout, msg, args);
		va_end(args);
		fprintf(stdout, "\n");
	}
}

static void WILDLY_VERBOSE(char *fmt, ...)
{
	va_list	args;
	va_start(args, fmt);
	vslogf(_SLOG_SETCODE(_SLOGC_TEST,0), _SLOG_INFO, fmt, args);
	va_end(args);
}

static void set_write_fail_code(int error)
{
	if(error == EBADFSYS)
		qkcp_fail_code = QK_FAIL_FSYSCORRUPT;
	else if(error == ENOSPC)
		qkcp_fail_code = QK_FAIL_NOSPACE;
	else 
		qkcp_fail_code = QK_FAIL_WRITING;

}

static int fatal(const char *errmsg, ...)
{
extern char	*__progname;
	va_list args;

	fprintf(stderr, "%s: ", __progname);
	va_start(args, errmsg);
	vfprintf(stderr, errmsg, args);
	va_end(args);
	fprintf(stderr, "\n");
	if(ckfile_fd != -1) //close checkpoint file
		close(ckfile_fd);
	if(shm_info != NULL) 
		shm_info->status = qkcp_fail_code;
	exit(qkcp_fail_code);
}

static int parsesize(char *str)
{
	char *cp;
	char  suffix;
	int   number;

	number = strtol(str, &cp, 0);
	if (cp == str || number <= 0)
		return(-1);
	suffix = toupper(*cp);
	if (suffix == 'B')
		number *= 1, ++cp;
	else if (suffix == 'K')
		number <<= 10, ++cp;
	else if (suffix == 'M')
		number <<= 20, ++cp;
	else if (suffix == 'G')
		number <<= 30, ++cp;
	return((*cp == '\0') ? number : -1);
}

static uint64_t rnd1k(uint64_t sz)
{
	return (sz+1023)/1024;
}

static void configure_target_fsys(char *dest, int checkpointing)
{
struct statvfs		stv;
struct _server_info	info;

	if ((fsys_target.destination_fd = open(dest, O_ACCMODE)) != -1) {
		if (fstatvfs(fsys_target.destination_fd, &stv) != -1) {
			if (!strcmp(stv.f_basetype, "qnx4")) {
				VERBOSE(2, "qkcp: using custom qnx4 settings");
				fsys_target.sync_after_create = fsys_target.sync_after_grow = fsys_target.sync_after_metadata = O_SYNC;
				fsys_target.sync_after_copy = 0;
				fsys_target.suspend_sync = 0;
				return;
			}
			if (!strcmp(stv.f_basetype, "qnx6")) {
				VERBOSE(2, "qkcp: using custom qnx6%s settings", checkpointing ? "/chkpt" : "");
				fsys_target.sync_after_create = fsys_target.sync_after_grow = fsys_target.sync_after_metadata = 0;
				if (checkpointing) {
					fsys_target.sync_after_copy = 0;
					fsys_target.suspend_sync = QNX6FS_SNAPSHOT_HOLD;
				}
				else {
					fsys_target.sync_after_copy = O_SYNC;
					fsys_target.suspend_sync = 0;
				}
				return;
			}
		}
		else if (ConnectServerInfo(getpid(), fsys_target.destination_fd, &info) != -1 && info.pid == SYSMGR_PID) {
			VERBOSE(2, "qkcp: using custom shmem settings");
			fsys_target.sync_after_create = fsys_target.sync_after_grow = fsys_target.sync_after_metadata = fsys_target.sync_after_copy = 0;
			fsys_target.suspend_sync = 0;
			++optS;
			return;
		}
	}
	VERBOSE(2, "qkcp: using default fsys settings");
}
static int suspend_fsys(int onoff)
{
struct fs_fileflags	flags;
int					error;

	if (fsys_target.destination_fd == -1 || !fsys_target.suspend_sync) {
		error = EOK;
	}
	else {
		memset(&flags, 0, sizeof(struct fs_fileflags));
		flags.mask[FS_FLAGS_FSYS] = fsys_target.suspend_sync;
		flags.bits[FS_FLAGS_FSYS] = onoff ? fsys_target.suspend_sync : 0;
		if ((error = devctl(fsys_target.destination_fd, DCMD_FSYS_FILE_FLAGS, &flags, sizeof(struct fs_fileflags), NULL)) == EOK && !onoff)
			error = (fsync(fsys_target.destination_fd) != -1) ? EOK : errno;
	}
	return(error);
}

// The function copysize() will check the disk space by
// ftruncate() and reserve the disk space for destination 
// file. So it is impossible that the disk space is not
// enough during writing. Thus, we don't need to call
// this function in copydata().
static void check_dspace(char *p)
{
	struct statvfs svfs;
   
	if ((fsys_target.destination_fd == -1 || fstatvfs(fsys_target.destination_fd, &svfs) == -1) && statvfs(p, &svfs)) {
		perror("statvfs()");
	} else {
		if (svfs.f_bfree/2 < 20000) {
			
			qkcp_fail_code = QK_FAIL_NOSPACE;
			fprintf(stderr, "No enough disk space!\n");
		}
	}

	return;
}

static int shmem_init(void)
{
	int fd;

	if (!shname) {
		return (0);
	}

	snprintf(sfile, 128, "%s%s", (shname[0] != '/') ? "/" : "", shname); 

	fd = shm_open(sfile, O_RDWR, 0);
	if (fd == -1) {
		perror("shm_open()");
		return (-1);
	}

	if(ftruncate(fd, sizeof(*shm_info)) == -1) {
		perror("ftruncate()");
		return (-1);
	}

	shm_info = mmap(0, sizeof(*shm_info), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	
	if (shm_info == MAP_FAILED) {
		perror("shmem - mmap");
		shm_info = NULL;
		return (-1);
	}
	memset(shm_info, 0, sizeof(*shm_info));
	return (0);
}

static void shmem_update()
{
	if (shm_info != NULL) {
		shm_info->phase          = job_phase;
		shm_info->percent        = job_pos*100/job_size;
		shm_info->job_size_done  = job_pos;
		shm_info->job_files_done = job_pos_files;
		shm_info->job_time_left  = t_remaining;
	}
}

static void progress_update(void)
{	
	static int lphase = 0, sc = 0;

	if (job_size == 0) {
		VERBOSE(4, "qkcp: job size is 0, no update");
		return;
	}

	t_remaining = (job_size-job_pos)/T1_SIZE + (job_files-job_pos_files)/T1_FILE;

	if ((sc++ > PER) || (job_phase != lphase)) {
		sc = 0;
		lphase = job_phase;
		VERBOSE(1, "qkcp - Phase %d %lld of %lld MB, %4d of %4d files, %lld s. left", 
			job_phase, job_pos/1024, job_size/1024, job_pos_files, job_files, t_remaining);
	}

	shmem_update();
}

static __inline__ int copymetadata(int fd, const struct stat *meta)
{
	int status, len;

	struct {
		struct _io_chown chown;
		struct _io_chmod chmod;
		struct _io_utime utime;
		struct _io_sync  sync;
	}msg;

	if(optS > 0) {
		return (EOK);
	}

	msg.chown.type = _IO_CHOWN;
	msg.chown.combine_len = _IO_COMBINE_FLAG | sizeof(msg.chown);
	msg.chown.uid = meta->st_uid, msg.chown.gid = meta->st_gid;
	msg.chmod.type = _IO_CHMOD;
	msg.chmod.combine_len = _IO_COMBINE_FLAG | sizeof(msg.chmod);
	msg.chmod.mode = meta->st_mode;
	msg.utime.type = _IO_UTIME;
	msg.utime.combine_len = _IO_COMBINE_FLAG | sizeof(msg.utime);
	msg.utime.cur_flag = 0;
	msg.utime.times.actime = meta->st_atime, msg.utime.times.modtime = meta->st_mtime;
	msg.sync.type = _IO_SYNC;
	msg.sync.combine_len = sizeof(msg.sync);
	msg.sync.flag = fsys_target.sync_after_metadata;
	len = sizeof(msg);

	if (!fsys_target.sync_after_metadata) {
		msg.utime.combine_len &= ~_IO_COMBINE_FLAG;
		len -= sizeof(msg.sync);
	}

	status = MsgSend(fd, &msg, len, NULL, 0);
	if (status == -1) {
		perror("copymetadata - MsgSend");
	}

	VERBOSE(4, "copymetadata fd=%d status=%d", fd, status);
  
	return((status == -1) ? errno : EOK);
}

static __inline__ int copysize(int fd, off_t oldsz, off_t newsz)
{
	uint64_t sz;

	if(optS > 0) {
		return(EOK);
	}

	if (oldsz != newsz) {
		devctl(fd, DCMD_FSYS_PREGROW_FILE, (sz = newsz, &sz), sizeof(sz), NULL);

		if (ftruncate(fd, newsz) == -1) {
			perror("copysize - ftruncate");
			return(errno);
		}
		
		if (fsys_target.sync_after_grow && fsync(fd)) {
			perror("copysize - fsync");
		}

		VERBOSE(4, "copysize - fd=%d oldsz=%lld newsz=%lld", fd, oldsz, newsz);
	}
	return(EOK);
}

static int unmake(char *path)
{
	int	fd;

	fd = _connect( _NTO_SIDE_CHANNEL, path, S_IFLNK, 
					O_ACCMODE | fsys_target.sync_after_create, SH_DENYNO, 
					_IO_CONNECT_UNLINK, !0, 0, _FTYPE_ANY,
					_IO_CONNECT_EXTRA_NONE, 0, NULL, 0, NULL, NULL);
	if (fd == -1) {
		return(errno);
	}
	
	ConnectDetach(fd);
	return(EOK);
}

static int makedir(char *dir, const struct stat *st)
{
	struct {
		struct _io_stat	stat;
	}msg;
	struct stat	actual;
	int	fd, error;

	if(optS > 0) {
		return (EOK);
	}

	if ((fd = _connect( _NTO_SIDE_CHANNEL, dir, 
						S_IFDIR | (st->st_mode & ~S_IFMT),
						O_ACCMODE | O_CREAT | fsys_target.sync_after_create, SH_DENYNO, 
						_IO_CONNECT_MKNOD, !0, 0, _FTYPE_ANY, 
						_IO_CONNECT_EXTRA_NONE, 0, NULL, 0,	
						NULL, NULL)) != -1) {
		ConnectDetach(fd);
	} else if (errno != EEXIST) {
		return(errno);
	}
	
	msg.stat.type = _IO_STAT;
	msg.stat.combine_len = sizeof(msg.stat);
	msg.stat.zero = 0;
 
	fd = _connect(0, dir, st->st_mode, O_ACCMODE, SH_DENYNO, 
				_IO_CONNECT_COMBINE, !0, 0, _FTYPE_ANY, 
				_IO_CONNECT_EXTRA_NONE, sizeof(msg), 
				&msg, sizeof(struct stat), &actual, NULL);

	VERBOSE(3, "makedir - fd=%d dir=%s", fd, dir);
 
	if (fd == -1)
		return(errno);
    
	if (!S_ISDIR(actual.st_mode))
		error = EEXIST;
	else
		error = copymetadata(fd, st);
		
	close(fd);
	return(error);
}

static int makefile(const char *file, const struct stat *st, struct stat *actual)
{
	struct {
		struct _io_devctl devctl;
		int               flags;
		struct _io_stat   stat;
	}msg;

	int result;

	if(optS > 0) {
		struct stat fstat;
		if(stat(file, &fstat) == -1) {
			result = shm_open(file, O_RDWR | O_CREAT | O_EXCL, 0777);
		} else {
			result = shm_open(file, O_RDWR, 0);
		}
		actual->st_blocksize = 4096;
		return result;
	}

	msg.devctl.type = _IO_DEVCTL;
	msg.devctl.combine_len = _IO_COMBINE_FLAG | (sizeof(msg.devctl) + sizeof(msg.flags));
	msg.devctl.dcmd = DCMD_ALL_SETFLAGS;
	msg.devctl.nbytes = sizeof(msg.flags);
	msg.devctl.zero = 0;
	msg.flags = O_LARGEFILE;
	msg.stat.type = _IO_STAT;
	msg.stat.combine_len = sizeof(msg.stat);
	msg.stat.zero = 0;
	result = _connect( 0, file, S_IFREG | (st->st_mode & ~S_IFMT), 
						O_WRONLY | O_CREAT | O_LARGEFILE | fsys_target.sync_after_create, SH_DENYNO, 
						_IO_CONNECT_COMBINE, !0, _IO_FLAG_WR, _FTYPE_ANY,
						_IO_CONNECT_EXTRA_NONE, sizeof(msg), 
						&msg, sizeof(struct stat), actual, NULL);

	VERBOSE(3, "makefile - file=%s result=%d", file, result);
	return(result);
}

static int makelink(char *link, const char *from, buffer_t *buffer)
{
	struct stat	st;
	int	fd, n;

	if(optS > 0) {
		return (EOK);
	}

	VERBOSE(3, "makelink - linking %s from %s\n", link, from);

	if ((n = readlink(from, buffer->vaddr, buffer->size)) == -1)
		return(errno);
		
	((char *)buffer->vaddr)[n] = '\0';
	fd = _connect( _NTO_SIDE_CHANNEL, link, S_IFLNK | S_IPERMS, 
					O_ACCMODE | O_CREAT | fsys_target.sync_after_create, SH_DENYNO, _IO_CONNECT_LINK,
					!0, 0, _FTYPE_ANY, _IO_CONNECT_EXTRA_SYMLINK, n + 1, 
					buffer->vaddr, 0, NULL, NULL);
	if (fd == -1) {
		if (errno != EEXIST) {
			return(errno);
		}
		
		if (lstat(link, &st) == -1 || !S_ISLNK(st.st_mode) || unmake(link) != EOK) {
			return(EEXIST);
		}
			
		if ((fd = _connect( _NTO_SIDE_CHANNEL, link, S_IFLNK | S_IPERMS, 
							O_ACCMODE | O_CREAT | fsys_target.sync_after_create, SH_DENYNO, 
							_IO_CONNECT_LINK, !0, 0, _FTYPE_ANY, 
							_IO_CONNECT_EXTRA_SYMLINK, n + 1, 
							buffer->vaddr, 0, NULL, NULL)) == -1) {
			return(errno);
		}
	}
	ConnectDetach(fd);
	return(EOK);
}

static void * start_read_thread(void * data)
{	
	struct thread_data *tdata = (struct thread_data *)data;
	int                 src = tdata->src;
	blksize_t           ssz = tdata->ssz;
	off_t               nbytes = tdata->nbytes;
	int                 last = 0;
	struct fs_directio  dio;
	off_t               posn = tdata->offset, remain = 0;
	int                 b = 0, sblks = 0;
	int                 error = EOK;
	int                 dio_enable = direct_io_rd;

	for (; !dio_read_stop && ((remain = nbytes - posn) > 0); posn += b) {
		buffer_t * buffer = (last == 0) ? &dio_buffer : &dio_buffer2;
		if((error = pthread_mutex_lock(&buffer->msync)) == EOK) {
			while( buffer->condition == 1 ) {
				pthread_cond_wait(&buffer->cond, &buffer->msync);
			}

			sblks = min((remain + ssz - 1) / ssz, (b = buffer->size / ssz));
			if(optW) {
				WILDLY_VERBOSE("DIO read - remain %d, posn %d, ssz %d, sblks %d, b %d \n", 
						(int)remain, (int)posn, (int)ssz, sblks, b);
			}

RETRY:
			if(!dio_enable) {
				if (read(src, buffer->vaddr, sblks < b ? remain : buffer->size) == -1) {
					error = errno;
					VERBOSE(1, "Normal read failed with errno=%d", error);
					qkcp_fail_code = QK_FAIL_READING;
					buffer->condition = 2;
   					pthread_cond_signal(&buffer->cond);
					pthread_mutex_unlock(&buffer->msync);
					break;
				}
			} else {
				dio.flags  = _IO_FLAG_RD | (buffer->sync ? O_SYNC : 0);
				dio.offset = posn;
				dio.nbytes = sblks*ssz;
				dio.paddr  = buffer->paddr;
				dio.vaddr  = buffer->vaddr;

				if ((error = devctl(src, DCMD_FSYS_DIRECT_IO, &dio, sizeof(dio), NULL)) != EOK) {
					VERBOSE(1, "DIO READ Error %d", error);
					if(error == ENOSYS || error == ENOTTY || error == EPERM || error == EINVAL) {
						dio_enable = 0, direct_io_rd = (error == EINVAL);
						error = EOK;
						goto RETRY;
					}
					qkcp_fail_code = QK_FAIL_READING;
					buffer->condition = 2;
					pthread_cond_signal(&buffer->cond);
					pthread_mutex_unlock(&buffer->msync);
					break;
				}
			}

			// A buffer is ready for writing, signal the writer thread
			buffer->posn = posn;
			buffer->sblks = sblks;
			buffer->condition = 1;
			pthread_cond_signal(&buffer->cond);

			if(optW) {
				WILDLY_VERBOSE("DIO read done - buffer %d size %d sync %d vaddr %p paddr %lld condition %d posn %d sblks %d ssz %d \n",
					buffer->idx, buffer->size, buffer->sync, buffer->vaddr, 
					buffer->paddr, buffer->condition, (int)buffer->posn, buffer->sblks, (int)ssz);
			}

			// Early bail
			if(sblks < b) {
				if(optW) {
					WILDLY_VERBOSE("read bail early\n");
				}
				pthread_mutex_unlock(&buffer->msync);
				break;
			}
			pthread_mutex_unlock(&buffer->msync);
			
		} else {
			perror("pthread_mutex_lock error!");
			tdata->error = error;
			break;
		}

		last = (last == 0) ? 1 : 0;
		b = sblks * ssz;
	}

	atomic_set_value(&dio_read_finished, 1);
	tdata->error = error;
	return(tdata);
}

static buffer_t * get_dio_buffer(void)
{	
	static int last = 0;

	if(dio_read_finished) {
		if(dio_buffer.condition) {
			return(&dio_buffer);
		} else if(dio_buffer2.condition) {
			return(&dio_buffer2);
		} else {
			atomic_clr_value(&dio_read_finished, 1);
			last = 0;
			return(NULL);
		}
	}

	if(last == 0) {
		last = 1;
		return(&dio_buffer);
	}
	else {
		last = 0;
		return(&dio_buffer2);
	}
}

static int copydata(int src, blksize_t ssz, int dst, blksize_t dsz, off_t nbytes, off_t offset)
{
	struct fs_directio dio;
	struct thread_data tdata;
	pthread_t          tid;
	buffer_t           *buffer = NULL;
	off_t              remain = 0;
	int                error = EOK, b = 0, dblks = 0;
	int                dio_enable = direct_io_wr;

	tdata.src    = src;
	tdata.ssz    = ssz;
	tdata.dst    = dst;
	tdata.nbytes = nbytes;
	tdata.error  = EOK;
	tdata.offset = offset;

	if(optS > 0) {
		if(offset != 0) {
			lseek(dst, offset, SEEK_SET);
		} else {
			ftruncate(dst, nbytes);
		}
		VERBOSE(1, "Copy %lld bytes from %lld to shared memory.", nbytes, offset);
	}

	if(pthread_create(&tid, NULL, start_read_thread, &tdata) != EOK)
		return(errno);

	while ((buffer = get_dio_buffer()) != NULL) {
		// Make sure we have synchronized with the reader thread
		if((error = pthread_mutex_lock(&buffer->msync)) == EOK) {

			while(buffer->condition == 0) {
				pthread_cond_wait(&buffer->cond, &buffer->msync);
			}

			remain = nbytes - buffer->posn;
			dblks = min(((remain + dsz - 1) / dsz), ((buffer->sblks * ssz + dsz - 1) / dsz));

			if((tdata.error == EOK) && (buffer->condition == 1)) {
				if(optW) {
					WILDLY_VERBOSE("DIO write - buffer %d size %d sync %d vaddr %p paddr %lld condition %d posn %d sblks %d ssz %d dblks %d dsz %d\n",
						buffer->idx, buffer->size, buffer->sync, buffer->vaddr, buffer->paddr, 
						buffer->condition, (int)buffer->posn, buffer->sblks, (int)ssz, dblks, (int)dsz);
				}
				b = remain < (dblks*dsz) ? remain : dblks*dsz;
				XFUNC(x_dll_data, buffer->posn, buffer->vaddr, b);

RETRY:
				if(!dio_enable) {
					if (write(dst, buffer->vaddr, b) == -1) {
						error = errno;
						VERBOSE(1, "Normal write failed with errno=%d", error);
						qkcp_fail_code = QK_FAIL_WRITING;
						atomic_add(&dio_read_stop, 1);
						buffer->condition = 0;
						pthread_cond_signal(&buffer->cond);
						pthread_mutex_unlock(&buffer->msync);
						pthread_join(tid, NULL);
						check_dspace(Destination);
						return(error);
					}
				} else {
					dio.flags  = _IO_FLAG_WR | (buffer->sync ? O_SYNC : 0);
					dio.offset = buffer->posn;
					dio.nbytes = dblks * dsz;
					dio.paddr  = buffer->paddr;
					dio.vaddr  = buffer->vaddr;
					if ((error = devctl(dst, DCMD_FSYS_DIRECT_IO, &dio, sizeof(dio), NULL)) != EOK) {
						VERBOSE(3, "DIO WRITE Error %d", error);
						if(error == ENOSYS || error == ENOTTY || error == EPERM || error == EINVAL) {
							dio_enable = 0, direct_io_wr = (error == EINVAL);
							error = EOK;
							goto RETRY;
						}
						qkcp_fail_code = QK_FAIL_WRITING;
						atomic_add(&dio_read_stop, 1);
						buffer->condition = 0;
						pthread_cond_signal(&buffer->cond);
						pthread_mutex_unlock(&buffer->msync);
						pthread_join(tid, NULL);
						return(error);
					}
				}

				copyFileCount += b;
				
				if (optV > 0) {
					copyFileCountPrinted += b;
					if ((copyFileCountPrinted > 1024*1024) || (copyFileCount >= nbytes)) {					    
						int kbsec;
						copyFileCountPrinted = 0;
						copyFileTimeNow = ClockCycles();
						kbsec = (buffer->posn-offset)/(( copyFileTimeNow - copyFileTimeStart ) / cpu_freq);
						fprintf (stdout, "%6.2f%% (%lld/%lld kbytes) %d kb/s           \r", 
							((float)copyFileCount/(float)nbytes)*100,
							copyFileCount/1024, (uint64_t)nbytes/1024, kbsec);
						fflush (stdout);
					}
				}

				if(optf != NULL) {
					copyCheckpointCount += b;
					if ((copyCheckpointCount + copyCheckpointCountF) >= optc*1048576) {
						copyCheckpointCount = copyCheckpointCountF = 0;
						if(copyFileCount > nbytes)
							copyFileCount = nbytes; 
						makeCheckpoint (optf, copyFile, copyFileCount, copyDstFile);
					}
				}
				if(progress_info > 0) {
					job_pos += rnd1k(b);
					progress_update();
				}
			}

			buffer->condition = 0;
			pthread_cond_signal(&buffer->cond);
			pthread_mutex_unlock(&buffer->msync);
		} else {
			atomic_add(&dio_read_stop, 1);
			buffer->condition = 0;
			pthread_cond_signal(&buffer->cond);
			pthread_join(tid, NULL);
			return(error);
		}
	}

	pthread_join(tid, NULL);
	return(tdata.error);
}

static int copy(const char *from, const struct stat *existing, char *to, off_t offset)
{
	struct stat replace;
	int error = 0;
	int src, dst;

	if (XFUNC(x_dll_fname, existing, to, PATH_MAX) != EOK) {
		VERBOSE(4, "copy - ignoring %s", from);
	} else if (offset == 0) {
		if((optv > 0) || (optV > 0)) {
			fprintf(stdout, "qkcp: copy %s to %s\n", from, to);
		}
		
		if ((src = open(from, O_RDONLY)) != -1) {
			if ((dst = makefile(to, existing, &replace)) != -1) {
				if ((error = copysize(dst, replace.st_size, existing->st_size)) == EOK) {
					if ((error = copymetadata(dst, existing)) == EOK) {
						strcpy (copyFile,from);
						strcpy (copyDstFile,to);
						copyFileCount = 0;
						copyFileTimeNow = 0;
						copyFileCountPrinted = 0;
						copyFileTimeStart = ClockCycles();
						if (existing->st_size > 0) {
							error = copydata(src, existing->st_blocksize, dst, replace.st_blocksize, existing->st_size, 0);
							if (error)
								VERBOSE(1, "copy - copydata error=%d", error);
							else if (fsys_target.sync_after_copy)
								fsync(dst);
						}
						if (copyFileTimeNow != 0)
							fprintf (stdout, "\n");
					} else {
						VERBOSE(1, "copy - copymetadata error");
						qkcp_fail_code = QK_FAIL_WRITING;
					}
				} else {
					VERBOSE(1, "copy - copysize error");
					set_write_fail_code(error);
				}
				close(dst);
			} else {
				error = errno;
				VERBOSE(1, "copy - makefile error=%d", error);
				set_write_fail_code(error);
			}
			close(src);
		} else {
			error = errno;
			VERBOSE(1, "copy - open error=%d", error);
			qkcp_fail_code = (error == EBADFSYS ? QK_FAIL_FSYSCORRUPT: QK_FAIL_READING);
		}

		if (!error) {
			if(progress_info > 0) {
				job_pos_files++;
				progress_update();
			}
			copyCheckpointCountF += 300000; // 1 file is "300k worth" for checkpoint purpose
		}
	} else {  
		VERBOSE(1, "qkcp: recovery %s at %d", from, (int)offset);
		if ((src = open(from, O_RDONLY)) != -1) {  
			if ((dst = makefile(to, existing, &replace)) != -1) {   
				strcpy (copyFile,from);
				strcpy (copyDstFile, to);
				copyFileCount = offset;
				copyFileCountPrinted = 0;
				copyFileTimeNow = 0;
				copyFileTimeStart = ClockCycles();
				if (existing->st_size > 0) {
					error = copydata(src, existing->st_blocksize, dst, replace.st_blocksize, existing->st_size, offset);
					if (error)
						VERBOSE(1, "copy - copydata error=%d", error);
					else if (fsys_target.sync_after_copy)
						fsync(dst);
				}
				if (copyFileTimeNow != 0)
					fprintf (stdout, "\n");
				if (!error) {
					if(progress_info > 0) {
						job_pos_files++;
						progress_update();
					}
				}
			} else {
				error = errno;
				VERBOSE(1, "copy - makefile: %d", error);
				set_write_fail_code(error);
			}
			close(src);
		} else {
			error = errno;
			VERBOSE(1, "copy - open error=%d", error);
			qkcp_fail_code = (error == EBADFSYS ? QK_FAIL_FSYSCORRUPT: QK_FAIL_READING);
		}
	}
	return(error);
}

static int preparebuffer(int sz)
{
	int fd, bufsz = sz<<1;
	void *vaddr;
	off64_t paddr;
	struct stat st;

	memset(&dio_buffer, 0, sizeof(buffer_t));
	memset(&dio_buffer2, 0, sizeof(buffer_t));

	if(shm_buff_name == NULL) {
		shm_buff_name = getenv("QKCP_SHM_BUFFER_NAME");
	}
	if(shm_buff_off == 0) {
		char *offset = getenv("QKCP_SHM_BUFFER_OFFSET");
		if(offset != NULL) {
			shm_buff_off = atoh(offset);
		}
	}
	if(shm_buff_name == NULL) {
		if(shm_buff_off != 0) {
			fatal("Shared buffer name not provided");
		}
		return (0);
	}

	VERBOSE(2, "Using %s at 0x%x", shm_buff_name, shm_buff_off);
	if(stat(shm_buff_name, &st) == -1) {
		perror("stat");
		return (-1);
	}
	if(st.st_size < bufsz) {
		fprintf(stderr, "Shared buffer size (%lld) too small! \n", st.st_size);
		return (-1);
	}
	fd = shm_open(shm_buff_name, O_RDWR, 0);
	if (fd == -1) {
		perror("shm_open()");
		return (-1);
	}

	vaddr = mmap(NULL, bufsz, PROT_READ|PROT_WRITE|PROT_NOCACHE, MAP_SHARED, fd, shm_buff_off);
	if (vaddr != MAP_FAILED) {
		size_t config_len;
		if (mem_offset64(vaddr, NOFD, bufsz, &paddr, &config_len) != -1) {
			VERBOSE(1, "Memory needed 0x%x, available 0x%x, vaddr 0x%x, paddr 0x%llx ", 
				sz<<1, config_len, (uint32_t)vaddr, paddr);
			dio_buffer.vaddr = vaddr;
			dio_buffer.paddr = paddr;
			dio_buffer2.vaddr = vaddr + sz;
			dio_buffer2.paddr = paddr + sz;
			return (0); 
		}
		perror("mem_offset64()");
	} else {
		perror("mmap()");
	}
	return (-1);
}

static int setupbuffer(buffer_t *buffer, int sz, int idx)
{
	int saved;

	buffer->sync = 1;
	pthread_mutex_init(&buffer->msync, NULL);
	pthread_cond_init(&buffer->cond, NULL);
	buffer->condition = 0;
	buffer->idx = idx;
	buffer->size = sz;

	if(buffer->vaddr != NULL) {
		return (EOK);
	}
	
	buffer->vaddr = mmap(NULL, buffer->size, 
				PROT_READ | PROT_WRITE | PROT_NOCACHE,
				MAP_ANON | MAP_PHYS | MAP_PRIVATE | MAP_NOX64K, 
				NOFD, 0);
	if (buffer->vaddr != MAP_FAILED) {
		if (mem_offset64(buffer->vaddr, NOFD, buffer->size, &buffer->paddr, NULL) != -1) {
			return(EOK);
		}
		saved = errno;
		munmap(buffer->vaddr, buffer->size);
		errno = saved;
	}
	return(errno);
}

static int walker_size(const char *name, const struct stat *st, int type, struct FTW *ftw)
{
	char  target[PATH_MAX];
	off_t offset = 0;
   
	if (optr2 > 0) {
		if (strcmp(recoveryFile, name) != 0) {
			VERBOSE(4, "walker_size - recovery skipping %s", name);
		} else {
			offset = recoveryFileOffset;
			job_pos_files = job_files;
			job_size += rnd1k(offset);
			job_pos = job_size;
			optr2 = 0;
		}
	}

	switch (type) {
	case FTW_F:	
		snprintf(target, PATH_MAX, "%s%s", Destination, &name[Prefix]);
		if (S_ISREG(st->st_mode) && XFUNC(x_dll_fname, st, target, PATH_MAX) == EOK) {
			job_size += rnd1k(st->st_size - offset);
			job_files++;
			if (shm_info) {
				shm_info->job_size  = job_size;
				shm_info->job_files = job_files;
				shmem_update();
			}
		}
		break;
	case FTW_D:	
		job_files++;
		break;
	default:
		break;
	}
	
	return(0);
}

static int walker(const char *name, const struct stat *st, int type, struct FTW *ftw)
{
	int   error;
	char  target[PATH_MAX];
	off_t offset = 0;

	if (errno) {
		VERBOSE(3, "walker - old errno = %d", errno);
		errno = 0;
	}

	snprintf(target, PATH_MAX, "%s%s", Destination, &name[Prefix]);

	if (optr > 0) {
		if (strcmp(recoveryFile, name) != 0) {
			VERBOSE(1, "qkcp: recovery skipping %s\n", name);
			return (0);
		} else {
			optr = 0;
			offset = recoveryFileOffset;
		}
	}

	VERBOSE(4, "walker - %s", name);

	switch (type) {
	case FTW_F:
		if (S_ISREG(st->st_mode) && (error = copy(name, st, target, offset)) != EOK) {
			VERBOSE(1, "walker - copy error %d", error);
			return(error);
		}
		break;
	case FTW_D:
		if ((error = makedir(target, st)) != EOK) {
			VERBOSE(1, "walker - makedir error %d", error);
			set_write_fail_code(error);
			return(error);
		}
		job_pos_files++;
		break;
	case FTW_SL:
		if ((error = makelink(target, name, &dio_buffer)) != EOK) {
			VERBOSE(1, "walker - makelink error %d", error);
			set_write_fail_code(error);
			return(error);
		}
		break;
	default:
		VERBOSE(1, "walker - invalid type %d", type);
		break;
	}

	if (errno) {
		VERBOSE(3, "walker - errno = %d", errno);
		errno = 0;
	}

	return(0);
}

/*
 *  Signal handler thread - graceful stop 
 */
static sigset_t signal_set;
static pthread_mutex_t sig_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* sig_handler( void* arg )
{
	int sig;

	while(1) {
		// Wait for any and all signals
		sigfillset(&signal_set);
		sigwait(&signal_set, &sig);

		if(optf == NULL) {
			fatal("qkcp interrupted by signal");
		}

		// When we get this far, we've caught a signal
		switch(sig) {
		case SIGQUIT:
		case SIGTERM:
		case SIGINT:
		case SIGABRT:
		case SIGALRM:
		case SIGHUP:
			pthread_mutex_lock(&sig_mutex);
			qkcp_fail_code = QK_FAIL_GRACESTOP;
			fatal("qkcp stopped gracefully");
			pthread_mutex_unlock(&sig_mutex);
			break;
		default:
			break;
		}
	}
	return (NULL);
}

/*
 * Main function
 */
int main(int argc, char *argv[])
{
	struct stat  st;
	struct stat  d_st;
	int          result;
	int          c;
	char         target[PATH_MAX];
	int          buffersize = BUFFER_SIZE;
	pthread_t    sig_thread;
	char		*cp;
   
	setvbuf(stdout,NULL,_IOLBF,STDOUT_BUFLEN);
	setvbuf(stderr,NULL,_IOLBF,STDERR_BUFLEN);

	cpu_freq = SYSPAGE_ENTRY( qtime )->cycles_per_sec;
	cpu_freq /= 1000;

	// Create signal handling thread 
	sigemptyset(&signal_set);
	sigaddset(&signal_set, SIGTERM);
	sigaddset(&signal_set, SIGINT);
	sigaddset(&signal_set, SIGQUIT);
	sigaddset(&signal_set, SIGABRT);
	sigaddset(&signal_set, SIGHUP);
	sigaddset(&signal_set, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &signal_set, NULL);
	pthread_create(&sig_thread, NULL, sig_handler, NULL);


	while( ( c = getopt( argc, argv, "b:vVWrsc:f:h:S:O:X:" ) ) !=-1 ) {
		switch( c )	{
		// The maximal amount of sectors required at once is defined by 
		// the maximum sector count supported in the filesystem->driver API
		// (128 sectors) and any HW consideration for DMA (some PCI 
		// controllers for instance have only 16-bit auto-accumulators and
		// hence a 64k limit). In general, the buffer size should stick to
		// using no more than 64k at one time.
		case 'b':
				if ((buffersize = parsesize(optarg)) == -1 || buffersize < MIN_BUFFER_SIZE) {
					perror("Invalid buffer size, use default size(64K)!");
					buffersize = BUFFER_SIZE;
				}
				break;
		case 'v':	optv++;
				break;
		case 'V':	optV++;
				break;
		case 'W':	optW++;
				break;
		case 'r':	optr++;
				break;    
		case 'S':	shm_buff_name = optarg;
				if(shm_buff_name == NULL) {
					fatal("Specify <shared buffer name>");
				}
				break;
		case 'O':	shm_buff_off = atoh(optarg);
				break;                                     
		case 's':	optS++;
				break;                                         
		case 'f':	optf = optarg;
				break;
		case 'h':	shname = optarg;
				break;
		case 'c':	optc = atoi (optarg);
				break;
		case 'X':	x_dll_name = optarg;
				break;
		default :	break;               
		}
	}

	if (optind != argc-2) {
		fatal("Specify <source file or directory> and <destination directory>");
	}

	if (x_dll_name != NULL) {
		x_dll_name = (strchr(x_dll_name, ':') != NULL && (cp = strdup(x_dll_name)) != NULL) ? cp : x_dll_name;
		if ((cp = strchr(x_dll_name, ':')) != NULL)
			*cp++ = '\0', x_dll_args = cp;
		if ((x_dll = dlopen(x_dll_name, RTLD_NOW)) == NULL)
			fatal("External DLL - %s", dlerror());
		if ((x_dll_init = dlsym(x_dll, "qkcp_init")) == NULL || (x_dll_fname = dlsym(x_dll, "qkcp_filename")) == NULL || (x_dll_data = dlsym(x_dll, "qkcp_filedata")) == NULL)
			fatal("External DLL - %s", dlerror());
	}

	if (optf != NULL && (optr == 0)) {
		VERBOSE(3, "qkcp - Make checkpoint every %dMB", optc);
		if ((ckfile_fd = open(optf, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1) {
			if(errno == EBADFSYS) {
				qkcp_fail_code = QK_FAIL_FSYSCORRUPT;
			}
			fatal("Error creating checkpoint file");
		}
	}

	if (optr > 0) {
		if ((!optf) || (strlen(optf) == 0)) {
			fatal("Checkpoint file not specified");
		}
		
		result = readCheckpoint (optf);
		if (result == 0) {
			VERBOSE(1, "Recovery OK");
		} else if (result == -1) {
			VERBOSE(1, "Recovery skipped");
			optr = 0;
		} else if (result == -2) {
			fatal("Recovery failed due to I/O error");
		} else {
			qkcp_fail_code = QK_FAIL_BADCKFILE;
			fatal("Recovery failed due to corrupted checkpoint");
		}
	}

	if(preparebuffer(buffersize) != 0) {
		fatal("Unable to set up predefined DMA buffer");
	}

	if ((result = setupbuffer(&dio_buffer, buffersize, 0)) != EOK) {
		fatal("Unable to set up DMA buffer - %s", strerror(result));
	}

	if ((result = setupbuffer(&dio_buffer2, buffersize, 1)) != EOK)	{
		fatal("Unable to set up DMA buffer2 - %s", strerror(result));
	}

	if (_fullpath(Source, argv[argc-2], sizeof(Source)) == NULL) {
		fatal("Invalid source path - %s", strerror(errno));
	} 
     
	if(lstat(Source, &st) == -1) {
		fatal("lstat(Source) failed - errno %d", errno);
	}

	//FIXME: hardcode for special source which block size is 0 
	if(st.st_blocksize == 0) {
		st.st_blocksize = 4096;
	}

	if (_fullpath(Destination, argv[argc-1], sizeof(Destination)) == NULL) {
		mode_t dir_mode = S_ISDIR(st.st_mode) ? st.st_mode : 0777;
		strcpy(Destination, argv[argc-1]); 
		rmtrailslash(Destination);
		if ((mkdir(Destination, dir_mode))==-1) {
			if ((result = form_path(Destination, dir_mode)) != EOK) {
				set_write_fail_code(errno);
				fatal("mkdir: %s - %s", Destination, strerror(result));
			}
			else if ( mkdir(Destination, dir_mode) == -1 && errno != EEXIST) {
				set_write_fail_code(errno);
				fatal("mkdir: %s - %s", Destination, strerror(errno));
			}
		}
	}

	if(lstat(Destination, &d_st) == -1 || !S_ISDIR(d_st.st_mode)) {
		fatal("Invalid destination path - %s", strerror(ENOTDIR));
	}

	Prefix = strlen(Source);

	if(shmem_init() == -1) {
		fatal("Failed to init shared memory object for progress indication");
	}

	// Initialize the progress information
	if((optv > 0) || (shm_info != NULL)) { 
		progress_info = 1;

		job_phase     = P_GET_JOB_SIZE;
		job_size      = 0; 
		job_files     = 0;
		job_pos       = 0;  
		job_pos_files = 0; 
		t_remaining   = 0;
	}

	// Optimise settings for destination fsys
	configure_target_fsys(Destination, optf != NULL);
	suspend_fsys(!0);

	// Initialise the optional '-X' DLL
	if ((result = XFUNC(x_dll_init, Source, Destination, x_dll_args)) != EOK) {
		fatal("External DLL - %s", strerror(result));
	}

	/*
	 * Copy directory-tree to directory *
	 * Copy file to file
	 * Copy file to directory
	 * Link file to file*
	 * Link file to directory*
	 * Link directory to Directory*
	 */
	if (S_ISDIR(st.st_mode)) {
		if(progress_info > 0) {
			// Get job size
			optr2 = optr;
			errno = 0;
			if ((result = nftw(Source, walker_size, _POSIX_OPEN_MAX - 2, FTW_PHYS | FTW_MOUNT)) != 0) {
				VERBOSE(1, "nftw - walker_size error=%d errno=%d", result, errno);
			}
			if (shm_info) {
				shm_info->job_time  = job_size/T1_SIZE + job_files/T1_FILE;
			}
			job_phase = P_COPY_DATA;
			progress_update();
			VERBOSE(1, "qkcp: job estimate %lld s\n", t_remaining);
		}

		// Copy directory tree
		if ((result = nftw(Source, walker, _POSIX_OPEN_MAX - 2, FTW_PHYS | FTW_MOUNT)) != 0) {
			fatal("ntfw - walker error");
		}

		if(optf != NULL) {
			makeCheckpoint (optf, copyFile, copyFileCount, copyDstFile);
		}

		if(progress_info > 0) {
			job_phase = P_COPY_DONE;
			progress_update();
		}
		
	} else if (S_ISLNK(st.st_mode)) {
		// Make Link
		if (S_ISDIR (d_st.st_mode))
			snprintf(target, PATH_MAX, "%s%s", Destination, basename(Source));
		else
			strcpy (target, Destination);
		makelink(target, Source, &dio_buffer);
	} else {
		if (S_ISDIR (d_st.st_mode))
			snprintf(target, PATH_MAX, "%s/%s", Destination, basename(Source));
		else
			strcpy (target, Destination);   

		if(progress_info > 0) {
			// Get job size 
			job_size  = rnd1k(st.st_size);
			job_files = 1;
			job_pos_files = 0;
			if (shm_info) {
				shm_info->job_size  = job_size;
				shm_info->job_files = job_files;
				shm_info->job_time  = job_size/T1_SIZE + job_files/T1_FILE;
			}
		}
	
		// Copy File
		if (optr > 0) {
			if (strcmp (Source, recoveryFile) == 0)	{
				if(progress_info > 0) {
					job_pos = rnd1k(recoveryFileOffset);
					job_phase = P_COPY_DATA;
					progress_update();
					VERBOSE(1, "qkcp: job estimate %lld s\n", t_remaining);
				}
				copyFileTimeNow = ClockCycles();
				if((result = copy(Source, &st, target, recoveryFileOffset)) != 0) {
					fatal("copy error %d", result); 
				}
			}
		} else {
			if(progress_info > 0) {
				job_pos = 0;
				job_phase = P_COPY_DATA;
				progress_update();
				VERBOSE(1, "qkcp: job estimate %lld s\n", t_remaining);
			}
			copyFileTimeStart = ClockCycles();
			if((result = copy(Source, &st, target, 0)) != 0) {
				fatal("copy error %d", result); 
			}
		}
		if(optf != NULL) { 
			makeCheckpoint (optf, copyFile, copyFileCount, copyDstFile);
		}
		if(progress_info > 0) {
			job_phase = P_COPY_DONE;
			progress_update();
		}
	}

	if(ckfile_fd != -1) //close checkpoint file
		close(ckfile_fd);
	suspend_fsys(0);

	return(EXIT_SUCCESS);
}

static unsigned calculate_cksum(unsigned char *buf, int len)
{
	unsigned d, cksum = 0x55aa;
   
	while(len--) {
		d = cksum & 0x1f;
		cksum <<= 5;
		cksum = (cksum ^ 0xb2c5 ^ *buf ^ d); 
		buf++;
	}
	return cksum;
}

static int makeCheckpoint (char *checkpointfile, char *file, off_t offset, char *dstFile)
{
	struct qkcp_checkpoint qc;

	memset(&qc, ' ', sizeof(qc));
	strcpy(qc.srcfile, file);
	strcpy(qc.dstfile, dstFile);
	qc.n1 = qc.n2 = '\n';
	sprintf(qc.offset, "%lld", (uint64_t)offset);
	sprintf(qc.cksum, "%8x", calculate_cksum((unsigned char *)&qc, sizeof(qc)-sizeof(qc.cksum)));
	pthread_mutex_lock(&sig_mutex);
	if (suspend_fsys(0) != EOK) {
		fatal("Error synchronising checkpoint state");
	}
	if (sizeof(qc) != pwrite (ckfile_fd, &qc, sizeof(qc), 0)) {
		fatal("Error writing checkpoint file");
	}
	fsync(ckfile_fd);
	suspend_fsys(!0);
	pthread_mutex_unlock(&sig_mutex);

	return (0);
}

static int readCheckpoint (char *checkpointfile)
{
	int n, sz;
	unsigned cksum;
	struct stat stbuf;
	struct qkcp_checkpoint qc;
  
	if (access(checkpointfile, F_OK)) {
		VERBOSE(1, "Checkpoint file not found");
		if ((ckfile_fd = open(checkpointfile, O_RDWR | O_CREAT | O_EXCL, 0600)) == -1) {
			if(errno == EBADFSYS) {
				qkcp_fail_code = QK_FAIL_FSYSCORRUPT;
			}
			VERBOSE(1, "Error opening checkpoint file");
			return (-2);
		}
		return (-1);
	}
 
	ckfile_fd = open (checkpointfile, O_RDWR);
	if (ckfile_fd == -1) {
		if(errno == EBADFSYS) {
			qkcp_fail_code = QK_FAIL_FSYSCORRUPT;
		}
		VERBOSE(1, "Error opening checkpoint file");
		return (-2);
	}

	sz = read (ckfile_fd, &qc, sizeof(qc));
	if(sz == 0) {
		VERBOSE(1, "Empty checkpoint file");
		return (-1);
	}
	if(sz != sizeof(qc)) {
		VERBOSE(1, "Error checkpoint file size");
		return (-3);
	}

	sscanf(qc.cksum, "%x", &cksum);
	if (cksum != calculate_cksum((unsigned char *)&qc, sizeof(qc)-sizeof(qc.cksum))) {
		VERBOSE(1, "Checkpoint file corrupt - cksum");
		return (-3);
	} 
	strcpy(recoveryFile, qc.srcfile);
	n = sscanf(qc.offset, "%lld", &recoveryFileOffset);
	if (n != 1) {
		VERBOSE(1, "Checkpoint file corrupt");
		return (-3);
	}

	if (stat(qc.dstfile, &stbuf) == -1) {
		VERBOSE(1, "stat - file %s errno = %d", qc.dstfile, errno);
	}

	if (stbuf.st_size < recoveryFileOffset) {
		VERBOSE(1, "Checkpoint file invalid");
		return (-3);
	}

	return (0);
}

#define SKIP_TO_SLASH(p)	{for(;*p && *p!='/';p++);}
static int form_path(char *fullpath, mode_t dir_mode)
{
	char *ptr = fullpath;

	if (fullpath[0] == '/' && fullpath[1] == '/') {
		// starts with '//'
		ptr += 2;

	} else if (fullpath[0] == '/') ptr++;

	SKIP_TO_SLASH(ptr);
	
	for (;*ptr;) {
		*ptr = (char)0x00;
		if (mkdir(fullpath, dir_mode)==-1) {
			if (errno!=EEXIST && errno != EPERM && errno != EACCES && errno != ENXIO) {
				*ptr = '/';
				return (errno);
			}
		}
		*ptr = '/';
		ptr++;
		SKIP_TO_SLASH(ptr);
	}
	return (EOK);
}

static void rmtrailslash(char *filename)
{
	int fnameend = strlen(filename) - 1;

	if (filename[fnameend] == '/' || filename[fnameend] == '\\')
		filename[fnameend] = '\0';
}

__SRCVERSION("qkcp.c $Rev: 210967 $");
