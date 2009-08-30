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

#include "externs.h"
#include "procmgr_internal.h"
#include "apm.h"
#include <spawn.h>
#include <sys/posix_spawn.h>


//#define EXTRA_DEBUG		/* enable display_factlist() */

static int fill_struct_inheritance(struct inheritance *inherit, posix_spawnattr_t *attrp);
static int fill_mempart_list(pid_t ppid, part_list_t **mempart_list, posix_spawnattr_t *attrp);
static int fill_schedpart_list(pid_t ppid, part_list_t **schedpart_list, posix_spawnattr_t *attrp);
static int parse_pspawn_msg(pid_t ppid, proc_posixspawn_t *msg, proc_spawn_t **spawn_msg, proc_create_attr_t *partlist);
static int add_fdmap(posix_spawn_file_actions_t *factp, int32_t fdmap[], unsigned fdcnt);
static int calc_fdcnt(posix_spawn_file_actions_t *factp, unsigned *fdcnt);
static int get_schedpartids(const posix_spawnattr_t *_Restrict attrp, unsigned *num, part_list_t **partlist);
static int get_mempartids(const posix_spawnattr_t *_Restrict attrp, unsigned *num, part_list_t **partlist);
static int _inherit_mempart_list(PROCESS *prp, part_list_t **part_list, unsigned behaviour);
static int _inherit_schedpart_list(PROCESS *prp, part_list_t **part_list, unsigned behaviour);

#ifdef EXTRA_DEBUG
static void display_factlist(posix_spawn_file_actions_t *fact_p);
#endif	/* NDEBUG */

/*******************************************************************************
 * procmgr_pspawn
 * 
 * Implement posix_spawn()
 * Currently we receive a _PROC_POSIX_SPAWN message and then reformat it into
 * a _PROC_SPAWN message with parse_pspawn_msg().
 * See rationale comment below
*/
int procmgr_pspawn(resmgr_context_t *ctp, void *vmsg) {
	pid_t								ppid = ctp->info.pid;
	proc_posixspawn_t					*msg = vmsg;
	proc_create_attr_t					partlist = {NULL, NULL};
	proc_spawn_t						*spawn_msg = NULL;

	/*
	 * FIX ME
	 * For now, we will not handle multipart messages. If all of the
	 * proc_posixspawn_t message could not be sent, none will be processed
	 * We do it now before we even call the endswap() which would try and
	 * operate on non-existent data
	*/
	if (ctp->info.srcmsglen > ctp->info.msglen) return EMSGSIZE;

	/*
	 * if remote spawn, need to use a local parent process for partition
	 * associations
	*/
	if(ND_NODE_CMP(ctp->info.nd, ND_LOCAL_NODE) != 0) {
		/* set ppid to netmgr's pid in network spawn case */
		ppid = pathmgr_netmgr_pid();
		if(ppid == 0) {
			ppid = SYSMGR_PID;
		}
	}

	if(ctp->info.flags & _NTO_MI_ENDIAN_DIFF) {
		ENDIAN_SWAP16(&msg->i.subtype);
		switch(msg->i.subtype) {
		case _PROC_SPAWN_START:
			ENDIAN_SWAP16(&msg->i.pathlen);
			ENDIAN_SWAP16(&msg->i.nargv);
			ENDIAN_SWAP16(&msg->i.narge);
			ENDIAN_SWAP32(&msg->i.filact_bytes);
			ENDIAN_SWAP32(&msg->i.attr_bytes);
			ENDIAN_SWAP32(&msg->i.argenv_bytes);
			if (msg->i.filact_bytes > 0) {
				/* endian swap the posix_spawn_file_actions_t structure */
				void *p = (void *)((uint8_t *)(msg + 1) + msg->i.attr_bytes);
				posix_spawn_file_actions_endswap(p);
			}
			if (msg->i.attr_bytes > 0) {
				/* endian swap the posix_spawnattr_t structure */
				void *p = (void *)(msg + 1);
				posix_spawn_file_actions_endswap(p);
			}
			break;
		default:
			return EENDIAN;
		}
	}

	switch(msg->i.subtype) {
	case _PROC_SPAWN_START:
	{
		int r;

		/*
		 * For now, the posix_spawn() message will be translated into a _PROC_SPAWN
		 * message. Originally this transformation was done in libc however the
		 * _PROC_SPAWN message is not capable of handling some of the posix_spawn()
		 * behaviour so a new _PROC_POSIX_SPAWN message was created.
		 * Peter V's suggested implementation was to eventually have spawn() call
		 * posix_spawn() (because posix_spawn() can handle all of the existing spawn()
		 * behaviour, not vice versa). With the new message we have effectively
		 * created the interface into procnto and now can focus on the internals,
		 * the first step of which is to parse the 'proc_posixspawn_t' message into
		 * a 'proc_spawn_t' message.  
		*/
		if ((r = parse_pspawn_msg(ctp->info.pid, msg, &spawn_msg, &partlist)) == EOK)
		{
			unsigned spawn_msg_len = sizeof(*spawn_msg)
									 + spawn_msg->i.searchlen
									 + spawn_msg->i.pathlen
									 + spawn_msg->i.nfds * sizeof(int32_t)
									 + spawn_msg->i.nbytes;

			r = EMSGSIZE;
			/* make sure that the entire constructed 'proc_spawn_t' message can be processed */ 
			if (spawn_msg_len <= ctp->msg_max_size)
			{
 				resmgr_context_t ctp2 = *ctp;				// make a local copy of the ctp

				ctp2.info.flags &= ~_NTO_MI_ENDIAN_DIFF;	// clear the ENDIAN flag (cannot be remote)
				/* fixup the ctp message length field to correspond to the proc_spawn_t message */
				ctp2.msg =  (resmgr_iomsgs_t *)spawn_msg;
				ctp2.info.msglen = ctp2.info.srcmsglen = spawn_msg_len;

				CRASHCHECK(ctp2.info.srcmsglen > ctp2.info.msglen);
				CRASHCHECK(ctp2.info.msglen > ctp2.msg_max_size);
				
				r = procmgr_spawn(&ctp2, spawn_msg, &partlist);
			}
		}
		if (spawn_msg != NULL) {
			free(spawn_msg);
		}
		if (partlist.mpart_list != NULL) {
			free(partlist.mpart_list);
		}
		if (partlist.spart_list != NULL) {
			free(partlist.spart_list);
		}
		return r;
	}
	default:
		return ENOSYS;
	}
}

/*******************************************************************************
 * inherit_mempart_list
 * inherit_schedpart_list
 * 
 * build the part_list from the inheritable partitions of the parent idenitfied
 * by <ppid>. This routine is also called from procmgr_spawn()/procmgr_fork()
 *
 * If <*part_list> is != NULL, it assumed that the caller has allocated
 * enough memory to hold the number of part_list_t entries to be inherited
 * and <part_list>->num_entries field should be set to indicate what the
 * expected number is.
 * If <*part_list> == NULL, then the necessary memory will be allocated and
 * the caller is responsible for releasing it unless an error is returned in
 * which case no memory will be allocated
 * 
 * Note that it is assumed that <prp> is safe to use (locked)
 * 
*/
int inherit_mempart_list(PROCESS *prp, part_list_t **part_list)
{
	return _inherit_mempart_list(prp, part_list, 0);
}
int inherit_schedpart_list(PROCESS *prp, part_list_t **part_list)
{
	return _inherit_schedpart_list(prp, part_list, 0);
}


/*
 * =============================================================================
 * 
 * 							Internal Support Routines
 * 
 * =============================================================================
*/

/*******************************************************************************
 * key_mask_create
 *
 * All of the libc posix_spawnattr_xx() and posix_spawn_file_actions_xx()
 * routines use KEY_MASK to ensure that the keyed values are 8 byte aligned.
 * When the data is messaged to proc, there is no way to guarantee that this
 * alignment will be preserved during the copy. In order to make use of the
 * posix_spawnattr_xx() and posix_spawn_file_actions_xx() routines to parse
 * the incoming message, we locally reconstruct the 'posix_spawnattr_t' and
 * 'posix_spawn_file_actions_t' objects however we must cause the libc routines
 * to use an alternate key mask (ie. one that is acceptable to the natural
 * alignment of the '_posix_spawnattr_t' and '_posix_spawn_file_actions_t'
 * with the message sent to us.) This is done by using the KEY_MASK bits to
 * specify the key mask in the pointer passed as the '_posix_spawnattr_t' or
 * '_posix_spawn_file_actions_t' object (see GET_ATTRP() and GET_FACTP() in
 * libc).
 * The net result is that if the messaged structures are aligned on an 8 byte
 * boundary, we simply use KEY_MASK as expected by the libc routines. If its
 * 4 byte aligned (the only other option), then we use a key mask of 3 and
 * encode the mask into the KEY_MASK bits of the '_posix_spawnattr_t' or
 * '_posix_spawn_file_actions_t' objects
 * 
 * The alternative to this solution is to
 * 		- weaken KEY_MASK by setting is to 3 requiring only 4 byte alignment
 * 		  which increases the chance of bogus pointer detection for user programs
 * 		  failing in the libc routines
 * 		- memmove() the messaged structures to an 8 byte alignment boundary which
 * 		  is costly and in efficient
 *
*/
#define ALIGNOF(x)			((unsigned int)(x) & KEY_MASK)

static unsigned int key_mask_create(void *_v)
{
	unsigned int v = (unsigned int)_v;
	unsigned int align = ALIGNOF(v);
	CRASHCHECK((align != 0) && (align != 4));
	return (align == 0) ? KEY_MASK : align - 1; 
}

static unsigned int obj_key_adj(void *_a, void *_b)
{
	unsigned int a = (unsigned int)_a;
	unsigned int b = (unsigned int)_b;
	unsigned int align = ALIGNOF(b);
	CRASHCHECK((align != 0) && (align != 4));
	return a + ((align == 0) ? 0 : align - 1); 
}

/*******************************************************************************
 * parse_pspawn_msg (temporary)
 * 
 * Create a proc_spawn_t message from a proc_posixspawn_t message
 * 
 * The memory for the new proc_spawn_t message and the part_list is allocated.
 * It is the responsibility of the caller to free it
*/
static int parse_pspawn_msg(pid_t ppid, proc_posixspawn_t *msg, proc_spawn_t **spawn_msg,
							proc_create_attr_t *partlist)
{
	size_t spawn_msg_len;
	posix_spawnattr_t  attrp = POSIX_SPAWNATTR_INITIALIZER;
	posix_spawn_file_actions_t  factp = POSIX_SPAWN_FILE_ACTIONS_INITIALIZER;
	proc_spawn_t  *new_msg;
	uint8_t  *p, *p_new;
	int r;
	unsigned fdcnt = 0;
	unsigned fdmap_size = 0;
	posix_spawnattr_t *attrp_p = NULL;
	posix_spawn_file_actions_t *factp_p = NULL;

	CRASHCHECK(msg == NULL);
	CRASHCHECK(spawn_msg == NULL);
	CRASHCHECK(partlist == NULL);

	*spawn_msg = NULL;
	partlist->mpart_list = NULL;
	partlist->spart_list = NULL;

	/*
	 * if file actions have been provided we need to figure out the size of
	 * the 'fdmap' for the _PROC_SPAWN message before its allocated. This size
	 * is not what you think however. Because the 'fdmap' is index based, we must
	 * allocate way more space than we really need. For example, if we only want
	 * to modify file descriptor 10, we need fdmap array entries 0 through 9.
	*/
	if (msg->i.filact_bytes > 0)
	{
		void *_factp = (void *)((unsigned int)(msg + 1) + msg->i.attr_bytes);
		void *open_action = (void *)((unsigned int)_factp + msg->i.filact_bytes);

		/*
		 * fix any open file_actions pointers so they point to the local messaged
		 * structure. All open actions are after the optional factp
		*/
		if (file_open_actions_fixup(_factp, open_action) != EOK) return EINVAL;

		/* create the 'posix_spawn_file_actions_t' object */
		set_factp(&factp, _factp, key_mask_create(_factp));
		factp_p = (posix_spawn_file_actions_t *)obj_key_adj(&factp, _factp);

		if ((r = calc_fdcnt(&factp, &fdcnt)) != EOK) {
			return r;
		}
		fdmap_size = fdcnt * sizeof(/*msg->i.fdmap[0]*/int32_t);

#ifdef EXTRA_DEBUG
		display_factlist(factp_p);
#endif
	}

	/*
	 * the new spawn message will have the following optional extensions layout
	 * as per procmsg.h ...
	 * 	- an fdmap built from the file actions structure (fdcnt * sizeof(int32_t))
	 * 	- no search - searchlen = 0 (because posix_spawn() will already have
	 * 	  provided a fully qualified path to the executable)
	 * 	- a path and length equivalent to msg->i.pathlen (same reason as above)
	 * 	- identical argv and env parameters (nbytes == argenv_bytes)
	*/
	spawn_msg_len = sizeof(new_msg->i) + fdmap_size + msg->i.pathlen + msg->i.argenv_bytes;
	if ((new_msg = calloc(1, spawn_msg_len)) == NULL) {
		return ENOMEM;
	}

	new_msg->i.type = _PROC_SPAWN;
	new_msg->i.subtype = msg->i.subtype;
	new_msg->i.searchlen = 0;
	new_msg->i.pathlen = msg->i.pathlen;
	new_msg->i.nfds = fdcnt;
	new_msg->i.nargv = msg->i.nargv;
	new_msg->i.narge = msg->i.narge;
	new_msg->i.nbytes = msg->i.argenv_bytes;

	/* now, add the 'fdmap' in the new message as required */
	if (factp_p != NULL) {
		if ((r = add_fdmap(factp_p, (int32_t *)(new_msg+1), fdcnt)) != EOK) {
			free(new_msg);
			return r;
		}
	}

	/* put in the path to the executable */
	CRASHCHECK(fdmap_size != (new_msg->i.nfds * sizeof(/*msg->i.fdmap[0]*/int32_t)));
	p_new = (uint8_t *)((unsigned)&new_msg->i.nbytes + sizeof(new_msg->i.nbytes) + fdmap_size);
	p = (uint8_t *)((unsigned)(msg + 1) + msg->i.attr_bytes + msg->i.filact_bytes + msg->i.filact_obytes);
	memcpy(p_new, p, new_msg->i.pathlen);

	/* add in remaining args and env */
	p_new += new_msg->i.pathlen;
	p += new_msg->i.pathlen;
	memcpy(p_new, p, new_msg->i.nbytes);

	/*
	 * create a valid 'posix_spawnattr_t' object. This allows us to use the
	 * posix_spawnattr_getxxx() functions to extract the necessary info
	*/
	p = (void *)(msg + 1);
	set_attrp(&attrp, p, key_mask_create(p));	// create the 'posix_spawnattr_t' object

	attrp_p = (posix_spawnattr_t *)obj_key_adj(&attrp, p);

	if ((r = fill_struct_inheritance(&new_msg->i.parms, attrp_p)) != EOK) {
		free(new_msg);
		return r;
	}
	if ((r = fill_mempart_list(ppid, &partlist->mpart_list, attrp_p)) != EOK) {
		free(new_msg);
		return r;
	}
	if ((r = fill_schedpart_list(ppid, &partlist->spart_list, attrp_p)) != EOK) {
		free(new_msg);
		return r;
	}
	*spawn_msg = new_msg;
	return EOK;
}

/*******************************************************************************
 * inherit_mempart_list
 * 
 * build the mempart_list from the inheritable partitions of the parent idenitfied
 * by <ppid>. This routine is also called from procmgr_spawn()
 *
 * The inheritable list consists of partitions associated with <prp> which satisfy
 * the mempart_flags_t_GETLIST_INHERITABLE and mempart_flags_t_GETLIST_CREDENTIALS
 * flags to MEMPART_GETLIST
 * 
 * <If behaviour> == 0, then default inherit behaviour is selected.
 * If <behaviour> == 1, then the alternate inherit behaviour is selected
 * What default and alternate actually do depends on the compilation defines
 * MEMPART_DFLT_INHERIT_BEHVR_1 and MEMPART_DFLT_INHERIT_BEHVR_2
 * 
 * If <*mempart_list> is != NULL, it assumed that the caller has allocated
 * enough memory to hold the number of part_list_t entries to be inherited
 * and <mempart_list>->num_entries field should be set to indicate what the
 * expected number is.
 * If <*mempart_list> == NULL, then the necessary memory will be allocated and
 * the caller is responsible for releasing it unless an error is returned in
 * which case no memory will be allocated
 * 
 * Note that it is assumed that <prp> is safe to use (locked)
 * 
*/
static int _inherit_mempart_list(PROCESS *prp, part_list_t **part_list, unsigned behaviour)
{
	CRASHCHECK ((behaviour != 0) && (behaviour != 1));
	CRASHCHECK(prp == NULL);

#if defined(MEMPART_DFLT_INHERIT_BEHVR_2)
	behaviour ^= 1;
#elif defined(MEMPART_DFLT_INHERIT_BEHVR_1)

#else
	#error
	#error no default partition inheritance defined
	#error
#endif	/* MEMPART_DFLT_INHERIT_BEHVR_x */

	switch(behaviour)
	{
		case 0:		// MEMPART_DFLT_INHERIT_BEHVR_1
		{
			int  num_parts;
			mempart_flags_t getlist_flags = mempart_flags_t_GETLIST_INHERITABLE | mempart_flags_t_GETLIST_CREDENTIALS;
			struct _cred_info cred;
			int n;
		
			CRASHCHECK(prp->cred == NULL);
			cred = prp->cred->info;

			/* get the number of inheritable partitions in the parent */
			num_parts = MEMPART_GETLIST(prp, NULL, 0, getlist_flags, &cred);
			if (num_parts <= 0) return ENOMEM;	// ENORSRC

			/* caller supplied the memory. Is it enough ? */
			if (*part_list != NULL) {CRASHCHECK((*part_list)->num_entries < num_parts);}

			/* caller didn't supply the memory so allocate it */
			if ((*part_list == NULL) && ((*part_list = malloc(PART_LIST_T_SIZE(num_parts))) == NULL)) {
				return ENOMEM;
			}

			/* get the list */
			n = MEMPART_GETLIST(prp, *part_list, (*part_list)->num_entries, getlist_flags, &cred);
			CRASHCHECK(n != 0);
			if (n != 0) {
				free(*part_list);
				return ENOMEM;	// ENORSRC
			}
			return EOK;
		}
		case 1:		// MEMPART_DFLT_INHERIT_BEHVR_2
		{
			/* only the sysram partition of the parent will be inherited */
			mempart_node_t *mp_node = MEMPART_NODEGET(prp, sys_memclass_id);
			CRASHCHECK(mp_node == NULL);		// have to have a sysram partition
			
			/* make sure the parents sysram partition is inheritable */
			if (mp_node->flags & part_flags_NO_INHERIT) {
				return ENOMEM;	// ENORSRC
			}
			/* caller supplied the memory. Is it enough ? */
			if (*part_list != NULL) {CRASHCHECK((*part_list)->num_entries < 1);}

			/* caller didn't supply the memory so allocate it */
			if ((*part_list == NULL) && ((*part_list = malloc(PART_LIST_T_SIZE(1))) == NULL)) {
				return ENOMEM;
			}
			(*part_list)->num_entries = 1;
			(*part_list)->i[0].id = MEMPART_T_TO_ID(mp_node->mempart);
			(*part_list)->i[0].flags = mp_node->flags;
			
			return EOK;
		}
		default:
			return EINVAL;
	}
}

/*******************************************************************************
 * _inherit_schedpart_list
 * 
 * build the part_list from the inheritable partitions of the parent idenitfied
 * by <ppid>. This routine is also called from procmgr_spawn()
 *
 * The inheritable list consists of partitions associated with <prp> which satisfy
 * the schedpart_flags_t_GETLIST_INHERITABLE and schedpart_flags_t_GETLIST_CREDENTIALS
 * flags to SCHEDPART_GETLIST
 * 
 * <If behaviour> == 0, then default inherit behaviour is selected.
 * If <behaviour> == 1, then the alternate inherit behaviour is selected
 * There is currently no defined alternate behaviour for scheduler partitions
 * and so this parameter is ignored
 * 
 * If <*part_list> is != NULL, it assumed that the caller has allocated
 * enough memory to hold the number of part_list_t entries to be inherited
 * and <part_list>->num_entries field should be set to indicate what the
 * expected number is.
 * If <*part_list> == NULL, then the necessary memory will be allocated and
 * the caller is responsible for releasing it unless an error is returned in
 * which case no memory will be allocated
 * 
 * Note that it is assumed that <prp> is safe to use (locked)
 * 
*/
static int _inherit_schedpart_list(PROCESS *prp, part_list_t **part_list, unsigned behaviour)
{
	CRASHCHECK (behaviour != 0);
	CRASHCHECK(prp == NULL);

	switch(behaviour)
	{
		case 0:
		{
			int  num_parts;
			schedpart_flags_t getlist_flags = schedpart_flags_t_GETLIST_INHERITABLE | schedpart_flags_t_GETLIST_CREDENTIALS;
			struct _cred_info cred;
			int n;
		
			CRASHCHECK(prp->cred == NULL);
			cred = prp->cred->info;

			/* get the number of inheritable partitions in the parent */
			num_parts = SCHEDPART_GETLIST(prp, NULL, 0, getlist_flags, &cred);
			if (num_parts <= 0) return EINVAL;	// ENORSRC

			/* caller supplied the memory. Is it enough ? */
			if (*part_list != NULL) {CRASHCHECK((*part_list)->num_entries < num_parts);}

			/* caller didn't supply the memory so allocate it */
			if ((*part_list == NULL) && ((*part_list = malloc(PART_LIST_T_SIZE(num_parts))) == NULL)) {
				return ENOMEM;
			}

			/* get the list */
			n = SCHEDPART_GETLIST(prp, *part_list, (*part_list)->num_entries, getlist_flags, &cred);
			CRASHCHECK(n != 0);
			if (n != 0) {
				free(*part_list);
				return EINVAL;	// ENORSRC
			}
			return EOK;
		}

		default:
			return EINVAL;
	}
}

/*******************************************************************************
 * fill_mempart_list
 * 
 * This function only does something if the POSIX_SPAWN_SETMPART flags is set
 *
 * As per design doc, only 3 conditions to consider
 * POSIX_SPAWN_SETMPART is set and posix_spawnattr_getpartid() indicates > 0
 * 		entries that are memory partitions
 * 	  - extract the partitions to associate with from <attrp>
 * POSIX_SPAWN_SETMPART is set and posix_spawnattr_getmempartid() indicates 0 entries
 * 	  - select the alternative inheritance behaviour
 * POSIX_SPAWN_SETMPART is NOT set
 * 	  - select the default inheritance behaviour
 * 		(set <*mempart_list> to NULL to cause default behaviour)
*/
static int fill_mempart_list(pid_t ppid, part_list_t **mempart_list, posix_spawnattr_t *attrp)
{
	int r;
	uint32_t flags;
	unsigned num_entries;
	part_list_t  *mplist;

	CRASHCHECK(attrp == NULL);
	CRASHCHECK(mempart_list == NULL);

	*mempart_list = NULL;

	r = posix_spawnattr_getxflags(attrp, &flags);
	CRASHCHECK(r != EOK);
	if (r != EOK) return r;	

	/* 3rd case, POSIX_SPAWN_SETMPART is not set so just return */
	if ((flags & POSIX_SPAWN_SETMPART) == 0) {
		CRASHCHECK(*mempart_list != NULL);
		return EOK;
	}

	if ((r = get_mempartids(attrp, &num_entries, &mplist)) != EOK) {
		return r;
	}
	
	if (num_entries == 0) {
		/* 2nd case, no partitions provided, select alternate inheritance behaviour */
		PROCESS *pprp = proc_lock_pid(ppid);
		CRASHCHECK(pprp == NULL);
		CRASHCHECK(*mempart_list != NULL);
		/* mempart_list will be allocated, use alternate inheritance behaviour */
		r = _inherit_mempart_list(pprp, mempart_list, 1);
		proc_unlock(pprp);
		return r;
	} else if (num_entries > 0) {
		/* 1st case, user provided partition list */
		unsigned  i;
		int  sysram_list_idx = -1;
		PROCESS *pprp;

		CRASHCHECK(num_entries != mplist->num_entries);

		/* in the case of num_entries > 0, need to make sure that a 'sysram'
		 * partition will have either the mempart_flags_HEAP_CREATE
		 * or mempart_flags_HEAP_SHARE flags set otherwise the association
		 * will fail. Also need to ensure that the caller has permissions
		 * to associate with the specified partitions.
		*/
		pprp = proc_lock_pid(ppid);
		CRASHCHECK(pprp == NULL);
		for (i=0; i<mplist->num_entries; i++)
		{
			CRASHCHECK(PART_TYPE(mplist->i[i].id) != parttype_MEM);

			/* ensure its a valid memory partition id */
			if (MEMPART_VALIDATE_ID(mplist->i[i].id) != EOK) {
				proc_unlock(pprp);
				free(mplist);
				return EINVAL;
			}

			/* make sure the process has access to it */
			if (MEMPART_VALIDATE_ASSOCIATION(mplist->i[i].id, &pprp->cred->info) != EOK) {
				proc_unlock(pprp);
				free(mplist);
				return EACCES;
			}

			/* if its the sysram class, make sure appropriate flags are set */
			if (mempart_get_classid(mplist->i[i].id) == sys_memclass_id)
			{
				sysram_list_idx = i;
				if ((mplist->i[i].flags & (mempart_flags_HEAP_CREATE | mempart_flags_HEAP_SHARE)) == 0)
						mplist->i[i].flags |= default_mempart_flags;
			}
		}
		proc_unlock(pprp);

		/* make sure that the caller provided a sysram partition */
		if (sysram_list_idx == -1) {
			free(mplist);
			return EINVAL;
		}

		/*
		 * if required, move the sysram partition to entry '0' of mempart_list
		 * while still preserving the users original order for other partitions
		*/
		if (sysram_list_idx != 0)
		{
			part_list_t  tmp;
			tmp.i[0] = mplist->i[sysram_list_idx];
			memmove(&mplist->i[1], &mplist->i[0], sysram_list_idx * sizeof(mplist->i[0]));
			mplist->i[0] = tmp.i[0];
		}

		*mempart_list = mplist;
		return EOK;
	} else {
		crash();
		/* NOTREACHED */
		return EINVAL;	// quiet compiler warning */
	}
}

/*******************************************************************************
 * fill_schedpart_list
 * 
 * This function only does something if the POSIX_SPAWN_SETSPART flags is set
 *
 * As per design doc, only 2 conditions to consider
 * POSIX_SPAWN_SETSPART is set and posix_spawnattr_getpartid() indicates > 0
 * 		entries that are scheduler partitions
 * 	  - use msg->posix_spawn.attr.u.sp_list to explicitly specify the
 * 		child partition association list
 * POSIX_SPAWN_SETSPART is NOT set
 * 	  - select the default inheritance behaviour
 * 		(set <*schedpart_list> to NULL to cause default behaviour)
*/
static int fill_schedpart_list(pid_t ppid, part_list_t **schedpart_list, posix_spawnattr_t *attrp)
{
	int r;
	uint32_t flags;
	unsigned num_entries;
	part_list_t  *splist;

	CRASHCHECK(attrp == NULL);
	CRASHCHECK(schedpart_list == NULL);

	*schedpart_list = NULL;

	r = posix_spawnattr_getxflags(attrp, &flags);
	CRASHCHECK(r != EOK);
	if (r != EOK) return r;	

	/* 3rd case, POSIX_SPAWN_SETSPART is not set so just return */
	if ((flags & POSIX_SPAWN_SETSPART) == 0) {
		CRASHCHECK(*schedpart_list != NULL);
		return EOK;
	}

	if ((r = get_schedpartids(attrp, &num_entries, &splist)) != EOK) {
		return r;
	}

	// POSIX_SPAWN_SETSPART should not be set without providing a partition
	if (num_entries == 0) {
		return EINVAL;
	} else {
		/* 1st case, user provided partition list */
		unsigned  i;
		PROCESS *pprp;

		CRASHCHECK(num_entries != splist->num_entries);

		/* in the case of num_entries > 0, need to ensure that the caller has
		 * permissions to associate with the specified partitions.
		*/
		pprp = proc_lock_pid(ppid);
		CRASHCHECK(pprp == NULL);
		for (i=0; i<splist->num_entries; i++)
		{
			CRASHCHECK(PART_TYPE(splist->i[i].id) != parttype_SCHED);

			/* ensure its a valid memory partition id */
			if (SCHEDPART_VALIDATE_ID(splist->i[i].id) != EOK) {
				proc_unlock(pprp);
				free(splist);
				return EINVAL;
			}

			/* make sure the process has access to it */
			if (SCHEDPART_VALIDATE_ASSOCIATION(splist->i[i].id, &pprp->cred->info) != EOK) {
				proc_unlock(pprp);
				free(splist);
				return EACCES;
			}
		}
		proc_unlock(pprp);

		*schedpart_list = splist;
		return EOK;
	}
	
}

/*******************************************************************************
 * fill_struct_inheritance
*/
static int fill_struct_inheritance(struct inheritance *inherit, posix_spawnattr_t *attrp)
{
	int r;

	CRASHCHECK(inherit == NULL);

	if ((r = posix_spawnattr_getxflags(attrp, (uint32_t *)&inherit->flags)) != EOK) {
		return r;
	}
	CRASHCHECK(inherit->flags & SPAWN_SEARCH_PATH);	// posix spawn message always has a fully qualified path

	if ((r = posix_spawnattr_getpgroup(attrp, &inherit->pgroup)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getsigmask(attrp, &inherit->sigmask)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getsigdefault(attrp, &inherit->sigdefault)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getsigignore(attrp, &inherit->sigignore)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getstackmax(attrp, (uint32_t *)&inherit->stack_max)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getschedpolicy(attrp, &inherit->policy)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getnode(attrp, &inherit->nd)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getrunmask(attrp, &inherit->runmask)) != EOK) {
		return r;
	}
	if ((r = posix_spawnattr_getschedparam(attrp, &inherit->param)) != EOK) {
		return r;
	}

	return EOK;
}

/*******************************************************************************
 * calc_fdcnt
 * 
 * Figure out from the file actions object, how many entries are required in the
 * 'fdmap' for the _proc_spawn message. It is not equivalent to the number of
 * file actions but rather to the highest index file action fd being manipulated.
 * This is one of the reasons that we need to eliminate the fdmap and have spawn()
 * use posix_spawn().
 * 
 * Returns: EOK on success, otherwise an errno and *fdcnt should be considered
 * 			garbage.
*/
static int calc_fdcnt(posix_spawn_file_actions_t *factp, unsigned *fdcnt)
{
	int num_fact, highest_fd;
	posix_spawn_file_actions_list_t *fact_list;
	unsigned i;
	int r;

	/* determine how much space to leave for fdmap entries */
	num_fact = 0;
	if ((r = posix_spawn_file_getactions(factp, &num_fact, NULL)) != EOK) return r;
	CRASHCHECK(num_fact > 0);
	num_fact = -num_fact;
	if ((fact_list = alloca(num_fact * sizeof(*fact_list))) == NULL) return ENOMEM;
	if ((r = posix_spawn_file_getactions(factp, &num_fact, fact_list)) != EOK) return r;
	CRASHCHECK(num_fact < 0);

	highest_fd = -1;
	/* now run the actions list looking for the highest 'fd' to be operated on */
	for (i=*fdcnt=0; i<num_fact; i++)
	{
		int fd = -1;
		switch(fact_list[i].type)
		{
			case posix_file_action_type_CLOSE:
				fd = fact_list[i]._type.close.fd;
				break;
			case posix_file_action_type_DUP:
				fd = fact_list[i]._type.dup.new_fd;
				break;
			case posix_file_action_type_OPEN:
				fd = fact_list[i]._type.open->new_fd;
				break;
			default:
				break;
		}
		if (fd > highest_fd) highest_fd = fd;
	}
	*fdcnt = highest_fd + 1;
	return EOK;
}

/*******************************************************************************
 * add_fdmap
 * 
 * FIX ME - we are only providing equivalent spawn() behaviour for now which
 * means we handle close() and dup() actions, not open. Note also that the
 * existing close() functionality provided by spawn() is not really a close(),
 * it's "don't open" which is not the same, hence why open actions are currently
 * not supported.
 * 
 * Returns: EOK on success, otherwise errno and the fdmap should be considered
 * 			garbage
*/
static int add_fdmap(posix_spawn_file_actions_t *factp, int32_t fdmap[], unsigned fdcnt)
{
	posix_spawn_file_actions_list_t *fact_list;
	int r, num_fact = 0;
	unsigned i;

	num_fact = 0;
	if ((r = posix_spawn_file_getactions(factp, &num_fact, NULL)) != EOK) return r;
	CRASHCHECK(num_fact > 0);
	num_fact = -num_fact;
	if ((fact_list = alloca(num_fact * sizeof(*fact_list))) == NULL) return ENOMEM;
	if ((r = posix_spawn_file_getactions(factp, &num_fact, fact_list)) != EOK) return r;
	CRASHCHECK(num_fact < 0);

	/* first, initialize entire fdmap to inherit */
	for (i=0; i<fdcnt; i++) {
		fdmap[i] = i;
	}
	/* now process the specific file actions provided entering them at the appropriate fdmap index */
	for (i=0; i<num_fact; i++)
	{
		switch(fact_list[i].type)
		{
			case posix_file_action_type_CLOSE:
				CRASHCHECK(fact_list[i]._type.close.fd >= fdcnt);
				fdmap[fact_list[i]._type.close.fd] = SPAWN_FDCLOSED;
				break;
			case posix_file_action_type_DUP:
				CRASHCHECK(fact_list[i]._type.dup.new_fd >= fdcnt);
				fdmap[fact_list[i]._type.dup.new_fd] = fact_list[i]._type.dup.fd;
				break;
			case posix_file_action_type_OPEN:
				/* FIX ME - for now silently ignore. Should we fail ? */
				break;
			default:
				return EINVAL;
		}
	}
	return EOK;
}

/*******************************************************************************
 * get_mempartids
 * get_schedpartids
 * 
 * A 'part_list_t' of the indicated type is allocated and filled in and
 * num_entries is set to the number of entries of the indicated type
 * 
 * Returns: EOK on success. If <*num> is > 0, the caller is responsible for
 * 			deallocating the 'memory for the 'part_list_t'
 * 			otherwise return an errno and no memory is allocated 
*/
static int getpartids(const posix_spawnattr_t *_Restrict attrp, unsigned *num, part_list_t **partlist, parttype_t type)
{
	int r;
	partlist_info_t *all_ids;
	int  num_ids = 0;
	unsigned i, idx;


	if ((r = posix_spawnattr_getpartid(attrp, &num_ids, NULL)) != EOK) return r;
	if (num_ids == 0) {
		*num = 0;
		return EOK;
	}
	num_ids = -num_ids;
	
	if ((*partlist = calloc(1, PART_LIST_T_SIZE(num_ids))) == NULL) return ENOMEM;
	if ((all_ids = alloca(num_ids * sizeof(*all_ids))) == NULL) return ENOMEM;

	if ((r = posix_spawnattr_getpartid(attrp, &num_ids, all_ids)) != EOK) {
		free(partlist);
		return r;
	}
	
	/* filter out types not requested */
	for (i=idx=0; i<num_ids; i++) {
		if (PART_TYPE(all_ids[i].id) == type) {
			(*partlist)->i[idx] = all_ids[i];
			++idx;
		}
	}
	*num = idx;
	(*partlist)->num_entries = idx;
	return EOK;
}

static int get_mempartids(const posix_spawnattr_t *_Restrict attrp, unsigned *num, part_list_t **partlist)
{
	return getpartids(attrp, num, partlist, parttype_MEM);
}
static int get_schedpartids(const posix_spawnattr_t *_Restrict attrp, unsigned *num, part_list_t **partlist)
{
	return getpartids(attrp, num, partlist, parttype_SCHED);
}


#ifdef EXTRA_DEBUG
static void display_factlist(posix_spawn_file_actions_t *fact_p)
{
	posix_spawn_file_actions_list_t *fact_list;
	int r, num_fact = 0;

	if ((r = posix_spawn_file_getactions(fact_p, &num_fact, NULL)) != EOK)
		kprintf("posix_spawn_file_getactions() failed, errno %d\n", r);
	else
	{
		num_fact = -num_fact;
		fact_list = alloca(num_fact * sizeof(*fact_list));
		if ((r = posix_spawn_file_getactions(fact_p, &num_fact, fact_list)) != EOK)
			kprintf("posix_spawn_file_getactions() failed, errno %d\n", r);
		else
		{
			unsigned i;
			kprintf("actions list for actions object %p\n", fact_p); 
			for (i=0; i<num_fact; i++)
			{
				kprintf("\t[%u] type = %u, ", i, fact_list[i].type);
				switch(fact_list[i].type)
				{
					case posix_file_action_type_CLOSE:
						kprintf("fd=%d\n", fact_list[i]._type.close.fd);
						break;
					case posix_file_action_type_DUP:
						kprintf("fd=%d, new_fd=%d\n", fact_list[i]._type.dup.fd, fact_list[i]._type.dup.new_fd);
						break;
					case posix_file_action_type_OPEN:
						kprintf("new_fd=%d, path=%s, oflags=0x%x, omode=0x%x\n", fact_list[i]._type.open->new_fd,
								fact_list[i]._type.open->path, fact_list[i]._type.open->flags, fact_list[i]._type.open->mode);
						break;
					default: kprintf("Unknown\n"); break;
				}
			}
		}
	}
}
#endif	/* EXTRA_DEBUG */


__SRCVERSION("procmgr_posix_spawn.c $Rev: 162527 $");
