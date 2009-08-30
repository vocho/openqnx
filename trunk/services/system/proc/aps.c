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
#include "aps.h"
#include <kernel/schedpart.h>
#include <sys/sched_aps.h>


/*******************************************************************************
 * aps.c
 * 
 * This file servers the purpose of providing implementations of scheduler
 * partitioning function that are the analogue of those in memmgr/apm/aps.c
 * and memmgr/mm_mespart.c.
 * Since APS is wholly contained in ker/aps/ and has a different interface
 * than what exists via the apsgr module, the code in this file will bridge
 * the gap.
 * It contains code that is permanently part of the system even if the either
 * the apsgr or aps modules are not included in the build.
 * 
*/
/*
 * rsrcmgr_mempart_fnctbl
 * 
 * When the memory partitioning resource manager initializes, it can optionally
 * register a table of interface functions for use by the memory partioning
 * module. In this case the 'rsrcmgr_mempart_fnctbl' variable will be set
 * accordingly and the following public API's enabled
*/
static rsrcmgr_schedpart_fnctbl_t *rsrcmgr_schedpart_fnctbl = NULL;

/*
 * proxy_schedpart_fncTbl
 * the following table is used when no scheduler partitioning module is installed.
 * It is not declared static so that the SCHEDPART_INSTALLED() macro can test
 * against it. This eliminates a jump to unneeded stub routines
*/
/* the following are proxy functions for when the scheduler partitioning module is not installed */
static int schedpart_proc_disassociate(PROCESS *prp, part_id_t schedpart_id, ext_lockfncs_t *lf);
static int schedpart_proc_associate(PROCESS *prp, part_id_t schedpart_id, schedpart_dcmd_flags_t flags, ext_lockfncs_t *lf);
static int schedpart_getlist(PROCESS *prp, part_list_t *spart_list, int n, schedpart_flags_t flags, struct _cred_info *cred);
static schedpart_node_t *schedpart_nodeget(PROCESS *prp);
static void proxy_schedpart_init(part_id_t spid, sched_aps_partition_info *aps_info);
static int _schedpart_validate_association(part_id_t spid, struct _cred_info *cred);

schedpart_fnctbl_t proxy_schedpart_fnctbl =
{
	STRUCT_FLD(associate) schedpart_proc_associate,
	STRUCT_FLD(disassociate) schedpart_proc_disassociate,
	STRUCT_FLD(get_schedpart) schedpart_nodeget,
	STRUCT_FLD(get_schedpartlist) schedpart_getlist,
	STRUCT_FLD(validate_association) NULL,
	STRUCT_FLD(rsrcmgr_attach) NULL,
};

/*
 * _schedpart_rsrcmgr_fnctbl
 * 
 * This table defines the scheduler partitioning interface routines provided for
 * the resource manager porition of the scheduler partitioning module. When the
 * resource manager module registers (_schedpart_fnctbl.rsrcmgr_attach()), it will
 * be returned a pointer to this table so that it can interact with the scheduler
 * partitioning module.
 * 
 * Note that although this code is current within the same file and not in a module
 * we maintain the same module as for apm. Technically, this table and the functions
 * it holds could be part of the aps module
 * 
*/
static part_id_t			_schedpart_create(part_id_t parent_spid, const char *name, schedpart_cfg_t *child_cfg);
static int					_schedpart_destroy(part_id_t spid);
static int 					_schedpart_config(part_id_t spid, schedpart_cfg_t *cfg, unsigned key);
static schedpart_info_t *	_schedpart_getinfo(part_id_t spid, schedpart_info_t *info);
static PROCESS *			_schedpart_find_pid(pid_t pid, part_id_t spid);
static int _validate_cfg_creation(part_id_t parent_mpid, schedpart_cfg_t *cfg);
static int _validate_cfg_modification(part_id_t mpid, schedpart_cfg_t *cfg);
static schedpart_rsrcmgr_fnctbl_t _schedpart_rsrcmgr_fnctbl =
{
	STRUCT_FLD(create) _schedpart_create,
	STRUCT_FLD(destroy) _schedpart_destroy,
	STRUCT_FLD(config) _schedpart_config,
	STRUCT_FLD(getinfo) _schedpart_getinfo,
	STRUCT_FLD(find_pid) _schedpart_find_pid,
	STRUCT_FLD(validate_cfg_new) _validate_cfg_creation,
	STRUCT_FLD(validate_cfg_change) _validate_cfg_modification,
};

/*
 * schedpart_fncTbl
 * 
 * This function table pointer will be initialized when the scheduler partitioning
 * module is initialized and schedpart_init() is called.
*/
schedpart_fnctbl_t  *schedpart_fnctbl = NULL;

/*
 * sys_schedpart
 * 
*/
schedpart_t  *sys_schedpart = NULL;

/*
 * schedpart_init
 * 
 * This function pointer will be initialized to point to _schedpart_init() when
 * the scheduler partition module is loaded (see initialize_aps()). This will
 * allow the _schedpart_init() function to be called to perform the remainder of
 * the initialization. The actual call is made through the SCHEDPART_INIT() macro.
*/
void (*schedpart_init)(part_id_t spid, sched_aps_partition_info *aps_info) = proxy_schedpart_init;

/*
 * schedpart_select_dpp
 * 
 * This function pointer is initialized by the aps module and provides the
 * mechanism by which a newly created process can be associated with a specific
 * scheduling partition via posix_spawn(). It is called via SCHEDPART_SELECT_DPP()
 * in kerext_process_create().
 * If the aps module is not installed, it will remain NULL
*/
DISPATCH * (*schedpart_select_dpp)(PROCESS *prp, int id) = NULL;

/*
 * default_schedpart_flags
 * 
 * This global specifies the default scheduler partitioning flags, specifically
 * what behaviour is exhibited (by default) at process creation if nothing is
 * explicitly specified. It also sets the flags which are applied to the first
 * processes created by procnto since we do not want procnto's flags inherited.
 * All of the gory details can be found for the description of schedpart_flags_t
 * in schedpart.h but suffice it to say that unless otherwise specified by the
 * application or configured with the -S option to procnto in the build file,
 * we will use no specific flags
*/
schedpart_dcmd_flags_t default_schedpart_flags = part_flags_NONE;

/*
 * spmgr_st_size
 * 
 * FIX ME
 * This function pointer exists to allow procfs to get the size of free scheduler
 * from a partitioning perspective (ie. a processes "world view" of free scheduler
 * is the free scheduler in the hieracrhy of partitions for its associated sysram
 * scheduler class). It is initialized in apsmgr_init() but needs to be here
 * since we cannot reference a variable in a module
*/
int (*spmgr_st_size)(void *attr, off_t *size) = NULL;

/*
 * spid_unique
 * 
 * Similar to process id's, scheduler partition id's will be kept unique
*/
int spid_unique = 0;

/* accessed as SCHEDPART_DEFAULT_POLICY */
static const schedpart_policy_t  _schedpart_dflt_policy = SCHEDPART_DFLT_POLICY_INITIALIZER;
#define SCHEDPART_DFLT_POLICY		_schedpart_dflt_policy



/* FIX ME - need the cur proc locks */
static int m_lock_init(pthread_mutex_t *m)
{
	if (!KerextAmInKernel()) {
		return pthread_mutex_init(m, NULL);
	}
	else {
		pthread_mutex_t  tmp = PTHREAD_MUTEX_INITIALIZER;
		memcpy(m, &tmp, sizeof(*m));
		return EOK;
	}
}
static void m_lock_destroy(pthread_mutex_t *m)
{
	if (!KerextAmInKernel()) {
		pthread_mutex_destroy(m);
	}
	else {
		memset(m, 0, sizeof(*m));
	}
}
static int m_lock(pthread_mutex_t *m)
{
	if (!KerextAmInKernel()) {
		return pthread_mutex_lock(m);
	} else {
		return EOK;
	}
}
static void m_unlock(pthread_mutex_t *m)
{
	if (!KerextAmInKernel()) {
		pthread_mutex_unlock(m);
	}
}
#define MUTEX_INIT(_l_, _v_)	m_lock_init((_l_))
#define MUTEX_DESTROY(_l_)		m_lock_destroy((_l_))
#define MUTEX_RDLOCK(_l_)		m_lock((_l_))
#define MUTEX_RDUNLOCK(_l_)		m_unlock((_l_))
#define MUTEX_WRLOCK(_l_)		m_lock((_l_))
#define MUTEX_WRUNLOCK(_l_)		m_unlock((_l_))

/*
 * FIND_PART_NAME
 * 
 * FIX ME - this should be common
*/
#define FIND_PART_NAME(_p_)		(((apxmgr_attr_t *)(_p_)->rmgr_attr_p)->name)


#ifndef NDEBUG
static int _list_audit(part_qnodehdr_t *list)
{
	unsigned count = 0;
	struct _s {
		part_qnode_t  hdr;
	} *n = (struct _s *)LIST_FIRST(*list);
	while(n != NULL)
	{
		++count;
		n = (struct _s *)LIST_NEXT(n);
	}
	return (count == LIST_COUNT(*list)) ? EOK : EINVAL;
}
#define prp_list_audit(_l_)		_list_audit((part_qnodehdr_t *)&(_l_))
#define spart_list_audit(_l_)	_list_audit((part_qnodehdr_t *)&(_l_))
#endif	/* NDEBUG */


/* internal prototypes */
static part_id_t i_schedpart_create(part_id_t parent_spid, const char *name, schedpart_cfg_t *child_cfg, part_id_t spid);
static int schedpart_proc_disassociate_1(PROCESS *prp, schedpart_t *spart, ext_lockfncs_t *lf);
static int schedpart_validate_association(schedpart_t *spart, struct _cred_info *cred);

/*******************************************************************************
 * _rsrcmgr_attach
 *
*/
bool apmgr_module_installed(void)
{
	return (rsrcmgr_schedpart_fnctbl != NULL);
}

/*******************************************************************************
 * _rsrcmgr_attach
 *
*/
static schedpart_rsrcmgr_fnctbl_t *_rsrcmgr_attach(rsrcmgr_schedpart_fnctbl_t *ifTbl)
{
#ifndef NDEBUG
	if (ker_verbose) kprintf("Attaching scheduler partitioning resource manager\n"); 
#endif
	if (ifTbl != NULL)
	{
		rsrcmgr_schedpart_fnctbl = ifTbl;
	}
	return &_schedpart_rsrcmgr_fnctbl;
}

void _schedpart_init(part_id_t spid, sched_aps_partition_info *aps_info)
{
	proxy_schedpart_init(spid, aps_info);
	schedpart_fnctbl->rsrcmgr_attach = _rsrcmgr_attach;
	schedpart_fnctbl->validate_association = _schedpart_validate_association;
}

/*******************************************************************************
 * schedpart_getid
 * 
 * This function will return the scheduler partition identifier for the process
 * <prp>.
 * 
 * If <prp> == NULL, the system scheduler partition identifier will be returned
 * 
 * Returns: the partition pointer or NULL if no partition of the scheduler class
 * 			is associated with the process
*/
part_id_t schedpart_getid(PROCESS *prp)
{
	if (prp == NULL)
		return SCHEDPART_T_TO_ID(sys_schedpart);
	else
	{
		schedpart_node_t  *m = SCHEDPART_NODEGET(prp);
		return ((m == NULL) ? part_id_t_INVALID : SCHEDPART_T_TO_ID(m->schedpart));
	}
}


/*******************************************************************************
 * schedpart_validate_id
 * 
 * this function is used to validate a partition identifier passed in from
 * user space.
 * 
 * It returns EOK if <schedpart_id> is valid otherwise it returns an errno.
*/
int schedpart_validate_id(part_id_t spid)
{
	schedpart_t *spart = SCHEDPART_ID_TO_T(spid);
	
	/* RDLOCK ? */
	if ((spart != NULL) && (spart->signature == SCHEDPART_SIGNATURE)) {
		return EOK;
	}
	return EINVAL;
}

/*******************************************************************************
 * schedpart_module_loaded
 * 
 * this function is used to determine whether the scheduler partitioning module
 * is loaded or not. The module is considered to be loaded of the (*schedpart_init)
 * function pointer is not NULL and not pointing to the proxy_schedpart_init()
 * (ie. it has been overridden with the real scheduler partitioning initialization
 * function). 
 * 
 * It returns TRUE or FALSE depending on whether the scheduler partitioning module
 * is considered to be loaded or not.
 * 
 * Note that loaded and installed/initialized are considered to be 2 different
 * states for the scheduler partitioning module.
*/
bool schedpart_module_loaded(void)
{
	return ((schedpart_init != NULL) && (schedpart_init != proxy_schedpart_init));
}

/*******************************************************************************
 * schedpart_lookup_spid
 * 
 * implements SCHEDPART_ID_TO_T macro
 * Can be called from kernel or proc thread
 * 
 * Returns: the 'schedpart_t *' corresponding to <spid> or NULL
*/
schedpart_t *schedpart_lookup_spid(part_id_t spid)
{
	schedpart_t *spart;

	if (!KerextAmInKernel()) {
		spart = QueryObject(_QUERY_SCHEDULER_PARTITION, SPINDEX(spid), 0/*_QUERY_SCHEDULER_PARTITION*/, 0, 0, 0, 0);
	}
	else {
		spart = vector_lookup(&schedpart_vector, SPINDEX(spid));
	}

	/* only allow a unique id match */
	return ((spart != NULL) && (spart->info.id == spid)) ? spart : NULL;
}

/*******************************************************************************
 * schedpart_vec_add
 * 
 * add <spart> to the 'schedpart_vector'
 * Called when a partition is created (procnto thread) so we must enter the
 * kernel since vector is queried from kernel
 * 
 * Returns: a 'part_id_t'
*/
struct kerargs_schedpart_vec
{
	part_id_t spid;
	schedpart_t *spart;
};

static void kerext_schedpart_vec_add(void *data)
{
	struct kerargs_schedpart_vec	*kap = data;
	int spid = vector_add(&schedpart_vector, kap->spart, 0);
	kap->spid = (spid < 0) ? part_id_t_INVALID : (part_id_t)spid;
}

part_id_t schedpart_vec_add(schedpart_t *spart)
{
	struct kerargs_schedpart_vec  data;
	CRASHCHECK(spart == NULL);
	data.spart = spart;

	if (KerextAmInKernel()) {
		kerext_schedpart_vec_add(&data);
	}
	else {
		__Ring0(kerext_schedpart_vec_add, &data);
	}

	if (data.spid != part_id_t_INVALID) data.spid |= (spid_unique | parttype_SCHED);

	return data.spid;
}

/*******************************************************************************
 * schedpart_vec_del
 * 
 * delete <spid> from the 'schedpart_vector'
 * Called when a partition is destroyed (procnto thread) so we must enter the
 * kernel since vector is queried from kernel
 * 
 * Returns: n/a
*/
static void kerext_schedpart_vec_del(void *data)
{
	struct kerargs_schedpart_vec *kap = data;
	kap->spart = vector_rem(&schedpart_vector, kap->spid);
}

schedpart_t *schedpart_vec_del(part_id_t spid)
{
	struct kerargs_schedpart_vec  data;
	CRASHCHECK(KerextAmInKernel());
	if (spid == part_id_t_INVALID) return NULL;
	data.spid = SPINDEX(spid);

	__Ring0(kerext_schedpart_vec_del, &data);
	return data.spart;
}

/*
 * ===========================================================================
 * 
 * 					Scheduler Partition interface routines
 *  
 * The following routines implement the scheduler partitioning interface for the
 * case when the scheduler partitioning module is not installed. The exception are
 * the process association and disassociation functions which are effectively
 * used by both. The implementations in this file are called out through
 * wrappers in the partitioning module when it is installed (which perform
 * additional things like event notification).
 * 
 * They are exposed via the 'schedpart_fncTbl' and in conjunction with
 * macros of the same name, provide the scheduler partitioning functionality in a format
 * which is optional. 
 *  
 * ===========================================================================
*/
/*
 * When the scheduler partitioning module is installed, <lf> will point to the
 * implementation specific lock/unlock functions in that module. Otherwise
 * <lf> will be NULL because we don't need to lock/unlock when no partitioning
 * is installed since process association/disassociation in that case is only
 * done from kerext_process_create() and kerext_process_destroy()
 * FIX ME - I don't think this is TRUE
*/
/* FIX ME - no need yet since no process association
#define MUTEX_RDLOCK(l)		((lf == NULL) ? EOK : lf->rdlock((l)))
#define MUTEX_RDUNLOCK(l)	((lf == NULL) ? EOK : lf->rdunlock(l))
#define MUTEX_WRLOCK(l)		((lf == NULL) ? EOK : lf->wrlock((l)))
#define MUTEX_WRUNLOCK(l)	((lf == NULL) ? EOK : lf->wrunlock(l))
*/

/*******************************************************************************
 * schedpart_proc_associate
 * 
 * Associate process <prp> with the scheduler partition <schedpart_id>.
 * A process can only be associated with 1 partition of a given scheduler class.
 * Attempts to associate with an already associated partition will return
 * EALREADY.
 * 
 * HEAP structure creation is controlled by the <flags> argument. This allows
 * the caller to specify whether or not a HEAP is considered necessary. Some
 * scheduler classes may not require a HEAP since it is known that no internal
 * allocations will be made to that scheduler class. In this case, creation of a
 * HEAP would be wasteful requiring that at least 1 page of the scheduler class be
 * allocated just to hold the HEAP structure which would go unused.
 * For this case, the 'schedpart_node_t' structure will be allocated from the
 * processes 'sysram' heap and have its HEAP pointer set to NULL (ie no heap
 * for the scheduler class even though it has partition association with the class)
 * 
 * All of the partition structures required for process association, currently
 * use the sysram scheduler class for their allocations hence when associating with
 * the sysram scheduler class, it is expected that a HEAP be either shared or created
 * (ie. flags & schedpart_flags_HEAP_CREATE | schedpart_flags_HEAP_SHARE) since we
 * know that most (if not all) internal scheduler allocations will be made from the
 * sysram heap for the process. See also schedpart.h
 * 
 * Returns EOK or an errno
*/
static int schedpart_proc_associate(PROCESS *prp, part_id_t schedpart_id,
									schedpart_dcmd_flags_t flags, ext_lockfncs_t *lf)
{
	schedpart_t  *schedpart = SCHEDPART_ID_TO_T(schedpart_id);
	schedpart_node_t  *spart_node;

	CRASHCHECK(schedpart == NULL);

	if (schedpart == NULL)
	{
		return EINVAL;
	}
	else if ((spart_node = SCHEDPART_NODEGET(prp)) != NULL)
	{
		/*
		 * a partition of <schedclass_id> is already associated with this process.
		 * Return EALREADY if it's the same partition (allowing the caller to
		 * determine if it thinks this is an error) or EEXIST if it's a different
		 * partition (of the same scheduler class) which is an error
		*/
		return (spart_node->schedpart == schedpart) ? EALREADY : EEXIST;
	}
	else
	{
#ifdef USE_PROC_OBJ_LISTS
		prp_node_t  *prp_node;
#endif	/* USE_PROC_OBJ_LISTS */

		if ((spart_node = calloc(1, sizeof(*spart_node))) == NULL)
		{
			return ENOMEM;
		}

		/* finish initializing the spart_node_t structure (all other fields are NULL) */
		spart_node->schedpart = schedpart;
		spart_node->flags = flags;

#ifdef USE_PROC_OBJ_LISTS
		if ((prp_node = calloc(1, sizeof(*prp_node))) == NULL)
		{
			free(spart_node);
			return ENOMEM;
		}
#endif	/* USE_PROC_OBJ_LISTS */

		/* associate 'spart_node' to the the process (can only be one) */
		(void)MUTEX_WRLOCK(&prp->spartlist_lock.mutex);
		prp->spart_list = (void *)spart_node;
		(void)MUTEX_WRUNLOCK(&prp->spartlist_lock.mutex);

#ifdef USE_PROC_OBJ_LISTS
		/* add 'prp' to the process list for the partition */
		prp_node->prp = prp;
		
		(void)MUTEX_WRLOCK(&spart_node->schedpart->prplist_lock);
		LIST_ADD(spart_node->schedpart->prp_list, prp_node);
		CRASHCHECK(prp_list_audit(spart_node->schedpart->prp_list) != EOK);
		(void)MUTEX_WRUNLOCK(&spart_node->schedpart->prplist_lock);
#endif	/* USE_PROC_OBJ_LISTS */

		return EOK;
	}
}

/*******************************************************************************
 * schedpart_proc_disassociate
 * 
 * Remove the process <prp> from the process list for partition <schedpart_id>.
 * If <schedpart_id> is part_id_t_INVALID, the process is disassociated from
 * all partitions in its partition list.
 * Otherwise, the process is disassociated from partition <schedpart_id>.
 * _schedpart_proc_disassociate_1() actually does the work.
*/
static int schedpart_proc_disassociate(PROCESS *prp, part_id_t schedpart_id, ext_lockfncs_t *lf)
{
	CRASHCHECK(prp == NULL);

	/* allow the caller to do a cleanup even if no partitions were associated */
	if ((schedpart_id == part_id_t_INVALID) && (prp->spart_list == NULL)) {
		return EOK;
	}

	/* disassociate from a specific partition id ? */
	if (schedpart_id != part_id_t_INVALID) {
		return schedpart_proc_disassociate_1(prp, SCHEDPART_ID_TO_T(schedpart_id), lf);
	}
	else
	{
		/* remove the process from all of its associated partitions */
		int  ret = EOK;
		schedpart_node_t *sp;

		while ((sp = prp->spart_list) != NULL)
		{
			int  r;
			r = schedpart_proc_disassociate_1(prp, sp->schedpart, lf);
			ret = ((r != EOK) ? r : ret);
		}
		return ret;
	}
}

/*******************************************************************************
 * schedpart_getlist
 * 
 * This is a proxy handler for the case when the scheduler partitioning module is
 * not installed. It minimally replicates the behaviour of the
 * _schedpart_getlist() function in aps.c.
 * 
 * Since there are no actual partitions, this function always returns the
 * same thing
*/
static int schedpart_getlist(PROCESS *prp, part_list_t *spart_list, int n,
							schedpart_flags_t flags, struct _cred_info *cred)
{
	schedpart_node_t *sp_node;
	unsigned  n_sparts, i;
	unsigned int  filtered = 0;

	CRASHCHECK(!SCHEDPART_INSTALLED());

	if ((spart_list == NULL) && (n == 0)) {
		/* caller just wants the (max) number of associated partitions for <prp> */
		/* FIX ME - consider allowing the filters to be factored into the return value */
		return 1;
	}

	CRASHCHECK(spart_list == NULL);

	spart_list->num_entries = 0;
	(void)MUTEX_RDLOCK(&prp->spartlist_lock.mutex);

	n_sparts = 1;

	for (i=0; i<n_sparts; i++)
	{
		if (spart_list->num_entries >= n) break;

		sp_node = (schedpart_node_t *)prp->spart_list;
		if (sp_node == NULL) continue;

		++i;	// this is a valid entry

		/* filter out entries based on inheritablility ? */
		if ((flags & schedpart_flags_t_GETLIST_INHERITABLE) &&
			(sp_node->flags & part_flags_NO_INHERIT))
		{
			++filtered;
			continue;
		}
		/* filter out entries based on credentials ? */
		if ((flags & schedpart_flags_t_GETLIST_CREDENTIALS) && (cred != NULL) &&
			(schedpart_validate_association(sp_node->schedpart, cred) != EOK))
		{
			++filtered;
			continue;
		}

		/* otherwise return this entry */
		spart_list->i[spart_list->num_entries].id = SCHEDPART_T_TO_ID(sp_node->schedpart);
		spart_list->i[spart_list->num_entries++].flags = sp_node->flags;
	}
	MUTEX_RDUNLOCK(&prp->spartlist_lock.mutex);
	
	return (n_sparts - (spart_list->num_entries + filtered));
}

/*******************************************************************************
 * schedpart_nodeget
 * 
 * This is a proxy handler for the case when the scheduler partitioning module is
 * not installed. It minimally replicates the behaviour of the _schedpart_get()
 * function in aps.c.
 * 
 * Since there are no actual partitions, this function always returns the
 * same thing
*/
static schedpart_node_t *schedpart_nodeget(PROCESS *prp)
{
//	CRASHCHECK(!SCHEDPART_INSTALLED());
	CRASHCHECK(prp == NULL);

	return prp->spart_list;
}

/*******************************************************************************
 * schedpart_validate_association
 * 
 * This API is used to determine whether or not the partition identified by
 * 'part_id_t' <mpid> can be associated with based on the 'struct _cred_info' <c>.
 * 
 * Returns: EOK if association is permitted, otherwise and errno.
 * 
 * If the resource manager module is not installed, EOK will be returned since
 * associations control is meaningless
 *
*/
static int schedpart_validate_association(schedpart_t *spart, struct _cred_info *cred)
{
	int r = EOK;

	CRASHCHECK(spart == NULL);
	CRASHCHECK(cred == NULL);

	if ((rsrcmgr_schedpart_fnctbl != NULL) && (rsrcmgr_schedpart_fnctbl->validate_association != NULL)) {
		r = rsrcmgr_schedpart_fnctbl->validate_association(spart->rmgr_attr_p, cred);
	}
	return r;
}
static int _schedpart_validate_association(part_id_t spid, struct _cred_info *cred)
{
	return schedpart_validate_association(SCHEDPART_ID_TO_T(spid), cred);
}

/* remove this when we no longer support WATCOM, or WATCOM supports inlines */
#if (defined(__WATCOMC__) || !defined(NDEBUG))
void *_select_dpp(PROCESS *prp, part_id_t spid) 
{
	if (!SCHEDPART_INSTALLED() || !apmgr_module_installed() || (schedpart_select_dpp == NULL)) {
		return NULL;
	} else {
		schedpart_t *spart = SCHEDPART_ID_TO_T(spid);
		CRASHCHECK(spart == NULL);
	
		return schedpart_select_dpp(prp, spart->info.info.id);
	}
}
#endif	/* (defined(__WATCOMC__) || !defined(NDEBUG)) */

/* ===========================================================================
 * 
 * 								support routines
 *  
 * ===========================================================================
*/


/*******************************************************************************
 * schedpart_disassociate_1
 * 
 * Disassociate process <prp> from the partition <spart>
 * 
 * Returns: EOK on success, otherwise and errno
*/
static int schedpart_proc_disassociate_1(PROCESS *prp, schedpart_t *spart, ext_lockfncs_t *lf)
{
	schedpart_node_t *n;

	CRASHCHECK(spart == NULL);
	CRASHCHECK(prp == NULL);

#ifdef USE_PROC_OBJ_LISTS
	/* remove <prp> from the process list for the partition */
	(void)MUTEX_WRLOCK(&spart->prplist_lock);
	{
		prp_node_t *pp = (prp_node_t *)LIST_FIRST(spart->prp_list);
		prp_node_t *nn = pp;

		while ((nn != NULL) && (nn->prp != prp))
		{
			pp = nn;
			nn = (prp_node_t *)LIST_NEXT(nn);
		}
		if (nn != NULL)
		{
			LIST_DEL(spart->prp_list, nn);
			CRASHCHECK(prp_list_audit(spart->prp_list) != EOK);
			(void)MUTEX_WRUNLOCK(&spart->prplist_lock);
			free(nn);
  		}
  		else	/* prp not found */
  		{
  			(void)MUTEX_WRUNLOCK(&spart->prplist_lock);
#ifndef NDEBUG
			crash();
#endif	/* NDEBUG */
  		}
	}
#endif	/* USE_PROC_OBJ_LISTS */

	/* remove <spart> from the partition list for the process */
	(void)MUTEX_WRLOCK(&prp->spartlist_lock.mutex);
	n = (schedpart_node_t *)prp->spart_list;
	prp->spart_list = NULL;
	(void)MUTEX_WRUNLOCK(&prp->spartlist_lock.mutex);
	free(n);

	return EOK;
}


/*******************************************************************************
 * proxy_schedpart_init
 * 
 * This is a proxy handler for the case when the scheduler partitioning module is
 * not installed. It minimally replicates the behaviour of the
 * _schedpart_init() function in aps.c.
 * 
*/
static void proxy_schedpart_init(part_id_t internal_sys_spid, sched_aps_partition_info *aps_info)
{
	static schedpart_fnctbl_t private_schedpart_fnctbl;
	part_id_t  sys_spid;

	/* can only be called once */	
	if ((sys_schedpart != NULL) || (internal_sys_spid == part_id_t_INVALID))
	{
		kprintf("Unable to create system partition, sys_schedpart: 0x%x, sys_schedpart_id: 0x%x\n",
				sys_schedpart, internal_sys_spid);
		crash();
	}
	/*
	 * for APS, the create_default_dispatch() has been called to create the
	 * system partition. Since this was not done via schedpart_create(), the
	 * necessary data structures don't exist in apmgr. The BEST way to fix this
	 * is to have the system partition created through schedpart_create() however
	 * that would call create_ppg() not create_default_dispatch(). I have not
	 * dissected the APS code yet and am not sure why the create_ppg() call could
	 * not have been used to create the system partiton but hopefully there was
	 * a good reason.
	 * Another way to accomplish this is to do a fake creation however the problem
	 * with that is we don't need '_sys_schedpart' allocated (its static).
	 * The last alternative (which I choose at this point) is to replicate the
	 * parts of schedpart_create() I need in this routine.
	 * 'System' partition info and
	*/
	sys_spid = i_schedpart_create(part_id_t_INVALID, NULL, NULL, internal_sys_spid);
	sys_schedpart = SCHEDPART_ID_TO_T(sys_spid);
	CRASHCHECK(sys_schedpart == NULL);
	CRASHCHECK(sys_spid != sys_schedpart->info.id);
	CRASHCHECK(internal_sys_spid != sys_schedpart->info.info.id);

	/* synchronize some fields */
	sys_schedpart->info.info = *aps_info;
	sys_schedpart->info.cre_cfg.pinfo_flags = aps_info->pinfo_flags;
	sys_schedpart->info.cre_cfg.attr.budget_percent = aps_info->budget_percent;
	//sys_schedpart->info.cre_cfg.attr.critical_budget_ms = 

	/* set cur_cfg == cre_cfg without messing up the policy flags */
	sys_schedpart->info.cur_cfg.attr = sys_schedpart->info.cre_cfg.attr;
	sys_schedpart->info.cur_cfg.pinfo_flags = sys_schedpart->info.cre_cfg.pinfo_flags;

	/*
	 * this private table is required because the install checks for
	 * schedpart_fnctbl == private_schedpart_fnctbl
	*/
	private_schedpart_fnctbl = proxy_schedpart_fnctbl;
	schedpart_fnctbl = &private_schedpart_fnctbl;
}

/*
 * ===========================================================================
 * 
 * 								support routines analogous
 * 									to memmgr/apm/apm.c
 * 					This is where we shim and interface with existing aps code
 *  
 * ===========================================================================
*/

/*******************************************************************************
 * _schedpart_create
 * 
 * Normally, partition creation is handled via the open call and there is no
 * means of providing the creation parameters so just create a '0' budget
 * partition. It will be modified after it is created.
 * If a valid <child_cfg> is provided, we do handle it
 * 
 * Unfortunately because the original aps code stores a name internally that
 * cannot be changed afterwards. I could add a new SchedCtl() that allowed the
 * name to be changed, but this is easier.
*/
static part_id_t _schedpart_create(part_id_t parent_spid, const char *name, schedpart_cfg_t *child_cfg)
{
	return i_schedpart_create(parent_spid, name, child_cfg, part_id_t_INVALID);
}

/*******************************************************************************
 * i_schedpart_create
 *
 * internal partition creation implementation with the added functionality
 * required for initialization as follows. 
 * If <spid> is != part_id_t_INVALID then the actual partition creation via
 * SchedCtl() is not performed and the internal (APS known) id should be <spid>.
 * All other aspects of schedpart_create() are performed.
 * Note that this feature is only used at initialization time in order to
 * synchronize with APS internal data structures for the System partition. 
 * 
 * A note on partition id's
 * The externally visible partition id is held in 'schedpart_info_t.id'
 * (see schedpart.h) and this id is generated based on the uniqueness provided
 * by the same mschanism as for memory partitions and process id's. The current
 * implementation of APS, has its own internal (non vectorized) id. This 'id'
 * WILL NOT be made externally visible and is only used as a cross reference
 * for interacting with the current APS user interface.
*/
/* note re create_key
 * there is nothing special about the 'parttype_SCHED' value being OR'd in other
 * than it differentiates the starting value for this 'create_key' from the apm.c
 * 'create_key' (useful during debug). The initial value of LSb=0b1 IS however
 * important and in conjunction with the increment of '2' ensures that we never
 * have a value of '0', which is regarded as special (ie. it's the "no key" value)
*/ 
static unsigned create_key = 1 | parttype_SCHED;
static part_id_t i_schedpart_create(part_id_t parent_spid, const char *name, schedpart_cfg_t *child_cfg, part_id_t spid)
{
	schedpart_t *schedpart;
	schedpart_t *parent = SCHEDPART_ID_TO_T(parent_spid);

	if (_validate_cfg_creation(parent_spid, child_cfg) != EOK)
	{
		return part_id_t_INVALID;
	}
	else if ((schedpart = malloc(sizeof(*schedpart))) == NULL)
	{
		return part_id_t_INVALID;
	}
	else
	{
		sched_aps_create_parms  aps_create_parms;

		/*
		 * initialize the state of the new partition. Use default policies and
		 * attributes so that the partition can be associated with but is
		 * otherwise not usable unless valid configuration data has been provided
		 * to override the default
		*/
		memset(schedpart, 0, sizeof(*schedpart));
		if (MUTEX_INIT(&schedpart->info_lock, NULL) != EOK)
		{
			free(schedpart);
			return part_id_t_INVALID;
		}
#ifdef USE_PROC_OBJ_LISTS
		if (MUTEX_INIT(&schedpart->prplist_lock, NULL) != EOK)
		{
			MUTEX_DESTROY(&schedpart->info_lock);
			free(schedpart);
			return part_id_t_INVALID;
		}
#endif	/* USE_PROC_OBJ_LISTS */

		/*
		 * wait until the locks are initialized since once added into the
		 * 'schedpart_vector', 'schedpart' will be accessible, hence the lock.
		 * We need the partition id before we call schedpart_config and before
		 * sending any events
		*/
		(void)MUTEX_WRLOCK(&schedpart->info_lock);
		if ((schedpart->info.id = schedpart_vec_add(schedpart)) == part_id_t_INVALID)
		{
			free(schedpart);
			return part_id_t_INVALID;
		}

		schedpart->info.cre_cfg.policy = schedpart->info.cur_cfg.policy = SCHEDPART_DFLT_POLICY;
		/*
		 * Generate the policy keys based on the currently selected policies. This
		 * must be done before calling _schedpart_config() 
		*/
		SET_SPART_POLICY_TERMINAL_KEY(&schedpart->info.cre_cfg);
		SET_SPART_POLICY_CFG_LOCK_KEY(&schedpart->info.cre_cfg);
		SET_SPART_POLICY_PERMANENT_KEY(&schedpart->info.cre_cfg);
		SET_SPART_POLICY_TERMINAL_KEY(&schedpart->info.cur_cfg);
		SET_SPART_POLICY_CFG_LOCK_KEY(&schedpart->info.cur_cfg);
		SET_SPART_POLICY_PERMANENT_KEY(&schedpart->info.cur_cfg);

		schedpart->signature = SCHEDPART_SIGNATURE;
		if ((schedpart->parent = parent) != NULL) {
			atomic_add(&parent->info.num_children, 1);
		}

		if (spid == part_id_t_INVALID)
		{
			CRASHCHECK(parent == NULL);
			APS_INIT_DATA(&aps_create_parms);	//initialize parameter block. Must be done
			aps_create_parms.name = (char *)name;
			aps_create_parms.parent_id = parent->info.info.id;	// use parents internal id
			aps_create_parms.aps_create_flags |= APS_CREATE_FLAGS_USE_PARENT_ID;

			if ((name == NULL) || (*name == '\0') ||
				(SchedCtl(SCHED_APS_CREATE_PARTITION, &aps_create_parms, sizeof(aps_create_parms)) != EOK))
			{
				(void)schedpart_vec_del(schedpart->info.id);
				MUTEX_DESTROY(&schedpart->info_lock);
#ifdef USE_PROC_OBJ_LISTS
				MUTEX_DESTROY(&schedpart->prplist_lock);
#endif	/* USE_PROC_OBJ_LISTS */
				free(schedpart);
				return part_id_t_INVALID;
			}
			/*
			 * we don't call _schedpart_getinfo() to sync internal data structures
			 * because we don't have a recursive (schedpart->info_lock) so we issue
			 * the SchedCtl(SCHED_APS_QUERY_PARTITION) directly. We only need the
			 * schedpart->info.info data anyway.
			 * Note that the query needs schedpart->info.info.id to contain the
			 * correct (internal) 'id' so that the information can be retrieved.
			 * The 'id' for the newly created partition was returned in the
			 * 'aps_create_parms' 
			*/
			CRASHCHECK(aps_create_parms.id <= 0);	// cannot be the system partition (0)
			APS_INIT_DATA(&schedpart->info.info);	//initialize parameter block. Must be done
			schedpart->info.info.id = aps_create_parms.id;
			if (SchedCtl(SCHED_APS_QUERY_PARTITION, &schedpart->info.info, sizeof(schedpart->info.info)) != EOK)
			{
				/*
				 * cannot destroy APS partitions internally so we now have an inconsistency but
				 * we do the best we can
				*/
				(void)schedpart_vec_del(schedpart->info.id);
				MUTEX_DESTROY(&schedpart->info_lock);
#ifdef USE_PROC_OBJ_LISTS
				MUTEX_DESTROY(&schedpart->prplist_lock);
#endif	/* USE_PROC_OBJ_LISTS */
				free(schedpart);
				return part_id_t_INVALID;
			}
			CRASHCHECK(schedpart->info.info.id != aps_create_parms.id);	// they better still be the same
		}
		else
		{
			/* set internal id to spid (note that no other fields are set) */
			schedpart->info.info.id = spid;
		}

		if (child_cfg != NULL)
		{
			if (_schedpart_config(schedpart->info.id, child_cfg, 0) != EOK)
			{
				(void)schedpart_vec_del(schedpart->info.id);
				MUTEX_DESTROY(&schedpart->info_lock);
#ifdef USE_PROC_OBJ_LISTS
				MUTEX_DESTROY(&schedpart->prplist_lock);
#endif	/* USE_PROC_OBJ_LISTS */
				free(schedpart);
				return part_id_t_INVALID;
			}
		}
		else
		{
			/* get a unique number */
			schedpart->create_key = atomic_add_value(&create_key, 2);
			CRASHCHECK(schedpart->create_key == 0);
		}
		MUTEX_WRUNLOCK(&schedpart->info_lock);

		/* FIX ME - send event notifications to any interested parties ??? */
		return schedpart->info.id;
	}
}

/*******************************************************************************
 * _schedpart_destroy
 * 
 * FIX ME - APS doesn't support this concept yet.
 * 			Don't forget to subtract num_children when/if this is implemented
*/
static int _schedpart_destroy(part_id_t spid)
{
	return ENOSYS;
}

/*******************************************************************************
 * _schedpart_config
 * 
*/
static int _schedpart_config(part_id_t spid, schedpart_cfg_t *cfg, unsigned key)
{
	schedpart_t *spart = SCHEDPART_ID_TO_T(spid);
	int r;

	CRASHCHECK(spart == NULL);
	CRASHCHECK(cfg == NULL);

	(void)MUTEX_WRLOCK(&spart->info_lock);

	if ((r = _validate_cfg_modification(spid, cfg)) == EOK)
	{
		sched_aps_modify_parms  aps_mod_parms;

		APS_INIT_DATA(&aps_mod_parms);	//initialize parameter block. Must be done
		aps_mod_parms.id = spart->info.info.id;	// use the internal id
		aps_mod_parms.new_budget_percent = cfg->attr.budget_percent;
		aps_mod_parms.new_critical_budget_ms = cfg->attr.critical_budget_ms;

		if (SchedCtl(SCHED_APS_MODIFY_PARTITION, &aps_mod_parms, sizeof(aps_mod_parms)) == EOK)
		{
			spart->info.cur_cfg = *cfg;
			SET_SPART_POLICY_TERMINAL_KEY(&spart->info.cur_cfg);
			SET_SPART_POLICY_CFG_LOCK_KEY(&spart->info.cur_cfg);
			SET_SPART_POLICY_PERMANENT_KEY(&spart->info.cur_cfg);

			if (aps_mod_parms.new_budget_percent != -1) {
				spart->info.cur_cfg.attr.budget_percent = aps_mod_parms.new_budget_percent;
			}
			if (aps_mod_parms.new_critical_budget_ms != -1) {
				spart->info.cur_cfg.attr.critical_budget_ms = aps_mod_parms.new_critical_budget_ms;
			}
				
			if ((spart->create_key != 0) && (spart->create_key == key))
			{
				/* this is an initial creation */
				spart->info.cre_cfg = *cfg;
				SET_SPART_POLICY_TERMINAL_KEY(&spart->info.cre_cfg);
				SET_SPART_POLICY_CFG_LOCK_KEY(&spart->info.cre_cfg);
				SET_SPART_POLICY_PERMANENT_KEY(&spart->info.cre_cfg);
			}	
		}
		else
		{
			/* FIX ME - have to assume EINVAL for now as SchedCtl() is not setting errno
			r = errno; */
			r = EINVAL;
		}
		spart->create_key = 0;
	}
	MUTEX_WRUNLOCK(&spart->info_lock);
	/* FIX ME - events ? */
	return r;
}

/*******************************************************************************
 * _schedpart_getinfo
 * 
*/
static schedpart_info_t *_schedpart_getinfo(part_id_t spid, schedpart_info_t *info)
{
	schedpart_t *spart = SCHEDPART_ID_TO_T(spid);

	CRASHCHECK(spart == NULL);
	CRASHCHECK(info == NULL);
	CRASHCHECK(spart->info.id != spid);

	if (MUTEX_RDLOCK(&spart->info_lock) != EOK) {
		return NULL;
	} else {
		/*
		 * we pass in a tmp 'sched_aps_partition_info/stats' structures in case
		 * the call fails we don't destroy our existing information
		*/
		sched_aps_partition_info  aps_info;
		sched_aps_partition_stats  aps_stats;
		APS_INIT_DATA(&aps_info);	//initialize parameter block. Must be done
		APS_INIT_DATA(&aps_stats);	//initialize parameter block. Must be done
		aps_info.id = aps_stats.id = spart->info.info.id;	// need the internal id
	
		if ((SchedCtl(SCHED_APS_QUERY_PARTITION, &aps_info, sizeof(aps_info)) != EOK) ||
			(SchedCtl(SCHED_APS_PARTITION_STATS, &aps_stats, sizeof(aps_stats)) != EOK))
		{
			MUTEX_RDUNLOCK(&spart->info_lock);
			info = NULL;
		} else {
			CRASHCHECK(aps_info.id != spart->info.info.id);	// make sure they're still the same
			CRASHCHECK(aps_stats.id != spart->info.info.id);	// make sure they're still the same
			/* synchronize some fields */
			spart->info.info = aps_info;
			spart->info.cur_cfg.pinfo_flags = spart->info.info.pinfo_flags;
			spart->info.cur_cfg.attr.budget_percent = spart->info.info.budget_percent;
			//spart->info.cur_cfg.attr.critical_budget_ms = spart->info.info.?;
			spart->info.stats = aps_stats;

			/* fill in the caller provided buffer */
			*info = spart->info;

			/* get the external (bool_t) representation of the policy settings */
			info->cre_cfg.policy.terminal = GET_SPART_POLICY_TERMINAL(&spart->info.cre_cfg);
			info->cre_cfg.policy.config_lock = GET_SPART_POLICY_CFG_LOCK(&spart->info.cre_cfg);
			info->cre_cfg.policy.permanent = GET_SPART_POLICY_PERMANENT(&spart->info.cre_cfg);
			info->cur_cfg.policy.terminal = GET_SPART_POLICY_TERMINAL(&spart->info.cur_cfg);
			info->cur_cfg.policy.config_lock = GET_SPART_POLICY_CFG_LOCK(&spart->info.cur_cfg);
			info->cur_cfg.policy.permanent = GET_SPART_POLICY_PERMANENT(&spart->info.cur_cfg);

			MUTEX_RDUNLOCK(&spart->info_lock);
		}
		return info;
	}
}

/*******************************************************************************
 * _schedpart_find_pid
 * 
 * determine if the process id <pid> is associated with the scheduler partition
 * identified by <spid>
 * 
 * FIX ME - can possibly make this faster by checking to see if the process has
 * 			the partition associated instead of whether the partition has the
 * 			process in its list
 * 
 * Returns: a PROCESS pointer if found, NULL otherwise
 * 
*/
static PROCESS *_schedpart_find_pid(pid_t pid, part_id_t spid)
{
	PROCESS *prp;

	if ((prp = proc_lookup_pid(pid)) != NULL)
	{
#if 1	// this is the FIX ME - not implemented in apm.c yet
		schedpart_node_t  *sp_node;

		(void)MUTEX_RDLOCK(&prp->spartlist_lock.mutex);
		sp_node = (schedpart_node_t *)prp->spart_list;
		if ((sp_node == NULL) || (sp_node->schedpart->info.id != spid)) {
			MUTEX_RDUNLOCK(&prp->spartlist_lock.mutex);
			prp = NULL;
		} else {
			MUTEX_RDUNLOCK(&prp->spartlist_lock.mutex);
		}
#else
		prp_node_t *prp_node;
		schedpart_t *schedpart = SCHEDPART_ID_TO_T(spid);

		CRASHCHECK(schedpart == NULL);
		MUTEX_RDLOCK(&schedpart->prplist_lock);
		prp_node = (prp_node_t *)LIST_FIRST(schedpart->prp_list);

		while (prp_node != NULL)
		{
			if (prp_node->prp == prp) {
				break;
			}
			prp_node = (prp_node_t *)LIST_NEXT(prp_node);
		}
		MUTEX_RDUNLOCK(&schedpart->prplist_lock);
		if (prp_node == NULL) prp = NULL;
#endif
	}
	return prp;
}

/*******************************************************************************
 * _validate_cfg_creation
 * 
*/
static int _validate_cfg_creation(part_id_t parent_spid, schedpart_cfg_t *cfg)
{
	schedpart_t *parent_sp = SCHEDPART_ID_TO_T(parent_spid);

	if (cfg != NULL)
	{
		/* assert config attribute budget falls within 0 and 100 % */
		if (cfg->attr.budget_percent > 100) {
			return EINVAL;
		}

		/* assert config attribute critical budget falls within > 0 */
		if (cfg->attr.critical_budget_ms < 0) {
			return EINVAL;
		}
	}
	
	if (parent_sp != NULL)
	{
		/* assert that the parent partition may have children */
		if (GET_SPART_POLICY_TERMINAL(&parent_sp->info.cur_cfg) == bool_t_TRUE) {
			return EPERM; 
		}
	
		/* the following are checks validate the config against the parent partition */
		if (cfg != NULL)
		{

		}
	}
	return EOK;
}

/*******************************************************************************
 * _validate_cfg_modification
 * 
*/
static int _validate_cfg_modification(part_id_t spid, schedpart_cfg_t *cfg)
{
	schedpart_t *sp = SCHEDPART_ID_TO_T(spid);
	
	/* assert modifying a valid partition */
	if (sp == NULL) {
		return ENOENT;
	}
	
	if (cfg == NULL) {
		return EINVAL;
	}

	/* assert that if the config lock policy is set, it cannot be changed */
	if (GET_SPART_POLICY_CFG_LOCK(&sp->info.cur_cfg) == bool_t_TRUE) {
		if (cfg->policy.config_lock == bool_t_FALSE) {
			return EPERM;
		}
	}

	/* assert that if the terminal partition policy is set, it cannot be changed */
	if (GET_SPART_POLICY_TERMINAL(&sp->info.cur_cfg) == bool_t_TRUE) {
		if (cfg->policy.terminal == bool_t_FALSE) {
			return EPERM;
		}
	}

	/* assert that if the permanent partition policy is set, it cannot be changed */
	if (GET_SPART_POLICY_PERMANENT(&sp->info.cur_cfg) == bool_t_TRUE) {
		if (cfg->policy.permanent == bool_t_FALSE) {
			return EPERM;
		}
	}

	/* assert config attribute budget falls within 0 and 100 % */
	if (cfg->attr.budget_percent > 100) {
		return EINVAL;
	}

	/* assert config attribute critical budget falls within > 0 */
	if (cfg->attr.critical_budget_ms < 0) {
		return EINVAL;
	}

	/*
	 * assert that if attribute or other policy changes are being made
	 * that they are permitted (ie. that the config lock policy is not set)
	*/
	if ((cfg->attr.budget_percent != sp->info.cur_cfg.attr.budget_percent) ||
		(cfg->attr.critical_budget_ms != sp->info.cur_cfg.attr.critical_budget_ms)) {
		if (GET_SPART_POLICY_CFG_LOCK(&sp->info.cur_cfg) == bool_t_TRUE) {
			return EPERM;
		}
	}
	return EOK;
}

	
__SRCVERSION("aps.c $Rev: 209913 $");
