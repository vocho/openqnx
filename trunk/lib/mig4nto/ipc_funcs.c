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
 *  ipc_funcs.c - QNX 4 to QNX Neutrino migration functions
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/iomsg.h>
#include <string.h>
#include <sys/netmgr.h>
#include <mig4nto.h>
#include <mig4nto_procmsg.h>
#include <mig4nto_rcvmsg.h>
#include <mig4nto_table.h>

#define INTERNAL_PROXY  0x80 /* Flag that indicates an Internal proxy, */
							 /* ORed into the                          */
							 /* proxy_data_t->data_size field          */

#define MAX_NODE_NAME   64  /* Maximum Node Name size   */

typedef struct {
	pid_t   lookup_pid;     /* The pid that is passed to Send       */
	int     coid;           /* The connection ID to use             */
	pid_t   remote_pid;     /* The pid of the remote node's process */
	char    node_name[MAX_NODE_NAME]; /* The node name of the remote node */ 
} connection_t;

typedef struct {
	pid_t	senders_pid;	/* Sender Message's pid          */
	int		rcvid;			/* Recieved Message's receive id */
	int 	proxy_count;	/* Number of these proxies       */
} queued_msg_t;

typedef struct {
	int     senders_pid;    /* Sender's pid Message's pid */
	int     rcvid;          /* Receive ID                 */
} received_msg_t;

typedef struct {
	void			*data;			/* Pointer to proxy data, could be NULL  */
	unsigned char	data_size;		/* _QNX_PROXY_SIZE_MAX = 100 bytes       */
	pid_t			creator_pid;	/* PID of process that created the proxy */
} proxy_data_t;

extern magic_t	magic;				/* defined in mig4nto.h */

static proxy_data_t	proxy_data_table[_PROXY_TABLE_MAX]; 

static int			pulse_coid = 0;	/* Connection ID for internal */
									/* proxys                     */  

static TABLE_T	connections_table;	/* Table of open connections       */
static TABLE_T	queued_msgs;		/* Table of queued messages        */
static TABLE_T	received_msgs;		/* Table of received messages      */

static int add_proxy_data(pid_t proxy, const void *data, int nbytes);
static proxy_data_t *get_proxy_data(pid_t proxy);
static int remove_proxy_data(pid_t proxy);
static int is_internal_proxy(pid_t proxy);
static int connections_compare(const void *p1, const void *p2);
static int get_connection_id(pid_t search_pid);
static int queued_msg_compare(const void *p1, const void *p2);
static int process_msg(int rcvid, receive_msg_t *rec_msg, iov_t * msgmx,  unsigned parts);
static int received_msg_compare(const void *p1, const void *p2);
static int get_rcvid_from_pid(pid_t pid, int remove_record);
static int get_queued_msg(pid_t pid, TABLE_T *qmess_table, unsigned parts, 
			 struct _mxfer_entry *msgmx, pid_t *ret_pid);
static int put_queued_msg(int rcvid, TABLE_T *qmess_table, receive_msg_t *rec_msg);

int
ipc_init(void)
{
	procmgr_msg_t	msg;
	procmgr_reply_t	reply;
	
	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_GET_TRIGGER_INFO;
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg),
						&reply, sizeof(reply)) == -1)
		return -1;
	if ((magic.trigger_coid = ConnectAttach(0,
						reply.un.trigger_info.trigger_mgr_pid,
						reply.un.trigger_info.trigger_chid,
						_NTO_SIDE_CHANNEL, 0)) == -1)
		return -1;

	if ((pulse_coid = ConnectAttach(ND_LOCAL_NODE, 0, magic.ipc_chid, 
						_NTO_SIDE_CHANNEL, 0)) == -1)
		return -1;
	if (init_table(&connections_table, sizeof(connection_t),
						connections_compare, F_SORT) != 1)
		return -1;
	if (init_table(&queued_msgs, sizeof(queued_msg_t), 
						queued_msg_compare, 0) != 1)
		return -1;
	if (init_table(&received_msgs, sizeof(received_msg_t), 
						received_msg_compare, F_SORT) != 1)
		return -1;
	return 0;
}
							
/*
 *  add_proxy_data
 *
 *  This function will add the given proxy data to the 
 *  proxy data table.  This proxy data table is used for proxies
 *  that are Triggered from this process or proxies that are
 *  returned from interrupt handlers from within this process.
 *  Note that the data is copied from the passed data pointer
 *  into an internal one.
 */
static int
add_proxy_data(pid_t proxy, const void *data, int nbytes)
{
	int	index;	/* Index into the proxy data table */

	index = GET_PROXYNUM(proxy);
	if (index >= 0 && index < _PROXY_TABLE_MAX) {
		if (proxy_data_table[_PROXY_TABLE_MAX].data) {
			free(proxy_data_table[index].data);
			proxy_data_table[index].data = NULL;
		}
		proxy_data_table[index].data_size = INTERNAL_PROXY;

		if (data) {	/* If there is data associated with this proxy */
			proxy_data_table[index].data = (void *) malloc(nbytes);
			if (proxy_data_table[index].data == NULL) {
				return 0;
			} else {
				memcpy(proxy_data_table[index].data, data, nbytes);
				proxy_data_table[index].data_size |= nbytes;
				proxy_data_table[index].creator_pid = getpid();
			}
		} else
			proxy_data_table[index].creator_pid = getpid();
	} else
		return 0;	/* error, invalid proxy number */
	return 1;
}

static proxy_data_t *
get_proxy_data(pid_t proxy)
{
	int             index; /* index into the proxy data table */
	proxy_data_t	*proxy_data = NULL; /* proxy data record */

	index = GET_PROXYNUM(proxy);
	if (index >= 0 && index < _PROXY_TABLE_MAX)
		proxy_data = &proxy_data_table[index];
	return proxy_data;
}

static int
remove_proxy_data(pid_t proxy)
{
	int index;  /* index into the proxy data table */

	index = GET_PROXYNUM(proxy);
	if (index >= 0 && index < _PROXY_TABLE_MAX) {
		if (proxy_data_table[_PROXY_TABLE_MAX].data) {
			free(proxy_data_table[index].data);
			proxy_data_table[index].data = NULL;
		}
		proxy_data_table[index].data_size = 0;
	} else
		return 0;	/* error, invalid proxy number */
	return 1;
}

static int
is_internal_proxy(pid_t proxy)
{
	int index;  /* index into the proxy data table */

	index = GET_PROXYNUM(proxy);
	if (index >= 0 && index < _PROXY_TABLE_MAX) {
		if ((proxy_data_table[index].data_size & INTERNAL_PROXY)
		 &&	proxy_data_table[index].creator_pid == getpid())
			return 1;
	}
	return 0;
}

/*
 *  get_connection_id
 *
 *	This function will get the corresponding connection id
 *	of the given pid.  If there currently is not a connection 
 *	established for the given pid, then one will be created and 
 *	saved for possible later use.
 */
static int
get_connection_id(pid_t search_pid)
{
	int					coid = -1;			/* Connection ID             */
	static connection_t	coid_search_rec;	/* Coid record to search for */    
	connection_t		*coid_found_rec;	/* Coid record found         */
	long				rec_num;

	/* First scan our table of current connection ID's for this process */
	coid_search_rec.lookup_pid = search_pid;
	coid_found_rec = get_record(&connections_table, &coid_search_rec, &rec_num);
	if (coid_found_rec) {
		if (coid_found_rec->node_name[0] != 0) {
			/* This is a remote process */
		} else {
			coid = coid_found_rec->coid; /* This is a local process */
		}
	} else {
		/* Record not found, need to add this one, also determine if it */
		/* is a local or remote node later                              */

		coid = ConnectAttach(0, search_pid, 1, _NTO_SIDE_CHANNEL, 0);
		if (coid == -1)
            return -1;
		coid_found_rec = (connection_t *) calloc(1, sizeof(connection_t));
		if (!coid_found_rec) {
			ConnectDetach(coid);
            return -1;
        }
		coid_found_rec->lookup_pid = search_pid;
		coid_found_rec->coid = coid;
		if (!add_record(&connections_table, coid_found_rec)) {
			free(coid_found_rec);
			ConnectDetach(coid);
			return  -1;
		}
	}
	return coid;
}

static int
connections_compare(const void *p1, const void *p2)
{                       
	connection_t	*pp1 = *((connection_t **) p1);
	connection_t	*pp2 = *((connection_t **) p2);

	if (pp1->lookup_pid < pp2->lookup_pid)
		return -1;
	if (pp1->lookup_pid == pp2->lookup_pid)
		return 0;                                         
		
	return 1;       
}

static int
queued_msg_compare(const void *p1, const void *p2)
{                       
	queued_msg_t	*pp1 = *((queued_msg_t **) p1);
	queued_msg_t	*pp2 = *((queued_msg_t **) p2);
	
	if (pp1->senders_pid < pp2->senders_pid)
		return -1;
	if (pp1->senders_pid == pp2->senders_pid)
		return 0;
		
	return 1;       
}

static int
received_msg_compare(const void *p1, const void *p2)
{                       
	received_msg_t	*pp1 = *((received_msg_t **) p1);
	received_msg_t	*pp2 = *((received_msg_t **) p2);
	
	if (pp1->senders_pid < pp2->senders_pid)
		return -1;
	if (pp1->senders_pid == pp2->senders_pid)
		return 0;                                         
	return 1;
}

static int
process_msg(int rcvid, receive_msg_t *rec_msg, iov_t* msgmx, unsigned parts)
{
	pid_t			ret_pid = -1;
	proxy_data_t	*proxy_data = NULL;	/* Internal Proxy data */     
	int				len;				/* Data lengths */   
	received_msg_t  *received_message = NULL; 

	if (rcvid == 0) {
		/* 
		 * we have received a proxy that we sent to ourself (as a pulse
		 * from an interrupt handler or by triggering our own proxy using
		 * Trigger()
		 */
		proxy_data = get_proxy_data(rec_msg->un.pulse.value.sival_int);

		if (proxy_data && proxy_data->data_size & INTERNAL_PROXY) {
			/* extract the proxy data length and check for enough space */
			/* in the receive iov buffer                                */
			len = (proxy_data->data_size & (~INTERNAL_PROXY));
			if (len > msgmx[0].iov_len)
				len = msgmx[0].iov_len;

			/* watch out, it may be a proxy with no data */
			if (len) {
				/* copy over the pulse/proxy data */
				memcpy(msgmx[0].iov_base, proxy_data->data, len);
			}
			msgmx[0].iov_len = len;

			/* fill in the receive pid value with the proxy number */
			ret_pid = rec_msg->un.pulse.value.sival_int;
			
		} else {
			errno = EFAULT;
			return -1;
		}
	} else {
		/* we have received a message, need to set up some buffers */
		switch (rec_msg->hdr.type) {
		case _RCVMSG_FROM_SEND: 
			/* received a message from Send*() */

			if (MsgReadv(rcvid, (iov_t *) msgmx, parts,
										sizeof(receive_msg_hdr_t)) == -1)
				return -1;
			/* add this Message to our list of received messages */
			if ((received_message = calloc(1, sizeof(received_msg_t))) == NULL)
				return -1;
			received_message->senders_pid = rec_msg->hdr.senders_pid;
			received_message->rcvid = rcvid;
			if (!add_record(&received_msgs, received_message)) {
				free(received_message);
				return -1;
			}
			ret_pid = rec_msg->hdr.senders_pid;
			break;

		case _RCVMSG_FROM_PROXY:
			/* Received a proxy message from the proxy manager.           */
			/* Some other process told the proxy manager to send it (done */
			/* via Trigger().  Proxies do not get put into the received   */
			/* message table.                                             */
			if (MsgReadv(rcvid, (iov_t *) msgmx, parts, 
					sizeof(receive_msg_t)+sizeof(receive_from_proxy_msg_t)) == -1)
				return -1;
			/* Immediately reply back to the server */
			if (MsgReply(rcvid, EOK, NULL, 0) == -1)
				return -1;
			ret_pid = rec_msg->hdr.senders_pid;
			break;

		default: 
			/* Problem here, undefined message type */
			errno = ENOSYS;
			return -1;
		}
	}
	return ret_pid;
}

pid_t
Receive(pid_t pid, void *msg, unsigned nbytes)
{
	iov_t	iov[1];

	SETIOV(&iov[0], msg, nbytes);
	return Receivemx(pid, 1, (struct _mxfer_entry*) iov);
}

static int
put_queued_msg(int rcvid, TABLE_T *qmess_table, receive_msg_t *rec_msg)
{
	pid_t			senders_pid;
	queued_msg_t	search_message;
	queued_msg_t	*found_message;
	long			rec_num;
	
	if (rcvid != 0)
		senders_pid = rec_msg->hdr.senders_pid;
	else
		senders_pid = rec_msg->un.pulse.value.sival_int;

	if (rcvid == 0 || rec_msg->hdr.type == _RCVMSG_FROM_PROXY) {
		/* Search for this proxy */                                 
		search_message.senders_pid = senders_pid; 
		found_message  = (queued_msg_t *) get_record(qmess_table,            
											  &search_message, &rec_num);  
		if (found_message) {
			/* This proxy exists, just increment its counter */
			found_message->proxy_count++;
			return 1;
		}
	}

	if ((found_message = calloc(1, sizeof(queued_msg_t))) == NULL)
		return 0;
	found_message->senders_pid = senders_pid;
	found_message->rcvid = rcvid;
	if (!add_record(qmess_table, found_message))
		return 0;

	return 1;
}

/* return values for the get_queued_msg() function */
#define QUEUED_NO_MSG	0
#define QUEUED_GOT_MSG	1
#define QUEUED_ERROR	-1

/*
 *  get_queued_msg
 *		
 *	This function will check for queued messages.  Messages get queued
 *  when we receive on specific pid's but receive messages from other pids.
 *
 *  Returns:
 *      QUEUED_NO_MSG  - there is no message queued
 *      QUEUED_GOT_MSG - processed a message
 *      QUEUED_ERROR   - error, errno is set
 */
static int
get_queued_msg(pid_t pid, TABLE_T *qmess_table, 
				unsigned parts, struct _mxfer_entry *msgmx, pid_t *ret_pid)
{
	queued_msg_t	search_message;
	queued_msg_t	*found_message;
	long			rec_num;
	receive_msg_t	rec_msg;

	/* First determine if we have any queued messages for the given process */    
	if (pid > 0) {
		/* Search the table for any queued message from this process */
		search_message.senders_pid = pid;
		found_message = (queued_msg_t *) get_record(&queued_msgs, 
											&search_message, &rec_num);
	} else if (pid == 0) {
		/* Take the first record (if there is any) */
		get_next_record(&queued_msgs, T_FORWARD, S_RESET, &rec_num);              
		found_message = (queued_msg_t *) get_next_record(&queued_msgs, 
							   				T_FORWARD, S_NEXT, &rec_num);        
	} else {
		errno = ESRCH; /* invalid process id */
		return QUEUED_ERROR;
	}

	if (!found_message)
		return QUEUED_NO_MSG;

	if (found_message->rcvid != 0) { /* message or proxy */
		if ((*ret_pid = MsgRead(found_message->rcvid, &rec_msg,
									sizeof(receive_msg_t), 0)) == -1) {
			if (found_message->proxy_count == 0)
				delete_record(&queued_msgs, NULL, rec_num);
			else
				found_message->proxy_count--;
			return QUEUED_ERROR;
		}
	} else { /* pulse */
		*ret_pid = found_message->senders_pid;
		rec_msg.un.pulse.value.sival_int = *ret_pid;
	}

	*ret_pid = process_msg(found_message->rcvid, &rec_msg, 
								(iov_t *) msgmx, parts);

	if (found_message->proxy_count == 0)
		delete_record(&queued_msgs, NULL, rec_num);
	else
		found_message->proxy_count--;

    if (*ret_pid == -1)
		return QUEUED_ERROR;
	return QUEUED_GOT_MSG;
}

/*
 * Receivemx
 *
 * We are expecting three 'types' of messages:
 * 1. Messages from Send*().
 * 2. Messages from mig4nto-procmgr that are regular messages but contain
 *    information about a proxy that is attached to us and that someone
 *    asked mig4nto-procmgr to trigger.
 * 3. Pulse from an interrupt handler or from ourself (we did a Trigger()
 *    for our own proxy).  In this case rcvid == 0 and
 *    pulse.code == PROXY_CODE.
 */
pid_t Receivemx( pid_t pid, unsigned parts, struct _mxfer_entry *msgmx)
{
	int				rcvid;
	receive_msg_t	rec_msg;
	pid_t			ret_pid;    

	/* First determine if we have any queued messages for the given process */    
	switch (get_queued_msg(pid, &queued_msgs, parts, msgmx, &ret_pid)) {
	case QUEUED_NO_MSG:
		break;
	case QUEUED_GOT_MSG:
		return ret_pid;
	case QUEUED_ERROR:
		return -1;
	}
	for (;;) {
		rcvid = MsgReceive(magic.ipc_chid, &rec_msg, sizeof(receive_msg_t), NULL);
		if (rcvid == -1)
			return -1;
		
		if (rcvid == 0) {
			struct _pulse	*pulse = (struct _pulse *) &rec_msg;
			pid_t			tmp;
			
			if (pulse->code != PROXY_CODE)
				continue; /* got an unexpected pulse, nothing to do */
			/*
			 * put the proxy pid where the code below expects it to be
			 * for the case where rcvid is 0
			 */
			tmp = (pid_t) pulse->value.sival_int;
			rec_msg.hdr.senders_pid = tmp;
			rec_msg.un.pulse.value.sival_int = (int) tmp;
		}
		/* Is this message received from a process we are receiving on? */
		if (pid == 0 || pid == rec_msg.hdr.senders_pid) {
			/* Yes, we have received a message from a process or proxy */
			/* that we are receiving on, process it and return from    */
			/* this function                                           */
			return process_msg(rcvid, &rec_msg, (iov_t *) msgmx, parts);
		} else {
			/* No, received a message from a process other than what   */
			/* we were listening on, need to cache it away             */
			if (!put_queued_msg(rcvid, &queued_msgs, &rec_msg))
				return -1;
		}
	}
}

int
Send(pid_t pid, void *smsg, void *rsmg, unsigned snbytes, unsigned rnbytes)
{
	iov_t	siov[1];
	iov_t	riov[1];

	SETIOV(&siov[0], smsg, snbytes);
	SETIOV(&riov[0], rsmg, rnbytes);
	return Sendmx(pid, 1, 1, (struct _mxfer_entry *) siov, 
					  (struct _mxfer_entry *) riov);
}

int
Sendmx(pid_t pid, unsigned sparts, unsigned rparts, 
		struct _mxfer_entry *smsgmx, struct _mxfer_entry *rmsgmx)
{
	int					ret_code;
	iov_t				*siov = NULL;
	receive_msg_hdr_t	msg_hdr;
	int					coid;
	
	if ((coid = get_connection_id(pid)) == -1)
		return -1;
	if ((siov = malloc((sparts+1) * sizeof(iov_t))) == NULL)
		return -1;
	msg_hdr.type = _RCVMSG_FROM_SEND;
	msg_hdr.senders_pid = getpid();
	SETIOV(&siov[0], &msg_hdr, sizeof(msg_hdr));
	memcpy(&siov[1], smsgmx, sizeof(struct _mxfer_entry) * sparts);
	ret_code = MsgSendv(coid, siov, sparts + 1, (iov_t *) rmsgmx, rparts);
	free(siov);
	return ret_code;
}

int
Reply(pid_t pid, void *msg, unsigned nbytes)
{
	iov_t	iov[1];

	SETIOV(&iov[0], msg, nbytes);
	return Replymx(pid, 1, (struct _mxfer_entry *) iov);
}

int
Replymx(pid_t pid, unsigned parts, struct _mxfer_entry *msgmx)
{
	int	rcvid;

	/* Need to convert this pid to it's corresponding receive ID */
	if ((rcvid = get_rcvid_from_pid(pid, 1)) == -1)
		return -1;
	return MsgReplyv(rcvid, 0, (const iov_t*) msgmx, parts);
}

/*
 *  get_rcvid_from_pid
 *
 *	This function will convert the given pid to it's respective
 *	receive id.  It does this by scanning the table of received
 *	messages by pid.  The pid's within this table are unique.
 */
static int
get_rcvid_from_pid(pid_t pid, int remove_record)
{
	int				rcvid = -1;
	received_msg_t	search_message; /* Record to search for     */
	received_msg_t	*found_message; /* Found record from search */
	long			rec_num;

	search_message.senders_pid = pid;
	found_message = get_record(&received_msgs, &search_message, &rec_num);
	if (found_message == NULL) {
		errno = ESRCH;
	} else {
		rcvid = found_message->rcvid;
		if (remove_record) {
			if (!delete_record(&received_msgs, NULL, rec_num)) {
				errno = ESRCH;
				rcvid = -1;
			}
		}
	}    
	return rcvid;
}

unsigned
Readmsgmx(pid_t pid, unsigned offset, unsigned parts, struct _mxfer_entry *msgmx)
{
	int	rcvid;

	/* Don't forget to skip over our secret message header */
	offset += sizeof(receive_msg_hdr_t); 
	
	/* Need to convert this pid to it's corresponding receive ID, 
	   However, don't delete the entry */
	rcvid = get_rcvid_from_pid(pid, 0);                            
	if (rcvid == -1)
		return -1;
	return (MsgReadv(rcvid, (const iov_t*) msgmx, parts, offset));
}

unsigned
Readmsg(pid_t pid, unsigned offset, void *msg, unsigned nbytes)
{
	iov_t	iov[1]; /* Read Message io Vectors */

	SETIOV(&iov[0], msg, nbytes);
	return Readmsgmx(pid, offset, 1, (struct _mxfer_entry *) iov);
}

unsigned
Writemsgmx(pid_t pid, unsigned offset, unsigned parts, struct _mxfer_entry *msgmx)
{
	int	rcvid;                                              
																	
	/* Need to convert this pid to it's corresponding receive ID,   
	   However, don't delete the entry */                           
	rcvid = get_rcvid_from_pid(pid, 0);
	if (rcvid == -1)
		return -1;
	return MsgWritev(rcvid, (const iov_t*) msgmx, parts, offset); 
}

unsigned
Writemsg(pid_t pid, unsigned offset, void *msg, unsigned nbytes)
{
	iov_t	iov[1];

	SETIOV(&iov[0], msg, nbytes);
	return Writemsgmx(pid, offset, 1, (struct _mxfer_entry *) iov);                                
}

pid_t
qnx_proxy_attach(pid_t pid, const void *data, int nbytes, int priority)
{
	procmgr_msg_t		msg;
	procmgr_reply_t		reply;
	struct sched_param	p;  
		
	reply.set_errno_to_this = EOK;
		
	if (nbytes > _QNX_PROXY_SIZE_MAX) {
		errno = EINVAL;
		return -1;
	} else {
		/* Try to kill the process with sig 0 to see */
		/* that it really exists.                    */
		if (pid && kill(pid, 0)) {
			errno = ESRCH;
			return -1;
		} else {
			if (priority == -1) {
				if (sched_getparam(0, &p) == -1)
					return -1;
				else
					msg.un.proxy.priority = p.sched_curpriority;
			} else
				msg.un.proxy.priority = priority;
		}
	}

	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_PROXY_ATTACH;
	msg.un.proxy.creator_nd = 0; /* getnid(); some day... */
	msg.un.proxy.creator_pid = getpid();
	msg.un.proxy.hit_this_nd = 0; /* getnid(); some day... */
	msg.un.proxy.hit_this_pid = pid ? pid : msg.un.proxy.creator_pid;
	msg.un.proxy.nbytes = nbytes;
	memcpy(msg.un.proxy.data, data, nbytes);
		
	if (MsgSend(magic.procmgr_fd, &msg, sizeof(msg),
				&reply, sizeof(reply)) == -1)
		return -1;

	if (reply.un.proxy_pid == -1) {
		errno = reply.set_errno_to_this;
		return -1;
	}

	/* If everything is ok to this point, and this is an "internal" proxy */
	/* then we also need to store proxy data locally to this process      */

	if (pid == 0 || pid == msg.un.proxy.creator_pid)
		add_proxy_data(reply.un.proxy_pid, data, nbytes);

	return reply.un.proxy_pid;
}

int
qnx_proxy_detach(pid_t pid)
{
	procmgr_msg_t	msg;
	procmgr_reply_t	reply;

	/* Remove the proxy data */
	remove_proxy_data(pid);

	msg.hdr.type = _IO_MSG;
	msg.hdr.combine_len = sizeof(msg.hdr);
	msg.hdr.mgrid = MGR_ID_PROCMGR;
	msg.hdr.subtype = _PROCMGR_QNX_PROXY_DETACH;
	msg.un.proxy.hit_this_pid = pid;

	if (MsgSend(magic.procmgr_fd, &msg, sizeof( msg ),
			 &reply, sizeof(reply)) == -1)
		return -1;

	errno = reply.set_errno_to_this;

	return reply.un.proxy_pid;
}

pid_t
Trigger(pid_t pid)
{
	procmgr_msg_t	msg;
	procmgr_reply_t	reply;

	if (is_internal_proxy(pid)) {
		/* we triggered a proxy that is attached to ourself */
		if (MsgSendPulse(pulse_coid, getprio(0), PROXY_CODE, pid) == -1)
			return -1;
		reply.un.proxy_pid = getpid();
	} else {
		msg.hdr.type = _IO_MSG;
		msg.hdr.combine_len = sizeof(msg.hdr);
		msg.hdr.mgrid = MGR_ID_PROCMGR;
		msg.hdr.subtype = _TRIGMGR_DO_TRIGGER;
		msg.un.proxy.hit_this_pid = pid;

		if (MsgSend(magic.trigger_coid, &msg, sizeof(proxy_msg_t),
			 &reply, sizeof(procmgr_reply_t)) == -1)
			return -1;

		errno = reply.set_errno_to_this;
	}
	return reply.un.proxy_pid;
}
