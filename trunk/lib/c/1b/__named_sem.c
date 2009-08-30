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



#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <share.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/iomsg.h>
#include <sys/stat.h>

struct nsem {
	sem_t			object;
	int				refcnt;
	dev_t			dev;
	ino_t			ino;
	struct nsem		*link;
};

static struct {
	pthread_mutex_t		mutex;
	struct nsem			*nsems;
} _semctrl = { PTHREAD_MUTEX_INITIALIZER, NULL };

static int semfd(const char *name, int oflag, mode_t mode, uint32_t value, dev_t *dev, ino_t *ino)
{
struct stat		st;
int				fd;

	st.st_ino = 0;
	if ((fd = _connect(0, name, mode, oflag | O_CLOEXEC, SH_DENYNO, _IO_CONNECT_OPEN, 0, _IO_FLAG_RD | _IO_FLAG_WR, _FTYPE_SEM, _IO_CONNECT_EXTRA_SEM, sizeof(value), &value, sizeof(struct stat), &st, NULL)) != -1) {
		if (st.st_ino != 0 || fstat(fd, &st) != -1) {
			*dev = st.st_dev;
			*ino = st.st_ino;
			return(fd);
		}
		close(fd);
	}
	return(-1);
}

sem_t *_nsem_open(const char *name, int oflag, mode_t mode, unsigned value)
{
struct nsem		*sem, *n;

	if ((sem = malloc(sizeof(struct nsem))) != NULL) {
		if ((sem->object.__count = semfd(name, oflag, mode, value, &sem->dev, &sem->ino)) != -1) {
			_mutex_lock(&_semctrl.mutex);
			for (n = _semctrl.nsems; n != NULL; n = n->link) {
				if (n->dev == sem->dev && n->ino == sem->ino) {
					++n->refcnt;
					_mutex_unlock(&_semctrl.mutex);
					close(sem->object.__count);
					free(sem);
					return(&n->object);
				}
			}
			sem->object.__owner = _NTO_SYNC_NAMED_SEM;
			sem->refcnt = 1;
			sem->link = _semctrl.nsems;
			_semctrl.nsems = sem;
			_mutex_unlock(&_semctrl.mutex);
			return(&sem->object);
		}
		free(sem);
	}
	else {
		errno = ENOMEM;
	}
	return(SEM_FAILED);
}

int _nsem_close(void *ptr)
{
struct nsem		**n, *sem;

	_mutex_lock(&_semctrl.mutex);
	for (n = &_semctrl.nsems; (sem = *n) != NULL; n = &sem->link) {
		if (ptr == &sem->object) {
			if (!--sem->refcnt) {
				close(sem->object.__count);
				sem->object.__count = -1;
				*n = sem->link;
				free(sem);
			}
			_mutex_unlock(&_semctrl.mutex);
			return(0);
		}
	}
	_mutex_unlock(&_semctrl.mutex);
	errno = EINVAL;
	return(-1);
}

__SRCVERSION("__named_sem.c $Rev: 153052 $");
