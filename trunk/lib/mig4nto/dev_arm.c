/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include <malloc.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
#include <mig4nto.h>

/* Macro to determine if a value passed in as the proxy is really a signal. */
#define SIG_RANGE(x) ( 0xFFFFFF00 == ((x) & 0xFFFFFF00) ) 

struct arm_item_s;
typedef struct arm_item_s {
	struct arm_item_s	*next;
	int					fd;
	unsigned			events;
	pid_t				proxy;
	int					chid;
	int					coid;
	pthread_t			arm_tid;
	pthread_t			drain_tid;
	struct sigevent		pevent;
} arm_item_t;

static arm_item_t			*armlist = NULL;
static pthread_mutex_t		armmutex;
static pthread_once_t		armonce = PTHREAD_ONCE_INIT;

static void *arm_thread(void *data);
static void disarm_events_for_all(int fd, unsigned events, int preserve_tid);
static void disarm_item(arm_item_t *item, pthread_t preserve_tid);
static void disarm_events(arm_item_t *item, int events);
static int check_and_arm(int fd, pid_t proxy, unsigned events);
static int ionotifies(arm_item_t *item, struct sigevent *pevent, int *deliver_proxy, int *armed);
static void *drain_thread(void *data);
static void do_deliver_proxy(arm_item_t *item);
static void armlist_add(arm_item_t *item);
static void armlist_remove(arm_item_t *item);
static void arm_init_once(void);

int
dev_arm(int fd, pid_t proxy, unsigned events)
{
	pthread_once(&armonce, arm_init_once);
	
	if (proxy == _DEV_DISARM) {
		pthread_mutex_lock(&armmutex);
		disarm_events_for_all(fd, events, 0);
		pthread_mutex_unlock(&armmutex);
	} else {
		if (proxy == 0)
			proxy = ~SIGDEV;
		if (check_and_arm(fd, proxy, events) == -1)
			return -1;
	}
	return 0;
}

static void *
arm_thread(void *data)
{
	int				rcvid, chid;
	arm_item_t		*item = (arm_item_t *) data;
	sigset_t		mask;
	struct _pulse	pulse;
	
	sigfillset(&mask);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	pthread_mutex_lock(&armmutex);
	chid = item->chid;
	pthread_mutex_unlock(&armmutex);

	for (;;) {
		if ((rcvid = MsgReceive(chid, &pulse, sizeof(pulse), NULL)) == -1)
			continue;
		pthread_mutex_lock(&armmutex);
		do_deliver_proxy(item);
		disarm_events_for_all(item->fd, item->events, pthread_self());
		pthread_mutex_unlock(&armmutex);
		pthread_exit(NULL);
	}
}

static void
disarm_events_for_all(int fd, unsigned events, int preserve_tid)
{
	arm_item_t	*a, *next;
	
	/* disarm all of these events for this fd (device) */
	if (!armlist)
		return;
	for (a = armlist, next = a->next; a; a = next, next = a ? a->next : NULL) {
		if (a->fd == fd && (a->events & events)) {
			if (a->events & ~events) {
				/* if there will still be some events after having removed
				   these, leave arm thread alone */
				disarm_events(a, a->events & events);
				a->events &= ~events;
			} else {
				disarm_item(a, preserve_tid);
				armlist_remove(a);
			}
		}
	}
}

static void
disarm_item(arm_item_t *item, pthread_t preserve_tid)
{
	pthread_mutex_lock(&armmutex);
	disarm_events(item, item->events);
	if (item->arm_tid && item->arm_tid != preserve_tid)
		pthread_cancel(item->arm_tid);
	ConnectDetach(item->coid);
	ChannelDestroy(item->chid);
	pthread_mutex_unlock(&armmutex);
}

static void
disarm_events(arm_item_t *item, int events)
{
	if (events & _DEV_EVENT_INPUT)
		ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, NULL);
	if (events & _DEV_EVENT_OUTPUT)
		ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_OUTPUT, NULL);
	if (events & _DEV_EVENT_EXRDY)
		ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_OBAND, NULL);
	if (events & _DEV_EVENT_DRAIN) {
		if (item->drain_tid) {
			pthread_cancel(item->drain_tid);
			item->drain_tid = 0;
		}
	}
}

static int
check_and_arm(int fd, pid_t proxy, unsigned events)
{
	int				deliver_proxy = 0, armed = 0;
	struct sigevent	pevent;
	arm_item_t		*item;
	
	/* 
	 * Check for events.  If any are true, then cancel any arming that
	 * may have been done as a result of the checking and deliver the
	 * proxy now.  If any are not true then they will be armed to deliver
	 * a pulse so we create a thread to wait for the pulse.
	 */

	if ((item = calloc(1, sizeof(arm_item_t))) == NULL)
		return -1;
	
	if ((item->chid = ChannelCreate(0)) == -1) {
		free(item);
		return -1;
	}
	if ((item->coid = ConnectAttach(0, 0, item->chid, _NTO_SIDE_CHANNEL, 0)) == -1) {
		ChannelDestroy(item->chid);
		free(item);
		return -1;
	}
	item->fd = fd;
	item->events = events;
	item->proxy = proxy;
	SIGEV_PULSE_INIT(&pevent, item->coid, getprio(0), _PULSE_CODE_MINAVAIL, 0);
	
	if (ionotifies(item, &pevent, &deliver_proxy, &armed) == -1) {
		ConnectDetach(item->coid);
		ChannelDestroy(item->chid);
		free(item);
		return -1;
	}
	
	if (deliver_proxy) {
		if (armed)
			disarm_item(item, 0);
		do_deliver_proxy(item);
		free(item);
    } else {
		pthread_attr_t	attr;
		
		pthread_mutex_lock(&armmutex);
		armlist_add(item);
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		attr.__flags |= PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_ASYNCHRONOUS;
		pthread_create(&item->arm_tid, &attr, arm_thread, item);
		pthread_mutex_unlock(&armmutex);
		/* item is now in the list, it will be freed later */
	}
	
	return 0;
}

static int
ionotifies(arm_item_t *item, struct sigevent *pevent, int *deliver_proxy, int *armed)
{
	int	ioret = 0;

	*armed = 0;

	if (item->events & _DEV_EVENT_INPUT) {
		ioret = ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT,
							pevent);
		if (ioret != -1) {
			 if (ioret & _NOTIFY_COND_INPUT) {
				*deliver_proxy = 1;
			} else {
				*armed &= _DEV_EVENT_INPUT;
			}
		}
	}

	if (ioret != -1 && (item->events & _DEV_EVENT_OUTPUT) && !*deliver_proxy) {
    	ioret = ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_OUTPUT,
							pevent);
	    if (ioret != -1) {
    		if (ioret & _NOTIFY_COND_OUTPUT)
	        	*deliver_proxy = 1;
			else
				*armed &= _DEV_EVENT_OUTPUT;
		}
	}
	
	if (ioret != -1 && (item->events & _DEV_EVENT_EXRDY) && !*deliver_proxy) {
    	ioret = ionotify(item->fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_OBAND, 
							pevent);
	    if (ioret != -1) {
    		if (ioret & _NOTIFY_COND_OBAND)
	        	*deliver_proxy = 1;
			else
				*armed &= _DEV_EVENT_EXRDY;
		}
	}

	if (ioret == -1) {
		disarm_events(item, *armed);
		return -1;
	}	

	if ((item->events & _DEV_EVENT_DRAIN) && !*deliver_proxy) {
		pthread_attr_t	attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		attr.__flags |= PTHREAD_CANCEL_ENABLE | PTHREAD_CANCEL_ASYNCHRONOUS;
		pthread_create(&item->drain_tid, &attr, drain_thread, item);
		*armed &= _DEV_EVENT_DRAIN;
	}

	return 0;
}

static void *
drain_thread(void *data)
{
	int			fd;
	arm_item_t	*item = (arm_item_t *) data;
	sigset_t	mask;
	
	sigfillset(&mask);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	pthread_mutex_lock(&armmutex);
	fd = item->fd;
	pthread_mutex_unlock(&armmutex);

	if (tcdrain(fd) != -1) {	/* this could take some time */
		pthread_mutex_lock(&armmutex);
		MsgSendPulse(item->coid, -1, _PULSE_CODE_MINAVAIL, 0);
		pthread_mutex_unlock(&armmutex);
	}

	pthread_mutex_lock(&armmutex);
	item->drain_tid = 0;	
	pthread_mutex_unlock(&armmutex);

	return NULL;	
}

static void
do_deliver_proxy(arm_item_t *item)
{
	if (SIG_RANGE(item->proxy))
	    kill(getpid(), ~item->proxy);
	else
		Trigger(item->proxy);
}

static void
armlist_add(arm_item_t *item)
{
	if (armlist)
		item->next = armlist;
	armlist = item;
}

static void
armlist_remove(arm_item_t *item)
{
	arm_item_t	*a, *prev;
	
	for (a=armlist, prev=NULL; a!=NULL && a!=item; prev=a, a=a->next) ;
	if (a) {
		if (prev == NULL)
			armlist = a->next;
		else
			prev->next = a->next;
		free(a);
	}
}

static void
arm_init_once(void)
{
	pthread_mutexattr_t	attr;
	
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_setrecursive(&attr, PTHREAD_RECURSIVE_ENABLE);
	pthread_mutex_init(&armmutex, &attr);
}
