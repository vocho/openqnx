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



/* This implements the kernel call SchedCtl for the case of the Adaptive Partition scheduler.
 *
 * parameter and security checking is here. implementation is in nano_aps.c
 */


#include "externs.h"
#include "aps_data.h"
#include <sys/sched_aps.h>
#include "proto_aps.h"

__SRCVERSION("ker_aps.c $Rev: 168521 $");

/* security */


#define IN_SYS(act) ((act)->dpp->id==0) 
#define IS_ROOT(act) (0==(act)->process->cred->info.euid && 0==(act)->process->cred->info.egid )

/* each of these macros tests one flag of SCHED_APS_SEC_* flags in aps_security. Returns true if the security
 * condition is not met and therefore the SchedCtl command should be terminated with EACESS */ 

#define FAIL_ROOT0_OVERALL(act) (SCHED_APS_SEC_ROOT0_OVERALL & aps_security && !(IS_ROOT(act) && IN_SYS(act)) )
#define FAIL_ROOT_MAKES_PARTITIONS(act) (SCHED_APS_SEC_ROOT_MAKES_PARTITIONS & aps_security && !IS_ROOT(act) )  
#define FAIL_SYS_MAKES_PARTITIONS(act) (SCHED_APS_SEC_SYS_MAKES_PARTITIONS & aps_security && !IN_SYS(act) ) 
#define FAIL_ROOT_JOINS(act) (SCHED_APS_SEC_ROOT_JOINS & aps_security && !IS_ROOT(act) ) 
#define FAIL_SYS_JOINS(act) (SCHED_APS_SEC_SYS_JOINS & aps_security && !IN_SYS(act) ) 
#define FAIL_PARENT_JOINS(act,ap) (SCHED_APS_SEC_PARENT_JOINS & aps_security && (act)->dpp->id != aps_get_parent(ap) )
#define FAIL_JOIN_SELF_ONLY(pid,tid) (SCHED_APS_SEC_JOIN_SELF_ONLY & aps_security && !( (pid)==0 && (tid)==0 ) ) 
#define FAIL_ROOT_MAKES_CRITICAL(act,critical_budget) (SCHED_APS_SEC_ROOT_MAKES_CRITICAL & aps_security && !IS_ROOT(act)\
			&& (critical_budget)>0 )
#define FAIL_ROOT_CHANGES_CRITICAL(act,critical_budget) (SCHED_APS_SEC_ROOT_MAKES_CRITICAL & aps_security && !(IS_ROOT(act)\
			|| (critical_budget)==-1) )
#define FAIL_SYS_MAKES_CRITICAL(act,critical_budget) (SCHED_APS_SEC_SYS_MAKES_CRITICAL & aps_security && !IN_SYS(act) \
			&& (critical_budget)>0 )
#define FAIL_SYS_CHANGES_CRITICAL(act,critical_budget) (SCHED_APS_SEC_SYS_MAKES_CRITICAL & aps_security && !(IN_SYS(act) \
			|| (critical_budget)==-1) )
#define FAIL_NONZERO_BUDGETS(budget) (SCHED_APS_SEC_NONZERO_BUDGETS & aps_security && (budget)==0) 
#define FAIL_PARENT_MODIFIES(act,ap) (SCHED_APS_SEC_PARENT_MODIFIES & aps_security && (act)->dpp->id!=aps_get_parent(ap) )
#define FAIL_PARTITIONS_LOCKED (SCHED_APS_SEC_PARTITIONS_LOCKED & aps_security) 



				
THREAD *get_thread(THREAD* act, uint32_t pid, uint32_t tid) {
/*     -----------
 *     a function local to ker_sched_ctl.
 *     pid/tid means:
 *     0/-1	top thread of current process
 *     0/0	calling thread
 *     0/x	thread x in current process
 *     x/-1	top thread of process x
 *     x/0	current thread (make sure x is also current process)
 *     x/x	thread x in process x
 */		
	PROCESS *prp;

	/* Covers 0/-1 and 0/0. 0/x case falls through into x/? treatement */
	if (0 == pid) {
		if ((uint32_t)-1 == tid) return act;//->process->valid_thp;
		if (0  == tid) return act;
		prp = act->process;
	} else {
		prp = lookup_pid(pid);
	}

	/* Make sure supplied pid is valid (checks for x/0 pid as well) */
	if (!prp || !tid && act->process != prp) return NULL;

	if ((uint32_t)-1 == tid) return prp->valid_thp;
	if ( 0 == tid) return act;

	return vector_lookup(&(prp->threads), tid - 1);
}

/* partition_name_ok 
 * rejects names containing a slash. Null names are valid since nano_aps.c will pick a default name
 * returns 1 is name is ok.
 */
char *bad_partition_name_chars = "/"; 
int partition_name_ok(char *name) { 
	int i,l; 
	if (!name || strlen(name)==0) return 1; 
	l = strlen(bad_partition_name_chars);
	for(i=0;i<l;i++) { 
		if(strchr(name, bad_partition_name_chars[i])) return 0;
	}
	return 1;
}		

//@@@ APS some file in /ker should define a default ker_sched_ctl that returns ENOSYS to everything. i.e. 
//		  for when the aps scheduler is not installed.


/* macros to check that reserved fields are all zero */
#define BAD_RESV1(x) ( (x)->reserved1)  
#define BAD_RESV2(x) ( (x)->reserved1 || (x)->reserved2 ) 
#define BAD_RESV3(x) ( (x)->reserved1 || (x)->reserved2 || (x)->reserved3 ) 
#define BAD_RESV4(x) ( (x)->reserved1 || (x)->reserved2 || (x)->reserved3 || (x)->reserved4) 

int kdecl
ker_sched_ctl(THREAD *act, struct kerargs_sched_ctl *kap) {
	switch(kap->cmd) {
			
		
	case SCHED_QUERY_SCHED_EXT: {
		struct sched_query *parms; 
		parms = (struct sched_query *)kap->data;
		if (kap->length!=sizeof(struct sched_query)) return EINVAL;
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  
		WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		KerextLock();
		parms->extsched = SCHED_EXT_APS;
		parms->reserved = 0;
		} break;

	case SCHED_APS_QUERY_PARMS: {
		sched_aps_info *parms; 
		parms = (sched_aps_info*)kap->data;
		if (kap->length!=sizeof(sched_aps_info)) return EINVAL;
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  
		WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV2(parms) return EDOM; 
		KerextLock();
		aps_get_info(parms);
		parms->sec_flags = aps_security;
		} break;
		
	case SCHED_APS_SET_PARMS: {
		sched_aps_parms		*parms; 
		if (FAIL_ROOT0_OVERALL(act) || FAIL_PARTITIONS_LOCKED) return EACCES;
		parms = (sched_aps_parms*)kap->data ;
		if (kap->length!=sizeof(sched_aps_parms)) return EINVAL;
		RD_VERIFY_PTR(act,parms,sizeof(*parms)); RD_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int));
		if (parms->bankruptcy_policyp){
			RD_VERIFY_PTR(act,parms->bankruptcy_policyp, sizeof(*(parms->bankruptcy_policyp))); 
			RD_PROBE_INT(act,parms->bankruptcy_policyp, sizeof(*(parms->bankruptcy_policyp))/sizeof(int));  
		}
		if (parms->scheduling_policy_flagsp){
			RD_VERIFY_PTR(act,parms->scheduling_policy_flagsp, sizeof(*(parms->scheduling_policy_flagsp))); 
			RD_PROBE_INT(act,parms->scheduling_policy_flagsp, sizeof(*(parms->scheduling_policy_flagsp))/sizeof(int));  
		}
		if BAD_RESV3(parms) return EDOM; 
		KerextLock(); 
		return aps_set_parms(parms);
		}
		
	case SCHED_APS_CREATE_PARTITION: { 
		sched_aps_create_parms		*parms; 
		int							ret;
		parms = (sched_aps_create_parms*)kap->data ;
		if (kap->length!=sizeof(sched_aps_create_parms)) return EINVAL;
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if (parms->name != NULL) {
			RD_VERIFY_PTR(act,parms->name,1);
			if(strlen(parms->name) > APS_PARTITION_NAME_LENGTH ) return ENAMETOOLONG;
			if (!partition_name_ok(parms->name)) return EINVAL; 
		}
		//aps_name_to_id only works if names are untruncated 
		if (FAIL_PARTITIONS_LOCKED || FAIL_ROOT_MAKES_PARTITIONS(act) || FAIL_SYS_MAKES_PARTITIONS(act) 
			|| FAIL_NONZERO_BUDGETS(parms->budget_percent) || FAIL_ROOT_MAKES_CRITICAL(act, parms->critical_budget_ms) 
			|| FAIL_SYS_MAKES_CRITICAL(act, parms->critical_budget_ms) ) return EACCES;
		//critical budget of -1, means "not set", but reject other negative values.
		if (parms->critical_budget_ms < -1) return EINVAL; 
		if BAD_RESV3(parms) return EDOM; 
		if ((parms->aps_create_flags & APS_CREATE_FLAGS_USE_PARENT_ID) && (parms->parent_id >= num_ppg)) return EINVAL;
		KerextLock();
		if (parms->name && -1!=aps_name_to_id(parms->name))  return EEXIST; 
		//there are addtional security checks in create_ppg()
		ret = create_ppg(parms, 0);
		if (ret>0) {
			parms->id = ret;
		} else { 
			parms->id = -1;
			return -ret;
		}
		break;
		}					 

	case SCHED_APS_LOOKUP: {  
		sched_aps_lookup_parms	*parms;
		parms = (sched_aps_lookup_parms*)kap->data ;
		if (kap->length!=sizeof(sched_aps_lookup_parms)) return EINVAL;
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if (parms->name == NULL) return EINVAL;
		RD_VERIFY_PTR(act, parms->name, 1); 
		if (*(parms->name) == '\0') return EINVAL;
		if(strlen(parms->name) > APS_PARTITION_NAME_LENGTH ) return ENAMETOOLONG;
		if BAD_RESV1(parms) return EDOM; 
		KerextLock();
		parms ->id = aps_name_to_id(parms->name); 
		if (-1==parms->id) return EINVAL;
		} break; 
		
	case SCHED_APS_QUERY_PARTITION: {
		sched_aps_partition_info *parms; 
		parms = (sched_aps_partition_info*)kap->data; 
		if (kap->length!=sizeof(sched_aps_partition_info)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV2(parms) return EDOM; 
		KerextLock();
		return aps_get_partition_info(parms);
		}
		
	case SCHED_APS_JOIN_PARTITION: {
		sched_aps_join_parms		*parms; 
		THREAD						*thp;
		parms = (sched_aps_join_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_join_parms)) return EINVAL;
		RD_VERIFY_PTR(act,parms,sizeof(*parms)); RD_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int));
		if (FAIL_ROOT_JOINS(act) 
			|| FAIL_SYS_JOINS(act) 
			|| FAIL_PARENT_JOINS(act,parms->id)
			|| FAIL_JOIN_SELF_ONLY(parms->pid,parms->tid) ) return EACCES;
		if BAD_RESV2(parms) return EDOM; 
		KerextLock();
		/* tid of -1 means join process */
		if (parms->tid == -1) {
			PROCESS 	*prp;
			prp = (parms->pid == 0) ? act->process : lookup_pid(parms->pid);
			if (!prp) return ESRCH;
			return join_process_ppg(prp, parms->id);
		} else {
			thp = get_thread(act, parms->pid, parms->tid);
			if (!thp) return ESRCH; 
			return join_ppg(thp,parms->id);
		}
		}
		
	case SCHED_APS_MODIFY_PARTITION: {
		sched_aps_modify_parms	*parms; 
		parms = (sched_aps_modify_parms*)kap->data ;
		if (kap->length!=sizeof(sched_aps_modify_parms)) return EINVAL;
		RD_VERIFY_PTR(act,parms,sizeof(*parms)); RD_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int));
		if (FAIL_PARTITIONS_LOCKED 
				|| FAIL_PARENT_MODIFIES(act,parms->id) 
				|| FAIL_ROOT_MAKES_PARTITIONS(act) ||
				FAIL_SYS_MAKES_PARTITIONS(act) || 
				FAIL_NONZERO_BUDGETS(parms->new_budget_percent) || 
				FAIL_ROOT_CHANGES_CRITICAL(act, parms->new_critical_budget_ms) ||
				FAIL_SYS_CHANGES_CRITICAL(act, parms->new_critical_budget_ms ) ) return EACCES;
		//checkthat requested budget is valid. -1 is valid, it means "no change" 
		if (parms->new_budget_percent <-1 || parms->new_budget_percent>100) return EINVAL;
		if (parms->new_critical_budget_ms < -1) return EINVAL;
		if BAD_RESV3(parms) return EDOM; 
		KerextLock(); 
		return aps_modify_partition(parms); //does extra check for parent becomming zero 
		}
		
				
		
	case SCHED_APS_PARTITION_STATS: {
		sched_aps_partition_stats *parms; 
		parms = (sched_aps_partition_stats*)kap->data; 
		if (kap->length==0 || (kap->length % sizeof(sched_aps_partition_stats) !=0)) return EINVAL; 
		//we only need to check the reserved fields of the first element of the parms array. That's beacuse if we
		//ever add new input fields it will only be to the first element. 
		WR_VERIFY_PTR(act,parms,kap->length);
		WR_PROBE_INT(act,parms,kap->length/sizeof(int)); 
		if BAD_RESV3(parms) return EDOM; 
		KerextLock();
		return aps_get_partition_stats(parms, kap->length/sizeof(sched_aps_partition_stats) ) ;  
		}
		
		
	case SCHED_APS_OVERALL_STATS: {
		sched_aps_overall_stats	*parms; 
		parms = (sched_aps_overall_stats*)kap->data;
		if (kap->length!=sizeof(sched_aps_overall_stats)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV4(parms) return EDOM; 
		KerextLock();
		(void)aps_get_overall_stats(parms); 
		} break;
		
	case SCHED_APS_MARK_CRITICAL: {
		// there is security on marking threads critical. Since there is no effect unless the partition has critical budget.
		sched_aps_mark_crit_parms	*parms; 
		THREAD						*thp;
		parms = (sched_aps_mark_crit_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_mark_crit_parms)) return EINVAL; 
		RD_VERIFY_PTR(act,parms,sizeof(*parms)); RD_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int));
		if BAD_RESV1(parms) return EDOM; 
		KerextLock();
		thp = get_thread(act, parms->pid, parms->tid);
		if (!thp) return ESRCH; 
		thp->sched_flags |= AP_SCHED_RUNNING_CRIT | AP_SCHED_PERM_CRIT;
		}break;
		
	case SCHED_APS_CLEAR_CRITICAL: {
		// there is security on marking threads critical. Since there is no effect unless the partition has critical budget.
		THREAD						*thp;
		sched_aps_clear_crit_parms	*parms; 
		parms = (sched_aps_clear_crit_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_clear_crit_parms)) return EINVAL; 
		RD_VERIFY_PTR(act,parms,sizeof(*parms)); RD_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int));
		if BAD_RESV1(parms) return EDOM; 
		KerextLock();
		thp = get_thread(act, parms->pid, parms->tid);
		if (!thp) return ESRCH; 
		thp->sched_flags &= ~(AP_SCHED_PERM_CRIT | AP_SCHED_RUNNING_CRIT | AP_SCHED_BILL_AS_CRIT); 
		} break;
		
	case SCHED_APS_QUERY_THREAD: {
		THREAD						*thp;
		sched_aps_query_thread_parms	*parms; 
		parms = (sched_aps_query_thread_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_query_thread_parms)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV2(parms) return EDOM; 
		KerextLock();
		thp = get_thread(act, parms->pid, parms->tid);
		if (!thp) return ESRCH; 
		return aps_get_thread_info(thp, parms);
		}
		
	case SCHED_APS_ATTACH_EVENTS: {				
		sched_aps_events_parm	*parms; 
		int id;
		parms = (sched_aps_events_parm*)kap->data;
		if (kap->length!=sizeof(sched_aps_events_parm)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if (parms->bankruptcy_notification) { 
			RD_VERIFY_PTR(act,parms->bankruptcy_notification,sizeof(struct sigevent));
			RD_PROBE_INT(act,parms->bankruptcy_notification, sizeof(struct sigevent)/sizeof(int)); 
		}
		if (parms->overload_notification) { 
			RD_VERIFY_PTR(act,parms->overload_notification,sizeof(struct sigevent));
			RD_PROBE_INT(act,parms->overload_notification, sizeof(struct sigevent)/sizeof(int));
		}
		if (parms->id ==-1) { 
			id = act->dpp->id;
		} else { 
			id = parms->id;
		} 
		if ( FAIL_PARENT_MODIFIES(act,id) || FAIL_ROOT_MAKES_PARTITIONS(act) ||
				FAIL_SYS_MAKES_PARTITIONS(act) ) return EACCES;
		//@@@ APS is there a way to verifu that a sigevent is correctly initalized? 
		if BAD_RESV3(parms) return EDOM; 
		parms->id = id; 
		KerextLock();
		return aps_attach_events(parms); 
		}
								  
								  
	case SCHED_APS_ADD_SECURITY: {
		sched_aps_security_parms	*parms; 
		parms = (sched_aps_security_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_security_parms)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV2(parms) return EDOM; 
		KerextLock();
		//regarless of security options set, this may only be called from root running in partition zero
		//and locking partitions still allows you to make security tigher.
		if (!IS_ROOT(act) || !IN_SYS(act) ) return EACCES;
		aps_security |= parms->sec_flags;
		parms->sec_flags = aps_security; 
		break;
		}

	case SCHED_APS_QUERY_PROCESS: {
		PROCESS		*prp;
		sched_aps_query_process_parms	*parms; 
		parms = (sched_aps_query_process_parms*)kap->data;
		if (kap->length!=sizeof(sched_aps_query_process_parms)) return EINVAL; 
		WR_VERIFY_PTR(act,parms,sizeof(*parms));  WR_PROBE_INT(act,parms,sizeof(*parms)/sizeof(int)); 
		if BAD_RESV4(parms) return EDOM; 
		KerextLock();
		prp = (parms->pid == 0) ? act->process : lookup_pid(parms->pid);
		if (!prp) return ESRCH;
		parms->id = prp->default_dpp->id;
		return EOK;
		}
		

	default:
		return EINVAL;
	}
	return EOK;
}
