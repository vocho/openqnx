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




#define _FILE_OFFSET_BITS       32
#define _IOFUNC_OFFSET_BITS     64
#define _LARGEFILE64_SOURCE     1
#include <share.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <share.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/iomsg.h>
#include <sys/dcmd_all.h>

#define BUF_SIZE		4096

#include "connect.h"

#define D_DEFAULT_FLAGS	(D_FLAG_FILTER)

struct _fd_block {
	int     count, index;
    int     *fds;
};

#define DATA_COUNT	50					/* The bigger the better */
typedef struct _cursor {
	struct _cursor *next;
	int				count;
	char 		   *data[DATA_COUNT];
} cursor_t;


struct _dir {
	struct _fd_block	dd_fd_block;	/* Storage for multiple fd's */
	struct _cursor	   *dd_cursor;		/* Unique directory stored names */
	unsigned			dd_flags;		/* Flags for stuff */
	int					dd_pos;			/* For telldir/seekdir */
	int					dd_loc;
	int					dd_size;
	char				*dd_buf;
	char				*dd_path;
};

static pthread_mutex_t		readdir_mutex = PTHREAD_MUTEX_INITIALIZER;

static void readdir_cleanup(void *data) {
	pthread_mutex_t	*mutex = data;

	pthread_mutex_unlock(mutex);
}

static int compare( const void *p1, const void *p2 ) {
    const char *p1c = (const char *) p1;
    const char **p2c = (const char **) p2;
    return( strcmp( p1c, *p2c ) );
}

/*
 Returns 0 if the item with the name did not
already exist in the cursor list, adds the item 
to the structure.
 Returns 1 if the item with the name did exist
in the list, doesn't add the item to the structure
*/
static int find_with_insert(cursor_t **chead, char *name) {
	cursor_t *ctarget;
	int		  indx;

	if (!(ctarget = *chead)) {
		ctarget = calloc(sizeof(cursor_t), 1);
		if (!(*chead = ctarget)) {
			errno = ENOMEM;
			return 0;		//XXX: We want the item reported
		}
		ctarget->data[0] = strdup(name);
		if (ctarget->data[0] == NULL) {
			free(ctarget);
			errno = ENOMEM;
			return 0;
		}
		ctarget->count++;
		return 0;
	}

	/* Search this target for the item */
	do {
		if (bsearch(name, ctarget->data, ctarget->count, sizeof(char *), compare)) {
			return 1;
		}
	} while (ctarget->next && (ctarget = ctarget->next));

	/* This item was not found, create a new block or insert internally */
	if (ctarget->count >= DATA_COUNT) {
		return find_with_insert(&ctarget->next, name);
	}

	//TODO: Fix this later to make it faster, use the bs results from above 
	for (indx = 0; indx < ctarget->count; indx++) {
		if (strcmp(name, ctarget->data[indx]) < 0) {
			break;
		}
	}
	memmove(&ctarget->data[indx+1], &ctarget->data[indx], (ctarget->count - indx) * sizeof(char *)); 
	ctarget->data[indx] = strdup(name);
	if (ctarget->data[indx] == NULL) {
		errno = ENOMEM;
		return 0;
	}
	ctarget->count++;
	return 0;
}

static void free_cursor(cursor_t **chead) {
	cursor_t *cursor;
	int i;

	for (cursor = *chead; cursor; cursor = *chead) {
		*chead = cursor->next;

		for (i=0; i<cursor->count; i++) {
			free(cursor->data[i]);
		}
		free(cursor);
	}
}

#if 0
static void dump_cursor(cursor_t *cursor) {
	int i, indx;

	printf("Cursor dump ----\n");

	indx = 0;
	while (cursor) {
		printf("Cursor %d %d items \n", indx++, cursor->count); 
		for (i=0; i<cursor->count; i++) {
			printf(" %s\n", cursor->data[i]);
		}
		cursor = cursor->next;
	}
}
#endif
/*** Exported  Functions ***/ 
DIR *opendir(const char *path) {
	DIR							*dirp;
	struct fcntl_stat {
		struct {
			struct _io_devctl			i;
			int							flags;
		}							devctl;
		struct _io_stat				stat;
	}							msg;
	struct stat					buff;

	//Add the extra 8 bytes to guarantee that dd_buf aligns properly
	if(!(dirp = malloc(sizeof *dirp + 8 + BUF_SIZE))) {
		errno = ENOMEM;
		return(0);
	}
	memset(dirp, 0, (sizeof(*dirp) + BUF_SIZE));
	dirp->dd_flags = D_DEFAULT_FLAGS;
	dirp->dd_buf = (char *)(((uintptr_t)(dirp + 1) + 7) & ~7);

	// A storage place for us to put our fd's for dirs we meet
	dirp->dd_fd_block.fds = NULL;
	dirp->dd_fd_block.index = 0;
	dirp->dd_fd_block.count = 0;

	// get the stat info
	msg.devctl.i.type = _IO_DEVCTL;
	msg.devctl.i.combine_len = offsetof(struct fcntl_stat, stat) | _IO_COMBINE_FLAG;
	msg.devctl.i.dcmd = DCMD_ALL_SETFLAGS;
	msg.devctl.i.nbytes = sizeof msg.devctl.flags;
	msg.devctl.i.zero = 0;
	msg.devctl.flags = O_CLOEXEC | O_NOCTTY;
	msg.stat.type = _IO_STAT;
	msg.stat.combine_len = sizeof msg.stat;
	msg.stat.zero = 0;


	if(_connect_fd(0, path, 0, O_RDONLY | O_NONBLOCK | O_CLOEXEC | O_NOCTTY, SH_DENYNO, _IO_CONNECT_COMBINE, 1,
					_IO_FLAG_RD, 0, 0, sizeof msg, &msg, sizeof buff, &buff, 0, 
					&dirp->dd_fd_block.count, &dirp->dd_fd_block.fds) == -1) {
		goto bad_alloc;
	}

	//This is only the stat for the first (and prefered) fd
	if(!S_ISDIR(buff.st_mode)) {
		dirp->dd_fd_block.count--; 
		while (dirp->dd_fd_block.fds && dirp->dd_fd_block.count >=0) {
			close(dirp->dd_fd_block.fds[dirp->dd_fd_block.count--]);
		}
		errno = ENOTDIR;
		goto bad_alloc;
	}

	dirp->dd_size = dirp->dd_loc = 0;
	//The count should be initialize to the number of fd's by connect
	dirp->dd_fd_block.index = 0;

	//Optimization ... turn off filtering if we only have one fd
	if (dirp->dd_fd_block.count <= 1) {
		dirp->dd_flags &= ~D_FLAG_FILTER;
	}

	return dirp;

bad_alloc:
	if (dirp) {
		if (dirp->dd_fd_block.fds) {
			free(dirp->dd_fd_block.fds);
		}
		free(dirp);
	}
	return 0;
}

struct dirent64 *readdir64(DIR *dirp) {
	struct dirent64	*d;
	int				xtype;

	xtype = (dirp->dd_flags & D_FLAG_STAT) ? _IO_XFLAG_DIR_EXTRA_HINT : 0;
do {

	if(dirp->dd_loc >= dirp->dd_size) {
		dirp->dd_loc = 0;

		do {
			if ((dirp->dd_fd_block.index >= dirp->dd_fd_block.count) ||
			    (dirp->dd_size = _readx(dirp->dd_fd_block.fds[dirp->dd_fd_block.index],
										dirp->dd_buf, BUF_SIZE, xtype, "", 0)) == -1) {
				//Error condition (read == -1) ... exit with a null
				dirp->dd_size = 0;
				return(NULL);
			}

			//PR 7994: "But what if servers lie", jump to the next server
			if (dirp->dd_size > BUF_SIZE) {
				dirp->dd_size = 0;
			}

			//Return null after traversing all the directories, on all fd's
			if (dirp->dd_size == 0) {
				if (++dirp->dd_fd_block.index >= dirp->dd_fd_block.count) {
					return 0;
				}
				else {
					;//We should probably bump ahead by two loc's to skip . and .. on subsequent fd's
				}
			}
		} while (dirp->dd_size == 0);

	}

	d = (struct dirent64 *)&dirp->dd_buf[dirp->dd_loc];
	dirp->dd_loc += d->d_reclen;

} while ((dirp->dd_flags & D_FLAG_FILTER) && 
		 find_with_insert(&dirp->dd_cursor, d->d_name));

	dirp->dd_pos++;
	return d;
}

struct dirent *readdir(DIR *dirp)
{
struct dirent				*d;
struct dirent_extra_stat    *extra;

	if ((d = (struct dirent *)readdir64(dirp)) == NULL)
		return(NULL);
	if (d->d_ino_hi != 0 || d->d_offset < 0 || d->d_offset_hi != 0)
		return(errno = EOVERFLOW, (struct dirent *)NULL);
	for (extra = (struct dirent_extra_stat*)_DEXTRA_FIRST(d); _DEXTRA_VALID(extra, d); extra = (struct dirent_extra_stat*)_DEXTRA_NEXT(extra)) {
		if (extra->d_type == _DTYPE_STAT || extra->d_type == _DTYPE_LSTAT)
			if (extra->d_stat.st_size < 0 || extra->d_stat.st_size_hi != 0 || extra->d_stat.st_ino_hi != 0 || extra->d_stat.st_blocks_hi != 0)
				extra->d_type = _DTYPE_NONE;
	}
	return(d);
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	int					save, status;

	save = errno;
	if((errno = pthread_mutex_lock(&readdir_mutex)) == EOK) {
		struct dirent				*ent;

		pthread_cleanup_push(readdir_cleanup, &readdir_mutex);
		if((*result = ent = readdir(dirp))) {
			memcpy(*result = entry, ent, offsetof(struct dirent, d_name) + ent->d_namelen + 1);
		}
		pthread_cleanup_pop(1);
	}
	status = errno;
	errno = save;
	return status;
}

void rewinddir(DIR *dirp) {
	int				i, save = errno;

	for (i=0; i<dirp->dd_fd_block.count;i++) {
		lseek(dirp->dd_fd_block.fds[i], 0, SEEK_SET);
	}
	dirp->dd_fd_block.index = 0;

	if (dirp->dd_cursor) {
		free_cursor(&dirp->dd_cursor);
	}

	errno = save;
	dirp->dd_pos = dirp->dd_size = dirp->dd_loc = 0;
}

int closedir(DIR *dirp) {
	int			ret = 0;

	while (--dirp->dd_fd_block.count >= 0) {
		ret |= close(dirp->dd_fd_block.fds[dirp->dd_fd_block.count]);
	}

	if (dirp->dd_fd_block.fds) {
		free(dirp->dd_fd_block.fds);
	}
	if (dirp->dd_cursor) {
		free_cursor(&dirp->dd_cursor);
	} 
	free(dirp);
	return ret;
}

/*** DIRCTL Functionality undocumented and in dirent.h ***/

#include <stdarg.h>
static int _dircntl(DIR *dir, int cmd, va_list ap) {

	switch(cmd) {
	case D_GETFLAG: 
		/* This value CANNOT BE NEGATIVE */
		return dir->dd_flags;

	case D_SETFLAG: 
		dir->dd_flags = va_arg(ap, int);
		return 0;

	default:
		break;
	}

	errno = ENOSYS;
	return -1;
}

int dircntl(DIR *dir, int cmd, ...) {
	va_list  	ap;
	int			ret;

	va_start(ap, cmd);
	ret = _dircntl(dir, cmd, ap);
	va_end(ap);
	return ret;
}

/* 
 These two functions are particularily gross because in a 
 unioned filesystem such as ours, you can't simply get an
 offset for just one entry but you need a combination of 
 entry/server + offset. 
 Two approaches: 
 1) Maintain a table of server+offset mappings and return
    that mapping to the client as the "value".
 2) Iterate through the directories and maintain a count
    value.

 Go for 2 now based on simplicity and code size, though
 method 1 should be implemented for correctness.
*/

//Return a 0 based index which will get us to the last readdir() call
long telldir(DIR *dirp) {
	return dirp->dd_pos;
}

void seekdir(DIR *dirp, long loc) {
unsigned	flags;

	rewinddir(dirp);
	flags = dirp->dd_flags, dirp->dd_flags &= ~D_FLAG_STAT;
	while(dirp->dd_pos != loc) {
		if(readdir64(dirp) == NULL) {
			break;
		}
	}
	dirp->dd_flags = flags;
}


__SRCVERSION("iodir.c $Rev: 206032 $");
