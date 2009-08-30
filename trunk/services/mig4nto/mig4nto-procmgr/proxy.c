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
 *  This will handle QNX 4 proxies; we receive messages that tell us that
 *  the client wants to do a qnx_proxy_attach(), and later a
 *  Trigger(). We handle both requests in the following way:
 *
 *  When a client does a qnx_proxy_attach it sends us a message.
 *  We make up a proxy number and then return this to the client; save
 *  the proxy number and data (if any) for later use.
 *
 *  To handle Trigger() in a timely fashion, we have a set of trigger
 *  threads (worker threads) that are all blocked on the same channel id.
 *  The Trigger() function MsgSend()s to this channel.  Having multiple
 *  threads waiting for messages on that channel increase the chance
 *  of the message being received in a timely fashion.  The trigger
 *  thread that receives the trigger message, replies back soonest
 *  (so that Trigger() may return) and then MsgSend()s another trigger
 *  message to whomever the proxy is attached to.
 *
 *  Having these trigger threads solves a couple of problems:
 *  1. we get to reply back to Trigger() sooner than if we were just
 *     using the resource manager threads
 *  2. the Trigger() gets acknowledgement that the proxy that it triggered
 *     is a valid one
 *  3. we can pass on the trigger message to the ultimate recipient
 *     without worrying too much about being send blocked.
 *
 *  There is also code here to hand out VIDs for virtual circuits 
 *  (in the future) and PIDs for proxies. The proxies are unique to a node,
 *  and this manager keeps track of them in order to make sure that there 
 *  is no possibility of a proxy ID and a VID overlapping, or of one of 
 *  these overlapping with a PID that is currently in use in the machine.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
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
#include <mig4nto_rcvmsg.h>

typedef struct {
	int           nd;			/* currently not supported */
	pid_t         pid;
	int           coid;
	int           creator_nd;
	pid_t         creator_pid;
	int           thread_priority;
	int           proxy_pid;
	int           nbytes;
	unsigned char data[_QNX_PROXY_SIZE_MAX];
} proxy_data_t;

#define NUMTRIGTHREADS_DEFAULT	4

#define PROXY_BITMAP_SIZE ((_PROXY_TABLE_MAX + 8) / 8) /* 251 bytes */
#define FIRST_BYTE_SET    0xfe /* The value for proxy_map[0]      */
#define LAST_BYTE_SET     0x01 /* The value for proxy_map[251]    */

extern unsigned char  	*progname;
extern unsigned char  	verbose;          	/* Handle -v option        */

static proxy_data_t   	**proxy_table;    	/* The table itself        */
static unsigned int   	proxy_table_size; 	/* The current size        */
static unsigned int   	proxy_table_used; 	/* The highest in use + 1  */
static pthread_mutex_t	ptable_mutex;		/* mutex to control access */
											/* to proxy_table          */
static int            	numtrigthreads = NUMTRIGTHREADS_DEFAULT;

static unsigned char or_on[] = 
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static unsigned char and_off[] = 
	{ 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };
static unsigned char	proxy_map[PROXY_BITMAP_SIZE];

static int			  	trigger_chid;		/* channel used by threads */
											/* which do triggering     */

static void setup_proxy_map(void);
static pid_t get_proxy_pid();
static pid_t release_proxy_pid(pid_t proxy_pid);
static unsigned find_and_clear(unsigned char *bitmap, unsigned length);
static unsigned find_and_set(unsigned char *bitmap, unsigned length, unsigned bit);
static unsigned hash(pid_t proxy);
static void *trigger_thread(void *data);
static int grow_table(void **table_ptr, unsigned *table_size,
		size_t entry_size, unsigned current_size);
static int shrink_table(void **table_ptr, unsigned *table_size,
		size_t entry_size, unsigned current_size);

int
proxy_init(int n)
{
	int				i;
	pthread_attr_t	attr;
	
	setup_proxy_map();

	pthread_mutex_init(&ptable_mutex, NULL);
	
	if (n != -1) {
		numtrigthreads = n;
		if (numtrigthreads < 1) {
			fprintf(stderr, "%s: number of threads must be at least 1\n", progname);
			return -1;
		}
	}
	
	/* Set up so that workers are detached from us. */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if ((trigger_chid = ChannelCreate(0)) == -1) {
		fprintf(stderr, "%s: Unable to create channel.\n", progname);
		exit(EXIT_FAILURE);
	}
	for (i = 0; i < numtrigthreads; i++) {
		/* Create threads.  They will all block on trigger_chid */
		if (pthread_create(NULL, &attr, trigger_thread, NULL) != EOK) {
			fprintf(stderr, "%s: Unable to create thread %d\n", progname, i);
			exit(EXIT_FAILURE);
		}
	}		
	return 0;
}

static void
setup_proxy_map(void)
{
	memset(proxy_map, 0xff, PROXY_BITMAP_SIZE - 1);
	proxy_map[0] = FIRST_BYTE_SET;
	proxy_map[PROXY_BITMAP_SIZE - 1] = LAST_BYTE_SET;
}

/*
 *  proxy_handle_close_ocb
 *
 *	We need to remove all proxies attached to this process from our
 *  proxy table.
 */
void
proxy_handle_close_ocb(int nd, pid_t pid)
{
	unsigned hint, biggest, found_one;

	pthread_mutex_lock(&ptable_mutex);

	/* Clean up all of the table entries that have this pid in them. */
	for (biggest = hint = found_one = 0; hint < proxy_table_used; hint++) {
		if (proxy_table[hint]
		 && proxy_table[hint]->nd == nd
		 && proxy_table[hint]->pid == pid) {
			biggest = hint;
			found_one = 1;
			if (verbose > 1)
				printf("%s: table[%d] nd %d pid %d proxy %d removed.\n",
					   progname, hint, nd, pid, 
					   proxy_table[hint]->proxy_pid);
			ConnectDetach(proxy_table[hint]->coid);
			release_proxy_pid(proxy_table[hint]->proxy_pid);
			free(proxy_table[hint]);
			proxy_table[hint] = NULL;
		}
	}

	if (found_one)
		/* Now shrink the table if possible */
		if (shrink_table((void **) &proxy_table, &proxy_table_size,
				sizeof(struct proxy_data_t *), biggest + 1))
			proxy_table_used = biggest + 1;

	pthread_mutex_unlock(&ptable_mutex);
}

void
do_qnx_proxy_attach(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	unsigned spot;
	int      new_coid;
	pid_t    new_proxy;

	reply->set_errno_to_this = EOK;

	if ((new_proxy = get_proxy_pid()) == 0) {
		reply->un.proxy_pid = -1;
		reply->set_errno_to_this = EAGAIN;
		return;
	}
	
	pthread_mutex_lock(&ptable_mutex);

	spot = hash(new_proxy);
	if (spot >= proxy_table_used) {
		if (!grow_table((void **) &proxy_table, &proxy_table_size,
				sizeof(struct proxy_table_t *),
				spot + 1)) {
			reply->un.proxy_pid = -1;
			reply->set_errno_to_this = EAGAIN;
			return;
		}
		proxy_table_used = spot + 1;
	}

	/* If we made it this far OK, get a connection. */

	new_coid = ConnectAttach(msg->un.proxy.hit_this_nd,
							msg->un.proxy.hit_this_pid, 1,
							_NTO_SIDE_CHANNEL, 0);
	if (new_coid == -1) {
		reply->un.proxy_pid = -1;
		reply->set_errno_to_this = errno;
		return;
	}

	/* If we are here, and errno is OK, we have the */
	/* table slot AND the channel ready to go...    */

	proxy_table[spot] = malloc(sizeof(proxy_data_t));
	if (!proxy_table[spot]) {
		reply->un.proxy_pid = -1;
		reply->set_errno_to_this = errno;
		return;
	}
	proxy_table[spot]->nd = msg->un.proxy.hit_this_nd;
	proxy_table[spot]->pid = msg->un.proxy.hit_this_pid;
	proxy_table[spot]->coid = new_coid;
	proxy_table[spot]->creator_nd = msg->un.proxy.creator_nd;
	proxy_table[spot]->creator_pid = msg->un.proxy.creator_pid;
	proxy_table[spot]->proxy_pid = new_proxy;
	proxy_table[spot]->thread_priority = msg->un.proxy.priority;
	proxy_table[spot]->nbytes = msg->un.proxy.nbytes;
	if (msg->un.proxy.nbytes > 0)
		memcpy(proxy_table[spot]->data, msg->un.proxy.data, msg->un.proxy.nbytes);
	if (verbose)
		printf("%s: Filling in proxy table[%d] with nd %d pid %d "
			   "proxy %d prio %d coid %d\n",
			   progname, spot, msg->un.proxy.hit_this_nd,
			   msg->un.proxy.hit_this_pid, new_proxy, 
			   msg->un.proxy.priority,
			   proxy_table[spot]->coid);

	reply->un.proxy_pid = proxy_table[spot]->proxy_pid;

	pthread_mutex_unlock(&ptable_mutex);
}

void
do_qnx_proxy_rem_attach(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	return;
}

void
do_qnx_proxy_detach(procmgr_msg_t *msg, procmgr_reply_t *reply, 
		struct _msg_info *info)
{
	reply->un.proxy_pid = -1;

	pthread_mutex_lock(&ptable_mutex);

	if (!proxy_table) {
		/* If it was never malloc'd, then we've never had */
		/* any requests, now have we? So errno it out.    */
		if (verbose)
			printf("%s: qnx_proxy_detach: no table allocated.\n", progname);
		reply->set_errno_to_this = ESRCH;
	} else {
		unsigned i = hash(msg->un.proxy.hit_this_pid);

		if (i >= proxy_table_used || proxy_table[i] == NULL) {
			if (verbose)
				printf("%s: qnx_proxy_detach: Can't find that one.\n", progname);
			reply->set_errno_to_this = ESRCH;
		} else {
			if (info->nd != proxy_table[i]->creator_nd
			 || info->pid != proxy_table[i]->creator_pid) {
				/* You aren't the ND/PID so you can't delete it. */
				if (verbose)
					printf("%s: qnx_proxy_detach: Not owner.\n", progname);
				reply->set_errno_to_this = EPERM;
			} else {
				if (verbose)
					printf("%s: Killing slot %d from the table\n", progname, i);
				ConnectDetach(proxy_table[i]->coid);
				free(proxy_table[i]);
				proxy_table[i] = NULL;
				release_proxy_pid(msg->un.proxy.hit_this_pid);

				/* Are we the very last entry? */
				if (i == proxy_table_used - 1) {
					if (shrink_table((void **) &proxy_table, &proxy_table_size,
							sizeof(proxy_data_t *),
							proxy_table_used - 1))
						proxy_table_used--;
				}
				reply->un.proxy_pid = 0;
				reply->set_errno_to_this = EOK;
			}
		}
	}
	
	pthread_mutex_unlock(&ptable_mutex);
}

void
do_qnx_proxy_rem_detach(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	return;
}

void
do_get_trigger_info(procmgr_msg_t *msg, procmgr_reply_t *reply)
{
	reply->set_errno_to_this = EOK;
	reply->un.trigger_info.trigger_chid = trigger_chid;
	reply->un.trigger_info.trigger_mgr_pid = getpid();
}

/*
 *  get_proxy_pid
 *
 *	This function makes up a guaranteed unique proxy number to use.
 *  This is because someone in the real world has called qnx_proxy_attach().
 *  We generate one here from those that are available.
 *
 *	We keep a bitmap of what proxy numbers are in use. There can be 2000
 *	of them in a node at one time. Each bit in the bitmap is a 1 iff that
 *	proxy is available, and it is a 0 iff that proxy number is in use. The
 *	proxy number here is 1..2000 (not 0.1999) so that 0x00000000 (see
 *	below) is not possible; the proxy number is then left shifted by 16
 *	before being passed back. This guarantees that the proxy ID is not the
 *	same as any real PID. For VIDs we make sure that these bits are zero so
 *	that no VID is the same as a proxy PID.
 *
 *	Recap:   0x07ff0000 is the proxy number part.
 * 	Example: 0x04530000 is a proxy PID
 *			 0x34530000 is not, but might be a VC PID or something else
 *
 *	By the way: We use / and %, so proxy 1 is in byte 0 bit 1,
 *				proxy 23 is in byte 2 bit 7, etc.
 *
 *	By the way: LAST_BYTE_SET is correct for 2000 proxies, but not too
 *				clean; it'd need to be changed if ever you change the
 *				setting for _PROXY_TABLE_MAX...
 */
static pid_t
get_proxy_pid(void)
{
	unsigned	bit_number;

	bit_number = find_and_clear(proxy_map, _PROXY_TABLE_MAX + 1);
	if (verbose > 1)
		printf("%s: returning proxy %u << 16\n", progname, bit_number);
	return bit_number << 16; /* could be 0 */
}

static pid_t
release_proxy_pid(pid_t proxy_pid)
{
	unsigned	bit_number;

	bit_number = proxy_pid >> 16;
	if (verbose > 1)
		printf("%s: clearing proxy %u << 16\n", progname, bit_number);
	if (find_and_set(proxy_map, _PROXY_TABLE_MAX + 1, bit_number))
		return proxy_pid;
	return 0;
}

/*
 *  find_and_clear
 *
 *	Find and clear a bit in the specified bitmap.
 *
 *	bitmap     Bitmap to use.
 *	length     Length of bitmap 
 */
static unsigned
find_and_clear(unsigned char *bitmap, unsigned length)
{
	/* Locate a candidate... We need memnotchr() */
	unsigned bit;

	for(bit = 0; bit < length; bit += 8)
		if (bitmap[bit / 8]) {
			/* This byte has some available. */
			unsigned which;
			for(which = 0; which < 8; which++)
				if (bitmap[bit / 8] & or_on[which]) {
					bitmap[bit / 8] &= and_off[which];
					return bit + which;
				}
		}
	return 0;
}

/*
 *	Find and Set a bit in the specified bitmap.
 *
 *	bitmap     Bitmap to use.
 *	length     Length of bitmap 
 *	bit        bit to set.
 */
static unsigned
find_and_set(unsigned char *bitmap, unsigned length, unsigned bit)
{
	if (bit >= length) {
		return 0;
	} else {
		bitmap[bit / 8] |= or_on[bit % 8];
		return 1;
	}
}

/*
 *  hash
 *
 *	Take a proxy number and return something in the range of 0..1999. We
 *	do this by some bit fiddling to get bits 16-26 from the proxy
 *	id. (Note that we could have one numbered 2047, but they are never
 *	generated past 1999.)
 *
 *  Returns: A number from 0..PROXY_TABLE_MAX that is the table entry 
 *           for this one
 */
static unsigned
hash(pid_t pid)
{
	return (pid >> 16) & 0x07ff;
}

static void *
trigger_thread(void *data)
{
	unsigned		pnum;
	int				rcvid, ret;
	procmgr_msg_t	msg;
	procmgr_reply_t	reply;
	receive_msg_t	outgoing;
	proxy_data_t	pdata;
	int				parts;
	iov_t			iov[2];
	
	for (;;) {
		if ((rcvid = MsgReceive(trigger_chid, &msg, sizeof(msg), NULL)) == -1) {
			if (verbose)
				printf("%s: Error receiving message %s.\n", progname, strerror(errno));
			continue;
		}
		
		pnum = hash(msg.un.proxy.hit_this_pid);
		
		pthread_mutex_lock(&ptable_mutex);

		if (!proxy_table) {
			if (verbose)
				printf("%s: No proxy table allocated.\n", progname);
			reply.un.proxy_pid = -1;
			reply.set_errno_to_this = ESRCH;
			MsgReply(rcvid, 0, &reply, sizeof(reply));
			pthread_mutex_unlock(&ptable_mutex);
			continue;
		}

		if (pnum >= proxy_table_used || proxy_table[pnum] == NULL) {
			if (verbose)
				printf("%s: Can't find the proxy number %d.\n", progname, pnum);
			reply.un.proxy_pid = -1;
			reply.set_errno_to_this = ESRCH;
			MsgReply(rcvid, 0, &reply, sizeof(reply));
			pthread_mutex_unlock(&ptable_mutex);
			continue;
		}
		
		/* copy so that we can unlock mutex soonest */
		memcpy(&pdata, proxy_table[pnum], sizeof(proxy_data_t));

		pthread_mutex_unlock(&ptable_mutex);
		
		/* proxy exists at least, let the Trigger() return */
		reply.un.proxy_pid = pdata.pid;
		reply.set_errno_to_this = EOK;
		MsgReply(rcvid, 0, &reply, sizeof(reply));
		
		parts = 1;
		outgoing.hdr.type = _RCVMSG_FROM_PROXY;
		outgoing.hdr.senders_pid = pdata.proxy_pid;
		outgoing.un.proxy.nbytes = pdata.nbytes;
		SETIOV(&iov[0], &outgoing, sizeof(receive_msg_t)+sizeof(receive_from_proxy_msg_t));
		if (pdata.nbytes) {
			parts++;
			SETIOV(&iov[1], pdata.data, pdata.nbytes);
		}                  
		 
		/* send the proxy message */
		ret = MsgSendv(pdata.coid, iov, parts, NULL, 0);

		if (verbose > 1)
			printf("%s: proxy MsgSend to coid %d returned %d, error %s\n",
				   progname, pdata.coid, 
				   ret, strerror(errno));
	}
}

/*
 *	grow_table, shrink_table
 *
 *	Grow or shrink a realloc'd table. Instead of growing it or shrinking
 *	it every time, we call realloc() only when we need to, asking for more
 *	than we actually need at a given time.
 *
 *	Inputs:  table_ptr  The address of the pointer to the table
 *			 table_size Pointer to an unsigned; this is the number of 
 *						entries in the table (not the number of bytes). 
 *						Note that some may be used, some not used, depending...
 *			 entry_size The number of bytes in one table entry
 *			 current_size
 *						The number of the largest table entry in use + 1.
 *						For example, if the biggest entry is #5, pass a
 *						current_size of 6 (0..5). 
 *	Outputs: table_ptr  May change if the table moves in memory
 *			 table_size May change if the table is grown (shrunk) OK
 *			 current_size
 *						May change if the table is grown (shrunk) OK
 *	Returns: 1 if the table was resized correctly, else 0
 */

#define GRANULARITY 32

static int
grow_table(void **table_ptr, unsigned *table_size,
		   size_t entry_size, unsigned current_size)
{
	size_t new_bytes, old_bytes;
	void   *new_address;

	if (current_size >= *table_size) {
		old_bytes = (*table_size) * entry_size;
		new_bytes = (current_size + GRANULARITY) * entry_size;
		new_address = realloc(*table_ptr, new_bytes);

		if (new_address) {
			/* Looks good; let's nuke the new ones. */
			memset(((char *) new_address) + old_bytes, 0x00, new_bytes - old_bytes);
			*table_ptr = new_address;
			*table_size = current_size + GRANULARITY;
#if DEBUG_OUT
			printf("New table has additional entries"
					", now %d with %d as the last one.\n"
					"There are now %d bytes at %p.\n",
					*table_size, current_size, new_bytes, new_address);
#endif
			return 1;
		} else {
			/* Couldn't get the space, but at least we have */
			/* the old table and it's info... Leave the #   */
			/* of entries unchainged and silently fail.     */
			return 0;
		}
	} else {
		/* There's room for that one, so do nothing. */
		return 1;
	}
}

static int
shrink_table(void **table_ptr, unsigned *table_size,
			 size_t entry_size, unsigned current_size)
{
	size_t new_bytes, old_bytes;
	void   *new_address;

	if (current_size < *table_size - GRANULARITY || current_size == 0) {
		/* Note that if current_size is 0, realloc free's it. */
		old_bytes = (*table_size) * entry_size;
		new_bytes = current_size * entry_size;
		new_address = realloc(*table_ptr, new_bytes);
		*table_ptr = new_address;
		*table_size = current_size;
#if DEBUG_OUT
		printf("New table has reduced"
			   ", now %d with %d as the last one.\n"
			   "Table is now %d bytes at %p\n",
			   *table_size, current_size, new_bytes, new_address );
#endif
		return 1;
	} else {
		/* Fewer entries, but don't shrink yet.     */
		return 1;
	}
}
