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

/*
 *  The code in this file handles the qnx_name_attach call by keeping a
 *  table of names and their corresponding process ID numbers internally;
 *  new names are added to this table. It handles the qnx_name_locate
 *  call by looking in the table for the corresponding name and nid,
 *  then returning the PID as needed. If the node is not the local node,
 *  it will (in the future) set up a connection to the process on the
 *  remote node. It handles qnx_name_query by returning messages one at 
 *  a time with the matching table info. And it handles qnx_name_detach
 *  by removing an entry from the internal table.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>
#include <sys/dispatch.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <pthread.h>
#include <atomic.h>
#include <semaphore.h>
#include <unistd.h>
#include <process.h>
#include <mig4nto.h>
#include <mig4nto_procmsg.h>
#include <mig4nto_table.h>

typedef struct {
	int    name_id;
	int    nid;
	pid_t  pid;
	char   name[_NAME_MAX_LEN + 1];
} name_table_t;

extern unsigned char   *progname;        	/* argv[0]                */
extern unsigned char   verbose;          	/* Handle -v option       */
										 
static TABLE_T         name_table;
static int             current_name_id = 1;
static pthread_mutex_t ntable_mutex;		/* mutex to control access */
											/* to proxy_table          */

static int find_entry(const void *l, const void *r);

int
name_init(void)
{
	pthread_mutex_init(&ntable_mutex, NULL);
	if (!init_table(&name_table, sizeof(name_table_t), find_entry, F_SORT)) {
		fprintf(stderr, "%s: Could not initialize the name table.\n", progname);
		return -1;
	}
	return 0;
}

/*
 *  name_handle_close_ocb
 *
 *	We need to remove all entries for this process from our name table.
 */
void
name_handle_close_ocb(pid_t pid)
{
	name_table_t *scan;
	long         rec_num;   

	pthread_mutex_lock(&ntable_mutex);
	get_next_record(&name_table, T_FORWARD, S_RESET, &rec_num);
	scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	while (scan) {
		if (verbose > 2)
			printf("Checking record pid %d against close pid %d\n",
				   scan->pid, pid);
		if (scan->pid == pid) {
			if (delete_record(&name_table, NULL, rec_num)) {
				if (verbose > 1)
					printf("%s: name_id %d nid %d pid %d"
						   " name \"%s\" removed.\n",
						   progname, scan->name_id, scan->nid,
						   scan->pid, scan->name );
			}
		}
		scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	}
	pthread_mutex_unlock(&ntable_mutex);
}

void
do_qnx_name_attach(procmgr_msg_t *msg, procmgr_reply_t *reply, pid_t from_pid)
{
	name_table_t *newbie;
	long         rec_num;

	reply->un.attach.name_id = -1;

	newbie = malloc(sizeof(name_table_t));
	if (!newbie)
		reply->set_errno_to_this = ENOMEM;
	else {
		newbie->name_id = current_name_id;
		newbie->nid = msg->un.attach.nid;
		newbie->pid = from_pid;
		/* Length for the name was checked on the client side. */
		strcpy(newbie->name, msg->un.attach.user_name_data);

		pthread_mutex_lock(&ntable_mutex);
		if (get_record(&name_table, newbie, &rec_num))
			reply->set_errno_to_this = EBUSY;
		else
			if (!add_record(&name_table, newbie))
				reply->set_errno_to_this = ENOMEM;
			else {
				if (verbose)
					printf("%s: name_id %d nid %d pid %d name \"%s\" added.\n",
						   progname, current_name_id, newbie->nid,
						   newbie->pid, newbie->name);
				reply->set_errno_to_this = EOK;
				reply->un.attach.name_id = current_name_id++;
			}
		pthread_mutex_unlock(&ntable_mutex);
	}
}

void
do_qnx_name_detach(procmgr_msg_t *msg, procmgr_reply_t *reply, pid_t from_pid)
{
	name_table_t *scan;
	long         rec_num;

	reply->un.detach.retval = -1;
	reply->set_errno_to_this = EINVAL;

	pthread_mutex_lock(&ntable_mutex);
	get_next_record(&name_table, T_FORWARD, S_RESET, &rec_num);    
	scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	while (scan) {
		if (verbose > 1)
			printf("Checking name_id %d nid %d"
				   " pid %d against %d %d %d name %s\n",
				   msg->un.detach.name_id,
				   (int) msg->un.detach.nid,
				   from_pid,
				   scan->name_id, scan->nid, scan->pid, 
				   scan->name);
		
		/* We must find it, and you must be the "owner" to detach it. */
		if (scan->name_id == msg->un.detach.name_id
		 && scan->nid == msg->un.detach.nid
		 && scan->pid == from_pid) {
			if (delete_record(&name_table, NULL, rec_num)) {
				reply->set_errno_to_this = EOK;
				reply->un.detach.retval = 0;
				if (verbose)
					printf("%s: name_id %d nid %d"
						   " pid %d name \"%s\" removed.\n",
						   progname, scan->name_id, scan->nid,
						   scan->pid, scan->name);
				break; /* Couldn't be registered twice anyway. */
			}
		} else
			scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	}
	pthread_mutex_unlock(&ntable_mutex);
}

void
do_qnx_name_locate(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	name_table_t *scan;
	long         rec_num;

	reply->set_errno_to_this = ESRCH;
	reply->un.locate.pid = -1;
	reply->un.locate.copies = 0;

	pthread_mutex_lock(&ntable_mutex);

	get_next_record(&name_table, T_FORWARD, S_RESET, &rec_num);
	scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	while (scan) {
		if (verbose > 1)
			printf("Checking name \"%s\" nid %d"
				   " against nid %d pid %d name \"%s\"\n",
				   msg->un.locate.name,
				   (int) msg->un.locate.nid,
				   scan->nid, scan->pid, scan->name );
		
		if (!strcmp(msg->un.locate.name, scan->name)) {
			reply->set_errno_to_this = EOK;
			reply->un.locate.pid = scan->pid;
			if (verbose)
				printf("Matched; returning pid %d\n", scan->pid);
			reply->un.locate.copies++;
		}
		scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	}
	pthread_mutex_unlock(&ntable_mutex);
}

/*
 *  do_qnx_name_query
 *
 *	The name query function looks up information about a registered name,
 *	but by name_id instead of by the string. In addition, if there is no
 *	exact match found, the next higher numbered name_id is returned. Thsi
 *	allows a client to "scan the table" by starting with name_id 1 and
 *	going from there.
 */
void
do_qnx_name_query(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	int   			min_difference = INT_MAX;
	long			rec_num;
	name_table_t	*scan;

	memset(reply, 0, sizeof(procmgr_reply_t));
	reply->set_errno_to_this = EINVAL;
	reply->un.name_query.name_id = -1;

	pthread_mutex_lock(&ntable_mutex);
	get_next_record(&name_table, T_FORWARD, S_RESET, &rec_num);
	scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	while (scan) {

		/* Find the name_id that is greater or equal, but the one */
		/* that is the smallest greater or equal.                 */
		if (msg->un.name_query.name_id <= scan->name_id
		 && scan->name_id - msg->un.name_query.name_id < min_difference) {
			min_difference = scan->name_id - 
							 msg->un.name_query.name_id;
			reply->set_errno_to_this = EOK;
			reply->un.name_query.name_id = scan->name_id;
			reply->un.name_query.buf.nid = scan->nid;
			reply->un.name_query.buf.pid = scan->pid;
			strcpy(reply->un.name_query.buf.name, scan->name);
			/* Short-cut - if the difference was zero, than there */
			/* is no need to look for one that is closer          */
			if (!min_difference)
				break;
		}
		scan = get_next_record(&name_table, T_FORWARD, S_NEXT, &rec_num);
	}
	pthread_mutex_unlock(&ntable_mutex);

	if (verbose > 1)
		printf("Returning closest match name_id %d"
			   " name \"%s\" nid %d pid %d\n",
			   reply->un.name_query.name_id,
			   reply->un.name_query.buf.name,
			   (int) reply->un.name_query.buf.nid,
			   reply->un.name_query.buf.pid );
}

/*
 *	find_entry
 *
 *	This is the comparison routine for the table search. We compare first
 *	by NID, and of the NID is the same then we compare by name.
 *
 *	l - a pointer to the left item
 *  r - a pointer to the right item
 *
 *	Returns: something < 0 if *l < *r, or...
 *			 something > 0 if *l > *r, or...
 *			 zero if *l == *r
 */
static int
find_entry(const void *l, const void *r)
{
	name_table_t *left, *right;
	
	left = *((name_table_t **) l);
	right = *((name_table_t **) r);
	if (left->nid != right->nid)
		return left->nid - right->nid;
	else
		return strcmp(left->name, right->name);
}
