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

#ifndef _APM_INTERNAL_H_
#define _APM_INTERNAL_H_


#include "externs.h"

#include <signal.h>



#ifndef EXTERN
	#define EXTERN	extern
#endif

/*
 * SET_MEMPART_POLICY_KEY
 * ISSET_MEMPART_POLICY_KEY
 * 
 * These macros are used to produce a unique 'key' for use in the internal
 * maintenance of mempart_policy_t types. There are (currently) 3 boolean policies
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
 * ISSET_MEMPART_POLICY_KEY() will evaluate (v) against the key and return
 * TRUE or FALSE according to whether (v) is equivalent to the key or not.
 * 
 * SET_MEMPART_POLICY_KEY() will set (v) to the key depending on whether the
 * current value of (v) is (p) or not.
*/
#define ISSET_MEMPART_POLICY_KEY(v)			((v) == (ap_bool_t)(uintptr_t)&(v))
#define SET_MEMPART_POLICY_KEY(v, p)		(((v) == (p)) ? ((v) = (ap_bool_t)((uintptr_t)&(v))):0)

/*
 * SET/GET_MPART_POLICY_CFG_LOCKED
 * SET/GET_MPART_POLICY_TERMINAL
 * SET/GET_MPART_POLICY_PERMANENT
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
#define GET_MPART_POLICY_CFG_LOCK(c)	(ISSET_MEMPART_POLICY_KEY((c)->policy.config_lock) ? bool_t_FALSE : bool_t_TRUE)
#define GET_MPART_POLICY_TERMINAL(c)	(ISSET_MEMPART_POLICY_KEY((c)->policy.terminal) ? bool_t_FALSE : bool_t_TRUE)
#define GET_MPART_POLICY_PERMANENT(c)	(ISSET_MEMPART_POLICY_KEY((c)->policy.permanent) ? bool_t_FALSE : bool_t_TRUE)

#define SET_MPART_POLICY_CFG_LOCK_KEY(c)	SET_MEMPART_POLICY_KEY((c)->policy.config_lock, bool_t_FALSE)
#define SET_MPART_POLICY_TERMINAL_KEY(c)	SET_MEMPART_POLICY_KEY((c)->policy.terminal, bool_t_FALSE)
#define SET_MPART_POLICY_PERMANENT_KEY(c)	SET_MEMPART_POLICY_KEY((c)->policy.permanent, bool_t_FALSE)


extern struct mempart_s	*sys_mempart;
extern bool mempart_module_loaded(void);


/*==============================================================================
 * 
 *			memmgr internal interfaces for memory partition management
 * 
*/
#define MEMPART_MODULE_LOADED()		mempart_module_loaded()


/*
 * =============================================================================
 * 
 * 									D E B U G
 * 
 * =============================================================================
*/
#ifndef NDEBUG

#define __dump_list(lp) \
		do { \
			while ((part_qnode_t *)lp != NULL) { \
				kprintf("%p(%x) -> ", (part_qnode_t *)lp, *((unsigned int *)&lp[1])); \
				(part_qnode_t *)lp = ((part_qnode_t *)lp)->next; \
			} \
			kprintf("@\n"); \
		} while(0)

/*******************************************************************************
 * dump_proc_partition_list
*/
#define DUMP_PROC_PARTITION_LIST(prp) \
		do { \
			kprintf("partition list for proc %p (%d) ...\n", (prp), (prp)->pid); \
			__dump_list((prp)->mpart_list.head); \
		} while(0)

#ifdef USE_PROC_OBJ_LISTS
/*******************************************************************************
 * dump_associated_proc_list
 * dump_associated_obj_list
*/

#define DUMP_ASSOCIATED_PROC_LIST(mp) \
		do { \
			kprintf("process list for mpart %p ...\n", (mp)); \
			__dump_list((mp)->prp_list.head); \
		} while(0)
#define DUMP_ASSOCIATED_OBJ_LIST(mp) \
		do { \
			kprintf("object list for mpart %p ...\n", (mp)); \
			__dump_list((mp)->obj_list.head); \
		} while(0)
#endif	/* USE_PROC_OBJ_LISTS */
#endif	/* NDEBUG */


#endif	/* _APM_INTERNAL_H_ */


/* __SRCVERSION("$IQ: apm.h,v 1.91 $"); */
