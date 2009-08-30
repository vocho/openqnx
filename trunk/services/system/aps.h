/* 
 * $QNXLicenseC:  
 * Copyright 2006, QNX Software Systems. All Rights Reserved.
 *
 * This source code may contain confidential information of QNX Software 
 * Systems (QSS) and its licensors.  Any use, reproduction, modification, 
 * disclosure, distribution or transfer of this software, or any software 
 * that includes or is based upon any of this code, is prohibited unless 
 * expressly authorized by QSS by written agreement.  For more information 
 * (including whether this source code file has been published) please
 * email licensing@qnx.com. $
*/

#ifndef _APS_H_
#define _APS_H_

#include <stdbool.h>

#ifdef __WATCOMC__
typedef char _Bool;	// not done in stdbool.h for watcom
#endif	/* __WATCOMC__ */ 

/*
 * this file contains external interfaces for the scheduler partitioning module
 * required outside of the scheduler manager and regardless of whether scheduler
 * partitioning is installed or not
*/

#include "ap.h"
// FIX ME #include <sys/schedpart.h>
#include <kernel/schedpart.h>
#include <kernel/event.h>

#ifndef EXTERN
	#define EXTERN	extern
#endif

/*
 * similar to process id's, scheduler partition id's are kept unique
 * Wrap will occur at SPID_UNIQUE_MASK
*/
#define SPID_MASK			0xfff
#define SPINDEX(spid)		((spid) & SPID_MASK)
#define SPID_UNIQUE_MASK	(UINT_MAX & ~parttype_MASK)
extern int spid_unique;

/*
 * schedpart_flags_t
 * 
*/
typedef enum
{
	/* flags used to filter returns from SCHEDPART_GETLIST()
	 *  The flags which can be specified are
	 * 	schedpart_flags_t_GETLIST_ALL			- no filtering, return all associated partitions
	 *	schedpart_flags_t_GETLIST_INHERITABLE	- filter out those partitions which have the
	 * 										  schedpart_flags_NO_INHERIT flag set
	 *	schedpart_flags_t_GETLIST_CREDENTIALS	- filter out those partitions which do not have
	 * 										  appropriate credentials
	*/
	schedpart_flags_t_GETLIST_ALL = 0,
	schedpart_flags_t_GETLIST_INHERITABLE = 0x01,
	schedpart_flags_t_GETLIST_CREDENTIALS = 0x02,
} schedpart_flags_t;

/*
 * schedpart_t
 * 
 * This is the internal scheduler partition structure. It is created when a scheduler
 * partition for a scheduler class is added to the system (typically through the
 * resource manager).
 * 
 * This structure contains all of the attributes and policies for a partition
 * of a given class of scheduler. This structure is all that is required in order
 * to maintain the accounting data and enforce scheduler partition policies when
 * a process attempts to allocate/deallocate scheduler for its objects.
 * 
 * A linked list of these 'schedpart_node_t' structures is maintained by each
 * process which contains a 'schedpart_t' pointer to this structure
 * 
 * The structure is referenced from the respective resource manager structure
 * representing the partition and (as previously stated) by all of the processes
 * which have associated with this partition.
 *
 * The structure maintains associated process and object lists (ie. pointers to
 * all of the processes/objects that reference this partition) as well as a parent
 * partition pointer (which will be NULL if this is a root partition).
 * In addition is event information which allows asynchronous events to be sent
 * to registered process when specified event conditions occur.  
*/
#define SCHEDPART_SIGNATURE		0x31911978

typedef struct schedpart_s
{
	_Uint32t		signature;		// used to help ensure not looking at a bogus schedpart_t

	struct schedpart_s  *parent;	// pointer to the partition from which this partition
								// was derived (for the purpose of establishing creation
								// attributes)
	//kerproc_lock_t	info_lock;	// protection for schedpart_info_t
	pthread_mutex_t		info_lock;	// FIX ME
	schedpart_info_t	info;		// (public) accounting and configuration data

#ifdef USE_PROC_OBJ_LISTS
	pthread_mutex_t	prplist_lock;	// FIX ME
//	kerproc_lock_t	prplist_lock;	// protects the 'prp_list' 
	part_qnodehdr_t	prp_list;		// associated process list
#endif	/* USE_PROC_OBJ_LISTS */

	void 			*rmgr_attr_p;	// back pointer to resource manager structure
	unsigned		create_key;		// used to establish initial partition creation configuration
} schedpart_t;

/*
 * schedpart_node_t
 * 
 * Per process partition structure.
 * When a process is associated with a partition, a 'schedpart_node_t' is created
 * to reference the associated partition as well as hold per process partition
 * information, such as flags and heap information.
 * The 'schedpart_node_t' added to the linked list of associated partitions for
 * the process. This also allows a partition to "reside" on several (process)
 * lists.
*/
typedef struct schedpart_node_s
{
	part_qnode_t			hdr;
	struct schedpart_s *	schedpart;
	schedpart_dcmd_flags_t	flags;		// per process partition flags
#if (_PADDR_BITS == 64) || defined(__PPC__)
	/* note that this conditionally compiled lock cannot be conditional on
	 * VARIANT_smp (even though it's only used in that case) because
	 * schedpart_node_t is used both inside and outside of modules and modules
	 * are always compiled for SMP. Also for reason described in kernel/types.h
	 * we must always have this variable for PPC family */
	struct intrspin 		lock;		// for atomic update of mem_used
#endif	/* (_PADDR_BITS == 64)  || defined(__PPC__)*/
} schedpart_node_t;

/*
 * rsrcmgr_schedpart_fnctbl_t
 * 
 * This type defines the table of interface functions provided by the scheduler
 * partitioning resource manager for use by the scheduler partitioning module.
*/
struct apsmgr_attr_s;
typedef struct rsrcmgr_schedpart_fnctbl_s
{
	int		(*validate_association)(struct apsmgr_attr_s *attr, struct _cred_info *cred);
	void	(*spart_event_handler)(struct apsmgr_attr_s *attr, evttype_t evtType, ...);

} rsrcmgr_schedpart_fnctbl_t;

/*
 * schedpart_rsrcmgr_fnctbl_t
 * 
 * This type defines the table of interface functions provided by the scheduler
 * partitioning module for use by the scheduler partitioning resource manager.
*/
typedef struct schedpart_rsrcmgr_fnctbl_s
{
	part_id_t 	(*create)(part_id_t parent_spid, const char *name, schedpart_cfg_t *child_cfg);
	int			(*destroy)(part_id_t spid);
	int			(*config)(part_id_t spid, schedpart_cfg_t *cfg, unsigned key);
	schedpart_info_t *	(*getinfo)(part_id_t spid, schedpart_info_t *info);
	PROCESS *	(*find_pid)(pid_t pid_filter, part_id_t spid);
	int			(*validate_cfg_new)(part_id_t parent_spid, schedpart_cfg_t *cfg);
	int			(*validate_cfg_change)(part_id_t spid, schedpart_cfg_t *cfg);
} schedpart_rsrcmgr_fnctbl_t;

/*
 * schedpart_fncTbl_t
 * 
 * This type defines the table of interface functions between the kernel
 * and the scheduler partitioning module. These functions (with the exception of
 * rsrcmgr_attach()), provide the means to account all allocations/deallocations
 * The rsrcmgr_attach() call is used by the partition resource manager code
 * to obtain the resource manager interfaces to the scheduler partitioning module.
 * It also allows the resource manager to supply a set of interface routines
 * for the scheduler partitioning module to use. 
*/
typedef struct schedpart_fnctbl_s
{
	int			(*associate)(PROCESS *prp, part_id_t spid, schedpart_dcmd_flags_t flags, ext_lockfncs_t *lf);
	int			(*disassociate)(PROCESS *prp, part_id_t, ext_lockfncs_t *lf);
	schedpart_node_t *	(*get_schedpart)(PROCESS *prp);
	int			(*get_schedpartlist)(PROCESS *prp, part_list_t *spart_list, int n, schedpart_flags_t flags, struct _cred_info *cred);
	int			(*validate_association)(part_id_t spid, struct _cred_info *cred); 
	schedpart_rsrcmgr_fnctbl_t *  (*rsrcmgr_attach)(rsrcmgr_schedpart_fnctbl_t *);
} schedpart_fnctbl_t;


extern schedpart_fnctbl_t *schedpart_fnctbl;
extern schedpart_fnctbl_t proxy_schedpart_fnctbl;
extern schedpart_dcmd_flags_t default_schedpart_flags;
extern schedpart_t  *sys_schedpart;
extern void (*schedpart_init)(part_id_t i_spid, sched_aps_partition_info *aps_info);
extern DISPATCH * (*schedpart_select_dpp)(PROCESS *prp, int id);

extern part_id_t schedpart_getid(PROCESS *prp);
extern memclass_id_t schedpart_get_classid(part_id_t spid);
extern int schedpart_validate_id(part_id_t spid);
extern schedpart_t *schedpart_lookup_spid(part_id_t spid);
extern schedpart_t *schedpart_vec_del(part_id_t spid);
extern part_id_t schedpart_vec_add(schedpart_t *spart);
extern bool schedpart_module_loaded(void);
extern bool apmgr_module_installed(void);


/*
 * FIX ME - this should be someplace common with apm
 * 
 * SET_SCHEDPART_POLICY_KEY
 * ISSET_SCHEDPART_POLICY_KEY
 * 
 * These macros are used to produce a unique 'key' for use in the internal
 * maintenance of schedpart_policy_t types. There are (currently) 3 boolean policies
 * 'terminal', permanent' and 'cfg_lock'. These policies are meant to provide an
 * enhanced level of configuration security and also have the property that once
 * set to their retrictive state, they cannot be changed from that state. In order
 * to achieve this, we do not want to rely on simple boolean values since it is
 * possible that an errant value could result in an unsecured state. Therefore,
 * a special key will be used to record the non restrictive state such that
 * modification can only occur if the key is present. Any other value results
 * in no possible modification.
 *
 * For example, the terminal partition policy specifies whether or not a partitiom
 * may be created as the child of an existing partition, TRUE being no, FALSE
 * being yes. The restrictive state of this policy is NO such that once set to
 * TRUE, the value cannot be changed and the partition is considered a terminal
 * partition for which no new children can be created. All values except the
 * key value will be taken to mean a value of TRUE, or no child configurations.
 * Only a valid key value will be taken as a policy value of FALSE, and hence
 * permit child partition creation.
 * 
 * ISSET_SCHEDPART_POLICY_KEY() will evaluate (v) against the key and return
 * TRUE or FALSE according to whether (v) is equivalent to the key or not.
 * 
 * SET_SCHEDPART_POLICY_KEY() will set (v) to the key depending on whether the
 * current value of (v) is (p) or not.
*/
#define ISSET_SCHEDPART_POLICY_KEY(v)			((v) == (ap_bool_t)(uintptr_t)&(v))
#define SET_SCHEDPART_POLICY_KEY(v, p)		(((v) == (p)) ? ((v) = (ap_bool_t)((uintptr_t)&(v))):0)

/*
 * SET/GET_SPART_POLICY_CFG_LOCKED
 * SET/GET_SPART_POLICY_TERMINAL
 * SET/GET_SPART_POLICY_PERMANENT
 * 
 * These 6 macros encapsulate the setting/retrieving of the internal keying
 * mechanism used to protect the policy states and translate between the internal
 * representation of the policy and the external (ap_bool_t) representation of the
 * boolean valued policies.
 * 
 * For all boolean valued policies, once they are set TRUE, they can never be
 * changed therefore we want to protect the TRUE state. This means that the key
 * value will be used to represent the FALSE state so that any other value except
 * the key value will be considered TRUE.
 * This protects against errant overwrites which if the key represented the TRUE
 * state would allow an overwrite to inadvertently change the state to FALSE. 
*/
#define GET_SPART_POLICY_CFG_LOCK(c)	(ISSET_SCHEDPART_POLICY_KEY((c)->policy.config_lock) ? bool_t_FALSE : bool_t_TRUE)
#define GET_SPART_POLICY_TERMINAL(c)	(ISSET_SCHEDPART_POLICY_KEY((c)->policy.terminal) ? bool_t_FALSE : bool_t_TRUE)
#define GET_SPART_POLICY_PERMANENT(c)	(ISSET_SCHEDPART_POLICY_KEY((c)->policy.permanent) ? bool_t_FALSE : bool_t_TRUE)

#define SET_SPART_POLICY_CFG_LOCK_KEY(c)	SET_SCHEDPART_POLICY_KEY((c)->policy.config_lock, bool_t_FALSE)
#define SET_SPART_POLICY_TERMINAL_KEY(c)	SET_SCHEDPART_POLICY_KEY((c)->policy.terminal, bool_t_FALSE)
#define SET_SPART_POLICY_PERMANENT_KEY(c)	SET_SCHEDPART_POLICY_KEY((c)->policy.permanent, bool_t_FALSE)


/*==============================================================================
 * 
 * 				public interfaces for scheduler partition management
 * 
*/

/*
 * SCHEDPART_INSTALLED
 * 
 * True or False depending on whether or not scheduler partitioning is installed
 * and initialized (SCHEDPART_INSTALLED) and whether the scheduler partitioning
 * module is loaded (SCHEDPART_MODULE_LOADED) respectively.
*/
#define SCHEDPART_INSTALLED()			((schedpart_fnctbl != NULL) && (schedpart_fnctbl != &proxy_schedpart_fnctbl))

/*
 * SCHEDPART_ASSOCIATE
 * 
 * Associate process <p> with partition identified by partition id <spid>
 * 
 * Returns: EOK if successful, otherwise errno
 * 
*/
#define SCHEDPART_ASSOCIATE(p, spid, f)	(SCHEDPART_INSTALLED() ? schedpart_fnctbl->associate((p), (spid), (f), NULL) : EOK)

/*
 * SCHEDPART_DISASSOCIATE
 * 
 * disassociate PROCESS <p> from the associated partition identified by partition
 * id <spid>. If <spid> == part_id_t_INVALID, the process will be disassociated
 * from all of its associated partitions (this is the situation when a process
 * is destroyed during process termination).
 * 
 * Returns: EOK if successful, otherwise errno
*/
#define SCHEDPART_DISASSOCIATE(p, spid)	(SCHEDPART_INSTALLED() ? schedpart_fnctbl->disassociate((p), (spid), NULL) : EOK)


/*
 * SCHEDPART_GETLIST
 * 
 * Retrieve the list of partitions associated with process <prp>.
 * This function will fill in the provided <mplist> for the process <p> filtering
 * the results based on the flags <f>.
 * The number of entries that can be filled in is specified by <n> and indicates
 * the space available in <mplist>
 * 
 * if the caller just wants the (max) number of associated partitions for <prp>
 * then <mplist> will be NULL and <n> will be 0. <f> and <c> are ignore in this
 * case
 * 
 * The flags which can be specified are
 * 		schedpart_flags_t_GETLIST_ALL			- no filtering, return all associated partitions
 * 		schedpart_flags_t_GETLIST_INHERITABLE	- filter out partitions which have the
 * 											  schedpart_flags_NO_INHERIT flag is set
 *		schedpart_flags_t_GETLIST_CREDENTIALS	- filter out partitions which do not have
 * 											  appropriate credentials as pointed to by <c>  
 * 
 * Returns: -1 on any error. If successful, the number of remaining partitions
 * 			which would not fit in <mplist> is returned. A return value of 0
 * 			indicates that all of the partitions are contained in <mplist>
*/
#define SCHEDPART_GETLIST(p, splist, n, f, c) \
		(SCHEDPART_INSTALLED() ? schedpart_fnctbl->get_schedpartlist((p), (splist), (n), (f), (c)) : 0)


/*
 * SCHEDPART_VALIDATE_ID
 * 
 * Validate a part_id_t coming in from user space
 * 
 * Returns: EOK is <spid> represents a valid scheduler partition identifier
 * 			otherwise an errno is returned
*/
#define SCHEDPART_VALIDATE_ID(spid) \
		(SCHEDPART_INSTALLED() ? schedpart_validate_id((spid)) : EINVAL)

/*
 * SCHEDPART_VALIDATE_ASSOCIATION
 * 
 * This API is used to determine whether or not the partition identified by
 * 'part_id_t' <spid> can be associated with based on the 'struct _cred_info' <c>.
 * 
 * Returns: EOK if association is permitted, otherwise and errno.
 * 
 * If the resource manager module is not installed, EOK will be returned since
 * associations control is meaningless
 *
*/
#define SCHEDPART_VALIDATE_ASSOCIATION(spid, c) \
		(SCHEDPART_INSTALLED() ? schedpart_fnctbl->validate_association((spid), (c)) : EINVAL)

/*
 * SCHEDPART_T_TO_ID
 * SCHEDPART_ID_TO_T
 * 
 * These macros translate between a part_id_t and a schedpart_t *
*/
#define SCHEDPART_T_TO_ID(s)	(((s) == NULL) ? part_id_t_INVALID : (s)->info.id)
#define SCHEDPART_ID_TO_T(i)	(((i) == part_id_t_INVALID) ? NULL : (schedpart_t *)schedpart_lookup_spid(i))

/*
 * SCHEDPART_NODEGET
 * 
 * This function will return a pointer to the 'schedpart_node_t' for the process
 * <p>
 * 
 * Returns: the partition pointer or NULL if no partition of the scheduler class
 * 			is associated with the process
*/
#define SCHEDPART_NODEGET(p)	(SCHEDPART_INSTALLED() ? schedpart_fnctbl->get_schedpart((p)) : NULL)

/*
 * SCHEDPART_SELECT_DPP
 * 
 * This function provides the mechanism by which a newly created process can be
 * associated with a specific scheduling partition via posix_spawn(). It is
 * called via SCHEDPART_SELECT_DPP() in kerext_process_create() with the partition
 * id <spid> for which the respective DISPATCH structure is to be selected.
 * 
 * If the DISPATCH structure could be selected, it will be returned (cast to void *) 
 * If the aps or apmgr modules are not installed, or any other error occurs, NULL
 * will be returned. 
*/
/* remove this when we no longer support WATCOM, or WATCOM supports inlines */
#if (defined(__WATCOMC__) || !defined(NDEBUG))
extern void *_select_dpp(PROCESS *prp, part_id_t spid);
#else	/* (defined(__WATCOMC__) || !defined(NDEBUG)) */
static __inline__ void *_select_dpp(PROCESS *prp, part_id_t spid) 
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
#define SCHEDPART_SELECT_DPP(p, spid)		_select_dpp((p), (spid))


/*
 * SCHEDPART_INIT
 * 
 * This function is called to initialize the scheduler partition module. There is
 * an module initialization function (initialize_aps()) which is called by
 * the module_init() which installs the (*schedpart_init)() function pointer used
 * by this macro. Therefore, if the module is not included, (*schedpart_init)()
 * will be NULL, and the module will not be initialized. This function will
 * install (*schedpart_fnctbl) and (*schedpart_rsrcmgr_fnctbl) function pointer tables
 * and effectively activate all aforementioned SCHEDPART macros.
 * See the comments in aps.c for initialize_aps() and _schedpart_init()
 * for rationale for this 2 stage initialization.
 * 
 * Since the default system scheduler partition is created at the time the
 * scheduler is initialized (see init_objects.c), and by the existing APS code,
 * this routine takes as its arguments, the internal 'id' of the created
 * system scheduler partition and a pointer to the internal partition info.
 * It will initialize any additional data structures required in order to sync
 * up and have the system operate as if the system scheduler partition was
 * created via a SCHEDPART_CREATE() call
 * 
 * Returns: (void) nothing
*/
#define SCHEDPART_INIT(i_spid, i)		if (schedpart_init != NULL) schedpart_init((i_spid), (i))





#endif	/* _APS_H_ */

/* __SRCVERSION("$IQ: aps.h,v 1.91 $"); */
