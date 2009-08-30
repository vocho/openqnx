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





#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <pwd.h>
#include <errno.h>
#include <pthread.h>
#include <process.h>
#include <devctl.h>
#include <sys/siginfo.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <sys/procmgr.h>
#include <sys/resmgr.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/dcmd_dumper.h>

typedef struct notifydata {
	pid_t rpid;              // to whom (can be determined from rcvid)
	int rcvid;               // whom to send it to
	int prio;                // priority of receiver
	struct sigevent event;   // what to send
} NotifyData;

typedef struct notifyitem {
  NotifyData data;
	struct notifyitem *next; // next in list
	struct notifyitem *prev; // prev in list
} NotifyItem;

NotifyItem *NotifyHead=NULL;
NotifyItem *NotifyTail=NULL;
pthread_mutex_t list_mutex        = PTHREAD_MUTEX_INITIALIZER;

int AddNotifyItem(pid_t rpid, int rcvid, struct sigevent *event);
int RemoveNotifyItem(pid_t rpid);
void DeleteItem(NotifyItem *remove);
void DeliverNotifies(pid_t pid);
NotifyItem *FindRcvid(int rcvid);
NotifyItem *FindPid(pid_t rpid);
int dumper_close_ocb(resmgr_context_t *ctp, void *reserved, iofunc_ocb_t *ocb);
int dumper_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb);
void RemoveAll(pid_t rpid);

int AddNotifyItem(pid_t rpid, int rcvid, struct sigevent *event)
{
	NotifyItem *temp=NULL;
	// first look for pid, type pair
	// if it exists, this is a replace notify message
	temp = FindPid(rpid);
	if (!temp) { // create
		temp = calloc(1,sizeof(NotifyItem));
		if (!temp) 
			return(-1);
		temp->data.rpid = rpid;
		temp->next = NULL;
		if (NotifyHead == NULL) { // first
			temp->prev = NULL;
			NotifyHead = NotifyTail = temp;
		}
		else { // add at the end
			temp->prev = NotifyTail;	
			NotifyTail = temp;
		}
	}
	temp->data.event = *event; // insert event
	temp->data.rcvid = rcvid;  // insert rcvid
	return(0);
}

// deletes specific item in list, 
// after deletion, remove is no longer valid to be accessed 
// in any way
void DeleteItem(NotifyItem *remove)
{
	if (remove == NULL)
		return;
	if (remove == NotifyHead) {
		NotifyHead = remove->next;
		if (NotifyHead == NULL)
			NotifyTail = NULL;
		else
			NotifyHead->prev = NULL;	
	}
	else if (remove == NotifyTail) {
		NotifyTail = remove->prev;
		if (NotifyTail == NULL)  // should never be: handled in earlier case
			NotifyHead = NULL;
		else
		  NotifyTail->next = NULL;
	}
	else { // somewhere in the middle
		remove->next->prev = remove->prev;
		remove->prev->next = remove->next;
	}
	free(remove);
	return;
}

int RemoveNotifyItem(pid_t rpid)
{
	// Search for pid, if it exists, remove it
	NotifyItem *remove=NULL;
	remove = FindPid(rpid);
	if (!remove) // no such item found	
		return(-1);
	DeleteItem(remove);
	return(0);
}

NotifyItem *FindPid(pid_t rpid)
{
	NotifyItem *temp=NULL;
	temp = NotifyHead;
	while (temp) {
		if (temp->data.rpid == rpid)
			return(temp);
		temp = temp->next;
	}
	return(NULL);	
}

NotifyItem *FindRcvid(int rcvid)
{
	NotifyItem *temp=NULL;
	temp = NotifyHead;
	while (temp) {
		if (temp->data.rcvid == rcvid)
			return(temp);
		temp = temp->next;
	}
	return(NULL);	
}

void RemoveAll(pid_t rpid)
{
	NotifyItem *temp;
	NotifyItem *remove;
	temp = NotifyHead;
	while (temp) {
		remove = NULL;
		if (temp->data.rpid == rpid) { // remove
		  remove = temp;
		}
		temp = temp->next;
		if (!remove)
		  DeleteItem(remove);
	}
	return;
}

void DeliverNotifies(pid_t pid)
{
	// deliver all the notifies
	NotifyItem *temp;
	NotifyItem *remove;
	int status;
	int value;
	temp = NotifyHead;
	while (temp) {
		if (temp->data.rpid != pid) {
			value = temp->data.event.sigev_value.sival_int;
			if (value == -1) // send pid instead
				temp->data.event.sigev_value.sival_int = pid;
			status = MsgDeliverEvent(temp->data.rcvid, &(temp->data.event));
			temp->data.event.sigev_value.sival_int = value;
			if (status == EOK) {
				temp = temp->next;	
				continue;
			}
		}
    	// either the pid that died is the same as the 
    	// one requesting the event, or the pid requesting the 
    	// event is no longer able to receive the event (deliver failed)
    	// in either case we remove the offending entry from the list
		remove = temp;
		temp = temp->next;
		DeleteItem(remove);
	}
	return;
}

int dumper_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb)
{
	void  *dptr;
	int   status;
	pid_t rpid;

	if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT)
		return (status);

	dptr = _DEVCTL_DATA (msg -> i);
	switch (msg -> i.dcmd) {
		case DCMD_DUMPER_NOTIFYEVENT:
			// request for an add notify event		    
			rpid = ctp->info.pid;
			AddNotifyItem(rpid, ctp->rcvid, (struct sigevent *)dptr);
			return(EOK);
		case DCMD_DUMPER_REMOVEEVENT:
			rpid = ctp->info.pid;
			// remove an event
			status = RemoveNotifyItem(rpid);
			if (!status)
				return(EINVAL);
			else
				return(EOK); 
		case DCMD_DUMPER_REMOVEALL:
			rpid = ctp->info.pid;
			// remove all notifies associated with a process
			RemoveAll(rpid);
			return(EOK);
		default:
			return(EINVAL); // invalid devctl message
	}
	// should never get here
	return(EINVAL);
}

int dumper_close_ocb(resmgr_context_t *ctp, void *reserved, iofunc_ocb_t *ocb)
{
	int status;
	pid_t rpid;
	struct _msg_info cl_info;

	if ((status = iofunc_close_ocb_default(ctp, NULL, ocb)) != _RESMGR_DEFAULT)
		return (status);

	MsgInfo(ctp->rcvid, &cl_info);
	rpid = cl_info.pid;
	// remove all notifies associated with a process
	RemoveAll(rpid);
	return(EOK);
}
