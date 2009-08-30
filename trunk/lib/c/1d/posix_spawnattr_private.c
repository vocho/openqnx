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

#include <unistd.h>		// determines whether _POSIX_SPAWN is defined or not
#ifdef _POSIX_SPAWN

#include "posix_spawnattr.h"


/*
 * =============================================================================
 * 
 * 			The following are NON PUBLIC QNX posix_spawnattr_t extensions
 * 				used by proc to parse a _PROC_POSIX_SPAWN message
 * 					(DO NOT DOCUMENT in library reference guide)
 * 
 * =============================================================================
*/


/*******************************************************************************
 * posix_spawnattr_endswap
 * 
 * Do an endian swap on the '_posix_spawnattr_t' pointed to by <p>.
 * This is an internal routine used by procmgr_pspawn()
 * Perhaps it should only compile when procnto is being built ?
*/
void posix_spawnattr_endswap(void *p)
{
	_posix_spawnattr_t *_attrp = p;
	_posix_spawnattr_partition_t  *partition_attr;

	if (_attrp == NULL) return;

	ENDIAN_SWAP32(&_attrp->flags);
	ENDIAN_SWAP32(&_attrp->pgrp);
	ENDIAN_SWAP32(&_attrp->node);
	ENDIAN_SWAP32(&_attrp->stack.max);

	ENDIAN_SWAP32(&_attrp->sig.mask.__bits[0]);
	ENDIAN_SWAP32(&_attrp->sig.mask.__bits[1]);
	ENDIAN_SWAP32(&_attrp->sig.dflt.__bits[0]);
	ENDIAN_SWAP32(&_attrp->sig.dflt.__bits[1]);
	ENDIAN_SWAP32(&_attrp->sig.ignore.__bits[0]);
	ENDIAN_SWAP32(&_attrp->sig.ignore.__bits[1]);

	ENDIAN_SWAP32(&_attrp->sched.policy);
	ENDIAN_SWAP32(&_attrp->sched.runmask);
/* FIX ME - swap the struct sched_param
	ENDIAN_SWAP32(&_attrp->sched.sched_priority);
	ENDIAN_SWAP32(&_attrp->sched.sched_curpriority);
*/
	partition_attr = (_posix_spawnattr_partition_t *)&_attrp->partition;
	ENDIAN_SWAP32(&partition_attr->size);

	if (partition_attr->size > 0)
	{
		unsigned i;

		ENDIAN_SWAP32(&partition_attr->part.num_entries);
		for (i=0; i<partition_attr->part.num_entries; i++) {
			ENDIAN_SWAP32(&partition_attr->part.i[i].id);
			ENDIAN_SWAP32(&partition_attr->part.i[i].flags);
		}
	}
}


/*******************************************************************************
 * get_attrp
 * set_attrp
 * 
 * These 2 routine translate between a real allocated '_posix_spawnattr_t *' and
 * the token stored in the 'posix_spawnattr_t *'.
 * 
 * A valid, initialized 'posix_spawnattr_t' object has 2 possible values.
 * 		- NULL
 * 		- a valid, allocated address
 * All other values are considered invalid. These 2 routines attempt to catch
 * uninitialized 'posix_spawnattr_t' objects from being used. The example is
 * an automatic 'posix_spawnattr_t' which contains stack garbage. It is non NULL
 * but does not represent valid, allocated memory.
 * 
 * Because we know that allocated memory is always aligned on given boundary
 * we use a key that will always turn a properly allocated memory address into
 * a token which always has the lower 3 bits set (8 byte alignment). This works
 * for all bogus values that do not have 7 as the LSB 
*/
void set_attrp(posix_spawnattr_t *pp, void *_p, unsigned key_mask)
{
	_posix_spawnattr_t *p = (_posix_spawnattr_t *)_p;
	assert(pp != NULL);
	assert(((unsigned)p == AKEY(key_mask)) || (((unsigned)p & key_mask) == 0)); // must be aligned or keying will fail
	*pp = ((unsigned)p ^ AKEY(key_mask));
}


#endif	/* _POSIX_SPAWN */

